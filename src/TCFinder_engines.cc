#include "ptmp-tcs/api.h"

// This file collects all known engines.  One day maybe it's replaced
// with a truly dynamic named factory.

// engines running https://github.com/dlast44/ProtoDuneTrigger
#include "TCFinder_pdt.h"

ptmp::tcs::tcfinder_engine_t* ptmp::tcs::tcfinder_engine(const std::string& name)
{
    if (name == "pdune-adjacency") {
        return new ptmp::tcs::pdt::PDUNEAdjacency_engine;
    }
    // if (name == "time-adjacency-counting") {
    //     return new ptmp::tcs::pdt::time_adjacency_counting_t;

    // }
    return NULL;
}
