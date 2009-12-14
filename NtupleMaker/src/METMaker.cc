//-*- C++ -*-
//
// Package:    METMaker
// Class:      METMaker
// 
/**\class METMaker METMaker.cc CMS2/METMaker/src/METMaker.cc

Description: <one line class summary>

Implementation:
<Notes on implementation>
*/
//
// Original Author:  pts/4
//         Created:  Fri Jun  6 11:07:38 CDT 2008
// $Id: METMaker.cc,v 1.19 2009/12/14 06:43:48 warren Exp $
//
//

// system include files
#include <memory>
#include <vector>
#include "TMath.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CMS2/NtupleMaker/interface/METMaker.h"

#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/MET.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonMETCorrectionData.h"
#include "DataFormats/Common/interface/ValueMap.h" 

#include "DataFormats/CaloTowers/interface/CaloTower.h"

typedef math::XYZTLorentzVectorF LorentzVector;
using namespace reco;
using namespace edm;
using namespace std;

//
// constructors and destructor
//

METMaker::METMaker(const edm::ParameterSet& iConfig) {

  /* 
     met -> raw caloMET. Does not include the HO by default in 21X
     metHO -> raw caloMET with HO
     metNOHF -> no HF but includes HO
     metNOHFHO -> no HF and no HO
     use scheme B thresholds
     https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMETObjects
  */

  produces<float> ("evtmet"          ).setBranchAlias("evt_met"          );
  produces<float> ("evtmetPhi"       ).setBranchAlias("evt_metPhi"       );
  produces<float> ("evtmetSig"       ).setBranchAlias("evt_metSig"       );
  produces<float> ("evtmetHO"        ).setBranchAlias("evt_metHO"        );
  produces<float> ("evtmetHOPhi"     ).setBranchAlias("evt_metHOPhi"     );
  produces<float> ("evtmetHOSig"     ).setBranchAlias("evt_metHOSig"     );
  produces<float> ("evtmetNoHF"      ).setBranchAlias("evt_metNoHF"      );
  produces<float> ("evtmetNoHFPhi"   ).setBranchAlias("evt_metNoHFPhi"   );
  produces<float> ("evtmetNoHFSig"   ).setBranchAlias("evt_metNoHFSig"   );
  produces<float> ("evtmetNoHFHO"    ).setBranchAlias("evt_metNoHFHO"    );
  produces<float> ("evtmetNoHFHOPhi" ).setBranchAlias("evt_metNoHFHOPhi" );
  produces<float> ("evtmetNoHFHOSig" ).setBranchAlias("evt_metNoHFHOSig" );

  //same as above, but using Opt
  produces<float> ("evtmetOpt"          ).setBranchAlias("evt_metOpt"          );
  produces<float> ("evtmetOptPhi"       ).setBranchAlias("evt_metOptPhi"       );
  produces<float> ("evtmetOptSig"       ).setBranchAlias("evt_metOptSig"       );
  produces<float> ("evtmetOptHO"        ).setBranchAlias("evt_metOptHO"        );
  produces<float> ("evtmetOptHOPhi"     ).setBranchAlias("evt_metOptHOPhi"     );
  produces<float> ("evtmetOptHOSig"     ).setBranchAlias("evt_metOptHOSig"     );
  produces<float> ("evtmetOptNoHF"      ).setBranchAlias("evt_metOptNoHF"      );
  produces<float> ("evtmetOptNoHFPhi"   ).setBranchAlias("evt_metOptNoHFPhi"   );
  produces<float> ("evtmetOptNoHFSig"   ).setBranchAlias("evt_metOptNoHFSig"   );
  produces<float> ("evtmetOptNoHFHO"    ).setBranchAlias("evt_metOptNoHFHO"    );
  produces<float> ("evtmetOptNoHFHOPhi" ).setBranchAlias("evt_metOptNoHFHOPhi" );
  produces<float> ("evtmetOptNoHFHOSig" ).setBranchAlias("evt_metOptNoHFHOSig" );

  //raw CaloMET corrected for Muons
  produces<float> ("evtmetMuonCorr"     ).setBranchAlias("evt_metMuonCorr"     );
  produces<float> ("evtmetMuonCorrPhi"  ).setBranchAlias("evt_metMuonCorrPhi"  );
  produces<float> ("evtmetMuonCorrSig"  ).setBranchAlias("evt_metMuonCorrSig"  );

  //raw CaloMET corrected for JES (L2L3) and Muons
  produces<float> ("evtmetMuonJESCorr"      ).setBranchAlias("evt_metMuonJESCorr"      );
  produces<float> ("evtmetMuonJESCorrPhi"   ).setBranchAlias("evt_metMuonJESCorrPhi"   );
  produces<float> ("evtmetMuonJESCorrSig"   ).setBranchAlias("evt_metMuonJESCorrSig"   );

  // sumet
  produces<float> ("evtsumet"               ).setBranchAlias("evt_sumet"                );  
  produces<float> ("evtsumetHO"             ).setBranchAlias("evt_sumetHO"              );
  produces<float> ("evtsumetNoHF"           ).setBranchAlias("evt_sumetNoHF"            );
  produces<float> ("evtsumetNoHFHO"         ).setBranchAlias("evt_sumetNoHFHO"          );  
  produces<float> ("evtsumetOpt"            ).setBranchAlias("evt_sumetOpt"             );
  produces<float> ("evtsumetOptHO"          ).setBranchAlias("evt_sumetOptHO"           );
  produces<float> ("evtsumetOptNoHF"        ).setBranchAlias("evt_sumetOptNoHF"         );
  produces<float> ("evtsumetOptNoHFHO"      ).setBranchAlias("evt_sumetOptNoHFHO"       );  
  produces<float> ("evtsumetMuonCorr"       ).setBranchAlias("evt_sumetMuonCorr"        );

  // store muon value map quantities
  produces<vector<int> >   ("musmetflag"   ).setBranchAlias("mus_met_flag"              );
  produces<vector<float> > ("musmetdeltax" ).setBranchAlias("mus_met_deltax"            );
  produces<vector<float> > ("musmetdeltay" ).setBranchAlias("mus_met_deltay"            );

  produces<float> ("evtecalmet"            ).setBranchAlias("evt_ecalmet"               );
  produces<float> ("evthcalmet"            ).setBranchAlias("evt_hcalmet"               );
  produces<float> ("evtecalmetPhi"         ).setBranchAlias("evt_ecalmetPhi"            );
  produces<float> ("evthcalmetPhi"         ).setBranchAlias("evt_hcalmetPhi"            );

  produces<float> ("evtendcappmet"         		).setBranchAlias("evt_endcapp_met"       		);
  produces<float> ("evtendcapmmet"         		).setBranchAlias("evt_endcapm_met"       		);
  produces<float> ("evtecalendcappmet"     		).setBranchAlias("evt_ecalendcapp_met"       	);
  produces<float> ("evtecalendcapmmet"     		).setBranchAlias("evt_ecalendcapm_met"       	);
  produces<float> ("evthcalendcappmet"     		).setBranchAlias("evt_hcalendcapp_met"       	);
  produces<float> ("evthcalendcapmmet"     		).setBranchAlias("evt_hcalendcapm_met"       	);
  produces<float> ("evtendcappmetPhi"         	).setBranchAlias("evt_endcapp_metPhi"       	);
  produces<float> ("evtendcapmmetPhi"         	).setBranchAlias("evt_endcapm_metPhi"       	);
  produces<float> ("evtecalendcappmetPhi"     	).setBranchAlias("evt_ecalendcapp_metPhi"       );
  produces<float> ("evtecalendcapmmetPhi"     	).setBranchAlias("evt_ecalendcapm_metPhi"       );
  produces<float> ("evthcalendcappmetPhi"     	).setBranchAlias("evt_hcalendcapp_metPhi"       );
  produces<float> ("evthcalendcapmmetPhi"     	).setBranchAlias("evt_hcalendcapm_metPhi"       );
  
  met_tag               = iConfig.getParameter<edm::InputTag>("met_tag_"               );       
  metHO_tag             = iConfig.getParameter<edm::InputTag>("metHO_tag_"             );     
  metNoHF_tag           = iConfig.getParameter<edm::InputTag>("metNoHF_tag_"           );   
  metNoHFHO_tag         = iConfig.getParameter<edm::InputTag>("metNoHFHO_tag_"         ); 

  metOpt_tag            = iConfig.getParameter<edm::InputTag>("metOpt_tag_"            );       
  metOptHO_tag          = iConfig.getParameter<edm::InputTag>("metOptHO_tag_"          );     
  metOptNoHF_tag        = iConfig.getParameter<edm::InputTag>("metOptNoHF_tag_"        );   
  metOptNoHFHO_tag      = iConfig.getParameter<edm::InputTag>("metOptNoHFHO_tag_"      ); 
  
  MuonJEScorMET_tag     = iConfig.getParameter<edm::InputTag>("MuonJEScorMET_tag_"     );
  corMetGlobalMuons_tag = iConfig.getParameter<edm::InputTag>("corMetGlobalMuons_tag_" );

  muon_vm_tag           = iConfig.getParameter<edm::InputTag>("muon_vm_tag_");
  muon_tag              = iConfig.getParameter<edm::InputTag>("muon_tag_"   );

  caloTowerInputTag     = iConfig.getParameter<edm::InputTag>("caloTower_tag_");
  towerEtThreshold      = iConfig.getParameter<double>       ("towerEtThreshold_");
  make_eta_rings        = iConfig.getParameter<bool>         ("make_eta_rings_");

  if( make_eta_rings ) {
    produces<vector<float> > ("evttowermetetaslice"         ).setBranchAlias("evt_towermet_etaslice"         );
	produces<vector<float> > ("evtecalmetetaslice"          ).setBranchAlias("evt_ecalmet_etaslice"          );
	produces<vector<float> > ("evthcalmetetaslice"          ).setBranchAlias("evt_hcalmet_etaslice"          );
	produces<vector<float> > ("evttowermetetaslicePhi"      ).setBranchAlias("evt_towermet_etaslicePhi"      );
	produces<vector<float> > ("evtecalmetetaslicePhi"       ).setBranchAlias("evt_ecalmet_etaslicePhi"       );
	produces<vector<float> > ("evthcalmetetaslicePhi"       ).setBranchAlias("evt_hcalmet_etaslicePhi"       );
  }

}

