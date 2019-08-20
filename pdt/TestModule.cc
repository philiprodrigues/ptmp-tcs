//some standard C++ includes
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

//some ROOT includes
#include "TInterpreter.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3D.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"
#include "TStopwatch.h"

//Candidate Necessary Includes
#include "AdjacencyAlgorithms.h"
#include "TriggerCandidate.h"
#include "ModuleTrigger.h"

//"larsoft" object includes
//#include "lardataobj/RecoBase/Hit.h"
//#include "nusimdata/SimulationBase/MCTruth.h"

//convenient for us! let's not bother with art and std namespaces!
using namespace std;

int main(int argc, char* argv[]){
//void AdjacencyAlgorithms_All(TString input_name, TString clustering = "") {
  TString input_name;
  TString clustering = "";

  cout << "ARGUMENT INITIALIZATION" << endl;
  if (argc < 2){
    cout << "Need an input .root file" << endl;
    return 1;
  }
  else if (argc == 2){
    input_name = argv[1];
  }
  else if (argc == 3){
    input_name = argv[1];
    clustering = argv[2];
  }
  else if (argc > 3){
    cout << "TOO MANY ARGUMENTS. ABORTING." << endl;
    return 2;
  }

  cout << "PASSED ARGUMENT INITIALIZATION." << endl;

  TString tag;
  vector<TString> tags = {"15", "18"};
  vector<vector<unsigned int>*> channels(tags.size(),0);
  vector<vector<unsigned int>*> times(tags.size(),0);
  vector<vector<unsigned int>*> tots(tags.size(),0);
  vector<vector<unsigned int>*> sum_adcs(tags.size(),0);

  vector<int> tmp(tags.size(),0);

  cout << input_name << endl;

  TFile* input_file = new TFile(input_name);

  int run;
  int event;

  TTree* tree= (TTree*)input_file->Get("tree");

  tree->SetBranchAddress("Run",&run);
  tree->SetBranchAddress("Event_Num",&event);

  for (unsigned int it=0; it < tags.size(); ++it){
    tag = tags.at(it);
    tree->SetBranchAddress("Hit_"+tag+"_Channels", &channels.at(it));
    tree->SetBranchAddress("Hit_"+tag+"_Peak_Time", &times.at(it));
    tree->SetBranchAddress("Hit_"+tag+"_TOT", &tots.at(it));
    tree->SetBranchAddress("Hit_"+tag+"_Sum_ADC", &sum_adcs.at(it));
  }

  vector<int> triggers(tags.size(),0);
  vector<int> non_empty_events(tags.size(),0);

  vector<int> candidate;
  vector<vector<int>> candidates;

  vector<TP> TPs;

  /*
  vector<vector<unsigned int>> APA_sorted_channels;
  vector<vector<unsigned int>> APA_sorted_times;
  vector<vector<unsigned int>> APA_sorted_tots;
  vector<vector<unsigned int>> APA_sorted_sum_adcs;
  */

  vector<vector<TP>> APA_sorted_TPs;

  /*
  vector<vector<unsigned int>> window_sorted_channels;
  vector<vector<unsigned int>> window_sorted_times;
  vector<vector<unsigned int>> window_sorted_tots;
  vector<vector<unsigned int>> window_sorted_sum_adcs;
  */

  vector<vector<TP>> window_sorted_TPs;

  /* NEED IMPLEMENT THIS SORT OF THING IN TriggerCandidate.cpp
  vector<vector<unsigned int>> tick_channels;
  vector<vector<unsigned int>> tick_times;
  vector<vector<unsigned int>> tick_adc;
  vector<vector<unsigned int>> tick_tot;
  */

  //vector<vector<unsigned int>> TPC_sorted_sum_adcs;

  for (int i=0; i < tree->GetEntries(); ++i){

    tree->GetEntry(i);
    cout << "Run: " << run <<" Event: " << event << endl;

    for (unsigned int j=0; j < tags.size(); ++j){
      if (times.at(j)->size() == 0) continue;
      cout << "THRESOLD: " << tags.at(j) << endl;
      ++non_empty_events.at(j);
      /*
      APA_sorted_channels = ResortHitsByAPA(*channels.at(j),*channels.at(j));
      APA_sorted_times = ResortHitsByAPA(*channels.at(j),*times.at(j));
      APA_sorted_tots = ResortHitsByAPA(*channels.at(j),*tots.at(j));
      APA_sorted_sum_adcs = ResortHitsByAPA(*channels.at(j),*sum_adcs.at(j));
      */
      //cout << "N Hits: " << times.at(j)->size() << endl;
      TPs=HitsToTPs(*channels.at(j),*times.at(j),*tots.at(j),*sum_adcs.at(j));
      //      cout << "N TPs: " << TPs.size() << endl;
      APA_sorted_TPs = ResortTPsByAPA(TPs);
      TPs.clear();

      for (unsigned int k=1; k < 4; ++k){
	if (k==2) continue;
	//if (APA_sorted_channels.at(k).size()==0) continue;
	if (APA_sorted_TPs.at(k).size() == 0){
	  continue;
	}

	int last_index=0;

	/*
	window_sorted_channels = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_channels.at(k));
	window_sorted_times = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_times.at(k));
	window_sorted_tots = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_tots.at(k));
	window_sorted_sum_adcs = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_sum_adcs.at(k));
	*/

	window_sorted_TPs = ResortTPsByWindow(APA_sorted_TPs.at(k),0);
	APA_sorted_TPs.at(k).clear();

	for (unsigned int ii=0; ii < window_sorted_TPs.size(); ++ii){
	  if (window_sorted_TPs.at(ii).size() == 0) continue;

	  if (clustering == "clus"){
	    /* NEED IMPLEMENT THIS SORT OF THING IN TriggerCandidate.cpp
	    tick_channels = SortByTick(window_sorted_times.at(ii),window_sorted_channels.at(ii),ii);
	    tick_times = SortByTick(window_sorted_times.at(ii),window_sorted_times.at(ii),ii);
	    tick_adc = SortByTick(window_sorted_times.at(ii),window_sorted_sum_adcs.at(ii),ii);
	    tick_tot = SortByTick(window_sorted_times.at(ii),window_sorted_tots.at(ii),ii);
	    */
	    
	    //candidate = TriggerCandidateHits(window_sorted_channels.at(ii),window_sorted_times.at(ii) ,window_sorted_sum_adcs.at(ii), window_sorted_tots.at(ii),1);
	    
	    candidate = TriggerCandidate(window_sorted_TPs.at(ii),1);
	  }
	  else{
	    candidate = TriggerCandidate(window_sorted_TPs.at(ii),0);
	    //candidate = TriggerCandidateHits(window_sorted_channels.at(ii),window_sorted_times.at(ii) ,window_sorted_sum_adcs.at(ii), window_sorted_tots.at(ii),0);
	  }

	  window_sorted_TPs.at(ii).clear();

	  if (candidate.size() > 0){
	    candidate.push_back((k-1)/2);
	    candidate.push_back(ii);
	    if ((k-1)/2 == 0 || candidates.size() == 0){
	      candidates.push_back(candidate);
	    }
	    else{
	      bool passed = false;
	      for (unsigned int jj=last_index; jj < candidates.size(); ++jj){
		if (max(candidate.at(6),candidate.at(7)) > max(candidates.at(jj).at(6),candidates.at(jj).at(7))) continue;
		else{
		  candidates.insert(candidates.begin()+jj,candidate);
		  last_index = jj;
		  passed=true;
		  break;
		}
	      }
	      if (!passed){
		candidates.push_back(candidate);
	      }
	    }
	  }

	  candidate.clear();
	}
      }

      triggers.at(j) += ModuleTrigger(candidates);
      candidates.clear();
    }

    cout << "" << endl;
  }

  for (unsigned int i=0; i < tags.size(); ++i){
    cout << "ADC threshold of: " << tags.at(i) << " had: " << non_empty_events.at(i) << " events that weren't empty." << endl;
    cout << "ADC thredhold of: " << tags.at(i) << " had: " << triggers.at(i) << " triggers issued." << endl;
    cout << "Corresponding efficiency: " << (double)triggers.at(i)/(double)non_empty_events.at(i) << endl;
    cout << "" << endl;
  }

  cout << "CLOSING" << endl;
  input_file->Close();
  cout << "DONE" << endl;
}
