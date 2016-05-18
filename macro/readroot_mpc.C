TH1D* htwr[2][7][4];//arm,r,phi
const double PI = 3.1415926;

int readroot_mpc(){
  gSystem->Load("libmpc.so");
  MpcMap* mpcmap;
  mpcmap = MpcMap::instance();
  TFile* rfile = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728.root","READONLY");
  TH2D* htower_e = (TH2D*)rfile->Get("htower_e");


//  htower_e->Draw("colz");
  

//test TPad coordinate
/*  
  TCanvas* c = new TCanvas("test","test",800,800);
  TPad *pad1 = new TPad("pad1","This is pad1",0.05,0.52,0.95,0.97);
  pad1->SetFillColor(11);
  pad1->Draw();
*/
  TFile* ofile = new TFile("mpc_output.root","RECREATE");

  for(int iarm = 0;iarm < 2;iarm++){
    char cname[100];
    sprintf(cname,"c_arm%d",iarm);
    TCanvas* c = new TCanvas(cname,cname,800,800);
    for(int i = 0;i < 288;i++ ){
      float tx = mpcmap->getX(i+288*iarm);
      float ty = mpcmap->getY(i+288*iarm);
      
      float tz = mpcmap->getZ(i+288*iarm);
      if(fabs(tz) < 2) continue;
      htower_e->SetAxisRange(i,i,"x");
      TH1D* htemp = htower_e->ProjectionY();
      double entries = htemp->GetEntries();
//      if(entries < 10) continue;
      char pname[200];
      sprintf(pname,"tower_%d_%d",iarm,i);
      htemp->SetName(pname);
      sprintf(pname,"tower_arm%d_%d",iarm,i);
      //coordinate looks like in ratio
      TPad* pad = new TPad(pname,pname,0.5+(tx-1.0)/48.,0.5+(ty-1.0)/48.,0.5+(tx+1.0)/48.,0.5+(ty+1.0)/48.);
//      cout <<0.5+(tx-1.0)/48.<<" "<<0.5+(ty-1.0)/48.
//          <<" "<<0.5+(tx+1.0)/48.<<" "<<0.5+(ty+1.0)/48.<<endl;
      pad->cd();
      pad->SetLogy();
      htemp->Draw("");
      c->cd();
//      pad->SetFillColor(kYellow);
      pad->DrawClone("same");
      delete pad;
    }
    ofile->cd();
    c->Write();
  }
  
   return 0;
}
