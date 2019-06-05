#ifndef PTMP_TCS_API
#define PTMP_TCS_API

#include <string>
#include <czmq.h>


namespace ptmp {
    namespace tcs {

        // A TC finder runs a thread which receives and sends TPSets.
        // The input TPSets are assumed to be in time order and
        // covering fixed windows in time such as prepared by PTMP's
        // TPWindow.  If multiple streams of TPSets need combining
        // they may be done prior to input to TCFinder using TPSorted.
        // The configuration requires an "input" and "output"
        // attribute with a "socket" data structure in "standard"
        // schema.
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
