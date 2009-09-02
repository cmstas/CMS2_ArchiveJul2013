// -*- C++ -*-
//
// Package:    JPTMaker
// Class:      JPTMaker
// 
/**\class JPTMaker JPTMaker.cc CMS2/NtupleMaker/src/JPTMaker.cc

   Description: copy reco::CaloJet JPT variables in simple data structures into the EDM event tree

   Implementation:
   - take JPT jets
   - extract and fill variables
*/
//
// Original Frank Golf
// Created:  Sun Jan  18 12:23:38 CDT 2008
// $Id: JPTMaker.cc,v 1.10 2009/09/02 10:06:50 fgolf Exp $
//
//


// system include files
#include <memory>
#include <vector>
#include <map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/JetReco/interface/CaloJet.h"

#include "CMS2/NtupleMaker/interface/JPTMaker.h"

typedef math::XYZTLorentzVector LorentzVector;

bool sortJptsByPt(reco::CaloJet jet1, reco::CaloJet jet2) {
  return jet1.pt() > jet2.pt();
}

//
// class decleration
//

//
// constructors and destructor
//

JPTMaker::JPTMaker(const edm::ParameterSet& iConfig)
{
  // product of this EDProducer
  produces<unsigned int>                ("evtnjpts"      ).setBranchAlias("evt_njpts"      );
  produces<std::vector<LorentzVector> >	("jptsp4"        ).setBranchAlias("jpts_p4"        );
  produces<std::vector<float> >	        ("jptsemFrac"    ).setBranchAlias("jpts_emFrac"    );
  produces<std::vector<float> >	        ("jptscor"       ).setBranchAlias("jpts_cor"       ); // ratio of L2L3 corrected JPT jet to uncorrected JPT jet

  // parameters from configuration
  jptsInputTag      = iConfig.getParameter<edm::InputTag>("jptInputTag"       );
  L2L3jptsInputTag  = iConfig.getParameter<edm::InputTag>("L2L3jptInputTag"   );

}

JPTMaker::~JPTMaker()
{
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void JPTMaker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::auto_ptr<unsigned int>                evt_njpts          (new unsigned int               );
  std::auto_ptr<std::vector<LorentzVector> > vector_jpts_p4     (new std::vector<LorentzVector> );
  std::auto_ptr<std::vector<float> >         vector_jpts_emFrac (new std::vector<float>         );
  std::auto_ptr<std::vector<float> >         vector_jpts_cor    (new std::vector<float>         );

  std::map<float, float> L2L3corJPT;

  edm::Handle<std::vector<reco::CaloJet> > jptsHandle;
  iEvent.getByLabel(jptsInputTag, jptsHandle); 

  if( !jptsHandle.isValid() ) {
    edm::LogInfo("OutputInfo") << " failed to retrieve JPT collection";
    edm::LogInfo("OutputInfo") << " JPTMaker cannot continue...!";
    return;
  }

  edm::Handle<std::vector<reco::CaloJet> > L2L3jptsHandle;
  iEvent.getByLabel(L2L3jptsInputTag , L2L3jptsHandle);

  if( !L2L3jptsHandle.isValid() ) {
    edm::LogInfo("OutputInfo") << " failed to retrieve L2L3 corrected JPT collection";
    edm::LogInfo("OutputInfo") << " JPTMaker cannot continue...!";
    return;
  }

  *evt_njpts = jptsHandle->size();

  std::vector<reco::CaloJet> v_jpts      = *( jptsHandle.product()      );
  std::vector<reco::CaloJet> v_L2L3jpts  = *( L2L3jptsHandle.product()  );

  std::sort( v_jpts.begin(), v_jpts.end(), sortJptsByPt );

  std::vector<reco::CaloJet>::const_iterator jptcor = v_L2L3jpts.begin();

  for ( std::vector<reco::CaloJet>::const_iterator jpt = v_jpts.begin(); jpt != v_jpts.end(); ++jpt, ++jptcor ) {

    vector_jpts_p4     ->push_back( jpt->p4()                           );
    vector_jpts_emFrac ->push_back( jpt->emEnergyFraction()             );
    L2L3corJPT[ jpt->p4().pt() ] = ( jptcor->p4().Et() / jpt->p4().Et() );
  }

  for( std::map<float, float>::const_iterator iter = L2L3corJPT.begin(); iter != L2L3corJPT.end(); ++iter ) {
    vector_jpts_cor->push_back( iter->second );
  }

  // put containers into event
  iEvent.put(evt_njpts          , "evtnjpts"  );
  iEvent.put(vector_jpts_p4     , "jptsp4"    );
  iEvent.put(vector_jpts_emFrac , "jptsemFrac");
  iEvent.put(vector_jpts_cor    , "jptscor"   );
}

// ------------ method called once each job just before starting event loop  ------------
void JPTMaker::beginJob(const edm::EventSetup&) {
}

// ------------ method called once each job just after ending the event loop  ------------
void JPTMaker::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(JPTMaker);
