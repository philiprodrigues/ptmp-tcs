#ifndef PTMP_TCS_TCFINDER_PDT
#define PTMP_TCS_TCFINDER_PDT

#include "ptmp-tcs/api.h"

#include <vector>



namespace ptmp {
    namespace tcs {
        namespace pdt {

            struct PDUNEAdjacency_engine : public ptmp::tcs::tcfinder_engine_t {
                ptmp::data::TPSet outbound;
                int64_t twindow{2500}; // 50us at 50MHz, fixme, need to set to match input.
                
                PDUNEAdjacency_engine();
                virtual ~PDUNEAdjacency_engine();
                virtual void operator()(const ptmp::data::TPSet& input_tpset,
                                        std::vector<ptmp::data::TPSet>& output_tpsets);

            };

        }
    }
}

#endif
