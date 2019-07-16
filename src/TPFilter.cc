#include "ptmp-tcs/api.h"
#include "ptmp/api.h"

#include "json.hpp"

using json = nlohmann::json;

#ifdef PTMP_AGENT
PTMP_AGENT(ptmp::tcs::TCFinder, tcfinder)
#endif

// The actor function
void filter_proxy(zsock_t* pipe, void* vargs)
{
    auto config = json::parse((const char*) vargs);
    std::string method = config["method"];
    // one day, maybe use named factory.  For now, hard-wired.
    ptmp::tcs::filter_engine_t* engine = ptmp::tcs::filter_engine(method);
    if (!engine) {
        zsys_error("No such filter: \"%s\"", method.c_str());
        return;
    }
    

    zsock_t* isock = ptmp::internals::endpoint(config["input"].dump());
    zsock_t* osock = ptmp::internals::endpoint(config["output"].dump());
    if (!isock or !osock) {
        zsys_error("filter requires socket configuration");
        return;
    }
    
    zpoller_t* pipe_poller = zpoller_new(pipe, isock, NULL);

    zsock_signal(pipe, 0);      // signal ready

    while (!zsys_interrupted) {

        void* which = zpoller_wait(pipe_poller, -1);
        if (!which) {
            zsys_info("filter interrupted");
            break;
        }
        if (which == pipe) {
            zsys_info("filter got quit");
            break;
        }

        zmsg_t* msg = zmsg_recv(isock);
        if (!msg) {
            zsys_info("filter interrupted");
            zmsg_destroy(&msg);
            break;
        }

        ptmp::data::TPSet tps;
        ptmp::internals::recv(msg, tps); // throws
        int64_t latency = zclock_usecs() - tps.created();

        std::vector<ptmp::data::TPSet> output_tpsets;
        (*engine)(tps, output_tpsets);
        if (output_tpsets.empty()) {
            continue;
        }
        if (osock) {            // allow null for debugging
            for (const auto& otpset : output_tpsets) {
                ptmp::internals::send(osock, otpset); // fixme: can throw
            }
        }
        else {
            zsys_debug("filter got %ld TPSets", output_tpsets.size());
        }
            
    }

    zpoller_destroy(&pipe_poller);
    zsock_destroy(&isock);
    zsock_destroy(&osock);
} 

ptmp::tcs::TPFilter::TPFilter(const std::string& config)
    : m_actor(zactor_new(filter_proxy, (void*)config.c_str()))

{
}

ptmp::tcs::TPFilter::~TPFilter()
{
    zsock_signal(zactor_sock(m_actor), 0); // signal quit
    zclock_sleep(1000);
    zactor_destroy(&m_actor);
}
