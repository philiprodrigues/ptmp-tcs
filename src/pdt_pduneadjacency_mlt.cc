#include "pdt_engines.h"
#include "pdt/ModuleTrigger.h"

// Fixme: TPSet holds 50MHz clocks, PDT assumes 2MHz
const int hwtick_per_internal = 25;

using namespace ptmp::tcs::pdt;

void PDUNEAdjacencyMLT_engine::ingest(const ptmp::data::TPSet& fresh)
{
    const ptmp::data::TrigPrim* special = nullptr;
    std::vector<const ptmp::data::TrigPrim*> tocopy;
    for (const auto& tp : fresh.tps()) {
        if (tp.flags() == PDT_SPECIAL_TP) {
            special = &tp;
            continue;
        }
        if (tp.tspan() == 0) {
            continue;
        }
        tocopy.push_back(&tp);
    }
    if (!special) {
        zsys_error("PDUNEAdjacencyMLT_engine: failed to find special TP from %d", fresh.detid());
        return;
    }
    for (const auto tp : tocopy) {
        ptmp::data::TrigPrim* newtp = outbound.add_tps();
        *newtp = *tp;
    }

    if (candidates.empty()) {   // first in batch
        // warning: This relies on input to be properly synchronous.  
        outbound.set_tstart(fresh.tstart());
        outbound.set_tstart(twindow);
        outbound.set_detid(fresh.detid());
    }

    candidate_t candidate(9, 0);
    // Adjacency value is outside the TPSet/TrigPrim model and is
    // special to the PDT TriggerCandidate() algorithm.  We've stashed
    // it in TrigPrim.adcpeak in a final TrigPrim with .flag set to
    // 0xdeadbeaf which is collected above.
    candidate[0] = special->adcpeak();
    // 1: not used
    // 2: not used
    // 3: not used
    candidate[4] = fresh.chanbeg();
    candidate[5] = fresh.chanend();
    // PDT works in 2MHz clock ticks, not 50MHz
    candidate[6] = special->tstart()/hwtick_per_internal;
    candidate[7] = (special->tstart() + special->tspan())/hwtick_per_internal;
    // Phil sets detid inside FELIX_BR with:
    //
    // (m_fiber_no << 16) | (m_slot_no << 8) | m_crate_no
    //
    // FIXME: for lack of any better understanding assume PDT wants
    // APA number same as m_crate_no:
    candidate[8] = fresh.detid() & 0x000000FF;
    candidates.push_back(candidate);

    outbound.set_chanbeg(std::min(outbound.chanbeg(), fresh.chanbeg()));
    outbound.set_chanend(std::min(outbound.chanend(), fresh.chanend()));
}

void PDUNEAdjacencyMLT_engine::operator()(const ptmp::data::TPSet& input_tpset,
                                          std::vector<ptmp::data::TPSet>& output_tpsets)
{
    const ptmp::data::data_time_t this_tstart = input_tpset.tstart();
    if (candidates.empty()) {
        ingest(input_tpset);
        return;
    }
    if (this_tstart < outbound.tstart()) {
        zsys_warning("PDUNEAdjacencyMLT_engine: dropping tardy at %ld by %ld",
                     this_tstart, outbound.tstart()-this_tstart);
        return;
    }
    if (this_tstart < outbound.tstart() + twindow) {
        ingest(input_tpset);
        return;
    }

    // ready to process

    // It is an 9-tuple of ints, with additional [8]'th entry from
    // what TriggerCommand() returns
    //     0         1       2       3      4      5     6    7   8
    // (adjacency, adcpeak, adcsum, tspan, chan1, chan2, t1, t2, apanum)

    // fixme: does MT() need any sorting of its input candidates?

    int ok = ModuleTrigger(candidates);

    if (ok) {                   // got one
        outbound.set_count(1+outbound.count());
        outbound.set_created(ptmp::data::now());
        output_tpsets.push_back(outbound);
    }

    clear();

    // don't forget the input that prompted processing
    ingest(input_tpset);
}




void PDUNEAdjacencyMLT_engine::clear()
{
    outbound.set_tstart(0);
    outbound.set_tspan(0);
    outbound.set_chanbeg(0);
    outbound.set_chanend(0);
    outbound.set_totaladc(0);
    outbound.clear_tps();
    candidates.clear();
}

PDUNEAdjacencyMLT_engine::PDUNEAdjacencyMLT_engine()
{
}

PDUNEAdjacencyMLT_engine::~PDUNEAdjacencyMLT_engine()
{
}
