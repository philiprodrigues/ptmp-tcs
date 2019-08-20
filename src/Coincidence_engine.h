#ifndef COINCIDENCE_ENGINE_H
#define COINCIDENCE_ENGINE_H

#include "ptmp/filter.h"
#include <czmq.h>


class Coincidence_engine : public ptmp::filter::engine_t
{
public:
    Coincidence_engine(const std::string& config);
    virtual ~Coincidence_engine();
    virtual void operator()(const ptmp::data::TPSet& input_tpset,
                            std::vector<ptmp::data::TPSet>& output_tpsets) override;
private:
    // The count value we'll put on the next TPSet we send out
    size_t count{0};
    
    size_t n_sets_total{0};
    size_t n_triggers{0};

    // The last tstart value we saw. If we trigger, this will be the
    // timestamp we'll request the trigger for
    uint64_t last_tstart{0};
    // The number of sources (links) we've seen so far for the current time window
    size_t n_sources{0};
    // The maximum number of sources (links) we've seen so far for
    // *any* time window: used to calculate the overall threshold
    size_t max_n_sources{0};

    size_t nway{0};
};

#endif
