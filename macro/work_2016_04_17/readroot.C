void readroot(){
  gSystem->Load("libMyMpcEx.so");
  TFile* rfile1 = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728_bk4.root","READONLY");
  if(!rfile1){
    cout <<"open AuAuAna_MinBias_NoCMN_Sub-450728.root failed !!!"<<endl;
    return;
  }
  Exogram* hgrammy_high1[2];
  hgrammy_high1[0] = (Exogram*)rfile1->Get("hgrammy_high0");
  hgrammy_high1[1] = (Exogram*)rfile1->Get("hgrammy_high1");  
  hgrammy_high1[0]->SetName("hgrammy_high10");
  hgrammy_high1[1]->SetName("hgrammy_high11");
  Exogram* hgrammy_low1[2];
  hgrammy_low1[0] = (Exogram*)rfile1->Get("hgrammy_low0");
  hgrammy_low1[1] = (Exogram*)rfile1->Get("hgrammy_low1");
  hgrammy_low1[0]->SetName("hgrammy_low10");
  hgrammy_low1[1]->SetName("hgrammy_low11");
 
  TFile* rfile2 = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728_bk2.root","READONLY");
  if(!rfile2){
    cout <<"open AuAuAna_MinBias_NoCMN_Sub-450728_bk2.root failed !!!"<<endl;
    return;
  }
  Exogram* hgrammy_high2[2];
  hgrammy_high2[0] = (Exogram*)rfile2->Get("hgrammy_high0");
  hgrammy_high2[1] = (Exogram*)rfile2->Get("hgrammy_high1");
  hgrammy_high2[0]->SetName("hgrammy_high20");
  hgrammy_high2[1]->SetName("hgrammy_high21");
 
  Exogram* hgrammy_low2[2];
  hgrammy_low2[0] = (Exogram*)rfile2->Get("hgrammy_low0");
  hgrammy_low2[1] = (Exogram*)rfile2->Get("hgrammy_low1");
  hgrammy_low2[0]->SetName("hgrammy_low20");
  hgrammy_low2[1]->SetName("hgrammy_low21");

  TFile* rfile3 = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728_bk3.root","READONLY");
  if(!rfile3){
    cout <<"open AuAuAna_MinBias_NoCMN_Sub-450728_bk3.root failed !!!"<<endl;
    return;
  }
  Exogram* hgrammy_high3[2];
  hgrammy_high3[0] = (Exogram*)rfile3->Get("hgrammy_high0");
  hgrammy_high3[1] = (Exogram*)rfile3->Get("hgrammy_high1");
  hgrammy_high3[0]->SetName("hgrammy_high30");
  hgrammy_high3[1]->SetName("hgrammy_high31");
 
  
  Exogram* hgrammy_low3[2];
  hgrammy_low3[0] = (Exogram*)rfile3->Get("hgrammy_low0");
  hgrammy_low3[1] = (Exogram*)rfile3->Get("hgrammy_low1");
  hgrammy_low3[0]->SetName("hgrammy_low30");
  hgrammy_low3[1]->SetName("hgrammy_low31");
  
  TFile* ofile = new TFile("output.root","RECREATE");
  for(int iarm=0;iarm < 2;iarm++){
    for(int i = 0;i < 8;i++){
      hgrammy_high1[iarm]->SetAxisRange(i,i,"z");
      hgrammy_low1[iarm]->SetAxisRange(i,i,"z");
      hgrammy_high2[iarm]->SetAxisRange(i,i,"z");
      hgrammy_low2[iarm]->SetAxisRange(i,i,"z");
      hgrammy_high3[iarm]->SetAxisRange(i,i,"z");
      hgrammy_low3[iarm]->SetAxisRange(i,i,"z");
    
      TH2D* htemp1_high = hgrammy_high1[iarm]->Project3D("yx");
      TH2D* htemp1_low = hgrammy_low1[iarm]->Project3D("yx");
      TH2D* htemp2_high = hgrammy_high2[iarm]->Project3D("yx");
      TH2D* htemp2_low = hgrammy_low2[iarm]->Project3D("yx");
      TH2D* htemp3_high = hgrammy_high3[iarm]->Project3D("yx");
      TH2D* htemp3_low = hgrammy_low3[iarm]->Project3D("yx");

      char c_name[100];
      sprintf(c_name,"c_layer_%d_%dhigh",iarm,i);
      TCanvas* c_high = new TCanvas(c_name,c_name,1600,800);
      c_high->Divide(3,1);
      c_high->cd(1);
      htemp1_high->Draw("colz");
      c_high->cd(2);
      htemp2_high->Draw("colz");
      c_high->cd(3);
      htemp3_high->Draw("colz");
      c_high->Write();

      sprintf(c_name,"c_layer_%d_%dlow",iarm,i);
      TCanvas* c_low = new TCanvas(c_name,c_name,1600,800);
      c_low->Divide(3,1);
      c_low->cd(1);
      htemp1_low->Draw("colz");
      c_low->cd(2);
      htemp2_low->Draw("colz");
      c_low->cd(3);
      htemp3_low->Draw("colz");
      c_low->Write();
    }
  }
}
