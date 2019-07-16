#include "ptmp-tcs/api.h"

// This file collects all known engines.  One day maybe it's replaced
// with a truly dynamic named factory.

// engines running https://github.com/dlast44/ProtoDuneTrigger
#include "pdt_engines.h"

ptmp::tcs::filter_engine_t* ptmp::tcs::filter_engine(const std::string& name)
{
    if (name == "pdune-adjacency-tc") {
        return new ptmp::tcs::pdt::PDUNEAdjacency_engine;
    }
    if (name == "pdune-adjacency-td") {
        return new ptmp::tcs::pdt::PDUNEAdjacencyMLT_engine;
    }
    return NULL;
}
ptmp::tcs::filter_engine_t::~filter_engine_t()
{
}
