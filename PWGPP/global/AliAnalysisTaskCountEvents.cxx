#include "AliInputEventHandler.h"
#include "AliAnalysisTaskSE.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliAODEvent.h"
#include "AliMultSelection.h"
#include <TSystem.h>
#include <TTree.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TChain.h>
#include "AliAnalysisTaskCountEvents.h"


/**************************************************************************
 * Copyright(c) 1998-2018, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

//*************************************************************************
// Implementation of class AliAnalysisTaskCountEvents
// AliAnalysisTaskSE to count events per trigger mask after various selections
// 
//
// Authors: 
//          F. Prino, prino@to.infn.it
//          
//*************************************************************************

ClassImp(AliAnalysisTaskCountEvents)
//______________________________________________________________________________
AliAnalysisTaskCountEvents::AliAnalysisTaskCountEvents() : 
  AliAnalysisTaskSE("CountEvents"), 
  fOutput{nullptr},
  fHistNEventsPhysSel{nullptr},
  fHistNEventsSPDVert{nullptr},
  fHistNEventsTrackVert{nullptr},
  fHistNEventsPhysSelVsCent{nullptr},
  fHistNEventsSPDVertVsCent{nullptr},
  fHistNEventsTrackVertVsCent{nullptr}
{
  //
  DefineInput(0, TChain::Class());
  DefineOutput(1, TList::Class());
}


//___________________________________________________________________________
AliAnalysisTaskCountEvents::~AliAnalysisTaskCountEvents(){
  //
  if (AliAnalysisManager::GetAnalysisManager()->IsProofMode()) return;
  if(fOutput && !fOutput->IsOwner()){
    delete fHistNEventsPhysSel;
    delete fHistNEventsSPDVert;
    delete fHistNEventsTrackVert;
    delete fHistNEventsPhysSelVsCent;
    delete fHistNEventsSPDVertVsCent;
    delete fHistNEventsTrackVertVsCent;
  }
  delete fOutput;
}
//___________________________________________________________________________
void AliAnalysisTaskCountEvents::ConfigureXaxis(TH1* histo){
  histo->GetXaxis()->SetBinLabel(1,"kAny");
  histo->GetXaxis()->SetBinLabel(2,"kINT7");
  histo->GetXaxis()->SetBinLabel(3,"kCentral");
  histo->GetXaxis()->SetBinLabel(4,"kSemiCentral");
  return;
}
//___________________________________________________________________________
void AliAnalysisTaskCountEvents::UserCreateOutputObjects() {
  // create output histos

  fOutput = new TList();
  fOutput->SetOwner();
  fOutput->SetName("OutputHistos");

  fHistNEventsPhysSel = new TH1F("hNEventsPhysSel", "Number of events after phys sel",4,-0.5,3.5);
  ConfigureXaxis(fHistNEventsPhysSel);
  fOutput->Add(fHistNEventsPhysSel);
  fHistNEventsSPDVert = new TH1F("hNEventsSPDVert", "Number of events after phys sel",4,-0.5,3.5);
  ConfigureXaxis(fHistNEventsSPDVert);
  fOutput->Add(fHistNEventsSPDVert);
  fHistNEventsTrackVert = new TH1F("hNEventsTrackVert", "Number of events after phys sel",4,-0.5,3.5);
  ConfigureXaxis(fHistNEventsTrackVert);
  fOutput->Add(fHistNEventsTrackVert);

  fHistNEventsPhysSelVsCent = new TH2F("hNEventsPhysSelVsCent","",4,-0.5,3.5,100,0.,100.);
  ConfigureXaxis(fHistNEventsPhysSelVsCent);
  fOutput->Add(fHistNEventsPhysSelVsCent);
  fHistNEventsSPDVertVsCent = new TH2F("hNEventsSPDVertVsCent","",4,-0.5,3.5,100,0.,100.);
  ConfigureXaxis(fHistNEventsSPDVertVsCent);
  fOutput->Add(fHistNEventsSPDVertVsCent);
  fHistNEventsTrackVertVsCent = new TH2F("hNEventsTrackVertVsCent","",4,-0.5,3.5,100,0.,100.);
  ConfigureXaxis(fHistNEventsTrackVertVsCent);
  fOutput->Add(fHistNEventsTrackVertVsCent);
  
  PostData(1,fOutput);

}
//______________________________________________________________________________
void AliAnalysisTaskCountEvents::UserExec(Option_t *)
{
  //

  AliVEvent *ev = fInputEvent;
  if(!ev) {
    printf("AliAnalysisTaskCountEvents::UserExec(): bad Event\n");
    return;
  }

  bool isAOD = ev->IsA()->InheritsFrom("AliAODEvent");

  UInt_t isPhysSel = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();

  Bool_t TrigMaskOn[4]={kFALSE,kFALSE,kFALSE,kFALSE};
  if(isPhysSel & AliVEvent::kAny) TrigMaskOn[0]=kTRUE;
  if(isPhysSel & AliVEvent::kINT7) TrigMaskOn[1]=kTRUE;
  if(isPhysSel & AliVEvent::kCentral) TrigMaskOn[2]=kTRUE;
  if(isPhysSel & AliVEvent::kSemiCentral) TrigMaskOn[3]=kTRUE;
  
  Int_t vertType=0;
  const AliVVertex* vtPrim = ev->GetPrimaryVertex();
  const AliVVertex* vtTPC = ev->GetPrimaryVertexTPC();
  const AliVVertex* vtSPD = ev->GetPrimaryVertexSPD();
  TString title=vtPrim->GetTitle();
  if(title.Contains("VertexerTracks")){
    if (TMath::Abs(vtPrim->GetZ()-vtTPC->GetZ())<1e-6 &&
	TMath::Abs(vtPrim->GetChi2perNDF()-vtTPC->GetChi2perNDF())<1e-6) {
      // TPC vertex
      vertType=1;
    }else{
      // track vertex
      vertType=4;
    }
  }else{
    // SPD vertex
    if(title.Contains("ertexer: Z")) vertType=2;
    if(title.Contains("ertexer: 3D")) vertType=3;
  }
  if(isAOD){
    const AliAODVertex* vtAOD = (AliAODVertex*)ev->GetPrimaryVertex();
    Int_t typp=vtAOD->GetType();
    if(typp==AliAODVertex::kPrimaryInvalid || typp==AliAODVertex::kUndef) vertType=0;
    else if(typp==AliAODVertex::kPrimaryTPC) vertType=1;
  }
  Double_t centr=0.1; // default = all in first bin
  AliMultSelection *multSelection = (AliMultSelection*)ev->FindListObject("MultSelection");
  if(multSelection) centr = multSelection->GetMultiplicityPercentile("V0M");
  else AliWarning("AliMultSelection could not be found in the list of objects");

  for(Int_t iBin=0; iBin<4; iBin++){
    if(TrigMaskOn[iBin]){
      fHistNEventsPhysSel->Fill(iBin);
      fHistNEventsPhysSelVsCent->Fill(iBin,centr);
      if(vtSPD && vtSPD->GetNContributors()>=1){
	fHistNEventsSPDVert->Fill(iBin);
	fHistNEventsSPDVertVsCent->Fill(iBin,centr);	
      }
      if(vertType==4){
	fHistNEventsTrackVert->Fill(iBin);
	fHistNEventsTrackVertVsCent->Fill(iBin,centr);
      }
    }
  }

  PostData(1,fOutput);
  
}
//______________________________________________________________________________
void AliAnalysisTaskCountEvents::Terminate(Option_t */*option*/)
{
  // Terminate analysis
  fOutput = dynamic_cast<TList*> (GetOutputData(1));
  if (!fOutput) {     
    printf("ERROR: fOutput not available\n");
    return;
  }
  return;
}





