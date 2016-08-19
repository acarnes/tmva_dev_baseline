// @(#)root/tmva $Id$
/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Root Macro: TMVARegression                                                     *
 *                                                                                *
 * This macro provides examples for the training and testing of the               *
 * TMVA classifiers.                                                              *
 *                                                                                *
 * As input data is used a toy-MC sample consisting of four Gaussian-distributed  *
 * and linearly correlated input variables.                                       *
 *                                                                                *
 * The methods to be used can be switched on and off by means of booleans, or     *
 * via the prompt command, for example:                                           *
 *                                                                                *
 *    root -l TMVARegression.C\(\"LD,MLP\"\)                                      *
 *                                                                                *
 * (note that the backslashes are mandatory)                                      *
 * If no method given, a default set is used.                                     *
 *                                                                                *
 * The output file "TMVAReg.root" can be analysed with the use of dedicated       *
 * macros (simply say: root -l <macro.C>), which can be conveniently              *
 * invoked through a GUI that will appear at the end of the run of this macro.    *
 **********************************************************************************/

#include <cstdlib>
#include <iostream> 
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVARegGui.h"


using namespace TMVA;
   
void TMVARegressionCalo( TString myMethodList = "" ) 
{
   // The explicit loading of the shared libTMVA is done in TMVAlogon.C, defined in .rootrc
   // if you use your private .rootrc, or run from a different directory, please copy the 
   // corresponding lines from .rootrc

   // methods to be processed can be given as an argument; use format:
   //
   // mylinux~> root -l TMVARegression.C\(\"myMethod1,myMethod2,myMethod3\"\)
   //

   //---------------------------------------------------------------
   // This loads the library
   TMVA::Tools::Instance();

   // --------------------------------------------------------------------------------------------------

   // --- Here the preparation phase begins

   // Create a new root output file
   TString outfileName( "TMVACaloReg.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory will
   // then run the performance analysis for you.
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/ 
   //
   // The second argument is the output file for the training results
   // All TMVA output can be suppressed by removing the "!" (not) in 
   // front of the "Silent" argument in the option string
   TMVA::Factory *factory = new TMVA::Factory( "TMVARegression", outputFile, 
                                               "!V:!Silent:Color:DrawProgressBar" );

   // If you wish to modify default settings 
   // (please check "src/Config.h" to see all available global options)
   //    (TMVA::gConfig().GetVariablePlotting()).fTimesRMS = 8.0;
   //    (TMVA::gConfig().GetIONames()).fWeightFileDir = "myWeightDirectory";

   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]
   factory->AddVariable( "e0",    "e0",    "", 'F' );
   factory->AddVariable( "e1", "e1", "", 'F' );
   factory->AddVariable( "e2", "e2", "", 'F' );
   factory->AddVariable( "e3",  "e3",  "", 'F' );
   factory->AddVariable( "e4",  "e4",  "", 'F' );
   factory->AddVariable( "e5",  "e5",  "", 'F' );
   factory->AddVariable( "e6",  "e6",  "", 'F' );
   factory->AddVariable( "e7",  "e7",  "", 'F' );
   factory->AddVariable( "e8",  "e8",  "", 'F' );
   factory->AddVariable( "e9",  "e9",  "", 'F' );
   factory->AddVariable( "e10",  "e10",  "", 'F' );
   factory->AddVariable( "e11",  "e11",  "", 'F' );
   factory->AddVariable( "e12",  "e12",  "", 'F' );
   factory->AddVariable( "eta",  "eta",  "", 'F' );
   factory->AddVariable( "phi",  "phi",  "", 'F' );
   factory->AddVariable( "eta0",  "eta0",  "", 'F' );
   factory->AddVariable( "phi0",  "phi0",  "", 'F' );
   factory->AddVariable( "esum",  "esum",  "", 'F' );

   // Add the variable carrying the regression target
   factory->AddTarget( "etruth" ); 

   // It is also possible to declare additional targets for multi-dimensional regression, ie:
   // -- factory->AddTarget( "fvalue2" );
   // BUT: this is currently ONLY implemented for MLP

   // Read training and test data (see TMVAClassification for reading ASCII files)
   // load the signal and background event samples from ROOT trees
   TFile *input(0);
   TString fname = "../ptrootfiles/testDataReg.root";

   if (!gSystem->AccessPathName( fname )) 
      input = TFile::Open( fname ); // check if file in local directory exists
   
   if (!input) 
   {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }

   std::cout << "--- TMVARegression           : Using input file: " << input->GetName() << std::endl;

   // --- Register the regression tree

   TTree *regTree = (TTree*)input->Get("TreeR");

   // global event weights per tree (see below for setting event-wise weights)
   Double_t regWeight  = 1.0;   

   // You can add an arbitrary number of regression trees
   factory->AddRegressionTree( regTree, regWeight );

   // Apply additional cuts on the signal and background samples (can be different)
   TCut mycut = ""; // for example: TCut mycut = "abs(var1)<0.5 && abs(var2-0.5)<1";

   // tell the factory to use all remaining events in the trees after training for testing:
   factory->PrepareTrainingAndTestTree( mycut, 
                                         "nTrain_Regression=25000:nTest_Regression=25000:SplitMode=Random:NormMode=NumEvents:!V" );
   // factory->PrepareTrainingAndTestTree( mycut, 
   //                                      "nTrain_Regression=0:nTest_Regression=0:SplitMode=Random:NormMode=NumEvents:!V" );

   // If no numbers of events are given, half of the events in the tree are used 
   // for training, and the other half for testing:
   //    factory->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );  

   // ---- Book MVA methods
   //
   // please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
   // it is possible to preset ranges in the option string in which the cut optimisation should be done:
   // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable

   factory->BookMethod( TMVA::Types::kBDT, "BDTG",
                           "!H:!V:NTrees=64::BoostType=Grad:Shrinkage=0.3:nCuts=99999:MaxDepth=4:MinNodeSize=0.001:NegWeightTreatment=IgnoreNegWeightsInTraining" );
   // --------------------------------------------------------------------------------------------------

   // ---- Now you can tell the factory to train, test, and evaluate the MVAs

   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();    

   // --------------------------------------------------------------
   
   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVARegression is done!" << std::endl;      


   // --- Book the MVA methods

   TString dir    = "weights/";
   TString prefix = "TMVARegression";

   delete factory;
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
   TMVARegressionCalo(methodList);
   return 0;
}
