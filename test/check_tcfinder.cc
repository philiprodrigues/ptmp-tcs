
#include "ptmp/api.h"
#include "ptmp/cmdline.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using json = nlohmann::json;
using namespace ptmp::cmdline;

int main(int argc, char* argv[])
{
    zsys_init();
    CLI::App app{"Run TC finder"};

    int detid = 0x12345678;
    app.add_option("-d,--detid", detid, "numerical detector ID for the TC output");


    std::string engine = "pdune_adjacency_tc";
    app.add_option("-e,--engine", engine, "The name of the TC finder filter engine");

    int countdown = -1;         // forever
    app.add_option("-c,--count", countdown,
                   "Number of seconds to count down before exiting, simulating real app work");
    
    CLI::App* isocks = app.add_subcommand("input", "Input socket specification");
    CLI::App* osocks = app.add_subcommand("output", "Output socket specification");
    app.require_subcommand(2);

    sock_options_t iopt{1000,"SUB", "connect"}, oopt{1000,"PUB", "bind"};
    add_socket_options(isocks, iopt);
    add_socket_options(osocks, oopt);

    CLI11_PARSE(app, argc, argv);
    
    json jcfg;
    jcfg["input"] = to_json(iopt);
    jcfg["output"] = to_json(oopt);
    jcfg["engine"] = engine;
    jcfg["detid"] = detid;
    
    // std::cerr << "Using config: " << jcfg << std::endl;

    std::string cfgstr = jcfg.dump();

    {
        ptmp::TPFilter proxy(cfgstr);

        int snooze = 1000;
        while (countdown != 0) {
            -- countdown;
            int64_t t1 = zclock_usecs ();
            zclock_sleep(snooze);
            int64_t t2 = zclock_usecs ();
            if (std::abs((t2-t1)/1000-snooze) > 10) {
                std::stringstream ss;
                ss << "check_tcfinder: sleep interrupted, "
                   << " dt=" << (t2-t1)/1000-snooze
                   <<" t1="<<t1<<", t2="<<t2<<", snooze="<<snooze;
                std::cerr << ss.str() << std::endl;
                zsys_info(ss.str().c_str());
                break;
            }

            zsys_debug("tick %d", countdown);
        }

        std::cerr << "check_tcfinder exiting\n";
    }

    return 0;
}
