#include "MichelTriggerModule.h"
#include "MichelCalculationFunction.h"

int MichelTriggerModule(std::vector<TriggerCandidate> candidates){
    //using namespace std;
  int prev_channel = -9000;
  int prev_time = -9000;
  int nChannels = 480;
  int last_APA = -9000;
  std::vector< std::pair<double,double> > Michel_candidates;
  std::pair<double,double> m_candidate;
  int michel_candidates_num = 0;
  for ( unsigned int i_candidate =0; i_candidate<candidates.size();i_candidate++){
  
    //check if the last channel in the trigger candidates is not at the end of the APA and previous trigger
    //candidates is also in the same APA
   
      if (last_APA == candidates.at(i_candidate).apanum){
      //if (last_APA == candidates.at(i_candidate).at(8)){
        if(fabs(candidates.at(i_candidate).first_ch - prev_channel) <=1 ){
        //if(fabs(candidates.at(i_candidate).at(4)- prev_channel) <=1  ){
           // Michel_candidates.push_back(i_candidate);
            michel_candidates_num++;
            //std::cout << " grouping possible trigger" <<std::endl;
            //std::cout << fabs(candidates.at(i_candidate).first_ch- prev_channel) << " Channel difference" << std::endl;
        }
        else{
            //std::cout <<candidates.at(i_candidate).at(4)- prev_channel<< " Too big of a channel gap" << std::endl;
            //if the next TC candidates is not withing a wire, check if the previous ones were, and try to trigger
            if (michel_candidates_num >0){
                m_candidate.first = candidates.at(i_candidate-michel_candidates_num-1).first_time;
                m_candidate.second = candidates.at(i_candidate-1).last_time;
                Michel_candidates.push_back(m_candidate);
                //std::cout << "Added a several candidate trigger " << i_candidate-michel_candidates_num << ","<< i_candidate<<std::endl;
                //std::cout << fabs(candidates.at(i_candidate).first_ch- prev_channel) << " Channel difference" << std::endl;
                michel_candidates_num = 0;
                
            }

            michel_candidates_num = 0;
        }
      
      
    }//check end channel of trigger candidates
      
      else{
          if (michel_candidates_num >0){
                    m_candidate.first = candidates.at(i_candidate-michel_candidates_num-1).first_time;
                m_candidate.second = candidates.at(i_candidate-1).last_time;
                Michel_candidates.push_back(m_candidate);
                //std::cout << "Added a several candidate trigger " << i_candidate-michel_candidates_num << ","<< i_candidate<<std::endl;
                //std::cout << fabs(candidates.at(i_candidate).first_ch- prev_channel) << " Channel difference" << std::endl;
                michel_candidates_num = 0;
                         
                }
          michel_candidates_num = 0;
          
          
      }

      //gets the last channel and APA for the first entry
    if (fabs(candidates.at(i_candidate).last_ch-candidates.at(i_candidate).first_ch )< nChannels){
      prev_channel = candidates.at(i_candidate).last_ch;
      prev_time = candidates.at(i_candidate).last_time;
      last_APA = candidates.at(i_candidate).apanum;
    }
  }//finish looping through the trigger candidates

    for( unsigned int michel_tc =0; michel_tc < 1; michel_tc++){
    //now look search through the TPs given their tstarts
        const auto tmp1 = Michel_candidates.at(michel_tc).first;
        const auto tmp2 = Michel_candidates.at(michel_tc).second;
    
        //std::cout <<std::setprecision(30)<<Michel_candidates.at(0).first <<std::endl;
        //std::cout <<std::setprecision(30)<<Michel_candidates.at(0).second <<std::endl;
        //std::cout << tmp2 << std::endl;
        //auto first_tp = std::find_if(candidates.begin(),candidates.end(),[](const TP& tps){return tps.tstart == Michel_candidates.at(0).first ;});
        auto first_tp = std::find_if(candidates.begin(),candidates.end(),[tmp1](const TP& tp_1){return (double)tp_1.tstart == tmp1 ;});
        auto last_tp = std::find_if(candidates.begin(),candidates.end(),[tmp2](const TP& tp_2){return (double)tp_2.tstart == tmp2 ;});

    //std::cout << first_tp -candidates.begin() <<std::endl;
    //std::cout << last_tp - candidates.begin() << std::endl;
        int tp1_index = (int)(first_tp -tpset.tps.begin());
        int tp2_index = (int)(last_tp - tpset.tps.begin());
    //std::cout << tp1_index <<std::endl;
    //std::cout << tp2_index << std::endl;
    
        std::vector<TP> trigger_hits;
        trigger_hits.reserve(tp2_index - tp1_index +1 );
    //std::vector<TriggerCandidate> trigger_hits;
        for (int i_tp = tp1_index; i_tp < tp2_index +1; i_tp++){
            trigger_hits.emplace_back(tpset.tps[i_tp]);
            //std::cout <<std::setprecision(30) <<trigger_hits.at(trigger_hits.size()-1).first_time << " " << trigger_hits.size()<< std::endl;
        }
        
        //now we proceed to calculate linearity and dqds..
        std::vector<size_t> ordered_TPs;
        std::vector<double> average_TP_distance, average_TP_distance_neighbors;
        double d_cutoff = 15;
        
        //this will order the TPs in
        ProcessTPs(trigger_hits, ordered_TPs, average_TP_distance, average_TP_distance_neighbors, d_cutoff);
        std::vector< double> truncated_mean, truncated_dqds,covariance, slope;
        size_t edge_fix = 3;
        int covariance_window = 11;
        int n_window_size = 15;
        double p_above = 0.25;
        int window_cutoff = 3;
        bool calculate_truncated = CalcTruncated(trigger_hits, average_TP_distance, truncated_mean, truncated_dqds, covariance, slope, edge_fix, covariance_windowm n_window_size, p_above,window_cutoff);
        
        //if calculate truncated worked, move on
        if (calculate_truncated){
            size_t maxDistance = 20;
            int boundary = 0;
            bool find_BoundaryFromTQMaxQ = BoundaryFromTQMaxQ(trigger_hits,truncated_mean,ordered_TPs,maxDistance,boundary);
            //if boundary found, move on
            if (find_BoundaryFromTQMaxQ)
            {
                bool lowCovinBoundary = RequireBoundaryInLowCov(trigger_hits,covariance,boundary,0.9);
                if (lowCovinBoundary) return 0;
                else return 1;
            }
            continue;
            
        }
        continue;
    }//
  //return 0;
}
