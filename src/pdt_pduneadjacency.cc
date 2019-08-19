#include "pdt_engines.h"
#include "ptmp/data.h"
#include "ptmp/filter.h"
#include "json.hpp"

#include <czmq.h>
#include <limits>

// #include "pdt/AdjacencyAlgorithms.h"
struct TP{
    unsigned int channel;
    unsigned int tstart;
    unsigned int tspan;
    unsigned int adcsum;
    unsigned int adcpeak;
    unsigned int flags;
};
// #include "pdt/TriggerCandidate.h"
std::vector<int> TriggerCandidate(std::vector<TP>, int clustering=0);

using json = nlohmann::json;

// Fixme: TrigPrim holds 50MHz clocks, PDT assumes 2MHz
const int hwtick_per_internal = 25;

static
TP convert(const ptmp::data::TrigPrim& tp)
{
    // Fixme: eventually get rid of this cast
    const uint32_t tstart = (uint32_t) (tp.tstart()/hwtick_per_internal);
    const uint32_t tspan = std::max((uint32_t)1, tp.tspan()/hwtick_per_internal);
    return TP{ tp.channel(), tstart, tspan, tp.adcsum(), tp.adcpeak(), tp.flags() };
}

static
void update(ptmp::data::TPSet& tofill, const ptmp::data::TPSet& fresh)
{
    for (const auto& tp : fresh.tps()) {
        if (tp.tspan() == 0) { continue; }
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

class pdt_tc_engine : public ptmp::filter::engine_t {
    ptmp::data::TPSet outbound;
    ptmp::data::data_time_t twindow{2500}; // 50us at 50MHz, fixme, need to set to match input.
public:
    pdt_tc_engine(const std::string& config) {
        auto jcfg = json::parse(config);
        int detid=0;
        if (jcfg["detid"].is_number()) {
            detid = jcfg["detid"];
        }
        outbound.set_detid(detid);
        outbound.set_count(0);
        outbound.set_created(0);
        clear(outbound);
    }

    virtual ~pdt_tc_engine() {}

    virtual void operator()(const ptmp::data::TPSet& input_tpset,
                            std::vector<ptmp::data::TPSet>& output_tpsets) {
        // ptmp::data::dump(input_tpset, "recv");

        const ptmp::data::data_time_t this_tstart = input_tpset.tstart();
        if (outbound.tps().empty()) {
            outbound.set_tstart(this_tstart);
            outbound.set_detid(input_tpset.detid());
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

        //ptmp::data::dump(outbound, "input");

        // ready to process and advance.
        std::vector<TP> pdt_tps;
        for (const auto& tp : outbound.tps()) {
            pdt_tps.push_back(convert(tp));
        }
        std::sort(pdt_tps.begin(), pdt_tps.end(),
                  [](const TP& a, const TP& b) {
                      return a.channel < b.channel;
                  });
        auto tcvec = TriggerCandidate(pdt_tps);
        // this returns an 8-tuple of ints which may be empty.
        //     0         1       2       3      4      5     6    7
        // (adjacency, adcpeak, adcsum, tspan, chan1, chan2, t1, t2)
    
        if (tcvec.size() == 8) {
            outbound.set_count(1+outbound.count());
            // detid set on first input
            outbound.set_created(ptmp::data::now());
            outbound.set_tstart(this_tstart); // sync input/output time.
            outbound.set_tspan(twindow);      // means time checked.
            outbound.set_chanbeg(tcvec.at(4));
            outbound.set_chanend(tcvec.at(5));
            outbound.set_totaladc(tcvec.at(2));

            // PDT requires passing the "adjacency" which is not in the
            // TPSet/TrigPrim model.  So, we cheat.
            ptmp::data::TrigPrim* newtp = outbound.add_tps();
            newtp->set_channel(0);
            newtp->set_tstart(hwtick_per_internal*std::min(tcvec.at(6), tcvec.at(7)));
            newtp->set_tspan(hwtick_per_internal*std::abs(tcvec.at(7) - tcvec.at(6)));
            newtp->set_adcsum(tcvec.at(2));
            newtp->set_adcpeak(tcvec.at(0));
            newtp->set_flags(PDT_SPECIAL_TP);

            output_tpsets.push_back(outbound);
            //ptmp::data::dump(outbound, "outbound");
        }
        clear(outbound);
        outbound.set_tstart(this_tstart);
        update(outbound, input_tpset);
    }
}; // class pdt_tc_engine

PTMP_FILTER(pdt_tc_engine, pdune_adjacency_tc)
