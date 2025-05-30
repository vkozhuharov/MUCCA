const int MAXNSIGNALS=20;
const int NTOMERGE=1;
const int MINDIST=30;
const float DTCUT=2;


typedef struct {
  float time;
  float amp;
} myhit;

typedef struct {
  int nHits;
  myhit hit[MAXNSIGNALS];
  
} ecalEvent;

void clearEvent( ecalEvent *evt){
  for(int i = 0;i<10;i++){
    evt->hit[i].time = -1000;
    evt->hit[i].amp = 0;
  }
  evt->nHits = 0;
  
}


float THRESHOLD=10.;
int up=4.0;

void recoEvent(float *wave,ecalEvent *evt){
  //clear the event
  float processWave[1024*up];
  float filterWave[1024*up];
  clearEvent(evt);

  for(int i = 0;i<1024*up;i++) {
    processWave[i] = 0;
    filterWave[i] = 0;
  }
  
  //loop over the samples

  //Merge signals if closer than NTOMERGE/2
  for(int i = NTOMERGE/2; i < 1024*up - NTOMERGE/2 ;i++) {
    processWave[i] = fabs(wave[i]);
    for(int j = 1;j<=NTOMERGE/2;j++) {
      processWave[i] += wave[i-j] + wave[i+j];
    }
  }
  
  
  //filter the signals - leave only the maximum in a window of MINDIST
  for(int i = MINDIST/2; i < 1024*up - MINDIST/2;i++) {
    int isMax = 1;
    for(int j = 1; j<= MINDIST/2; j++) {
      if(processWave[i] < processWave[i-j]) isMax = 0;
      if(processWave[i] < processWave[i+j]) isMax = 0;
    }
    
    if(isMax) {
      filterWave[i] = processWave[i];
      for(int j = 1; j<=MINDIST/2; j++) {
	filterWave[i] += processWave[i-j] + processWave[i+j];
      }

    }

    
  }



  
  for(int i=0;i<1024*up;i++) {
    
    if(filterWave[i] > THRESHOLD && evt->nHits < MAXNSIGNALS) {
      //Add the hit to the event
      evt->hit[evt->nHits].time = i/up;
      evt->hit[evt->nHits].amp = filterWave[i];
      evt->nHits++;
    }
  }
  
}

void mergeHits(ecalEvent *evt,ecalEvent *evtMerge){
  
}


void printWaveForm(float *wave) {
  printf("Waveform: \n");
  for(int i = 0; i<1024*up;i++) {
    printf("%f   ",wave[i]);
  }
  printf("\n");
}


