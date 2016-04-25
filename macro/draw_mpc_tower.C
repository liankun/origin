void draw_mpc_tower(){
  gSystem->Load("libmpc.so");
  MpcMap* mpcmap;
  mpcmap = MpcMap::instance();
  TH2D* hmpc_tower0 = new TH2D("hmpc_tower_arm0","hmpc_tower_arm0",900,-24,24,900,-24,24);
  TH2D* hmpc_tower1 = new TH2D("hmpc_tower_arm1","hmpc_tower_arm1",900,-24,24,900,-24,24);

  TCanvas* c0 = new TCanvas("c0","c0",800,800);
  hmpc_tower0->Draw("");

  TCanvas* c1 = new TCanvas("c1","c1",800,800);
  hmpc_tower1->Draw("");

  for(int i = 0;i < 288;i++ ){
    float tx = mpcmap->getX(i);
    float ty = mpcmap->getY(i);
    float tz = mpcmap->getZ(i);
    if(fabs(tz) < 2) continue;
    TPaveText *pt = new TPaveText(tx-1.0,ty-1.0,tx+1.,ty+1.);
    pt->SetFillColor(kYellow);
    char text[100];
    sprintf(text,"%d",i);
    pt->AddText(text);
    c0->cd();
    pt->DrawClone("same");
    delete pt;
  }

  for(int i = 0;i < 288;i++ ){
    float tx = mpcmap->getX(i+288);
    float ty = mpcmap->getY(i+288);
    float tz = mpcmap->getZ(i+288);
    if(fabs(tz) < 2) continue;
    TPaveText *pt = new TPaveText(tx-1.0,ty-1.0,tx+1.,ty+1.);
    pt->SetFillColor(kYellow);
    char text[100];
    sprintf(text,"%d",i);
    pt->AddText(text);
    c1->cd();
    pt->DrawClone("same");
    delete pt;
  }

}
