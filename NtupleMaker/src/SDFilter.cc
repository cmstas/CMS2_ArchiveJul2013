//-*- C++ -*-
//
// Package:    SDFilter
// Class:      SDFilter
// 
/**\class SDFilter SDFilter.cc CMS2/src/SDFilter.cc

   Description: filter for cms2

   Implementation:
   see header file
*/
//
// Original Author:  Ingo Bloch
//         Created:  Fri Jun  6 11:07:38 CDT 2008
// $Id: SDFilter.cc,v 1.2 2010/06/14 13:06:46 dbarge Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/CaloMETFwd.h"
#include "DataFormats/METReco/interface/MET.h"
#include "DataFormats/METReco/interface/METFwd.h"
#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/PFMETFwd.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"

#include "CMS2/NtupleMaker/interface/SDFilter.h"

#include "DataFormats/Math/interface/LorentzVector.h"

typedef math::XYZTLorentzVectorF LorentzVector;
using namespace std;

//
// constructors and destructor
//

SDFilter::SDFilter(const edm::ParameterSet& iConfig) {
     elsInputTag     = iConfig.getParameter<edm::InputTag>("elsInputTag_"   );
     musInputTag     = iConfig.getParameter<edm::InputTag>("musInputTag_"   );
     pfjetsInputTag  = iConfig.getParameter<edm::InputTag>("pfjetsInputTag_");
     photonInputTag  = iConfig.getParameter<edm::InputTag>("photonInputTag_");
     metInputTag     = iConfig.getParameter<edm::InputTag>("metInputTag_"   );
     tcmetInputTag   = iConfig.getParameter<edm::InputTag>("tcmetInputTag_" );
     pfmetInputTag   = iConfig.getParameter<edm::InputTag>("pfmetInputTag_" );

     elsPt     = iConfig.getParameter<double>("elsPt_"   );
     musPt     = iConfig.getParameter<double>("musPt_"   );
     photonPt  = iConfig.getParameter<double>("photonPt_");
     pfjetPt   = iConfig.getParameter<double>("pfjetPt_" );
     metPt     = iConfig.getParameter<double>("metPt_"   );
     tcmetPt   = iConfig.getParameter<double>("tcmetPt_" );
     pfmetPt   = iConfig.getParameter<double>("pfmetPt_" );
}


SDFilter::~SDFilter() {}

void  SDFilter::beginJob() {
}

void SDFilter::endJob() {
}


// ------------ method called to produce the data  ------------
bool SDFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
     // get met collections
     edm::Handle<reco::CaloMETCollection> met_h;
     iEvent.getByLabel(metInputTag, met_h);
     float met = (met_h->front()).et();

     edm::Handle<reco::METCollection> tcmet_h;
     iEvent.getByLabel(tcmetInputTag, tcmet_h);
     float tcmet = (tcmet_h->front()).et();

     edm::Handle<reco::PFMETCollection> pfmet_h;
     iEvent.getByLabel(pfmetInputTag, pfmet_h);
     float pfmet = (pfmet_h->front()).et();

     if (met > metPt || tcmet > tcmetPt || pfmet > pfmetPt)
	  return true;

     edm::Handle<reco::GsfElectronCollection> els_h;
     iEvent.getByLabel(elsInputTag, els_h);

     for (reco::GsfElectronCollection::const_iterator iter = els_h->begin(); iter != els_h->end(); iter++)
     {
	  if (iter->pt() > elsPt)
	       return true;
     }

     edm::Handle<reco::MuonCollection> mus_h;
     iEvent.getByLabel(musInputTag, mus_h);

     for (reco::MuonCollection::const_iterator iter = mus_h->begin(); iter != mus_h->end(); iter++)
     {
	  if (iter->pt() > musPt)
	       return true;
     }

     edm::Handle<reco::PhotonCollection> photon_h;
     iEvent.getByLabel(photonInputTag, photon_h);

     for (reco::PhotonCollection::const_iterator iter = photon_h->begin(); iter != photon_h->end(); iter++)
     {
	  if (iter->pt() > photonPt)
	       return true;
     }

     edm::Handle<reco::PFJetCollection> pfjet_h;
     iEvent.getByLabel(pfjetsInputTag, pfjet_h);

     for (reco::PFJetCollection::const_iterator iter = pfjet_h->begin(); iter != pfjet_h->end(); iter++)
     {
	  if (iter->pt() > pfjetPt)
	       return true;
     }

     return false;
}

//define this as a plug-in
DEFINE_FWK_MODULE(SDFilter);





  