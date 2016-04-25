
 static const unsigned short NARMS = 2;
 static const unsigned short NPACKETS_PER_ARM = 8;
 static const unsigned short NROCBONDS_PER_CHIP = 64;
 static const unsigned short NCHIPS_PER_CHAIN = 12;
 static const unsigned short NCHAINS_PER_PACKET = 4;
 static const unsigned int NMINIPADS_PER_PACKET = NROCBONDS_PER_CHIP*NCHIPS_PER_CHAIN*NCHAINS_PER_PACKET;

//find the 


TH2F* hmpcex_nxy[2][8];
void MpcExNxNyCheck(){
  gSystem->Load("libMyMpcEx.so");
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  Exogram* grammy[2]; 
  for(unsigned int arm = 0;arm < 2;arm++){
    char name[100];
    sprintf(name,"arm%d",arm);
    grammy[arm] = new Exogram(name,name,1800,-24,24,1800,-24,24,8,-0.5,7.5);
    for(unsigned int layer = 0;layer < 8;layer++){
      sprintf(name,"hmpcex_nxy_layer%d_arm%d",layer,arm);
      int Nx = 198*2;
      float ScaleX = 1;
      int Ny = 24*2;
      float ScaleY = 8;
      if(layer%2 == 1){
        Nx = 24*2;
	ScaleX = 8;
	Ny = 198*2;
	ScaleY = 1;
      }
      hmpcex_nxy[arm][layer] = new TH2F(name,name,Nx,-0.5*ScaleX,(Nx-0.5)*ScaleX,Ny,-0.5*ScaleY,(Ny-0.5)*ScaleY);
    }
  }


  for(unsigned int arm = 0;arm < NARMS;arm++){
    for(unsigned int packet =0;packet < NPACKETS_PER_ARM;packet++){
      for(unsigned int chipmap = 0;chipmap < NMINIPADS_PER_PACKET;chipmap++){
	unsigned int key = mpcex_mapper->generate_key(arm,packet,chipmap);
	unsigned int layer = mpcex_mapper->get_layer(key);
	unsigned int nx = mpcex_mapper->get_nx(key);
	unsigned int ny = mpcex_mapper->get_ny(key);
	unsigned int lx = mpcex_mapper->get_lx(key);
	unsigned int ly = mpcex_mapper->get_ly(key);
	unsigned int sensor = mpcex_mapper->get_sensor_in_quadrant(key);
//	if(arm == 1){
//          if(layer%2 == 0) ny = 23 - ny;
//	  else ny = 197 - ny;
//	}
	unsigned int weight = nx + 198*ny;
//	unsigned weight = ny;
	unsigned int weight2 = lx + 32*ly;
	if(layer%2 == 1) {
	  weight = ny + 198*nx;
//	  weight2 = ly + 32*lx;
//	  weight = nx;
	}

        //check sensor position
	weight2 = sensor+1;

	grammy[arm]->FillEx(key,weight2);
	hmpcex_nxy[arm][layer]->SetBinContent(2*nx+1+1,2*ny+1+1,weight2);
      }
    }
  }

  TCanvas* c0 = new TCanvas("c0","c0",1300,800);
  c0->Divide(4,2);
  TCanvas* c1 = new TCanvas("c1","c1",1500,800);
  c1->Divide(4,2);

  TCanvas* c2 = new TCanvas("c2","c2",1300,800);
  c2->Divide(4,2);
  TCanvas* c3 = new TCanvas("c3","c3",1500,800);
  c3->Divide(4,2);


  for(int i = 0;i < 8;i++){
    c0->cd(i+1);
    grammy[0]->DrawLayer(i,"colz");
    c1->cd(i+1);
    grammy[1]->DrawLayer(i,"colz");
    c2->cd(i+1);
    hmpcex_nxy[0][i]->Draw("colz");
    c3->cd(i+1);
    hmpcex_nxy[1][i]->Draw("colz");
  }

}
