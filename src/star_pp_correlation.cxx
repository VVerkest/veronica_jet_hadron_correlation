//  PERFORM JET-FINDING AND HADRON
//  CORRELATION FOR STAR DATA
//  Veronica Verkest     March 11, 2017

#include "corrFunctions.hh"  // functions for correlation

  int main ( ) {

    double hardPtCut = 2.0;                               // cut on minimum pt for initial hard clustering

    double subJetPtMin  = 10.0;                               // subleading jet minimum pt requirement

    double leadJetPtMin = 20.0;                               // leading jet minimum pt requirement

    
    std::string executable = "./bin/pp_correlation";
    std::string analysisType = "ppdijet";                               // choose dijet or jet ( decides value of requireDijets )
    double jetPtMax = 100.0;                               // maximum jet pt
    double jetRadius = 0.4;                               // jet radius for jet finding
    double hardPtCut = 2.0;                              // cut on minimum pt for initial hard clustering
    unsigned binsEta = 22;                               // default number of bins for eta for correlation histograms
    unsigned binsPhi = 22;                               // default number of bins for phi for correlation histograms
    std::string outputDir = "tmp/";                               // directory where everything will be saved
    std::string corrOutFile = "starppcorr.root";                               // histograms will be saved here
    std::string treeOutFile = "starppjet.root";                               // jets will be saved in a TTree here
    std::string inputFile = "ppHT/picoDst_*.root";                               // input file: can be .root, .txt, .list
    std::string chainName = "JetTree";                               // Tree name in input file

    
    TH1::SetDefaultSumw2( );         // Histograms will calculate gaussian errors
    TH2::SetDefaultSumw2( );
    TH3::SetDefaultSumw2( );

    jetHadron::BeginSummaryDijet ( jetRadius, leadJetPtMin, subJetPtMin, jetPtMax, hardPtCut, jetHadron::trackMinPt, jetHadron::binsVz, jetHadron::vzRange, treeOutFile, corrOutFile );

    
    return 0;
  }  // END
