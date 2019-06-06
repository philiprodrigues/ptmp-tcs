#include "TCFinder_pdt.h"
#include "pdt/AdjacencyAlgorithms.h"

using namespace ptmp::tcs::pdt;


static
TP convert(const ptmp::data::TrigPrim& tp)
{
    return TP{tp.channel(),
            (uint32_t)tp.tstart(),        // beware, truncates 64 to 32 bits!!!!
            tp.tspan(),
            tp.adcsum(),
            tp.adcpeak(),
            tp.flags()
            };
}

static
void update(ptmp::data::TPSet& tofill, const ptmp::data::TPSet& fresh)
{
    for (const auto& tp : fresh.tps()) {
        ptmp::data::TrigPrim* newtp = tofill.add_tps();
        *newtp = tp;
    }
}

static 
void clear(ptmp::data::TPSet& tps)
{
    tps.set_tstart(0);
    tps.set_tspan(0);
    tps.set_chanbeg(0);
    tps.set_chanend(0);
    tps.set_totaladc(0);
    tps.clear_tps();
}

PDUNEAdjacency_engine::PDUNEAdjacency_engine()
{
    outbound.set_count(0);
    outbound.set_detid(0);           // fixme
    outbound.set_created(0);
    clear(outbound);
}

PDUNEAdjacency_engine::~PDUNEAdjacency_engine()
{
}



//
// detid is set to (m_fiber_no << 16) | (m_slot_no << 8) | m_crate_no


void PDUNEAdjacency_engine::operator()(const ptmp::data::TPSet& input_tpset,
                                       std::vector<ptmp::data::TPSet>& output_tpsets)
{
    const int64_t this_tstart = input_tpset.tstart();
    if (outbound.tps().empty()) {
        outbound.set_tstart(this_tstart);
        update(outbound, input_tpset);
        return;
    }
    if (this_tstart < outbound.tstart()) {
        zsys_warning("dropping tardy at %ld by %ld",
                     this_tstart, outbound.tstart()-this_tstart);
        return;
    }
    if (this_tstart < outbound.tstart() + twindow) {
        update(outbound, input_tpset);
        return;
    }

    // ready to process and advance.
    std::vector<TP> pdt_tps;
    for (const auto& tp : outbound.tps()) {
        pdt_tps.push_back(convert(tp));
    }
    auto tcvec = PDUNEAdjacency(pdt_tps);
    // this returns an 8-tuple of ints
    //     0         1       2       3      4      5     6    7
    // (adjacency, adcpeak, adcsum, tspan, chan1, chan2, t1, t2)
    
    // Fixme: this is a minimal selection and should be made stronger.
    if (tcvec.at(0) > 1 and tcvec.at(2) > 0) {
        outbound.set_count(1+outbound.count());
        // detid
        outbound.set_created(zclock_usecs());
        outbound.set_tstart(std::min(tcvec.at(6), tcvec.at(7)));
        outbound.set_tspan(std::abs(tcvec.at(7) - tcvec.at(6)));
        outbound.set_chanbeg(tcvec.at(4));
        outbound.set_chanend(tcvec.at(5));
        outbound.set_totaladc(tcvec.at(6));
        output_tpsets.push_back(outbound);
    }
    clear(outbound);
    outbound.set_tstart(this_tstart);
    update(outbound, input_tpset);
}
