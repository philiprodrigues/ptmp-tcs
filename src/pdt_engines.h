#ifndef PTMP_TCS_PDT_ENGINES
#define PTMP_TCS_PDT_ENGINES

#include "ptmp-tcs/api.h"

#include <vector>

// A TrigPrim.flag value indicating the TP should be treated as special TC info.
// This is ugly and wrong and will go away when PTMP schema v2 is enacted.
#define PDT_SPECIAL_TP 0xdeadbeaf

namespace ptmp {
    namespace tcs {
        namespace pdt {

            struct PDUNEAdjacency_engine : public ptmp::tcs::filter_engine_t {
                ptmp::data::TPSet outbound;
                ptmp::data::data_time_t twindow{2500}; // 50us at 50MHz, fixme, need to set to match input.
                
                PDUNEAdjacency_engine();
                virtual ~PDUNEAdjacency_engine();
                virtual void operator()(const ptmp::data::TPSet& input_tpset,
                                        std::vector<ptmp::data::TPSet>& output_tpsets);
            };

            struct PDUNEAdjacencyMLT_engine : public ptmp::tcs::filter_engine_t {
                // when new TPSet is later than last_tstart, we call ModuleTrigger()
                typedef std::vector<int> candidate_t;
                std::vector<candidate_t> candidates;
                ptmp::data::TPSet outbound;
                ptmp::data::data_time_t twindow{2500}; // 50us at 50MHz, fixme, need to set to match input.

                PDUNEAdjacencyMLT_engine();
                virtual ~PDUNEAdjacencyMLT_engine();
                virtual void operator()(const ptmp::data::TPSet& input_tpset,
                                        std::vector<ptmp::data::TPSet>& output_tpsets);

                // Ingest new TPSet
                void ingest(const ptmp::data::TPSet& input_tpset);
                // Clear cached candidates and outbound
                void clear();
            };

        }
    }
}

#endif
