// HEADER FOR corrFunctions.cxx

// Veronica Verkest    March 11, 2017

#include "include.h"   // TStarPico, FastJet, ROOT, etc.


#ifndef CORRFUNCTIONS_HH
#define CORRFUNCTIONS_HH


namespace jetAnalysis {

  bool HasEnding (std::string const &full_string, std::string const &ending);  	// Helper to build the TChain, used to decide which input format

  std::string GetDirFromPath( std::string path );   // Used to pull the current directory from its absolute path

  std::string getPWD();	// Used to find the path to current working directory

  double CalcAj ( std::vector<fastjet::PseudoJet>& jets );// Checks there are the proper number of jets then calculates Aj
  
  int GetReferenceCentrality( int gRefMult );     // Used to get reference centrality from gRefMult

  int GetVzBin( double Vz );    // Find the vertex Z bin that corresponds to each Vz Returns -1 if Vz outside of accepted range
  
  void BeginSummaryDijet ( double jetRadius, double leadJetPtMin, double subLeadJetPtMin, double jetMaxPt, double hardJetConstPt, double softJetConstPt, int nVzBins, double VzRange, std::string dijetFile, std::string corrFile );      // Summary of initial settings for dijet-hadron correlation


}  // END jetAnalysis
#endif
