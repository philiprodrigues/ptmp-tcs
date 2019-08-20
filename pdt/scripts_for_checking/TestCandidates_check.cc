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

  //  vector<int> tmp(tags.size(),0);
  /*  vector<vector<int>> nWindows_cands(2,tmp);
   vector<vector<int>> nWindows_total(2,tmp);
  */

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

  /*
  vector<TP> TPs_15;
  vector<TP> TPs_18;
  //vector<TP> TPs_15;
  vector<vector<TP>> APA_sorted_TPs_15;
  vector<vector<TP>> APA_sorted_TPs_18;
  vector<vector<TP>> window_sorted_TPs_15;
  vector<vector<TP>> window_sorted_TPs_18;
  */

  for (int i=0; i < tree->GetEntries(); ++i){
    tree->GetEntry(i);
    cout << "Run: " << run <<" Event: " << event << endl;
    if (channels.at(0)->size() < channels.at(1)->size()){
      cout << "MORE ADC 18 Hits than ADC 15" << endl;
    }
  }

  cout << "CLOSING" << endl;
  input_file->Close();
  cout << "DONE" << endl;
}
