#include "Coincidence_engine.h"

#include "json.hpp"

#include "ptmp/data.h" // for ptmp::data::now()

Coincidence_engine::Coincidence_engine(const std::string& config)
{
    auto jcfg = nlohmann::json::parse(config);
    nway=2;
    try{
        if (jcfg["algconfig"]["nway"].is_number()) {
            nway = jcfg["algconfig"]["nway"];
        }
    }
    catch(nlohmann::detail::exception& e){
        zsys_warning("Caught exception while reading config: %s", e.what());
    }
}

Coincidence_engine::~Coincidence_engine()
{
    zsys_info("Received %ld TPSets", n_sets_total);
    zsys_info("Issued %ld trigger candidates", n_triggers);
}

void Coincidence_engine::operator()(const ptmp::data::TPSet& in_set,
                                           std::vector<ptmp::data::TPSet>& output_tpsets)
{

    ++n_sets_total;

    if(in_set.tstart() > last_tstart){
        // This is the first item from a new time
        // window. Decide whether the previous one should
        // trigger, based on n_sources and how many items in
        // has_hit are set, then reset last_tstart, clear has_hit and set n_sources=0
        max_n_sources=std::max(n_sources, max_n_sources);

        last_tstart=in_set.tstart();
        n_sources=0;
    } // end if(new time window)

    ++n_sources;

    if(n_sources==nway){
        // Trigger!
        zsys_debug("Requesting trigger at 0x%lx at 0x%lx with %ld sources", in_set.tstart(), ptmp::data::now(), n_sources);

        ptmp::data::TPSet trigger_tpset;
        trigger_tpset.set_count(count++);
        trigger_tpset.set_detid(0xffffff);
        trigger_tpset.set_created(ptmp::data::now());
        trigger_tpset.set_tstart(in_set.tstart());

        output_tpsets.push_back(trigger_tpset);
        ++n_triggers;
    }
}

PTMP_FILTER(Coincidence_engine, coincidence_td)
