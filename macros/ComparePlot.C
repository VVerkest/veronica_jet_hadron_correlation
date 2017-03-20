void ComparePlot(){
  const float pi = 3.141592;
  const Int_t nfiles = 4;
  const Int_t nPtBins = 5;
  Int_t g,h,i,j;

  // create a new Root file
  TFile *top = new TFile("GeantPythiaStar.root","recreate");

  double ptBinLo[nPtBins] = { 3, 5, 9, 13, 17 };
  double ptBinHi[nPtBins] = { 4, 8, 12, 16, 24 };
  TString ptBinString[nPtBins] = { "0.5-1.0", "1.0-2.0", "2.0-3.0", "3.0-4.0", "4.0-6.0" };
  TString PtBinName[nPtBins] = {"assoc05to10", "assoc10to20", "assoc20to30", "assoc30to40", "assoc40to60"};
  TString minSubPt[nfiles] = {"7",  "8",  "9", "10"};
  TString minLeadPt[nfiles] = {"14", "16", "18", "20"};
  TString LeadSub[nfiles] = {"lead14_sub7", "lead16_sub8", "lead18_sub9", "lead20_sub10", };
  TString CTitle[nPtBins] = {"0.5 < Pt_{assoc} < 1.0","1.0 < Pt_{assoc} < 2.0","2.0 < Pt_{assoc} < 3.0","3.0 < Pt_{assoc} < 4.0","4.0 < Pt_{assoc} < 6.0"};
  int color[nfiles] = { 8, 4, 51, 2 };
  TDirectory *ptbin[nPtBins];

  //  HISTOGRAMS
  TH1D *GeantCorr[nPtBins][nfiles];
  TH1D *PythiaCorr[nPtBins][nfiles];
  TH1D *StarCorr[nPtBins][nfiles];
  TH1D* GeantPtPlot[nPtBins][nfiles];
  TH1D* PythiaPtPlot[nPtBins][nfiles];
  TH1D* StarPtPlot[nPtBins][nfiles];
  TF1 *GeantFit[nfiles];
  TF1 *PythiaFit[nfiles];
  TF1 *StarFit[nfiles];

  
  h=0;
  for (h=0; h<nPtBins; h++) {
    TString directory = PtBinName[h];
    ptbin[h] = top->mkdir(directory);   // create a new subdirectory for each Pt bin range
    ptbin[h]->cd();
    
    for (j=0; j<nfiles; j++){
      


      
      
//________________________________________________________________________________________________
//                                           PYTHIA
//________________________________________________________________________________________________
      TString importName = "../out/pythia_";
      importName += LeadSub[j];
      importName += ".root";
      TFile* PythiaDijetFILE = new TFile( importName, "READ");   // Import Pythia Correlation File
      TH3D* PythiaLeadCorr = (TH3D*) PythiaDijetFILE->Get("ppleadjetcorr");
      TH3D* PythiaSubCorr = (TH3D*) PythiaDijetFILE->Get("ppsubjetcorr");
      TH3D* PythiaLeadPt = (TH3D*) PythiaDijetFILE->Get("ppleadjetpt");
      TH2D* PythiaEvents = (TH2D*) PythiaDijetFILE->Get("binvzdist");
      TH2D* PythiaLead = (TH2D*) PythiaLeadCorr->Project3D("ZY");   // Project correlation histogram
      TH2D* PythiaSub = (TH2D*) PythiaSubCorr->Project3D("ZY");
      
      TString nameSet = "PythiaLeadCorr";  // Name projected histograms
      PythiaLead->SetName( nameSet );
      nameSet = "PythiaSubCorr";
      PythiaSub->SetName( nameSet );
      nameSet = "PythiaLeadCorr_";
      nameSet += LeadSub[j];
      PythiaLead->SetName( nameSet );
      ptbin[h]->cd();
      PythiaCorr[h][j] = (TH1D*) PythiaLead->ProjectionX( nameSet , ptBinLo[h], ptBinHi[h] );
      PythiaCorr[h][j]->Scale(1/PythiaCorr[h][j]->GetBinWidth(1));
      PythiaCorr[h][j]->Scale( 1/double(PythiaEvents->Integral()) );

      TString fiteq = "[0]+[1]*exp(-0.5*((x-[2])/[3])**2)+[4]*exp(-0.5*((x-[5])/[6])**2)";
      double phiMin = -pi + pi/2.0;
      double phiMax = pi + pi/2.0;
      nameSet = "PythiaFit_";
      nameSet += LeadSub[j];
      PythiaFit[j] = new TF1( nameSet, fiteq, phiMin, phiMax);      // FIT PYTHIA
      PythiaFit[j]->FixParameter(2, 0);
      PythiaFit[j]->FixParameter(5, pi);
      PythiaFit[j]->SetParameter(3, 0.2);
      PythiaFit[j]->SetParameter(6, 0.2);
      PythiaFit[j]->SetLineColor(color[j]);
      PythiaFit[j]->SetLineWidth(2);
      PythiaCorr[h][j]->Fit(PythiaFit[j]);
      gROOT->SetEditHistograms();
      PythiaCorr[h][j]->SetLineColor(color[j]);
      PythiaCorr[h][j]->SetLineWidth(2);
      PythiaCorr[h][j]->SetMaximum(200);
      PythiaCorr[h][j]->SetMinimum(0);
      gStyle->SetOptStat(1);
      ptbin[h]->cd();
      PythiaCorr[h][j]->Write();   // WRITE PYTHIA CORRELATION PROJECTION
//________________________________________________________________________________________________
//////////////////////////////////////////////////////////////////////////////////////////////////


//________________________________________________________________________________________________
//                                           STAR
//________________________________________________________________________________________________
      importName = "../out/star_";
      importName += LeadSub[j];
      importName += ".root";
      TFile* StarDijetFILE = new TFile( importName, "READ");   // Import Star Correlation File
      TH3D* StarLeadCorr = (TH3D*) StarDijetFILE->Get("ppleadjetcorr");
      TH3D* StarSubCorr = (TH3D*) StarDijetFILE->Get("ppsubjetcorr");
      TH3D* StarLeadPt = (TH3D*) StarDijetFILE->Get("ppleadjetpt");
      TH2D* StarEvents = (TH2D*) StarDijetFILE->Get("binvzdist");
      TH2D* StarLead = (TH2D*) StarLeadCorr->Project3D("ZY");   // Project correlation histogram
      TH2D* StarSub = (TH2D*) StarSubCorr->Project3D("ZY");
      
      nameSet = "StarLeadCorr";  // Name projected histograms
      StarLead->SetName( nameSet );
      nameSet = "StarSubCorr";
      StarSub->SetName( nameSet );
      nameSet = "StarLeadCorr_";
      nameSet += LeadSub[j];
      StarLead->SetName( nameSet );
      ptbin[h]->cd();
      StarCorr[h][j] = (TH1D*) StarLead->ProjectionX( nameSet , ptBinLo[h], ptBinHi[h] );
      StarCorr[h][j]->Scale(1/StarCorr[h][j]->GetBinWidth(1));
      StarCorr[h][j]->Scale( 1/double(StarEvents->GetEntries()) );

      fiteq = "[0]+[1]*exp(-0.5*((x-[2])/[3])**2)+[4]*exp(-0.5*((x-[5])/[6])**2)";
      phiMin = -pi + pi/2.0;
      phiMax = pi + pi/2.0;
      nameSet = "StarFit_";
      nameSet += LeadSub[j];
      StarFit[j] = new TF1( nameSet, fiteq, phiMin, phiMax);      // FIT STAR
      StarFit[j]->FixParameter(2, 0);
      StarFit[j]->FixParameter(5, pi);
      StarFit[j]->SetParameter(3, 0.2);
      StarFit[j]->SetParameter(6, 0.2);
      StarFit[j]->SetLineColor(color[j]);
      StarFit[j]->SetLineWidth(2);
      StarCorr[h][j]->Fit(StarFit[j]);
      gROOT->SetEditHistograms();
      StarCorr[h][j]->SetLineColor(color[j]);
      StarCorr[h][j]->SetLineWidth(2);
      StarCorr[h][j]->SetMaximum(2.5);
      StarCorr[h][j]->SetMinimum(0);
      gStyle->SetOptStat(1);
      ptbin[h]->cd();
      StarCorr[h][j]->Write();   // WRITE STAR CORRELATION PROJECTION
//________________________________________________________________________________________________
//////////////////////////////////////////////////////////////////////////////////////////////////




      
      
//________________________________________________________________________________________________
//                                           GEANT
//________________________________________________________________________________________________
      importName = "../out/geant_";
      importName += LeadSub[j];
      importName += ".root";
      TFile* GeantDijetFILE = new TFile( importName, "READ");   // Import Geant Correlation File
      TH3D* GeantLeadCorr = (TH3D*) GeantDijetFILE->Get("ppleadjetcorr");
      TH3D* GeantSubCorr = (TH3D*) GeantDijetFILE->Get("ppsubjetcorr");
      TH3D* GeantLeadPt = (TH3D*) GeantDijetFILE->Get("ppleadjetpt");
      TH2D* GeantEvents = (TH2D*) GeantDijetFILE->Get("binvzdist");
      TH2D* GeantLead = (TH2D*) GeantLeadCorr->Project3D("ZY");   // Project correlation histogram
      TH2D* GeantSub = (TH2D*) GeantSubCorr->Project3D("ZY");
      
      nameSet = "GeantLeadCorr";  // Name projected histograms
      GeantLead->SetName( nameSet );
      nameSet = "GeantSubCorr";
      GeantSub->SetName( nameSet );
      nameSet = "GeantLeadCorr_";
      nameSet += LeadSub[j];
      GeantLead->SetName( nameSet );
      ptbin[h]->cd();
      GeantCorr[h][j] = (TH1D*) GeantLead->ProjectionX( nameSet , ptBinLo[h], ptBinHi[h] );
      GeantCorr[h][j]->Scale(1/GeantCorr[h][j]->GetBinWidth(1));
      GeantCorr[h][j]->Scale( 1/double(GeantEvents->Integral()) );

      fiteq = "[0]+[1]*exp(-0.5*((x-[2])/[3])**2)+[4]*exp(-0.5*((x-[5])/[6])**2)";
      phiMin = -pi + pi/2.0;
      phiMax = pi + pi/2.0;
      nameSet = "GeantFit_";
      nameSet += LeadSub[j];
      GeantFit[j] = new TF1( nameSet, fiteq, phiMin, phiMax);      // FIT GEANT
      GeantFit[j]->FixParameter(2, 0);
      GeantFit[j]->FixParameter(5, pi);
      GeantFit[j]->SetParameter(3, 0.2);
      GeantFit[j]->SetParameter(6, 0.2);
      GeantFit[j]->SetLineColor(color[j]);
      GeantFit[j]->SetLineWidth(2);
      GeantCorr[h][j]->Fit(GeantFit[j]);
      gROOT->SetEditHistograms();
      GeantCorr[h][j]->SetLineColor(color[j]);
      GeantCorr[h][j]->SetLineWidth(2);
      GeantCorr[h][j]->SetMaximum(200);
      GeantCorr[h][j]->SetMinimum(0);
      gStyle->SetOptStat(1);
      ptbin[h]->cd();
      GeantCorr[h][j]->Write();   // WRITE GEANT CORRELATION PROJECTION
//________________________________________________________________________________________________
//////////////////////////////////////////////////////////////////////////////////////////////////


      if (h==5) {
	top->cd();
	//  WRITE GEANT HISTOGRAMS
	GeantLead->Write();
	GeantSub->Write();
	GeantEvents->Write();
	GeantLeadPt->Write();
	//  WRITE PYTHIA HISTOGRAMS
	PythiaLead->Write();
	PythiaSub->Write();
	PythiaEvents->Write();
	PythiaLeadPt->Write();
	//  WRITE STAR HISTOGRAMS
	StarLead->Write();
	StarSub->Write();
	StarEvents->Write();
	StarLeadPt->Write();
      }
    }

  }
  
  

//////////////////////////////////////////////////////////////////////////////////////////////////
//                           DRAW,  SUPERIMPOSE,  AND  SAVE  CANVASES                           //
//////////////////////////////////////////////////////////////////////////////////////////////////
//  h denotes Pt Bin counter!
//  define i as nfiles counter


  for (h=0; h<nPtBins; h++){
    top->cd();
    ptbin[h]->cd();
    //  GEANT CANVASES
    i=0;
    TString canvasTitle = "Geant : ";
    canvasTitle += CTitle[h];
    TString canvasName = "Geant_";
    canvasName += PtBinName[h];   
    TCanvas *GeantCanvas = new TCanvas( canvasName , canvasTitle ,0 ,23 ,1280 ,709 );
    GeantCorr[h][i]->Draw();
    GeantCorr[h][i]->SetTitle( canvasTitle );
    i+=1;
    TString SaveName = "weighted_";
    SaveName += canvasName;
    SaveName += ".png";
    for (i=1; i<nfiles; i++){
      GeantCorr[h][i]->Draw("SAME");
    }
    //top->cd();
    GeantCanvas->Write();
    GeantCanvas->SaveAs(SaveName);
    
    //  PYTHIA CANVASES
    i=0;
    canvasTitle = "Pythia : ";
    canvasTitle += CTitle[h];
    canvasName = "Pythia_";
    canvasName += PtBinName[h];   
    TCanvas *PythiaCanvas = new TCanvas( canvasName , canvasTitle ,0 ,23 ,1280 ,709 );
    PythiaCorr[h][i]->Draw();
    PythiaCorr[h][i]->SetTitle( canvasTitle );
    i+=1;
    SaveName = "weighted_";
    SaveName += canvasName;
    SaveName += ".png";
    for (i=1; i<nfiles; i++){
      PythiaCorr[h][i]->Draw("SAME");
    }
    //top->cd();
    PythiaCanvas->Write();
    PythiaCanvas->SaveAs(SaveName);

    //  STAR CANVASES
    i=0;
    canvasTitle = "Star : ";
    canvasTitle += CTitle[h];
    canvasName = "Star_";
    canvasName += PtBinName[h];   
    TCanvas *StarCanvas = new TCanvas( canvasName , canvasTitle ,0 ,23 ,1280 ,709 );
    StarCorr[h][i]->Draw();
    StarCorr[h][i]->SetTitle( canvasTitle );
    i+=1;
    SaveName = canvasName;
    SaveName += ".png";
    for (i=1; i<nfiles; i++){
      StarCorr[h][i]->Draw("SAME");
    }
    //top->cd();
    StarCanvas->Write();
    StarCanvas->SaveAs(SaveName);

  }

    
  
  delete top;
}


     

 
