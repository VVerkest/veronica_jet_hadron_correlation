void pythia_monojet(){
  const float pi = 3.141592;

  TString exportName = "pythia_monojet";
  TString fileSaveName = exportName;
  fileSaveName += ".root";
  TString canvasTitle = "Pythia Monojet : ";
  
  // create a new Root file
  TFile *top = new TFile(fileSaveName,"recreate");
  
  // create a new subdirectory for each plane
  const Int_t nPtBins = 11;
  double ptBinLo[nPtBins] = { 3, 4, 5, 7,  9,  11, 15, 25, 35, 45, 55 };
  double ptBinHi[nPtBins] = { 4, 5, 7, 9, 11, 15, 25, 35, 45, 55, 65 };
  TString ptBinString[nPtBins] = { "3.0-4.0", "4.0-5.0", "5.0-7.0","7.0-9.0", "9.0-11.0", "11.0-15.0", "15.0-25.0", "25.0-35.0", "35.0-45.0", "45.0-55.0", "55.0-65.0" };
  Int_t i;
  double l, h;
  TString importName, nameSet;
    
  for (i=0;i<nPtBins;i++) {
    importName = "pythia_ppjet_lead_20_max_100__";
    l = ptBinLo[i];
    h = ptBinHi[i];
    importName += l;
    importName += "_";
    importName += h;

    nameSet = importName;
    
    importName += ".root";

    //  IMPORT
    TFile* ppjetFILE = new TFile( importName, "READ" );
    TH1D* leadJetPt = (TH1D*) ppjetFILE->Get("ppleadjetpt");
    TH1D* ppjetEvents = (TH1D*) ppjetFILE->Get("binvzdist");
    TString nameSet = "pp_jetPt_"
    
    leadJetPt->SetName(nameSet);
    leadJetPt->Scale( 1/double(ppjetEvents->Integral()) );
       
    gStyle->SetOptStat(0);
     
    // WRITE
    top->cd();
    leadJetPt->Write();
  }

  delete top;
}
