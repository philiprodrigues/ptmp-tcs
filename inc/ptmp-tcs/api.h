#ifndef PTMP_TCS_API
#define PTMP_TCS_API

#include <string>
#include "ptmp/api.h"


namespace ptmp {
    namespace tcs {

        // The TP filter merely marshals messages between sockets and
        // an "engine" which does the real work.
        struct filter_engine_t {
            virtual ~filter_engine_t();
            virtual void operator()(const ptmp::data::TPSet& input_tpset,
                                    std::vector<ptmp::data::TPSet>& output_tpsets) = 0;
        };

        // Look up filter engine by its name.  Throw if not found.
        filter_engine_t* filter_engine(const std::string& name);

        // A TP filter runs a thread which receives, passes them to
        // its engine and if any are returned, sends those on.  The
        // configuration requires an "input" and "output" attribute
        // with a "socket" data structure in "standard" schema.  A
        // "method" gives the filter engine to use.
        class TPFilter {
        public:
            TPFilter(const std::string& config);
            ~TPFilter();
        private:
            zactor_t* m_actor;
        };
    }
}

#endif