void cmpLabelsOutput(){
  const int NEvents = 100000;
  float waveLabel[1024*up];
  float waveOut[1024*up];
  float waveOut2[1024*up];
  ecalEvent labEvent;
  ecalEvent outEvent;
  ecalEvent outEvent2;
    
  
  
  char *labelFile = "4096labelsForFourier.dat";
  //  char *outputFile = "testresults_expsignals.dat";
  char *outputFile = "testresults_UP.dat";
  //char *outputFile = "testresults16filt.dat";
  //  int arrlabel[NEvents], arroutput[Nevents];

  TString RootFileName(outputFile);
  RootFileName+=".root";
  
  //  TFile *outRoot = new TFile("output.root","RECREATE");
  TFile *outRoot = new TFile(RootFileName,"RECREATE");

  
  TH1F *hNSignalsLab = new TH1F("hNSignalsLab","Number of signals - true",MAXNSIGNALS,0.0,MAXNSIGNALS);
  TH1F *hNSignalsOut = new TH1F("hNSignalsOut","Number of signals - found",MAXNSIGNALS,0.0,MAXNSIGNALS);

  TH1F *hNSignalsRatio = new TH1F("hNSignalsRatio","Reconstruction efficiency",MAXNSIGNALS,0.0,MAXNSIGNALS);

  TH1F *hdt = new TH1F("hdt","Difference in time between found and original hit",100, -50, 50);

  TH2F *hTimevsTime = new TH2F("hTimevsTime","Time true vs Time reconstructed",500,0,500,500,0,500);
  TH2F *hAmpvsAmp = new TH2F("hAmpvsAmp","True Amplitude vs Amplitude reconstructed",500,0,1000,500,0,1000);
  TProfile *AmpDeltaAmp = new TProfile("AmpDeltaAmp","Delta amplitude as a function of amplitude",500,0,1000,-50.,100);
  //TProfile *AmpDeltaReco = new TProfile("AmpDeltaReco","Delta reco amplitude as a function of amplitude",500,0,1000,-50.,100);
  
  TH2F *hAmpvsAmpClean = new TH2F("hAmpvsAmpClean","Reconstructed vs. true amplitude",500,0,1000,500,0,1000);
  TH1F *hMeanvsAmp = new TH1F("hMeanvsAmp","Mean reconstructed as a function of the signal amplitude",500,0,1000);
  TH1F *hSigmavsAmp = new TH1F("hSigmavsAmp","Sigma reconstructed as a function of the signal amplitude",500,0,1000);
  TH1F *hDeltaMean = new TH1F("hDeltaMean","Mean amplitude deviation",500,0,1000);
  TH1F *hDeltaMean2 = new TH1F("hDeltaMean2","Mean reco amplitude deviation",500,0,1000);

  TProfile *AmpDeltaAmpClean = new TProfile("AmpDeltaAmpClean","Amplitude deviation",500,0,1000,-50.,100);

  TH2F *nTrueVSNMatched = new TH2F("nTrueVSNMatched","True vs matched hits", MAXNSIGNALS,0.0,MAXNSIGNALS,MAXNSIGNALS,0.0,MAXNSIGNALS);

  TH1F *timeAll = new TH1F("timeAll","All time differences",200,0.0,200.0);
  TH1F *mindtFullyMatched = new TH1F("mindtFullyMatched","Matched and missed events based on distance",200,0.0,200.0);
  TH1F *mindtOneMissed = new TH1F("mindtOneMissed","Minimal distance of events with a missed hit",200,0.0,200.0);

  TH1F *mindtFullyMatched3hits = new TH1F("mindtFullyMatched3hits","Minimal dT of fully matched events with 3 hits",200,0.0,200.0);
  TH1F *mindtOneMissed3hits = new TH1F("mindtOneMissed3hits","Minimal dT of events with 3 hits and 1 missed hit",200,0.0,200.0);
  TH1F *mindtTwoMissed3hits = new TH1F("mindtTwoMissed3hits","Minimal dT of events with 3 hits and 2 missed hits",200,0.0,200.0);

  
  TH1F *ampFound = new TH1F("ampFound","Amplitudes of identified events",1000,0.0,1000.0);
  TH1F *ampMissed = new TH1F("ampMissed","Amplitudes of missed hits",1000,0.0,1000.0);
  TH1F *amp2sigFound = new TH1F("amp2signalsFound","Amplitude of identified events",400,0.0,400.0);
  TH1F *amp2sig1miss = new TH1F("amp2sig1Missed","Amplitude of the missed hit",400,0.0,400.0);
  TH1F *amp2sig2miss = new TH1F("amp2sig2Missed","Amplitude of the missed hit",400,0.0,400.0);
  TH1F *ampAll = new TH1F("ampAll","Matched and missed events based on amplitude",1000,0.0,1000.0);
  TH1F *ampAll2 = new TH1F("ampAll2","Amplitude of the all hit",400,0.0,400.0);

  TProfile *times = new TProfile("times","Matched events based on time difference",1000,0.0,1000.0,0.0,1.0);
  
  FILE *flab;
  FILE *fout;
  flab = fopen(labelFile,"r");
  fout = fopen(outputFile,"r");
  
  for(int iev = 0;iev<NEvents;iev++){
    for(int is = 0;is<1024*up;is++) {
      fscanf(flab,"%f",&waveLabel[is]);
      fscanf(fout,"%f",&waveOut[is]);
    }
    //read event from labels
    //read event from Output
    //Get the time and amplitude of the hits

    recoEvent(waveLabel,&labEvent);
    recoEvent(waveOut,&outEvent);
    
    hNSignalsLab->Fill(labEvent.nHits);
    hNSignalsOut->Fill(outEvent.nHits);
   

    //Compare the values


    int nMatched = 0;

    float mindT = 10000.;
        
    for(int ihit = 0;ihit<labEvent.nHits-1;ihit++){
      for(int ihit2 = ihit+1 ;ihit2<labEvent.nHits;ihit2++){
	if( fabs(labEvent.hit[ihit].time - labEvent.hit[ihit2].time) < mindT ) {
	  mindT = fabs(labEvent.hit[ihit].time - labEvent.hit[ihit2].time);
	  
	}
      }
    }
    
    
    for(int ihit = 0;ihit<labEvent.nHits;ihit++){
      int ifound = -1;
      float dt = 1024*up;
      //Search for the closest identified hit
      for(int ih = 0; ih <outEvent.nHits;ih++) {
	if( fabs(outEvent.hit[ih].time - labEvent.hit[ihit].time) < dt) {
	  dt = fabs(outEvent.hit[ih].time - labEvent.hit[ihit].time);
	  ifound = ih;
	}	
      }
      
      if(ifound >=0) {
	outEvent.hit[ifound].amp*=1.18;
	hdt->Fill(outEvent.hit[ifound].time - labEvent.hit[ihit].time);
	hTimevsTime->Fill(labEvent.hit[ihit].time,outEvent.hit[ifound].time);
	hAmpvsAmp->Fill(labEvent.hit[ihit].amp,outEvent.hit[ifound].amp);	
	AmpDeltaAmp->Fill(labEvent.hit[ihit].amp, labEvent.hit[ihit].amp-outEvent.hit[ifound].amp);
	//AmpDeltaReco->Fill(labEvent.hit[ihit].amp, outEvent.hit[ihit].amp-outEvent2.hit[ifound].amp);
	
      }

      
      if(ifound >=0 && fabs(outEvent.hit[ifound].time - labEvent.hit[ihit].time) < DTCUT) {
	hAmpvsAmpClean->Fill(labEvent.hit[ihit].amp,outEvent.hit[ifound].amp);	
	AmpDeltaAmpClean->Fill(labEvent.hit[ihit].amp, labEvent.hit[ihit].amp-outEvent.hit[ifound].amp);
	nMatched ++;
      }
      
    }


    nTrueVSNMatched->Fill(labEvent.nHits,nMatched);
    if(labEvent.nHits == 3) {
    
      if(labEvent.nHits == nMatched) {
	mindtFullyMatched->Fill(mindT);
	//times->Fill();
      }

      if(labEvent.nHits - nMatched == 1) {
	mindtOneMissed->Fill(mindT);
      }
	timeAll->Fill(mindT);

    }

    if(labEvent.nHits == 4) {
      if(labEvent.nHits == nMatched) {
	mindtFullyMatched3hits->Fill(mindT);
      }

      if(labEvent.nHits - nMatched == 1) {
	mindtOneMissed3hits->Fill(mindT);
      }
      if(labEvent.nHits - nMatched == 2) {
	mindtTwoMissed3hits->Fill(mindT);
      }

    }

    
    
    if( labEvent.nHits == 1) {
      ampAll->Fill(labEvent.hit[0].amp);
      if (nMatched == 1) {
	ampFound->Fill(labEvent.hit[0].amp);
      }
      if (nMatched == 0) {
	ampMissed->Fill(labEvent.hit[0].amp);
      }

    }

    if( labEvent.nHits == 2) {
      ampAll2->Fill(labEvent.hit[0].amp);
      if (nMatched == 2) {
	amp2sigFound->Fill(labEvent.hit[0].amp);
      }
      if (nMatched == 1) {
	amp2sig1miss->Fill(labEvent.hit[0].amp);
      }
      if (nMatched == 0) {
	amp2sig2miss->Fill(labEvent.hit[0].amp);
      }

    }
  }

  fclose(flab);
  fclose(fout);


  TCanvas *c1  = new TCanvas(); c1->cd();
  hNSignalsLab->Draw();

  TCanvas *c2  = new TCanvas(); c2->cd();
  hNSignalsOut->Draw();

  
  hNSignalsRatio->Divide(hNSignalsOut,hNSignalsLab );
  TCanvas *c3  = new TCanvas(); c3->cd();
  hNSignalsRatio->Draw();


  
  TCanvas *c4  = new TCanvas(); c4->cd();
  hdt->Draw();
  
  TCanvas *c5  = new TCanvas(); c5->cd();
  hTimevsTime->Draw("colz");
  
  TCanvas *c6  = new TCanvas(); c6->cd();
  hAmpvsAmp->Draw("colz");
  
  TCanvas *c7  = new TCanvas(); c7->cd();
  AmpDeltaAmp->Draw();

  
  TCanvas *c8  = new TCanvas(); c8->cd();
  hAmpvsAmpClean->Draw("colz");
 
  TCanvas *c9  = new TCanvas(); c9->cd();
  AmpDeltaAmpClean->Draw();
  
  TCanvas *c10  = new TCanvas(); c10->cd();
  nTrueVSNMatched->Draw();


  TCanvas *c11  = new TCanvas(); c11->cd();
  timeAll->Draw();
  mindtFullyMatched->Draw("same");
  mindtOneMissed->Draw("same");
  
  TCanvas *c20  = new TCanvas(); c20->cd();
  TH1F *timediff = (TH1F *) timeAll->Clone();
  timediff->Divide(mindtFullyMatched,timeAll);
  timediff->Draw();
  
  TCanvas *c12  = new TCanvas(); c12->cd();
  ampAll->Draw();
  ampFound->Draw("same");
  ampMissed->Draw("same");
  
  
  TCanvas *c13  = new TCanvas(); c13->cd();
  TH1F *heff = (TH1F *) ampAll->Clone();
  heff->Divide(ampFound,ampAll);
  heff->Draw();

  TCanvas *c14  = new TCanvas(); c14->cd();
  mindtFullyMatched3hits->Draw();
  mindtOneMissed3hits->Draw("same");
  mindtTwoMissed3hits->Draw("same");

  TCanvas *c15  = new TCanvas(); c15->cd();
  ampAll2->Draw();
  amp2sigFound->Draw("same");
  amp2sig1miss->Draw("same");
  amp2sig2miss->Draw("same");

   TCanvas *c16  = new TCanvas(); c16->cd();
  TH1F *hefft = (TH1F *) mindtFullyMatched->Clone();
  hefft->Divide(mindtFullyMatched,timeAll);
  hefft->Draw();

  TH1D *hproj;
  
  for(int i = 1; i<= hAmpvsAmpClean->GetNbinsX();i++) {

    hproj = hAmpvsAmpClean->ProjectionY("_py",i,i);
    if(hproj->Integral() > 100) {
      int maxbin = hproj->GetMaximumBin();
      //Fit with gauss
      TF1 *f1 = new TF1("f1", "gaus",2*maxbin - 20,2*maxbin+20);
      hproj->Fit("f1","R");

      //Get the params
      float mean = f1->GetParameter(1);
      float meanE = f1->GetParError(1);

      float sigma = f1->GetParameter(2);
      float sigmaE = f1->GetParError(2);
      
      hMeanvsAmp->SetBinContent(i,mean);hMeanvsAmp->SetBinError(i,meanE);
      hSigmavsAmp->SetBinContent(i,sigma);hSigmavsAmp->SetBinError(i,sigmaE);
      hDeltaMean->SetBinContent(i,-(mean-2*i - 1 ));
      delete f1;
      
    }
  }
  
  
  TCanvas *c17  = new TCanvas(); c17->cd();
  hMeanvsAmp->Draw();

  TCanvas *c18  = new TCanvas(); c18->cd();
  hSigmavsAmp->Draw();

  TCanvas *c19  = new TCanvas(); c19->cd();
  hDeltaMean->Draw();

 // TCanvas *c20  = new TCanvas(); c20->cd();
 //  AmpDeltaReco->Draw();
  
  outRoot->Write();
  // getchar();
  // outRoot->Close();

  
}