METMaker::~METMaker()
{
}

void  METMaker::beginJob(const edm::EventSetup&)
{
}

void METMaker::endJob()
{
}

// ------------ method called to produce the data  ------------
void METMaker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  
  auto_ptr<float>   evt_met                 (new float     );
  auto_ptr<float>   evt_metPhi              (new float     );
  auto_ptr<float>   evt_metSig              (new float     );
  auto_ptr<float>   evt_metHO               (new float     );
  auto_ptr<float>   evt_metHOPhi            (new float     );
  auto_ptr<float>   evt_metHOSig            (new float     );
  auto_ptr<float>   evt_metNoHF             (new float     );
  auto_ptr<float>   evt_metNoHFPhi          (new float     );
  auto_ptr<float>   evt_metNoHFSig          (new float     );
  auto_ptr<float>   evt_metNoHFHO           (new float     );
  auto_ptr<float>   evt_metNoHFHOPhi        (new float     );
  auto_ptr<float>   evt_metNoHFHOSig        (new float     );

  auto_ptr<float>   evt_metOpt              (new float     );
  auto_ptr<float>   evt_metOptPhi           (new float     );
  auto_ptr<float>   evt_metOptSig           (new float     );
  auto_ptr<float>   evt_metOptHO            (new float     );
  auto_ptr<float>   evt_metOptHOPhi         (new float     );
  auto_ptr<float>   evt_metOptHOSig         (new float     );
  auto_ptr<float>   evt_metOptNoHF          (new float     );
  auto_ptr<float>   evt_metOptNoHFPhi       (new float     );
  auto_ptr<float>   evt_metOptNoHFSig       (new float     );
  auto_ptr<float>   evt_metOptNoHFHO        (new float     );
  auto_ptr<float>   evt_metOptNoHFHOPhi     (new float     );
  auto_ptr<float>   evt_metOptNoHFHOSig     (new float     );
  
  auto_ptr<float>   evt_metMuonCorr         (new float     );
  auto_ptr<float>   evt_metMuonCorrPhi      (new float     );
  auto_ptr<float>   evt_metMuonCorrSig      (new float     );
  auto_ptr<float>   evt_metMuonJESCorr      (new float     );
  auto_ptr<float>   evt_metMuonJESCorrPhi   (new float     );
  auto_ptr<float>   evt_metMuonJESCorrSig   (new float     );

  auto_ptr<float>   evt_sumet               (new float     );
  auto_ptr<float>   evt_sumetHO	            (new float     );
  auto_ptr<float>   evt_sumetNoHF           (new float     );
  auto_ptr<float>   evt_sumetNoHFHO         (new float     );
  auto_ptr<float>   evt_sumetOpt            (new float     );
  auto_ptr<float>   evt_sumetOptHO          (new float     );
  auto_ptr<float>   evt_sumetOptNoHF        (new float     );
  auto_ptr<float>   evt_sumetOptNoHFHO      (new float     );
  auto_ptr<float>   evt_sumetMuonCorr       (new float     );

  auto_ptr<vector<int>   > mus_met_flag   ( new vector<int>   );
  auto_ptr<vector<float> > mus_met_deltax ( new vector<float> );
  auto_ptr<vector<float> > mus_met_deltay ( new vector<float> );

  auto_ptr<float> evt_ecalmet         (new float     );
  auto_ptr<float> evt_hcalmet         (new float     );
  auto_ptr<float> evt_ecalmetPhi      (new float     );
  auto_ptr<float> evt_hcalmetPhi      (new float     );

  auto_ptr<float> evt_endcapp_met         (new float );
  auto_ptr<float> evt_endcapm_met      	  (new float );
  auto_ptr<float> evt_ecalendcapp_met  	  (new float );
  auto_ptr<float> evt_ecalendcapm_met  	  (new float );
  auto_ptr<float> evt_hcalendcapp_met  	  (new float );
  auto_ptr<float> evt_hcalendcapm_met  	  (new float );
  auto_ptr<float> evt_endcapp_metPhi   	  (new float );
  auto_ptr<float> evt_endcapm_metPhi   	  (new float );
  auto_ptr<float> evt_ecalendcapp_metPhi  (new float );
  auto_ptr<float> evt_ecalendcapm_metPhi  (new float );
  auto_ptr<float> evt_hcalendcapp_metPhi  (new float );
  auto_ptr<float> evt_hcalendcapm_metPhi  (new float );

  auto_ptr<vector<float> > evt_towermet_etaslice        (new vector<float>     ); //towermet = ecalmet + hcalmet
  auto_ptr<vector<float> > evt_ecalmet_etaslice         (new vector<float>     );
  auto_ptr<vector<float> > evt_hcalmet_etaslice         (new vector<float>     );
  auto_ptr<vector<float> > evt_towermet_etaslicePhi     (new vector<float>     ); //towermet = ecalmet + hcalmet
  auto_ptr<vector<float> > evt_ecalmet_etaslicePhi      (new vector<float>     );
  auto_ptr<vector<float> > evt_hcalmet_etaslicePhi      (new vector<float>     );

  edm::Handle<CaloTowerCollection> h_caloTowers;
  iEvent.getByLabel("towerMaker", h_caloTowers);

  edm::Handle< edm::View<reco::CaloMET> > met_h;
  edm::Handle< edm::View<reco::CaloMET> > metHO_h;
  edm::Handle< edm::View<reco::CaloMET> > metNoHF_h;
  edm::Handle< edm::View<reco::CaloMET> > metNoHFHO_h;

  edm::Handle< edm::View<reco::CaloMET> > metOpt_h;
  edm::Handle< edm::View<reco::CaloMET> > metOptHO_h;
  edm::Handle< edm::View<reco::CaloMET> > metOptNoHF_h;
  edm::Handle< edm::View<reco::CaloMET> > metOptNoHFHO_h;

  edm::Handle< edm::View<reco::CaloMET> > metMuonCorr_h;
  edm::Handle< edm::View<reco::CaloMET> > metMuonJESCorr_h;

  edm::Handle< edm::ValueMap<reco::MuonMETCorrectionData> > muon_vm_h;
  edm::Handle< reco::MuonCollection > muon_h;
  
  iEvent.getByLabel(met_tag      , met_h       );
  iEvent.getByLabel(metHO_tag    , metHO_h     );
  iEvent.getByLabel(metNoHF_tag  , metNoHF_h   );
  iEvent.getByLabel(metNoHFHO_tag, metNoHFHO_h );

  iEvent.getByLabel(metOpt_tag      , metOpt_h       );
  iEvent.getByLabel(metOptHO_tag    , metOptHO_h     );
  iEvent.getByLabel(metOptNoHF_tag  , metOptNoHF_h   );
  iEvent.getByLabel(metOptNoHFHO_tag, metOptNoHFHO_h );
  
  iEvent.getByLabel(corMetGlobalMuons_tag, metMuonCorr_h      );
  iEvent.getByLabel(MuonJEScorMET_tag,     metMuonJESCorr_h   );

  iEvent.getByLabel(muon_vm_tag, muon_vm_h );
  iEvent.getByLabel(muon_tag   , muon_h    );
    
  *evt_met          = ( met_h->front()       ).et();
  *evt_metPhi       = ( met_h->front()       ).phi();
  *evt_metSig       = ( met_h->front()       ).metSignificance();
  *evt_metHO        = ( metHO_h->front()     ).et();
  *evt_metHOPhi     = ( metHO_h->front()     ).phi();
  *evt_metHOSig     = ( metHO_h->front()     ).metSignificance();
  *evt_metNoHF      = ( metNoHF_h->front()   ).et();
  *evt_metNoHFPhi   = ( metNoHF_h->front()   ).phi();
  *evt_metNoHFSig   = ( metNoHF_h->front()   ).metSignificance();
  *evt_metNoHFHO    = ( metNoHFHO_h->front() ).et();
  *evt_metNoHFHOPhi = ( metNoHFHO_h->front() ).phi();
  *evt_metNoHFHOSig = ( metNoHFHO_h->front() ).metSignificance();

  *evt_metOpt          = ( metOpt_h->front()       ).et();
  *evt_metOptPhi       = ( metOpt_h->front()       ).phi();
  *evt_metOptSig       = ( metOpt_h->front()       ).metSignificance();
  *evt_metOptHO        = ( metOptHO_h->front()     ).et();
  *evt_metOptHOPhi     = ( metOptHO_h->front()     ).phi();
  *evt_metOptHOSig     = ( metOptHO_h->front()     ).metSignificance();
  *evt_metOptNoHF      = ( metOptNoHF_h->front()   ).et();
  *evt_metOptNoHFPhi   = ( metOptNoHF_h->front()   ).phi();
  *evt_metOptNoHFSig   = ( metOptNoHF_h->front()   ).metSignificance();
  *evt_metOptNoHFHO    = ( metOptNoHFHO_h->front() ).et();
  *evt_metOptNoHFHOPhi = ( metOptNoHFHO_h->front() ).phi();
  *evt_metOptNoHFHOSig = ( metOptNoHFHO_h->front() ).metSignificance();

  *evt_metMuonCorr         =  ( metMuonCorr_h->front()    ).et();
  *evt_metMuonCorrPhi      =  ( metMuonCorr_h->front()    ).phi();
  *evt_metMuonCorrSig      =  ( metMuonCorr_h->front()    ).metSignificance();
  *evt_metMuonJESCorr      =  ( metMuonJESCorr_h->front() ).et();
  *evt_metMuonJESCorrPhi   =  ( metMuonJESCorr_h->front() ).phi();
  *evt_metMuonJESCorrSig   =  ( metMuonJESCorr_h->front() ).metSignificance();

  *evt_sumet           = ( met_h->front()          ).sumEt();     
  *evt_sumetHO         = ( metHO_h->front()        ).sumEt();   
  *evt_sumetNoHF       = ( metNoHF_h->front()      ).sumEt();
  *evt_sumetNoHFHO     = ( metNoHFHO_h->front()    ).sumEt();
  *evt_sumetOpt        = ( metOpt_h->front()       ).sumEt();      
  *evt_sumetOptHO      = ( metOptHO_h->front()     ).sumEt();    
  *evt_sumetOptNoHF    = ( metOptNoHF_h->front()   ).sumEt();  
  *evt_sumetOptNoHFHO  = ( metOptNoHFHO_h->front() ).sumEt();
  *evt_sumetMuonCorr   = ( metMuonCorr_h->front()  ).sumEt();

  edm::ValueMap<reco::MuonMETCorrectionData> muon_data = *muon_vm_h;

  const unsigned int nMuons = muon_h->size();

  // loop over muons and extract quantities from ValueMap
  for( unsigned int mus = 0; mus < nMuons; mus++ ) {

    reco::MuonRef muref( muon_h, mus);
    reco::MuonMETCorrectionData muCorrData = (muon_data)[muref];

    mus_met_flag  ->push_back( muCorrData.type()  );
    mus_met_deltax->push_back( muCorrData.corrX() );
    mus_met_deltay->push_back( muCorrData.corrY() );
  }

  //MET from Towers
  const double etaborder = 1.479; //set where barrel ends in eta (+/- this)--this should be an entry in etaranges unless you want to mess up comparison with eta slices
  const double hf_eta = 3.; //HF eta
  double ecalmetx=0., ecalmety=0.;
  double hcalmetx=0., hcalmety=0.;
  double epmetx=0., epmety=0., emmetx=0., emmety=0.; //endcap plus and minus eta--both ecal and hcal
  double epecalx=0., epecaly=0., emecalx=0., emecaly=0.; //endcap ecal plus and minus eta
  double ephcalx=0., ephcaly=0., emhcalx=0., emhcaly=0.; //endcap hcal plus and minus eta
  const unsigned int N = 84; //number eta slices: 41 on each side, plus overflow on each side (just in case)
  double twretax[N], twretay[N]; //these six are for eta slices
  double ecaletax[N], ecaletay[N];
  double hcaletax[N], hcaletay[N];
  for( unsigned int j=0; j<N; j++ ) {
	twretax[j]=0.;
	twretay[j]=0.; 
	ecaletax[j]=0.;
	ecaletay[j]=0.;
	hcaletax[j]=0.;
	hcaletay[j]=0.;
  }
	  
  //ranges are symmetric about 0, last entry is upper edge of last tower.
  //this array has 83 entries, making 82 bins. So offset is 1 entry from the arrays above
  double etaranges[] = {-5.191, -4.889, -4.716, -4.538, -4.363, -4.191, -4.013, -3.839, -3.664, -3.489, -3.314, -3.139, -2.964, -2.853, -2.650, -2.5, -2.322, -2.172, -2.043, -1.93, -1.83, -1.74, -1.653, -1.566, -1.479, -1.392, -1.305, -1.218, -1.131, -1.044, -0.957, -0.879, -0.783, -0.696, -0.609, -0.522, -0.435, -0.348, -0.261, -0.174, -0.087, 0, 0.087, 0.174, 0.261, 0.348, 0.435, 0.522, 0.609, 0.696, 0.783, 0.879, 0.957, 1.044, 1.131, 1.218, 1.305, 1.392, 1.479, 1.566, 1.653, 1.74, 1.83, 1.93, 2.043, 2.172, 2.322, 2.5, 2.650, 2.853, 2.964, 3.139, 3.314, 3.489, 3.664, 3.839, 4.013, 4.191, 4.363, 4.538, 4.716, 4.889, 5.191};

  for(CaloTowerCollection::const_iterator it = h_caloTowers->begin();
      it != h_caloTowers->end(); it++) {
    
    if(it->et() < towerEtThreshold)
      continue;

    double phi   = it->phi();
    double eta   = it->eta();
    double emEt  = it->emEt();
    double hadEt = it->hadEt();
	if( TMath::Abs(eta) > hf_eta ) { //no ecal in HF (fix broken method)
	  emEt = 0.;
	  hadEt = it->et();
	}

    ecalmetx += emEt*cos(phi);
    ecalmety += emEt*sin(phi);
    hcalmetx += hadEt*cos(phi);
    hcalmety += hadEt*sin(phi);
	if( eta < -etaborder ) { //endcap (HE+HF) minus
	  emmetx += emEt*cos(phi) + hadEt*cos(phi);
	  emmety += emEt*sin(phi) + hadEt*sin(phi);
	  emecalx += emEt*cos(phi);
	  emecaly += emEt*sin(phi);
	  emhcalx += hadEt*cos(phi);
	  emhcaly += hadEt*sin(phi);
	}
	else if( eta >= etaborder ) {
	  epmetx += emEt*cos(phi) + hadEt*cos(phi);
	  epmety += emEt*sin(phi) + hadEt*sin(phi);
	  epecalx += emEt*cos(phi);
	  epecaly += emEt*sin(phi);
	  ephcalx += hadEt*cos(phi);
	  ephcaly += hadEt*sin(phi);
	}

	if( !make_eta_rings ) continue; //rest of loop is just eta rings, so skip if false

    int index = -1;
    for( unsigned int j=0; j<N-2; j++ ) { //see comments above, below
      if( eta < etaranges[0] ) //overflow negative eta
		index = 0;
      else if( eta >= etaranges[j] && eta < etaranges[j+1] )
		index = j + 1;      //don't j++ here bc that changes j--see comment above
      else if( eta >= etaranges[N-2] ) //overflow positive eta--warning: uses N (another -1 bc of c++ convention of starting at 0)
		index = N-1;        //warning: uses N
    }
    if( index == -1 ) { //to be safe
      cout << "METMaker: error in finding eta slice for tower " << it - h_caloTowers->begin() << endl;
      continue;
    }
    
    twretax[index]  += emEt*cos(phi) + hadEt*cos(phi);
    twretay[index]  += emEt*sin(phi) + hadEt*sin(phi);
    
    ecaletax[index] += emEt*cos(phi);
    ecaletay[index] += emEt*sin(phi);

    hcaletax[index] += hadEt*cos(phi);
    hcaletay[index] += hadEt*sin(phi);
  }	

  *evt_ecalmet  = sqrt( ecalmetx*ecalmetx + ecalmety*ecalmety );
  *evt_hcalmet  = sqrt( hcalmetx*hcalmetx + hcalmety*hcalmety );
  *evt_ecalmetPhi  = atan2( -ecalmety, -ecalmetx );
  *evt_hcalmetPhi  = atan2( -hcalmety, -hcalmetx );

  *evt_endcapp_met        = sqrt( epmetx*epmetx + epmety*epmety );
  *evt_endcapm_met        = sqrt( emmetx*emmetx + emmety*emmety );
  *evt_ecalendcapp_met    = sqrt( epecalx*epecalx + epecaly*epecaly );
  *evt_ecalendcapm_met    = sqrt( emecalx*emecalx + emecaly*emecaly );
  *evt_hcalendcapp_met    = sqrt( ephcalx*ephcalx + ephcaly*ephcaly ); 
  *evt_hcalendcapm_met    = sqrt( emhcalx*emhcalx + emhcaly*emhcaly ); 
  *evt_endcapp_metPhi     = atan2( -epmety, -epmetx );
  *evt_endcapm_metPhi     = atan2( -emmety, -emmetx );
  *evt_ecalendcapp_metPhi = atan2( -epecaly, -epecalx );
  *evt_ecalendcapm_metPhi = atan2( -emecaly, -emecalx );
  *evt_hcalendcapp_metPhi = atan2( -ephcaly, -ephcalx );
  *evt_hcalendcapm_metPhi = atan2( -emhcaly, -emhcalx ); 

  for( unsigned int i=0; i<N; i++ ) { //put all eta slices in vector
	if( !make_eta_rings ) break; //no point pushing if not putting
    evt_towermet_etaslice->push_back( sqrt( twretax[i]*twretax[i]   + twretay[i]*twretay[i] ) );
    evt_ecalmet_etaslice ->push_back( sqrt( ecaletax[i]*ecaletax[i] + ecaletay[i]*ecaletay[i] ) );
    evt_hcalmet_etaslice ->push_back( sqrt( hcaletax[i]*hcaletax[i] + hcaletay[i]*hcaletay[i] ) );
    evt_towermet_etaslicePhi->push_back( atan2( -twretay[i] , -twretax[i] ) );
    evt_ecalmet_etaslicePhi ->push_back( atan2( -ecaletay[i], -ecaletax[i] ) );
    evt_hcalmet_etaslicePhi ->push_back( atan2( -hcaletay[i], -hcaletax[i] ) );
  }

  iEvent.put(evt_met               ,"evtmet"              );
  iEvent.put(evt_metPhi            ,"evtmetPhi"           );
  iEvent.put(evt_metSig            ,"evtmetSig"           );
  iEvent.put(evt_metHO             ,"evtmetHO"            );
  iEvent.put(evt_metHOPhi          ,"evtmetHOPhi"         );
  iEvent.put(evt_metHOSig          ,"evtmetHOSig"         );
  iEvent.put(evt_metNoHF           ,"evtmetNoHF"          );
  iEvent.put(evt_metNoHFPhi        ,"evtmetNoHFPhi"       );
  iEvent.put(evt_metNoHFSig        ,"evtmetNoHFSig"       );
  iEvent.put(evt_metNoHFHO         ,"evtmetNoHFHO"        );
  iEvent.put(evt_metNoHFHOPhi      ,"evtmetNoHFHOPhi"     );
  iEvent.put(evt_metNoHFHOSig      ,"evtmetNoHFHOSig"     );

  iEvent.put(evt_metOpt            ,"evtmetOpt"           );
  iEvent.put(evt_metOptPhi         ,"evtmetOptPhi"        );
  iEvent.put(evt_metOptSig         ,"evtmetOptSig"        );
  iEvent.put(evt_metOptHO          ,"evtmetOptHO"         );
  iEvent.put(evt_metOptHOPhi       ,"evtmetOptHOPhi"      );
  iEvent.put(evt_metOptHOSig       ,"evtmetOptHOSig"      );
  iEvent.put(evt_metOptNoHF        ,"evtmetOptNoHF"       );
  iEvent.put(evt_metOptNoHFPhi     ,"evtmetOptNoHFPhi"    );
  iEvent.put(evt_metOptNoHFSig     ,"evtmetOptNoHFSig"    );
  iEvent.put(evt_metOptNoHFHO      ,"evtmetOptNoHFHO"     );
  iEvent.put(evt_metOptNoHFHOPhi   ,"evtmetOptNoHFHOPhi"  );
  iEvent.put(evt_metOptNoHFHOSig   ,"evtmetOptNoHFHOSig"  );
  
  iEvent.put(evt_metMuonCorr       ,"evtmetMuonCorr"      );
  iEvent.put(evt_metMuonCorrPhi    ,"evtmetMuonCorrPhi"   );
  iEvent.put(evt_metMuonCorrSig    ,"evtmetMuonCorrSig"   );
  iEvent.put(evt_metMuonJESCorr    ,"evtmetMuonJESCorr"   );
  iEvent.put(evt_metMuonJESCorrPhi ,"evtmetMuonJESCorrPhi");
  iEvent.put(evt_metMuonJESCorrSig ,"evtmetMuonJESCorrSig");

  iEvent.put(evt_sumet             ,"evtsumet"            );  
  iEvent.put(evt_sumetHO           ,"evtsumetHO"	  );
  iEvent.put(evt_sumetNoHF         ,"evtsumetNoHF"        );
  iEvent.put(evt_sumetNoHFHO       ,"evtsumetNoHFHO"      );
  iEvent.put(evt_sumetOpt          ,"evtsumetOpt"         );
  iEvent.put(evt_sumetOptHO        ,"evtsumetOptHO"       );
  iEvent.put(evt_sumetOptNoHF      ,"evtsumetOptNoHF"     );
  iEvent.put(evt_sumetOptNoHFHO    ,"evtsumetOptNoHFHO"   );
  iEvent.put(evt_sumetMuonCorr     ,"evtsumetMuonCorr"    );

  iEvent.put(mus_met_flag          ,"musmetflag"          );
  iEvent.put(mus_met_deltax        ,"musmetdeltax"        );
  iEvent.put(mus_met_deltay        ,"musmetdeltay"        );

  iEvent.put(evt_ecalmet           ,"evtecalmet"          );
  iEvent.put(evt_hcalmet           ,"evthcalmet"          );
  iEvent.put(evt_ecalmetPhi        ,"evtecalmetPhi"       );
  iEvent.put(evt_hcalmetPhi        ,"evthcalmetPhi"       );

  iEvent.put(evt_endcapp_met       ,  "evtendcappmet"  );
  iEvent.put(evt_endcapm_met       ,  "evtendcapmmet"  );
  iEvent.put(evt_ecalendcapp_met   ,  "evtecalendcappmet"  );
  iEvent.put(evt_ecalendcapm_met   ,  "evtecalendcapmmet"  );
  iEvent.put(evt_hcalendcapp_met   ,  "evthcalendcappmet"  );
  iEvent.put(evt_hcalendcapm_met   ,  "evthcalendcapmmet"  );
  iEvent.put(evt_endcapp_metPhi    ,  "evtendcappmetPhi"  );
  iEvent.put(evt_endcapm_metPhi    ,  "evtendcapmmetPhi"  );
  iEvent.put(evt_ecalendcapp_metPhi,  "evtecalendcappmetPhi"  );
  iEvent.put(evt_ecalendcapm_metPhi,  "evtecalendcapmmetPhi"  );
  iEvent.put(evt_hcalendcapp_metPhi,  "evthcalendcappmetPhi"  );
  iEvent.put(evt_hcalendcapm_metPhi,  "evthcalendcapmmetPhi"  );

  if( make_eta_rings ) {
	iEvent.put(evt_towermet_etaslice ,"evttowermetetaslice" );
	iEvent.put(evt_ecalmet_etaslice  ,"evtecalmetetaslice"  );
	iEvent.put(evt_hcalmet_etaslice  ,"evthcalmetetaslice"  );
	iEvent.put(evt_towermet_etaslicePhi, "evttowermetetaslicePhi" );
	iEvent.put(evt_ecalmet_etaslicePhi,  "evtecalmetetaslicePhi"  );
	iEvent.put(evt_hcalmet_etaslicePhi,  "evthcalmetetaslicePhi"  );
  }
  
}

//define this as a plug-in
DEFINE_FWK_MODULE(METMaker);




  
