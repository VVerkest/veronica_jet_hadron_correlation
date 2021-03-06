//  PERFORM JET-FINDING AND HADRON
//  CORRELATION FOR GEANT DATA
//  Veronica Verkest     March 11, 2017

#include "corrFunctions.hh"  // functions for correlation
#include "corrParameters.hh"
#include "include.h"
// -------------------------
// Command line arguments: ( Defaults
// Defined for debugging in main )
// [0]: Choose between dijet-hadron correlations or jet-hadron correlations
//      choices: dijet || jet
// [1]:	Choose to use particle efficiency corrections or not: true/false
// [2]: trigger coincidence: require a trigger with the leading jet
// [3]: subleading jet pt min ( not used if doing jet-hadron correlations )
// [4]: leading jet pt min
// [5]: jet pt max
// [6]: jet radius, used in the jet definition
// [7]: output directory
// [8]: name for the correlation histogram file
// [9]: name for the dijet TTree file
// [10]: input data: can be a single .root or a .txt or .list of root files
// [11]: MB AuAu event file for embedding, can be .root, .txt, .list

// DEF MAIN()
int main ( int argc, const char** argv) {
  
  // First check to make sure we're located properly
  std::string currentDirectory = corrAnalysis::getPWD( );
  
  // If we arent in the analysis directory, exit
  if ( !(corrAnalysis::HasEnding ( currentDirectory, "jet_hadron_corr" ) || corrAnalysis::HasEnding ( currentDirectory, "jet_hadron_correlation" )) ) {
    std::cerr << "Error: Need to be in jet_hadron_corr directory" << std::endl;
    return -1;
  }
  
  //Start a timer
  TStopwatch TimeKeeper;
  TimeKeeper.Start( );
  
  // Histograms will calculate gaussian errors
  // -----------------------------------------
  TH1::SetDefaultSumw2( );
  TH2::SetDefaultSumw2( );
  TH3::SetDefaultSumw2( );
  
  // Read in command line arguments
  // ------------------------------
  // Geant Defaults
  std::string 	executable    = "./bin/geant_pp_correlation"; 	// placeholder
  std::string 	analysisType  = "ppdijet"; // choose dijet or jet ( decides value of requireDijets )
  bool requireDijets	= true; // this switches between dijet-hadron and jet-hadron
  bool useEfficiency = false; // choose to use particle-by-particle efficiency
  bool requireTrigger= false; // require leading jet to be within jetRadius of a trigger tower
  double subJetPtMin   = 10.0; // subleading jet minimum pt requirement
  double leadJetPtMin  = 20.0; // leading jet mus of a trigger tower
  double jetPtMax = 100.0;  // maximum jet pt
  double jetRadius = 0.4; // jet radius for jet finding
  std::string outputDir = "tmp/";				// directory where everything will be saved
  std::string corrOutFile =  "geantcorr.root";					// histograms will be saved here
  std::string treeOutFile = "geantdijet.root";					// jets will be saved in a TTree here
  std::string inputFile = "Data/AddedGeantPythia/picoDst*.root";			// input file: can be .root, .txt, .list
  std::string mbInputFile = "test/mbAuAu.root";				// min bias background event - .root, .txt, .list
  std::string chainName = "JetTree";	// Tree name in INPUT file
	
	
	
  // Now check to see if we were given modifying arguments
  switch ( argc ) {
    case 1: // Default case
      __OUT( "Using Default Settings" )
      break;
    case 13: { // Custom case
      __OUT( "Using Custom Settings" )
      std::vector<std::string> arguments( argv+1, argv+argc );
      
      // Set non-default values
      // ----------------------
      
      // Choose the analysis type
      if ( arguments[0] == "ppdijet" ) {
        analysisType 	= "ppdijet";
        requireDijets = true;
      }
      else if ( arguments[0] == "ppjet" ) {
        analysisType 	= "ppjet";
        requireDijets = false;
      }
      else {
        __ERR( "Unknown analysis type: Either ppdijet or ppjet " )
        return -1;
      }
      
      // Choose if we use particle-by-particle efficiency
      if ( arguments[1] == "true" ) 			{ useEfficiency = true; }
      else if ( arguments[1] == "false" ) 	{ useEfficiency = false; }
      else { __ERR( "useEfficiency must be true or false" ) return -1; }
      
      // Choose if we require a trigger tower to be within
      // jetRadius of the leading jet
      if ( arguments[2] == "true" ) 				{ requireTrigger = true; }
      else if ( arguments[2] == "false" ) 	{ requireTrigger = false; }
      else { __ERR( "useEfficiency must be true or false" ) return -1; }
      
      // jet kinematics
      subJetPtMin 	= atof ( arguments[3].c_str() );
      leadJetPtMin 	= atof ( arguments[4].c_str() );
      jetPtMax 			= atof ( arguments[5].c_str() );
      jetRadius 		= atof ( arguments[6].c_str() );
      
      // output and file names
      outputDir 		= arguments[7];
      corrOutFile		= arguments[8];
      treeOutFile		= arguments[9];
      inputFile 		= arguments[10];
      mbInputFile		= arguments[11];
      
      break;
    }
    default: { // Error: invalid custom settings
      __ERR( "Invalid number of command line arguments" )
      return -1;
      break;
    }
  }
  
  
  // Announce our settings
  if ( requireDijets ) { corrAnalysis::BeginSummaryDijet ( jetRadius, leadJetPtMin, subJetPtMin, jetPtMax, corrAnalysis::hardTrackMinPt, corrAnalysis::trackMinPt, corrAnalysis::binsVz, corrAnalysis::vzRange, treeOutFile, corrOutFile ); }
  else { corrAnalysis::BeginSummaryJet ( jetRadius, leadJetPtMin, jetPtMax, corrAnalysis::hardTrackMinPt, corrAnalysis::binsVz, corrAnalysis::vzRange, treeOutFile, corrOutFile ); }
  
  // We know what analysis we are doing now, so build our output histograms
  corrAnalysis::histograms* histograms = new corrAnalysis::histograms( analysisType );
  histograms->Init();
  
  // Build our input now
  // First for PP
  // --------------------
  TChain* chain = new TChain( chainName.c_str() );
  
  // Check to see if the input is a .root file or a .txt
  bool inputIsRoot = corrAnalysis::HasEnding( inputFile.c_str(), ".root" );
  bool inputIsTxt  = corrAnalysis::HasEnding( inputFile.c_str(), ".txt"  );
  bool inputIsList = corrAnalysis::HasEnding( inputFile.c_str(), ".list" );
  
  // If its a recognized file type, build the chain
  // If its not recognized, exit
  if ( inputIsRoot )      { chain->Add( inputFile.c_str() ); }
  else if ( inputIsTxt )  { chain = TStarJetPicoUtils::BuildChainFromFileList( inputFile.c_str() ); }
  else if ( inputIsList)  { chain = TStarJetPicoUtils::BuildChainFromFileList( inputFile.c_str() ); }
  else { __ERR("data file is not recognized type: .root or .txt only.") return -1; }
  
  // Intialize the reader and set the chain
  // All analysis parameters are located in
  // corrParameters.hh
  // --------------------------------------
  TStarJetPicoReader reader;
  corrAnalysis::InitReader( reader, chain, "pp", corrAnalysis::triggerAll, corrAnalysis::allEvents );
  
  // Data classes
  TStarJetVectorContainer<TStarJetVector>* container;
  TStarJetVector* sv; // TLorentzVector* would be sufficient
  TStarJetPicoEventHeader* header;
  TStarJetPicoEvent* event;
  TClonesArray* triggerObjs;
  TClonesArray* towers;
  
  // Now do the same for MB data
  // ---------------------------
  TChain* mbChain = new TChain( chainName.c_str() );
  
  // Check to see if the input is a .root file or a .txt
  bool mbInputIsRoot = corrAnalysis::HasEnding( mbInputFile.c_str(), ".root" );
  bool mbInputIsTxt  = corrAnalysis::HasEnding( mbInputFile.c_str(), ".txt"  );
  bool mbInputIsList = corrAnalysis::HasEnding( mbInputFile.c_str(), ".list" );
  
  // // If its a recognized file type, build the chain
  // // If its not recognized, exit
  // if ( mbInputIsRoot )		 	{ mbChain->Add( mbInputFile.c_str() ); }
  // else if ( mbInputIsTxt )  { mbChain = TStarJetPicoUtils::BuildChainFromFileList( mbInputFile.c_str() ); }
  // else if ( mbInputIsList)  { mbChain = TStarJetPicoUtils::BuildChainFromFileList( mbInputFile.c_str() ); }
  // else { __ERR("data file is not recognized type: .root or .txt only.") return -1; }
  
  // // Intialize the reader and set the chain
  // // All analysis parameters are located in
  // // corrParameters.hh
  // // --------------------------------------
  // TStarJetPicoReader mbReader;
  // corrAnalysis::InitReader( mbReader, mbChain, "auau", corrAnalysis::triggerAll, corrAnalysis::allEvents );
  
  // Data classes
  TStarJetVectorContainer<TStarJetVector>* mbContainer;
  
  TStarJetPicoEventHeader* mbHeader;
  TStarJetPicoEvent* mbEvent;
  TClonesArray* mbTriggerObjs;
  
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
  fastjet::JetDefinition 	analysisDefinition = corrAnalysis::AnalysisJetDefinition( jetRadius );
  // Second: background estimation - kt with radius jetRadius
  fastjet::JetDefinition	backgroundDefinition = corrAnalysis::BackgroundJetDefinition( jetRadius );
  
  // Build Selectors for the jet finding
  // -----------------------------------
  // Constituent selectors
  fastjet::Selector selectorLowPtCons  = corrAnalysis::SelectLowPtConstituents( corrAnalysis::maxTrackRap, corrAnalysis::trackMinPt );
  fastjet::Selector selectorHighPtCons = corrAnalysis::SelectHighPtConstituents( corrAnalysis::maxTrackRap, corrAnalysis::hardTrackMinPt );
  
  // Jet candidate selector
  fastjet::Selector	selectorJetCandidate;
  if ( requireDijets )
    selectorJetCandidate = corrAnalysis::SelectJetCandidates( corrAnalysis::maxTrackRap, jetRadius, subJetPtMin, jetPtMax );
  else
    selectorJetCandidate = corrAnalysis::SelectJetCandidates( corrAnalysis::maxTrackRap, jetRadius, leadJetPtMin, jetPtMax );
  
  // Create the Area definition used for background estimation
  fastjet::GhostedAreaSpec	areaSpec = corrAnalysis::GhostedArea( corrAnalysis::maxTrackRap, jetRadius );
  fastjet::AreaDefinition 	areaDef  = corrAnalysis::AreaDefinition( areaSpec );
  
  // selector used to reject hard jets in background estimation
  fastjet::Selector	selectorBkgEstimator	= corrAnalysis::SelectBkgEstimator( corrAnalysis::maxTrackRap, jetRadius );
  
  
  // When we do event mixing we need the jets, so save them
  // in trees
  // Tree to hold the di-jets
  TTree* correlatedDiJets;
  // To hold the TLorentzVectors for leading, subleading
  TLorentzVector leadingJet, subleadingJet;
  // Records centrality and vertex information for event mixing
  Int_t centralityBin, vertexZBin;
  // Branchs to be written to file
  TBranch* CDJBranchHi, * CDJBranchLo;
  TBranch* CDJBranchCentralityBin;
  TBranch* CDJBranchVertexZBin;
  
  if ( requireDijets ) {
    correlatedDiJets = new TTree("pp_dijets","Correlated PP Dijets" );
    CDJBranchHi = correlatedDiJets->Branch("leadJet", &leadingJet );
    CDJBranchLo = correlatedDiJets->Branch("subLeadJet", &subleadingJet );
    CDJBranchVertexZBin = correlatedDiJets->Branch("vertexZBin", &vertexZBin );
  }
  else {
    correlatedDiJets = new TTree("pp_jets","Correlated PP Jets" );
    CDJBranchHi = correlatedDiJets->Branch("triggerJet", &leadingJet );
    CDJBranchVertexZBin = correlatedDiJets->Branch("vertexZBin", &vertexZBin );
  }
  
  // Finally, make ktEfficiency obj for pt-eta
  // Efficiency corrections
  ktTrackEff efficiencyCorrection( corrAnalysis::y7EfficiencyFile );
  
  // Now everything is set up
  // We can start the event loop
  // First, our counters
  int nEvents = 0;
  int nHardDijets = 0;
  int nMatchedHard = 0;
  
  try{
    while ( reader.NextEvent() ) {
      
      // Count the event
      nEvents++;
      
      // Print out reader status every 10 seconds
      reader.PrintStatus(10);
      
      // // loop the MB events
      // // If the reader runs out, start again
      // if ( !mbReader.NextEvent() ) {
      //   mbReader.ReadEvent(0);
      //   std::cout<<"RESET MB events"<<std::endl;
      // }
      
      
      // Get the event header and event
      event = reader.GetEvent();
      header = event->GetHeader();
      
      // // and MB header/event
      // mbEvent = mbReader.GetEvent();
      // mbHeader = mbEvent->GetHeader();
      
      // Get the output container from the reader
      container = reader.GetOutputContainer();
      // and trigger objects
      triggerObjs = event->GetTrigObjs();
      
      // // and MB container
      // mbContainer = mbReader.GetOutputContainer();
      
      // We don't use reference centrality
      // so set a dummy
      int refCent = -1;
      
      // Find vertex Z bin
      double vertexZ = header->GetPrimaryVertexZ();
      int VzBin = corrAnalysis::GetVzBin( vertexZ );

// ***** WEIGHTING FOR GEANT!! *****
      double  weight=1;										// default histogram weight
      TString geantFile =  reader.GetInputChain()->GetCurrentFile()->GetName();
      if ( geantFile.Contains("Geant") ){					// determine if data is Geant data
		  weight= corrAnalysis::LookupXsec( geantFile );			// weight histograms by xsection
//		  std::cout<<weight<<std::endl;
//		  std::cout<<geantFile<<std::endl;
      }
  
      
      // Check to see if Vz is in the accepted range; if not, discard
      if ( VzBin == -1 )																				{ continue; }
      
      // Convert TStarJetVector to PseudoJet
      corrAnalysis::ConvertTStarJetVector( container, particles, true );
      corrAnalysis::ConvertTStarJetVector( container, ppParticles, true );
      // and MB data to the full event that will be used for jet finding
      //      corrAnalysis::ConvertTStarJetVector( mbContainer, particles, false);         // uncomment for AuAu embedding
      
      // Get HT triggers ( using the pp version since the HT data cant be gotten)
      //corrAnalysis::GetTriggers( requireTrigger, triggerObjs, triggers );
      corrAnalysis::GetTriggersPP( requireTrigger, ppParticles, triggers );
      
      // If we require a trigger and we didnt find one, then discard the event
      if ( requireTrigger && triggers.size() == 0 ) 						{ continue; }
      
      // Start FastJet analysis
      // ----------------------
      
      // get our two sets of particles:
      // low: |eta| < maxTrackRap && pt > 0.2 GeV Used second when we've found viable hard dijets
      // high: |eta| < maxTrackRap && pt > 2.0 GeV Used first to find hard jets
      std::vector<fastjet::PseudoJet> lowPtCons = selectorLowPtCons( particles );
      std::vector<fastjet::PseudoJet> highPtCons = selectorHighPtCons( particles );
      
      // Find high constituent pT jets
      // NO background subtraction
      // -----------------------------
      // First cluster
      fastjet::ClusterSequence clusterSequenceHigh ( highPtCons, analysisDefinition );
      // Now first apply global jet selector to inclusive jets, then sort by pt
      std::vector<fastjet::PseudoJet> HiResult = fastjet::sorted_by_pt( selectorJetCandidate ( clusterSequenceHigh.inclusive_jets() ) );
      
      // Check to see if there are enough jets,
      // and if they meet the momentum cuts - if dijet, checks if they are back to back
      if ( !corrAnalysis::CheckHardCandidateJets( analysisType, HiResult, leadJetPtMin, subJetPtMin ) ) 	{ continue; }
      
      // count "dijets" ( monojet if doing jet analysis )
      nHardDijets++;
      
      // make our hard dijet vector
      std::vector<fastjet::PseudoJet> hardJets = corrAnalysis::BuildHardJets( analysisType, HiResult );
      
      // now recluster with all particles if necessary ( only used for dijet analysis )
      // Find corresponding jets with soft constituents
      // ----------------------------------------------
      std::vector<fastjet::PseudoJet> LoResult;
      if ( requireDijets ) {
        fastjet::ClusterSequenceArea ClusterSequenceLow ( lowPtCons, analysisDefinition, areaDef ); // WITH background subtraction
        
        // Background initialization
        // -------------------------
        
        // Energy density estimate from median ( pt_i / area_i )
        fastjet::JetMedianBackgroundEstimator bkgdEstimator ( selectorBkgEstimator, backgroundDefinition, areaDef );
        bkgdEstimator.set_particles( lowPtCons );
        // Subtract A*rho from the original pT
        fastjet::Subtractor bkgdSubtractor ( &bkgdEstimator );
        LoResult = fastjet::sorted_by_pt( bkgdSubtractor( ClusterSequenceLow.inclusive_jets() ) );
      }
      
      // Get the jets used for correlations
      // Returns hardJets if doing jet analysis
      // it will match to triggers if necessary - if so, trigger jet is at index 0
      std::vector<fastjet::PseudoJet> analysisJets = corrAnalysis::BuildMatchedJets( analysisType, hardJets, LoResult, requireTrigger, triggers, jetRadius );
      
      // if zero jets were returned, exit out
      if ( analysisJets.size() == 0 )		{ continue; }
      nMatchedHard++;
      
      // now we have analysis jets, write the trees
      // for future event mixing
      vertexZBin = VzBin;
      if ( requireDijets ) {
        // leading jet
        leadingJet.SetPtEtaPhiE( analysisJets.at(0).pt(), analysisJets.at(0).eta(), analysisJets.at(0).phi_std(), analysisJets.at(0).E() );
        subleadingJet.SetPtEtaPhiE( analysisJets.at(1).pt(), analysisJets.at(1).eta(), analysisJets.at(1).phi_std(), analysisJets.at(1).E() );
        
      }
      else {
        leadingJet.SetPtEtaPhiE( analysisJets.at(0).pt(), analysisJets.at(0).eta(), analysisJets.at(0).phi_std(), analysisJets.at(0).E() );
      }


      
      // now write
      correlatedDiJets->Fill();



      
      // Now we can fill our event histograms
      histograms->CountEvent( VzBin, weight );
      histograms->FillVz( vertexZ, weight );
      if ( requireDijets ) {
        histograms->FillAjHigh( corrAnalysis::CalcAj( hardJets ), weight );
        histograms->FillAjLow( corrAnalysis::CalcAj( analysisJets ), weight );
        histograms->FillLeadJetPt( analysisJets.at(0).pt(), weight );
        histograms->FillLeadEtaPhi( analysisJets.at(0).eta(), analysisJets.at(0).phi_std(), weight );
        histograms->FillSubJetPt( analysisJets.at(1).pt(), weight );
        histograms->FillSubEtaPhi( analysisJets.at(1).eta(), analysisJets.at(1).phi_std(), weight );
      }
      else {
        histograms->FillJetPt( analysisJets.at(0).pt(), weight );
        histograms->FillJetEtaPhi( analysisJets.at(0).eta(), analysisJets.at(0).phi_std(), weight );
      }
      
      // Now we can perform the correlations
      // Only on the pp particles
      for ( int i = 0; i < ppParticles.size(); ++i ) {
        fastjet::PseudoJet assocParticle = ppParticles.at(i);
        
        // if we're using particle - by - particle efficiencies, get it,
        // else, set to one
        double assocEfficiency = 1.0;
        if ( useEfficiency ) assocEfficiency = (efficiencyCorrection.EffPPY06( assocParticle.eta(), assocParticle.pt() ) * weight );
		  
        // now correlate it with jets
        if ( requireDijets ) {
          corrAnalysis::correlateLeading( analysisType, VzBin, refCent, histograms, analysisJets.at(0), assocParticle, assocEfficiency );
          corrAnalysis::correlateSubleading( analysisType, VzBin, refCent, histograms, analysisJets.at(1), assocParticle, assocEfficiency );
        }
        else {
          corrAnalysis::correlateTrigger( analysisType, VzBin, refCent, histograms, analysisJets.at(0), assocParticle, assocEfficiency );
        }
      }
      
      
    }
  }catch ( std::exception& e) {
    std::cerr << "Caught " << e.what() << std::endl;
    return -1;
  }
  
  if ( requireDijets )
    corrAnalysis::EndSummaryDijet ( nEvents, nHardDijets, nMatchedHard, TimeKeeper.RealTime() );
  else
    corrAnalysis::EndSummaryJet ( nEvents, nHardDijets, TimeKeeper.RealTime() );
  
  // write out the dijet/jet trees
  TFile*  treeOut   = new TFile( (outputDir + treeOutFile).c_str(), "RECREATE" );
  treeOut->cd();
  correlatedDiJets->Write();
  treeOut->Close();
  
  // write out the histograms
  TFile* histOut = new TFile( (outputDir + corrOutFile).c_str(), "RECREATE");
  histOut->cd();
  histograms->Write();
  histOut->Close();
  
  
  
  return 0;
}
