/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Exectuable: TMVARegressionApplication                                          *
 *                                                                                *
 * This macro provides a simple example on how to use the trained regression MVAs *
 * within an analysis module                                                      *
 **********************************************************************************/

#include <cstdlib>
#include <vector>
#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "TNtuple.h"

#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/LossFunction.h"

using namespace TMVA;

void TMVARegressionApplication( TString myMethodList = "" ) 
{
   //---------------------------------------------------------------
   // This loads the library
   TMVA::Tools::Instance();

   std::cout << std::endl;
   std::cout << "==> Start TMVARegressionApplication" << std::endl;

   // --------------------------------------------------------------------------------------------------

   // --- Create the Reader object

   TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );    

   // Create a set of variables and declare them to the reader
   // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
   Float_t GenPt, Eta, dPhi12, dEta12, clct1, clct2;
   reader->AddVariable( "Eta", &Eta );
   reader->AddVariable( "dPhi12", &dPhi12 );
   reader->AddVariable( "dEta12", &dEta12 );
   reader->AddVariable( "clct1", &clct1 );
   reader->AddVariable( "clct2", &clct2 );

   // Spectator variables declared in the training have to be added to the reader, too
   //reader->AddSpectator( "GenPt",  &GenPt );

   // --- Book the MVA methods

   TString dir    = "dataset/weights/";
   TString prefix = "TMVARegression";

   // Book method(s)
   TString methodName = "BDTG method";
   TString weightfile = dir + prefix + "_" + TString("BDTG") + ".weights.xml";
   reader->BookMVA( methodName, weightfile ); 
   
   // Prepare input tree (this must be replaced by your data source)
   // in this example, there is a toy tree with signal and one with background events
   // we'll later on use only the "signal" events for the test in this example.
   //   
   TFile *input(0);
   TString fname = "/afs/cern.ch/user/a/acarnes/public/iml/ptrootfiles/inc/Output_Trimmed_97p5_TEST_Mode3_100k.root";
   if (!gSystem->AccessPathName( fname )) 
   {
      input = TFile::Open( fname ); // check if file in local directory exists
   } 
   if (!input) 
   {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }
   std::cout << "--- TMVARegressionApp        : Using input file: " << input->GetName() << std::endl;

   // --- Event loop

   // Prepare the tree
   // - here the variable names have to corresponds to your tree
   // - you can use the same variables as above which is slightly faster,
   //   but of course you can use different ones and copy the values inside the event loop
   //
   TTree* theTree = (TTree*)input->Get("theNtuple");
   std::cout << "--- Select signal sample" << std::endl;
   theTree->SetBranchAddress( "Eta", &Eta );
   theTree->SetBranchAddress( "dPhi12", &dPhi12 );
   theTree->SetBranchAddress( "dEta12", &dEta12 );
   theTree->SetBranchAddress( "clct1", &clct1 );
   theTree->SetBranchAddress( "clct2", &clct2 );
   theTree->SetBranchAddress( "GenPt", &GenPt );

   TFile* outfile = new TFile("test_results_TMVA_ad_finalish.root", "RECREATE");
   outfile->cd();
   TString ntupleVars("GenPt:BDTPt:Eta:dPhi12:dEta12:clct1:clct2");
   TNtuple* n = new TNtuple("BDTresults", "BDTresults", ntupleVars);

   std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
   TStopwatch sw;
   sw.Start();
   for (Long64_t ievt=0; ievt<theTree->GetEntries();ievt++) 
   {

      //if (ievt%1000 == 0) std::cout << "--- ... Processing event: " << ievt << std::endl;
      
      theTree->GetEntry(ievt);

      // Retrieve the MVA target values (regression outputs) and fill into histograms
      // NOTE: EvaluateRegression(..) returns a vector for multi-target regression
      Float_t val = (reader->EvaluateRegression( "BDTG method" ))[0];
      std::vector<Float_t> y;
      y.push_back(TMath::Abs(GenPt));
      y.push_back(TMath::Abs(val));
      y.push_back(dPhi12);
      y.push_back(dEta12);
      y.push_back(clct1);
      y.push_back(clct2);
      n->Fill(&y[0]);
   }
   sw.Stop();
   std::cout << "--- End of event loop: "; sw.Print();

   outfile->cd();
   n->Write();
   outfile->Close();
   delete outfile;

   delete reader;
    
   std::cout << "==> TMVARegressionApplication is done!" << std::endl << std::endl;
}

int main( int argc, char** argv )
{
   // Select methods (don't look at this code - not of interest)
   TString methodList; 
   for (int i=1; i<argc; i++) {
      TString regMethod(argv[i]);
      if(regMethod=="-b" || regMethod=="--batch") continue;
      if (!methodList.IsNull()) methodList += TString(","); 
      methodList += regMethod;
   }
   TMVARegressionApplication(methodList);
   return 0;
}
