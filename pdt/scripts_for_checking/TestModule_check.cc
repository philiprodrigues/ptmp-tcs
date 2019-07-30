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

  int trigger_15;
  int trigger_18;

  vector<int> ntriggers(tags.size(),0);

  vector<int> candidate;
  vector<vector<int>> candidates;

  vector<TP> TPs_15;
  vector<TP> TPs_18;

  /*
  vector<vector<unsigned int>> APA_sorted_channels;
  vector<vector<unsigned int>> APA_sorted_times;
  vector<vector<unsigned int>> APA_sorted_tots;
  vector<vector<unsigned int>> APA_sorted_sum_adcs;
  */

  vector<vector<TP>> APA_sorted_TPs_15;
  vector<vector<TP>> APA_sorted_TPs_18;

  /*
  vector<vector<unsigned int>> window_sorted_channels;
  vector<vector<unsigned int>> window_sorted_times;
  vector<vector<unsigned int>> window_sorted_tots;
  vector<vector<unsigned int>> window_sorted_sum_adcs;
  */

  vector<vector<TP>> window_sorted_TPs_15;
  vector<vector<TP>> window_sorted_TPs_18;

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

    TPs_15=HitsToTPs(*channels.at(0),*times.at(0),*tots.at(0),*sum_adcs.at(0));
    APA_sorted_TPs_15 = ResortTPsByAPA(TPs_15);

    TPs_18=HitsToTPs(*channels.at(1),*times.at(1),*tots.at(1),*sum_adcs.at(1));
    APA_sorted_TPs_18 = ResortTPsByAPA(TPs_18);

    if (TPs_15.size() < TPs_18.size()){
      cout << "MORE 18 THAN 15" << endl;
    }

    TPs_15.clear();
    TPs_18.clear();

    for (unsigned int k=1; k < 4; ++k){
      if (k==2) continue;
      if (APA_sorted_TPs_15.at(k).size() == 0)	continue;

      int last_index=0;
      window_sorted_TPs_15 = ResortTPsByWindow(APA_sorted_TPs_15.at(k),0);
      APA_sorted_TPs_15.at(k).clear();

      for (unsigned int ii=0; ii < window_sorted_TPs_15.size(); ++ii){
	if (window_sorted_TPs_15.at(ii).size() == 0) continue;
	candidate = TriggerCandidate(window_sorted_TPs_15.at(ii),0);

	window_sorted_TPs_15.at(ii).clear();

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
    trigger_15 = ModuleTrigger(candidates);
    candidates.clear();

    for (unsigned int k=1; k < 4; ++k){
      if (k==2) continue;
      if (APA_sorted_TPs_18.at(k).size() == 0)	continue;

      int last_index=0;
      window_sorted_TPs_18 = ResortTPsByWindow(APA_sorted_TPs_18.at(k),0);
      APA_sorted_TPs_18.at(k).clear();

      for (unsigned int ii=0; ii < window_sorted_TPs_18.size(); ++ii){
	if (window_sorted_TPs_18.at(ii).size() == 0) continue;
	candidate = TriggerCandidate(window_sorted_TPs_18.at(ii),0);

	window_sorted_TPs_18.at(ii).clear();

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
    trigger_18 = ModuleTrigger(candidates);
    candidates.clear();
    if (trigger_18 != trigger_15){
      if (trigger_18 ==1){
	cout << "18 but not 15" << endl;
      }
      else{
	cout << "15 but not 18" << endl;
      }
    }
    ntriggers.at(0)+= trigger_15;
    ntriggers.at(1)+= trigger_18;

    trigger_15=0;
    trigger_18=0;
  }

  cout << "triggers at 18: " << ntriggers.at(1) << endl;
  cout << "triggers at 15: " << ntriggers.at(0) << endl;

  cout << "CLOSING" << endl;
  input_file->Close();
  cout << "DONE" << endl;
}
