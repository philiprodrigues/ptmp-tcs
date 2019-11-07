// Note, compilation unit (file) should export only one function:
// MichelFinder().  If other functions are needed, they should be
// static.  Note, no header is needed by ptmp-tcs.

#include "ptmp/data.h"          // for TPSet

// MichelFinder is the function called by ptmp-tcs.
//
// The function is called with a TPSet holding TPs spanning a time
// window and a unit of detector.  These two spans depend on what
// uptream TPSet producers exist but the nominal configuration may
// assume the TPSet spans 1 APA face and span a time which is on order
// of a drift time.
//
// The function SHALL return true to indicate the TPSet shall be
// emitted to the network.  It MAY modify the TPSet and in particular
// SHOULD set TPSet.tstart and TPSet.tspan to indicate a region of
// time around the Michel activity.  The function SHALL return false
// to inticate the input TPSet holds no interesting information.

bool MichelFinder(ptmp::data::TPSet& tpset)
{
    // This code to be filled in by Iris.  Some example skeleton is
    // included.

    // loop over  TPs in the TPSet
    uint64_t dumb_selection = 0;
    for (const auto& tp : tpset.tps()) {
        if (tp.flags() != 0) {
            // Don't expect any but skip error TPs.
            continue;
        }
        dumb_selection += tp.adcsum();
    }

    // a truly bogus selection
    const uint64_t magic_number = 10000;
    if (dumb_selection > magic_number) {
        tpset.set_tstart(tpset.tstart() + tpset.tspan()/4);
        tpset.set_tspan(tpset.tspan() - tpset.tspan()/4);
        return true;
    }

    return false;
}
