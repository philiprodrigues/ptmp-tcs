// engine/wrapper for the MichelFinder function

#include "ptmp/data.h"
#include "ptmp/filter.h"
#include "json.hpp"
#include "czmq.h"

bool MichelFinder(ptmp::data::TPSet& tpset);

using json = nlohmann::json;

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

class met_tc_engine : public ptmp::filter::engine_t {
    ptmp::data::TPSet outbound;
    bool transfer_detid {true};
public:
    met_tc_engine(const std::string& config) {
        auto jcfg = json::parse(config);
        int detid=0;
        if (jcfg["detid"].is_number()) {
            detid = jcfg["detid"];
            transfer_detid = false;
        }
        outbound.set_detid(detid);
        outbound.set_count(0);
        outbound.set_created(0);
        clear(outbound);
    }
    virtual ~met_tc_engine() {}
    virtual void operator()(const ptmp::data::TPSet& input_tpset,
                            std::vector<ptmp::data::TPSet>& output_tpsets) {

        const ptmp::data::data_time_t this_tstart = input_tpset.tstart();
        if (outbound.tps().empty()) {
            outbound.set_tstart(this_tstart);
            if (transfer_detid) {
                outbound.set_detid(input_tpset.detid());
            }
            update(outbound, input_tpset);
            return;
        }
        if (this_tstart < outbound.tstart()) {
            zsys_warning("dropping tardy at %ld by %ld",
                         this_tstart, outbound.tstart()-this_tstart);
            return;
        }
        if (this_tstart == outbound.tstart()) { // same window
            update(outbound, input_tpset);
            return;
        }

        // if here, then the input is in a subsequent window.  Process
        // the collected and stash the new one.

        bool got_one = MichelFinder(outbound);
        if (got_one) {
            outbound.set_count(1+outbound.count());
            // detid set on first input
            outbound.set_created(ptmp::data::now());
            // the rest are left to MichelFinder() to set as desired
            output_tpsets.push_back(outbound);
        }
        clear(outbound);
        outbound.set_tstart(this_tstart);
        update(outbound, input_tpset);

    }

};

PTMP_FILTER(met_tc_engine, met_michel_tc)
