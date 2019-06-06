#ifndef PTMP_TCS_API
#define PTMP_TCS_API

#include <string>
#include "ptmp/api.h"


namespace ptmp {
    namespace tcs {

        // The TCFinder delegates actual TC finding to other code.  To
        // allow dynamic choice of what code to use, it is wrapped in
        // this simple interface.  
        struct tcfinder_engine_t {
            virtual ~tcfinder_engine_t();
            virtual void operator()(const ptmp::data::TPSet& input_tpset,
                                    std::vector<ptmp::data::TPSet> output_tpsets);
        };

        // Look up TC finder engine  by its name.  Throw if not found.
        tcfinder_engine_t* tcfinder_engine(const std::string& name);

        // A TC finder runs a thread which receives and sends TPSets.
        // The input TPSets are assumed to be in time order and
        // covering fixed windows in time such as prepared by PTMP's
        // TPWindow.  If multiple streams of TPSets need combining
        // they may be done prior to input to TCFinder using TPSorted.
        // The configuration requires an "input" and "output"
        // attribute with a "socket" data structure in "standard"
        // schema.  A "method" gives the TC finder engine to use.
        class TCFinder {
        public:
            TCFinder(const std::string& config);
            ~TCFinder();
        private:
            zactor_t* m_actor;
        };
    }
}

#endif
