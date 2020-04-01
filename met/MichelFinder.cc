// Note, compilation unit (file) should export only one function:
// MichelFinder().  If other functions are needed, they should be
// static.  Note, no header is needed by ptmp-tcs.

#include "ptmp/data.h"          // for TPSet
#include "MichelCalculation.h"
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

const uint32_t hwtick_per_internal = 25;

/*
struct TP{
    uint32_t channel;
    uint64_t tstart;
    uint32_t tspan;
    uint32_t adcsum;
    uint32_t adcpeak;
    uint32_t flags;
};
*/
static
TP convert(const ptmp::data::TrigPrim& tp){
   const uint64_t tstart =  tp.tstart() / (uint64_t)hwtick_per_internal;
   const uint32_t tspan = std::max((uint32_t)1, tp.tspan()/hwtick_per_internal);
   return TP{ tp.channel(), tstart, tspan, tp.adcsum(), tp.adcpeak(), tp.flags() };
}

bool MichelFinder(ptmp::data::TPSet& tpset)
{
    // This code to be filled in by Iris.  Some example skeleton is
    // included.

    int nticks = 6000;
    int windows = 100;
    int nAPAs = 6;
    int nChannels = 2560;
    int nColl = 960;
    int prim_ticks = 10;
    int adj = 1;
    int next = 0;
    int channel = 0;
    int next_channel = 0;
    std::cout<< "hello " <<std::endl;
     
    std::vector<TP> met_tps;//this is where we will store the tps from tpset
    TP emptyTP = {0,0,0,0,0,0};//break between 
    std::vector< TP> temporaryTPs;
    
    //loop through tpset to get tps out.
    for (const auto& tp : tpset.tps()){
 	met_tps.push_back(convert(tp));
    }

    //sort tps by channel withing a window

    //check for adjacent hits in channel space, if two hits are in the same channel, check the next hit for adjecency
    int first_channel = met_tps.at(0).channel;
    int first_time = met_tps.at(0).tstart;
    for (unsigned int i =0; i<met_tps.size(); i++){
 	next = (i+1)%met_tps.size();
	channel = (int)met_tps.at(i).channel;
	if (next==0){
	   next_channel=channel-1;
	}//checking out of bound
	if (next_channel == channel){
	   temporaryTPs.push_back(met_tps.at(i));
	}
	//Look for hits withing 4 channels due to missed TPs	
	else if((next_channel - channel) <= 5){
	   adj += (next_channel - channel);
	   temporaryTPs.push_back(met_tps.at(i));
	}//end checking adjacencies
	else{
	   temporaryTPs.push_back(emptyTP);
	   //now we want to check if have any potential "clusters" with high adjacency for, and check for the clusters that crossed the APA on one side but did not end on the other side of the APA. Once we find a cluster meeting that requirement we will calculate local linearity and dq/ds

	   //now we to order the TPs from distance. d=0 being the location for the first hit
	   //in the cluster
	   std::vector<size_t> ordered_TPs;
	   std::vector<double> average_TP_distance, average_TP_distance_neighbors; //these are needed for other calculations
	   double d_cutoff = 15; //based on MicroBooNE's algorithm we will need to modify
	   bool StartoneSideofAPA = (first_channel< 20 || first_channel > 940);
	   if ( adj >= 100 && StartoneSideofAPA)//check if cluster started within the first 20 channels or last 20 channels
	   {

		//order our TPs in distance and other things needed to calculate linearity and dq/ds
		ProcessTPs(temporaryTPs, ordered_TPs, average_TP_distance, average_TP_distance_neighbors, d_cutoff);		
		std::vector< double> truncated_mean, truncated_dqds,covariance, slope; 
		size_t edge_fix = 3;
		int covariance_window = 11;
		int n_window_size = 15;
		double p_above = 0.25;
		int window_cutoff = 3;
		bool calculate_truncated = CalcTruncated(temporaryTPs, ordered_TPs, average_TP_distance, truncated_mean, truncated_dqds, covariance, slope, edge_fix, covariance_window, n_window_size, p_above,window_cutoff);
		//if calculate truncated worked, move on
		if (calculate_truncated)
		{
		   size_t maxDistance = 20;
		   int boundary = 0;
		   bool find_Boundary = BoundaryFromTQMaxQ(temporaryTPs,truncated_mean,ordered_TPs,maxDistance,boundary);
		    //if boundary found based on the Maximum charge, move on to calculate local linearity
		    if (find_Boundary)
		    {
			bool lowCovinBoundary = RequireBoundaryInLowCov(temporaryTPs,covariance,boundary,0.9);
			if (lowCovinBoundary) return 0;//if the local linearity goes below 0.9 we should trigger
			else return 1;
		    }
		    continue;		   
		}
		continue;
	   }   
	   
	   //if we didn't find a suitable and the TPs are no longer adjacent proceed to keep looking for more clusters
	   first_channel = next_channel;
	   first_time = met_tps.at(next).tstart;
	   adj = 1;
	}
    }//finish looping through tps
    

    

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
