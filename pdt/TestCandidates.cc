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
  vector<vector<int>> nWindows_cands(2,tmp);
  vector<vector<int>> nWindows_total(2,tmp);

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

  vector<int> candidate;

  vector<vector<unsigned int>> APA_sorted_channels;
  vector<vector<unsigned int>> APA_sorted_times;
  vector<vector<unsigned int>> APA_sorted_tots;
  vector<vector<unsigned int>> APA_sorted_sum_adcs;

  vector<vector<unsigned int>> window_sorted_channels;
  vector<vector<unsigned int>> window_sorted_times;
  vector<vector<unsigned int>> window_sorted_tots;
  vector<vector<unsigned int>> window_sorted_sum_adcs;

  /* NEED IMPLEMENT THIS SORT OF THING IN TriggerCandidate.cpp
  vector<vector<unsigned int>> tick_channels;
  vector<vector<unsigned int>> tick_times;
  vector<vector<unsigned int>> tick_adc;
  vector<vector<unsigned int>> tick_tot;
  */

  vector<vector<unsigned int>> TPC_sorted_sum_adcs;

  for (int i=0; i < tree->GetEntries(); ++i){
    tree->GetEntry(i);
    cout << "Run: " << run <<" Event: " << event << endl;
    cout << "" << endl;
    for (unsigned int j=0; j < tags.size(); ++j){
      if (times.at(j)->size() == 0) continue;

      // cout << "SORTING" << endl;

      APA_sorted_channels = ResortHitsByAPA(*channels.at(j),*channels.at(j));
      APA_sorted_times = ResortHitsByAPA(*channels.at(j),*times.at(j));
      APA_sorted_tots = ResortHitsByAPA(*channels.at(j),*tots.at(j));
      APA_sorted_sum_adcs = ResortHitsByAPA(*channels.at(j),*sum_adcs.at(j));

      for (unsigned int k=1; k < 4; ++k){
	//cout << "APA: " << k << endl;

	if (k==2) continue;
	if (APA_sorted_channels.at(k).size()==0) continue;

	window_sorted_channels = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_channels.at(k));
	window_sorted_times = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_times.at(k));
	window_sorted_tots = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_tots.at(k));
	window_sorted_sum_adcs = ResortHitsByWindow(APA_sorted_times.at(k),APA_sorted_sum_adcs.at(k));

	for (unsigned int ii=0; ii < window_sorted_channels.size(); ++ii){
	  //cout << "Window: " << ii <<endl;
	  
	  if (window_sorted_channels.at(ii).size() == 0) continue;
	  ++nWindows_total.at((k-1)/2).at(j);

	  //cout << "ADJACENCY/CLUSTERING HAPPENING" << endl;
	  //cout << "" << endl;
	  if (clustering == "clus"){
	    /* NEED IMPLEMENT THIS SORT OF THING IN TriggerCandidate.cpp
	    tick_channels = SortByTick(window_sorted_times.at(ii),window_sorted_channels.at(ii),ii);
	    tick_times = SortByTick(window_sorted_times.at(ii),window_sorted_times.at(ii),ii);
	    tick_adc = SortByTick(window_sorted_times.at(ii),window_sorted_sum_adcs.at(ii),ii);
	    tick_tot = SortByTick(window_sorted_times.at(ii),window_sorted_tots.at(ii),ii);
	    */
	    candidate = TriggerCandidateHits(window_sorted_channels.at(ii),window_sorted_times.at(ii) ,window_sorted_sum_adcs.at(ii), window_sorted_tots.at(ii),1);
	  }
	  else{
	    candidate = TriggerCandidateHits(window_sorted_channels.at(ii),window_sorted_times.at(ii) ,window_sorted_sum_adcs.at(ii), window_sorted_tots.at(ii),0);
	  }

	  cout << "CHECK THE CANDIDATE" << endl;
	  if (candidate.size() > 0){
	    ++nWindows_cands.at((k-1)/2).at(j);
	    cout << "APA: " << k << " Window: " << ii << " issued a trigger candidate for threshold: " << tags.at(j) << endl;
	    cout << "First channel: " << candidate.at(4) << " at tick: " << candidate.at(6) << endl;
	    cout << "Last channel: " << candidate.at(5) << " at tick: " << candidate.at(7) << endl;
	  }
	}
      }
    }
  }

  cout << "Total candidate efficiency for first tag, first interesting APA: " << (double)nWindows_cands.at(0).at(0)/(double)nWindows_total.at(0).at(0) << endl;
  cout << "Total candidate efficiency for first tag, second interesting APA: " << (double)nWindows_cands.at(1).at(0)/(double)nWindows_total.at(1).at(0) << endl;
  cout << "Total candidate efficiency for second tag, first interesting APA: " << (double)nWindows_cands.at(0).at(1)/(double)nWindows_total.at(0).at(1) << endl;
  cout << "Total candidate efficiency for second tag, second interesting APA: " << (double)nWindows_cands.at(1).at(1)/(double)nWindows_total.at(1).at(1) << endl;

  cout << "CLOSING" << endl;
  input_file->Close();
  cout << "DONE" << endl;
}
