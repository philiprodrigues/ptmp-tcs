#include "IsoMuon_engine.h"

#include "json.hpp"

#include "ptmp/data.h" // for ptmp::data::now()

IsoMuon_engine::IsoMuon_engine(const std::string& config)
    : hits_per_link_threshold_(85)
{
    auto jcfg = nlohmann::json::parse(config);
    try{
        if (jcfg["algconfig"]["hits_per_link_threshold"].is_number()) {
            hits_per_link_threshold_ = jcfg["algconfig"]["hits_per_link_threshold"];
        }
    }
    catch(nlohmann::detail::exception& e){
        zsys_warning("Caught exception while reading config: %s", e.what());
    }
}

IsoMuon_engine::~IsoMuon_engine()
{
    zsys_info("Received %ld TPSets. TPC-facing: %ld, Wall-facing: %ld ", n_sets_total, n_sets_tpc, n_sets_wall);
    zsys_info("Maximum number of channels hit in a window: %ld", max_n_chan_hit);
    zsys_info("Issued %ld trigger candidates", n_triggers);
}

void IsoMuon_engine::operator()(const ptmp::data::TPSet& in_set,
                                           std::vector<ptmp::data::TPSet>& output_tpsets)
{

    ++n_sets_total;
    // If the data is from a wall-facing link, just ignore
    // it. In APA 5, it turns out that the wall-facing links
    // are all fiber 2. detid contains (fiber_no << 16) |
    // (slot_no << 8) | m_crate_no
    size_t fiber_no=(in_set.detid() >> 16) & 0xff;
    if(fiber_no==2){
        ++n_sets_wall;
        return;
    }
    ++n_sets_tpc;
    if(in_set.tstart() > last_tstart){
        // This is the first item from a new time
        // window. Decide whether the previous one should
        // trigger, based on n_sources and how many items in
        // has_hit are set, then reset last_tstart, clear has_hit and set n_sources=0
        //
        // This approach (of waiting for a new time window before
        // finalizing the previous one) has the disadvantage that we
        // don't send out a trigger for window N until we've got the
        // first TPSet for window N+1: if N+1 arrives much later, then
        // we get a big latency for N. In this particular case, this
        // turns out not to be too much of a problem, because the rate
        // of TPSets from links is high. Further along the chain,
        // where the input rate is lower, this approach results in
        // unacceptable latencies. We use it here because it allows us
        // to form the trigger candidate without having to know the
        // number of input links. That's useful because sometimes
        // FELIX links are dead even when they're enabled in the RC,
        // and this way, we don't have to try to detect that and deal
        // with it
        max_n_sources=std::max(n_sources, max_n_sources);

        size_t n_chan_hit=has_hit.count();
        if(n_chan_hit>=hits_per_link_threshold_*max_n_sources){
            // Trigger!
            zsys_debug("Requesting trigger at 0x%lx with %ld (threshold %ld, max_n_sources %ld)",
                       last_tstart, n_chan_hit, hits_per_link_threshold_*max_n_sources, max_n_sources);
            ptmp::data::TPSet trigger_tpset;
            trigger_tpset.set_count(count++);
            trigger_tpset.set_detid(0xffffff);
            trigger_tpset.set_created(ptmp::data::now());
            trigger_tpset.set_tstart(last_tstart);
            output_tpsets.push_back(trigger_tpset);
            ++n_triggers;
        }

        last_tstart=in_set.tstart();
        // Reset the channels to "not hit"
        has_hit.reset();
        n_sources=0;
    } // end if(new time window)

    // For every input TPSet, we increment the number of sources and
    // mark its channel as having been hit
    ++n_sources;
    for(auto const& tp: in_set.tps()){
        if(tp.channel()-min_channel >= has_hit.size()){
            zsys_error("Got too-large channel: %d with min channel %d and bitset capacity %ld",
                       tp.channel(), min_channel, has_hit.size());
        }
        else{
            has_hit.set(tp.channel()-min_channel);
        }
    }
}

PTMP_FILTER(IsoMuon_engine, isomuon_tc)
