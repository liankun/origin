void readroot_mpcex_compare(){
  TFile* file0 = new TFile("output_mpcex_layers_dead_hot.root","READONLY");
  TFile* file1 = new TFile("output_mpcex_layers_normal.root","READONLY");
  TFile* ofile = new TFile("mpcex_compare_low.root","RECREATE");
  for(int iarm = 0;iarm < 2;iarm++){
    char c_name[100];
    sprintf(c_name,"c_%d",iarm);
    TCanvas* c = new TCanvas(c_name,c_name,1300,800);
    c->Divide(4,2);
    for(int ilayer = 0;ilayer < 8;ilayer++){
      char hname[200];
      sprintf(hname,"hsensor_arm%d_layer%d_index%d_dead_hot_low",iarm,ilayer,0);
      TH1D* htemp0 = (TH1D*)file0->Get(hname);
      double entries = htemp0->GetEntries();
      if(entries < 10) continue;
      sprintf(hname,"hsensor_arm%d_layer%d_index%d_low",iarm,ilayer,0);
      TH1D* htemp1 = (TH1D*)file1->Get(hname);
      c->cd(ilayer+1);
      c->SetLogy();
      htemp0->SetLineColor(kRed);
      htemp0->DrawNormalized("");
      htemp1->DrawNormalized("same");
    }
    ofile->cd();
    c->Write();
  }
}
