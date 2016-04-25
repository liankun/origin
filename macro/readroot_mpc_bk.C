TH1D* htwr[2][7][4];//arm,r,phi
const double PI = 3.1415926;

int readroot_mpc(){
  gSystem->Load("libmpc.so");
  MpcMap* mpcmap;
  mpcmap = MpcMap::instance();
  TFile* rfile = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728","READONLY");
  TH2D* htower_e = (TH2D*)rfile->Get("htower_e");


  for(int iarm = 0;iarm < 2;iarm++){
    for(int ir = 0;ir < 7;ir++){
      for(int iphi = 0;iphi < 4;iphi++){
        char name[100];
	sprintf(name,"htwr_arm%d_r%d_phi%d",iarm,ir,iphi);
	htwr[iarm][ir][iphi] = new TH1D(name,name,400,0,100);
      }
    }
  }


//  htower_e->Draw("colz");
  

//test TPad coordinate
/*  
  TCanvas* c = new TCanvas("test","test",800,800);
  TPad *pad1 = new TPad("pad1","This is pad1",0.05,0.52,0.95,0.97);
  pad1->SetFillColor(11);
  pad1->Draw();
*/

  for(int iarm = 0;iarm < 2;iarm++){
    char cname[100];
    sprintf(cname,"c_arm%d",iarm);
    TCanvas* c = new TCanvas(cname,cname,800,800);
    for(int i = 0;i < 288;i++ ){
      float tx = mpcmap->getX(i+288*iarm);
      float ty = mpcmap->getY(i+288*iarm);
      float r = sqrt(tx*tx+ty*ty);
      float phi = atan2(ty,tx);
      cout <<"ir "<<ir<<" iphi "<<iphi<<endl;
//      cout<<"r "<<r<<endl;
//      if(r < 15) continue;
      
//      cout <<tx<<" "<<ty<<endl;
      float tz = mpcmap->getZ(i+288*iarm);
      if(fabs(tz) < 2) continue;
      int iphi = (int)((phi+PI)/(PI/2.));
      int ir = (int)((r-8)/2.);
      htower_e->SetAxisRange(i+288*iarm,i+288*iarm);
      TH1D* htemp = htower_e->ProjectionY();
      double entries = htemp->GetEntries();
      if(entries < 10) continue;
      htwr[iarm][ir][iphi]->Add(htemp);
      char pname[100];
      sprintf(pname,"tower_%d_%d",iarm,i);
      htemp->SetName(pname);
      sprintf(pname,"tower_arm%d_%d",iarm,i);
      //coordinate looks like in ratio
      TPad* pad = new TPad(pname,pname,0.5+(tx-1.0)/48.,0.5+(ty-1.0)/48.,0.5+(tx+1.0)/48.,0.5+(ty+1.0)/48.);
//      cout <<0.5+(tx-1.0)/48.<<" "<<0.5+(ty-1.0)/48.
//          <<" "<<0.5+(tx+1.0)/48.<<" "<<0.5+(ty+1.0)/48.<<endl;
      pad->cd();
//      pad->SetLogy();
//      htemp->Draw("");
      c->cd();
      pad->SetFillColor(kYellow);
      pad->DrawClone("same");
      delete pad;
    }
  }
  TFile* ofile = new TFile("mpc_output.root","RECREATE");
  for(int iarm = 0;iarm < 2;iarm++){
    for(int iphi = 0;iphi < 4;iphi++){
      char coname[100];
      sprintf(coname,"c_arm%d_phi%d",iarm,iphi);
      TCanvas* c = new TCanvas(coname,coname,800,800);
      bool first_draw = true;
      for(int ir = 0;ir < 7;ir++){
        double entries = htwr[iarm][ir][iphi]->GetEntries();
	if(entries < 10) continue;
	htwr[iarm][ir][iphi]->SetLineColor(ir+1);
        if(first_draw){
	  htwr[iarm][ir][iphi]->DrawNormalized("");
	  first_draw = false;
	}
	else htwr[iarm][ir][iphi]->DrawNormalized("same");
      }
      ofile->cd();
      c->Write();
    }
  }
  return 0;
}
