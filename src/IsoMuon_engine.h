#ifndef ISOMUON_ENGINE_H
#define ISOMUON_ENGINE_H

#include "ptmp/filter.h"
#include <czmq.h>
#include <bitset>

// A simple TPFilter engine designed for finding isochronous
// ("quasi-horizontal") tracks. The input is assumed to be windowed
// with TPWindow and sorted with TPZipper, and then the trigger
// condition is that at least N channels are hit in one of the time
// windows.
//
// This is a particularly simple and not very smart condition: a
// nearly-horizontal muon will trigger if it happens to fit in one
// time window, but will not trigger if the same muon is displace in
// time a bit so that it straddles two windows. Also, there's nothing
// in the algorithm itself forcing the muon to be "horizontal": if you
// make the TPWindow wider, the muon can go at any angle as long as it
// "fits" inside the time window. Horizontalness is imposed by making
// the TPWindow narrow.  Despite these limitations, this is still good
// enough for testing the trigger things we care about.
//
// The number-of-channels threshold is set as number of channels _per
// link_, so that we can feed this algorithm any number of links
// (which could be multiple APAs). The condition is just applied as a
// total number of channels hit: that is, the threshold is (hits per
// link)*(n links) and there's no requirement that *every* link has a
// given number of hits.
//
// There are 96 collection channels per link, some of which are dead
// or suppressed because they're noisy, so sensible values for the
// number of channels per link are 80-90 or so.
//
// We also need to know the number of links that are feeding this
// algorithm, which we take to be the maximum number of links seen so
// far for one time window.
class IsoMuon_engine : public ptmp::filter::engine_t
{
public:
    IsoMuon_engine(const std::string& config);
    virtual ~IsoMuon_engine();
    virtual void operator()(const ptmp::data::TPSet& input_tpset,
                            std::vector<ptmp::data::TPSet>& output_tpsets) override;
private:
    
    size_t hits_per_link_threshold_;
    // The count value we'll put on the next TPSet we send out
    size_t count{0};
    
    // Debugging stats
    size_t n_n_sources[10]{0,};
    size_t n_sets_total{0};
    size_t n_sets_wall{0};
    size_t n_sets_tpc{0};
    size_t max_n_chan_hit{0};
    size_t n_triggers{0};

    // The last tstart value we saw. If we trigger, this will be the
    // timestamp we'll request the trigger for
    uint64_t last_tstart{0};
    // The number of sources (links) we've seen so far for the current time window
    size_t n_sources{0};
    // The maximum number of sources (links) we've seen so far for
    // *any* time window: used to calculate the overall threshold
    size_t max_n_sources{0};
            
    static constexpr size_t apa5_offline_number{1};
    // const size_t apa6_offline_number=3;
    static constexpr size_t channels_per_apa{2560};
    static constexpr size_t min_channel{channels_per_apa*apa5_offline_number};
    // Did we get a hit on this channel in this time window? Extremely
    // dumb because it has a spot for every possible channel in APA 5
    // and 6, even though most are induction and won't have hits. But
    // this makes the later code easier
    std::bitset<3*channels_per_apa> has_hit;
            
};

#endif
