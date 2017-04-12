void geant_jet(){

  double pi = 3.141592653589793238462643383;
  TString fileSaveName = "geant_jets_min_3_GeV";
  fileSaveName += ".root";
  TString canvasTitle = "Geant Jets (pt min: 3.0 GeV)";
  
  // create a new Root file
  TFile *top = new TFile(fileSaveName,"recreate");
  
  // create a new subdirectory for each plane
  const Int_t nPtBins = 11;
  double ptBinLo[nPtBins] = { 3, 4, 5, 7,  9,  11, 15, 25, 35, 45, 55 };
  double ptBinHi[nPtBins] = { 4, 5, 7, 9, 11, 15, 25, 35, 45, 55, 65 };
  // int nFiles = { 18, 18, 10, 11, 11, 11, 11, 11, 7, 4, 4 };
  TString ptBinString[nPtBins] = { "3.0-4.0", "4.0-5.0", "5.0-7.0","7.0-9.0", "9.0-11.0", "11.0-15.0", "15.0-25.0", "25.0-35.0", "35.0-45.0", "45.0-55.0", "55.0-65.0" };
  Int_t i;
  double l, h, events;
  TString importName, ptNameSet, eventsNameSet, hname;
  TH2D* leadJetPt[nPtBins];
  TH1D* ppjetEvents[nPtBins];
    
  for (i=0;i<nPtBins;i++) {
    importName = "out/geant_ppjet_lead_3_max_100__";
    l = ptBinLo[i];
    h = ptBinHi[i];
    importName += l;
    importName += "_";
    importName += h;
    hname = importName;
    hname+=".png";
    importName += ".root";

    ptNameSet = "Geant Jets:  ";
    ptNameSet += ptBinString[i];
    ptNameSet += " GeV";
    eventsNameSet = "Geant Events: ";
    eventsNameSet += ptBinString[i];
    eventsNameSet += " GeV";
      
    //  IMPORT
    TFile* ppjetFILE = new TFile( importName, "READ" );
    leadJetPt[i] = (TH2D*) ppjetFILE->Get("pptriggerjetpt");
    ppjetEvents[i] = (TH1D*) ppjetFILE->Get("binvzdist");
    
    leadJetPt[i]->SetTitle(ptNameSet);
    ppjetEvents[i]->SetTitle(eventsNameSet);

    events = double(ppjetEvents[i]->GetEntries());

    std::cout << " pt bin range:   " << ptBinString[i] << "              number of events:  " << events << std::endl;
    
    if ( events != 0 ) {    // DO NOT divide by zero!
      //leadJetPt[i]->Scale( 1/(6*pi) );  // divide by 2pi and divide by 3 (delta Eta)
      //leadJetPt[i]->Scale( 1/(leadJetPt[i]->GetXaxis()->GetBinWidth(1)) );
      gStyle->SetOptStat(1);

      // WRITE
      top->cd();
      leadJetPt[i]->Write();
      leadJetPt[i]->Draw();
      c1->SetLogy();
      c1->SaveAs(hname);
    }
  }

  TH2D *JetPt = (TH2D*)leadJetPt[0]->Clone("JetPt");

  for (int j=0; j<nPtBins; j++){
    JetPt->Add(leadJetPt[j]);
  }

  JetPt->SetTitle(canvasTitle);
  JetPt->Write();
  JetPt->Draw();
  hname = "geant_jets_min_3_GeV.png";
  c1->SaveAs(hname);  
  delete top;
}
