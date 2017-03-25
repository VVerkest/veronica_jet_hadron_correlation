//  PERFORM JET-FINDING ON STAR DATA
//  Veronica Verkest     March 22, 2017

#include "corrFunctions.hh"  // functions for correlation
#include "corrParameters.hh"
#include "include.h"

int main() {

  std::string analysisType  = "ppjet";
  bool requireDijets	= false;
  bool useEfficiency = false; // choose to use particle-by-particle efficiency
  bool requireTrigger= true; // require leading jet to be within jetRadius of a trigger tower
  double leadJetPtMin  = 20.0; // leading jet minimum pt requirement
  double jetPtMax = 100.0;  // maximum jet pt
  double jetRadius = 0.4; // jet radius for jet finding
  std::string outputDir = "tmp/";										// directory where everything will be saved
  std::string treeOutFile = "ppMonojet.root";						// jets will be saved in a TTree here
  std::string inputFile = "Data/ppHT/*.root";			// input file: can be .root, .txt, .list
  std::string chainName = "JetTree";            // Tree name in INPUT file

  // Announce Jet-Finding Settings
  corrAnalysis::BeginSummaryJet ( jetRadius, leadJetPtMin, jetPtMax, corrAnalysis::hardTrackMinPt, corrAnalysis::binsVz, corrAnalysis::vzRange, treeOutFile, corrOutFile );

  corrAnalysis::histograms* histograms = new corrAnalysis::histograms( analysisType );   // Construct histogram class
  histograms->Init();    // Initiate histograms
  
  TChain* chain = new TChain( chainName.c_str() );
  chain->Add( inputFile.c_str() );
  
  TStarJetPicoReader reader;
  corrAnalysis::InitReader( reader, chain, "pp", corrAnalysis::triggerAll, corrAnalysis::allEvents );

  // Data classes
  TStarJetVectorContainer<TStarJetVector>* container;
  TStarJetVector* sv; // TLorentzVector* would be sufficient
  TStarJetPicoEventHeader* header;
  TStarJetPicoEvent* event;
  TClonesArray* triggerObjs;
  TClonesArray* towers;

  // Build fastjet selectors, containers and definitions
  // ---------------------------------------------------
  
  // Particle container
  std::vector<fastjet::PseudoJet> particles;
  std::vector<fastjet::PseudoJet> ppParticles;
  // Trigger container - used to match
  // leading jet with trigger particle
  std::vector<fastjet::PseudoJet> triggers;
  
  // clustering definitions
  // First: used for the analysis - anti-kt with radius jetRadius
  fastjet::JetDefinition analysisDefinition = corrAnalysis::AnalysisJetDefinition( jetRadius );
  // Second: background estimation - kt with radius jetRadius
  fastjet::JetDefinition backgroundDefinition = corrAnalysis::BackgroundJetDefinition( jetRadius );
  
  // Build Selectors for the jet finding
  // -----------------------------------
  // Constituent selectors
  fastjet::Selector selectorLowPtCons = corrAnalysis::SelectLowPtConstituents( corrAnalysis::maxTrackRap, corrAnalysis::trackMinPt );
  fastjet::Selector selectorHighPtCons = corrAnalysis::SelectHighPtConstituents( corrAnalysis::maxTrackRap, corrAnalysis::hardTrackMinPt );
  
  // Jet candidate selector
  fastjet::Selector selectorJetCandidate;

  selectorJetCandidate = corrAnalysis::SelectJetCandidates( corrAnalysis::maxTrackRap, jetRadius, leadJetPtMin, jetPtMax );
  
  // Create the Area definition used for background estimation
  fastjet::GhostedAreaSpec areaSpec = corrAnalysis::GhostedArea( corrAnalysis::maxTrackRap, jetRadius );
  fastjet::AreaDefinition areaDef  = corrAnalysis::AreaDefinition( areaSpec );
  
  // selector used to reject hard jets in background estimation
  fastjet::Selector selectorBkgEstimator = corrAnalysis::SelectBkgEstimator( corrAnalysis::maxTrackRap, jetRadius );
  
  
  // When we do event mixing we need the jets, so save them
  // in trees
  // Tree to hold the jets
  TTree* correlatedJets;
  // To hold the TLorentzVectors for leading, subleading
  TLorentzVector leadingJet;
  // Records centrality and vertex information for event mixing
  Int_t centralityBin, vertexZBin;
  // Branchs to be written to file
  TBranch* CDJBranchHi, * CDJBranchLo;
  TBranch* CDJBranchCentralityBin;
  TBranch* CDJBranchVertexZBin;

  correlatedJets = new TTree("pp_jets","Correlated PP Jets" );
  CJBranchHi = correlatedJets->Branch("triggerJet", &leadingJet );
  CJBranchVertexZBin = correlatedJets->Branch("vertexZBin", &vertexZBin );

  // Finally, make ktEfficiency obj for pt-eta
  // Efficiency corrections
  ktTrackEff efficiencyCorrection( corrAnalysis::y7EfficiencyFile );
  
  // Now everything is set up
  // We can start the event loop
  // First, our counters
  int nEvents = 0;
  int nHardJets = 0;
  int nMatchedHard = 0;


  try{
    while ( reader.NextEvent() ) {
      
      nEvents++;      // Count the event
      
      reader.PrintStatus(10);      // Print out reader status every 10 seconds

      event = reader.GetEvent();           // Get the event header and event
      header = event->GetHeader();

      container = reader.GetOutputContainer();      // Get the output container from the reader
      triggerObjs = event->GetTrigObjs();      // and trigger objects

      int refCent = -1;      // We don't use reference centrality
      
      double vertexZ = header->GetPrimaryVertexZ();      // Find vertex Z bin
      int VzBin = corrAnalysis::GetVzBin( vertexZ );

      if ( VzBin == -1 )                 { continue; }      // Check to see if Vz is in the accepted range; if not, discard

      corrAnalysis::ConvertTStarJetVector( container, particles, true );      // Convert TStarJetVector to PseudoJet
      corrAnalysis::ConvertTStarJetVector( container, ppParticles, true );
      corrAnalysis::GetTriggersPP( requireTrigger, ppParticles, triggers );            // Get HT triggers ( using the pp version since the HT data cant be gotten)

      // If we require a trigger and we didnt find one, then discard the event
      if ( requireTrigger && triggers.size() == 0 ) 						{ continue; }

      // Start FastJet analysis
      
      // high: |eta| < maxTrackRap && pt > 2.0 GeV Used first to find hard jets
      std::vector<fastjet::PseudoJet> highPtCons = selectorHighPtCons( particles );
      
      // Find high constituent pT jets (NO background subtraction)
      // First cluster
      fastjet::ClusterSequence clusterSequenceHigh ( highPtCons, analysisDefinition );
      // Now first apply global jet selector to inclusive jets, then sort by pt
      std::vector<fastjet::PseudoJet> HiResult = fastjet::sorted_by_pt( selectorJetCandidate ( clusterSequenceHigh.inclusive_jets() ) );
      
      // Check to see if there are enough jets,
      // and if they meet the momentum cuts - if dijet, checks if they are back to back
      if ( !corrAnalysis::CheckHardCandidateJets( analysisType, HiResult, leadJetPtMin, subJetPtMin ) ) 	{ continue; }
      
      // count "dijets" ( monojet if doing jet analysis )
      nHardJets++;

      // make our hard dijet vector
      std::vector<fastjet::PseudoJet> hardJets = corrAnalysis::BuildHardJets( analysisType, HiResult );
      std::vector<fastjet::PseudoJet> LoResult;

      std::vector<fastjet::PseudoJet> analysisJets = corrAnalysis::BuildMatchedJets( analysisType, hardJets, LoResult, requireTrigger, triggers, jetRadius );

      // RETURN HARD JETS
      std::vector<fastjet::PseudoJet> analysisJets = corrAnalysis::BuildMatchedJets( analysisType, hardJets, LoResult, requireTrigger, triggers, jetRadius );

      // if zero jets were returned, exit out
      if ( analysisJets.size() == 0 )		{ continue; }
      nMatchedHard++;
      
      vertexZBin = VzBin;

      // FILL PT, ETA, AND PHI FOR MONOJET
      leadingJet.SetPtEtaPhiE( analysisJets.at(0).pt(), analysisJets.at(0).eta(), analysisJets.at(0).phi_std(), analysisJets.at(0).E() );
      
      correlatedDiJets->Fill();      // now write
      
      histograms->CountEvent( VzBin, weight );      // Now we can fill our event histograms
      histograms->FillVz( vertexZ, weight );
      histograms->FillJetPt( analysisJets.at(0).pt(), weight );
      histograms->FillJetEtaPhi( analysisJets.at(0).eta(), analysisJets.at(0).phi_std(), weight );
      
      corrAnalysis::EndSummaryJet ( nEvents, nHardJets, TimeKeeper.RealTime() );
  
      // write out the dijet/jet trees
      TFile*  treeOut   = new TFile( (outputDir + treeOutFile).c_str(), "RECREATE" );
      treeOut->cd();
      correlatedJets->Write();
      treeOut->Close();
  
      // write out the histograms
      TFile* histOut = new TFile( (outputDir + corrOutFile).c_str(), "RECREATE");
      histOut->cd();
      histograms->Write();
      histOut->Close();
      
    }
  }catch ( std::exception& e) {
    std::cerr << "Caught " << e.what() << std::endl;
    return -1;
  }
  
  return 0;
}
