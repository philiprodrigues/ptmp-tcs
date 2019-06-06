#ifndef PTMP_TCS_TCFINDER_PDT
#define PTMP_TCS_TCFINDER_PDT

#include "ptmp-tcs/api.h"
#include "pdt/AdjacencyAlgorithms.h"

#include <vector>



namespace ptmp {
    namespace tcs {
        namespace pdt {

            struct PDUNEAdjacency_engine : public ptmp::tcs::tcfinder_engine_t {
                std::vector<TP> tps;
                virtual ~PDUNEAdjacency_engine();
                virtual void operator()(const ptmp::data::TPSet& input_tpset,
                                        std::vector<ptmp::data::TPSet> output_tpsets);
            };

        }
    }
}

#endif
