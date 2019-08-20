
#include "pdt_engines.h"
#include "ptmp/data.h"
#include "ptmp/filter.h"
#include "json.hpp"

#include <czmq.h>

#include <vector>

// #include "pdt/ModuleTrigger.h"
int ModuleTrigger( std::vector<std::vector<int> > candidates);

using json = nlohmann::json;

/* 
From David:

It holds an APA number where I've set it such that 0 is the Upstream
Apa instrumented with Felix, that 1 is the midstream and so on. It can
be calculated from the channel so either we don't change the detid and
calculate it later, or we add a little functionality to calculate it
using the channel of any hit and set it when creating the output TPSet
that represents the TC.


From Phil:

 detid set inside FELIX_BR:

 (m_fiber_no << 16) | (m_slot_no << 8) | m_crate_no
    
m_crate_no is the same as the "online" APA number ("Installation numbering" below):

 Installation numbering:
             APA5  APA6  APA4
  beam -->
             APA3  APA2  APA1

  The Offline numbering:
             APA1  APA3  APA5
  beam -->
             APA0  APA2  APA4

So, based on David's message, the mapping is:

m_crate_no	candidate[8]
5		0
6		1
4		2

(m_crate_no=4 will never occur in the current system, as APA 4 isn't
instrumented with FELIX yet, but the plan is that it will be
eventually).

*/
static
int detid_to_offline_apa_number(int detid)
{
    //                installation APA number  1   2   3   4   5   6
    const std::vector<int> i2o_apa_map = {-1, -1, -1, -1,  2,  0,  1};
    const size_t inst_apa = detid & 0x000000FF;
    if (inst_apa < 0 or inst_apa > 6) { return -1; }
    return i2o_apa_map[inst_apa];
}

// Fixme: TPSet holds 50MHz clocks, PDT assumes 2MHz
const int hwtick_per_internal = 25;

class pdt_td_engine : public ptmp::filter::engine_t {
    // when new TPSet is later than last_tstart, we call ModuleTrigger()
    typedef std::vector<int> candidate_t;
    std::vector<candidate_t> candidates;
    ptmp::data::TPSet outbound;
    ptmp::data::data_time_t twindow{2500}; // 50us at 50MHz, fixme, need to set to match input.
public:
    pdt_td_engine(const std::string& /*unused for now*/) {
    }

    virtual ~pdt_td_engine() {
    }


    void ingest(const ptmp::data::TPSet& fresh) {
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

        // ModuleTrigger() wants a 9-tuple of ints.  First eight entries
        // are what TriggerCommand() returns.  The ninth is the *offline*
        // APA number (not installation number).
        // 
        //     0         1       2       3      4      5     6    7   8
        // (adjacency, adcpeak, adcsum, tspan, chan1, chan2, t1, t2, apanum)
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
        candidate[8] = detid_to_offline_apa_number(fresh.detid());
        candidates.push_back(candidate);

        outbound.set_chanbeg(std::min(outbound.chanbeg(), fresh.chanbeg()));
        outbound.set_chanend(std::min(outbound.chanend(), fresh.chanend()));
    }

    virtual void operator()(const ptmp::data::TPSet& input_tpset,
                            std::vector<ptmp::data::TPSet>& output_tpsets) {
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

    void clear() {
        // don't clear detid
        outbound.set_tstart(0);
        outbound.set_tspan(0);
        outbound.set_chanbeg(0);
        outbound.set_chanend(0);
        outbound.set_totaladc(0);
        outbound.clear_tps();
        candidates.clear();
    }

}; // class pdt_td_engine

PTMP_FILTER(pdt_td_engine, pdune_adjacency_td)
