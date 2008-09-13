// -*- C++ -*-
//
// Package:    NtupleMaker
// Class:      JetToMuAssMaker
// 
/**\class JetToMuAssMaker JetToMuAssMaker.h CMS2/NtupleMaker/interface/JetToMuAssMaker.h

Description: make associations between jets and muons

*/
//
// Original Author:  Oliver Gutsche
//         Created:  Tue Jun 17 20:40:42 UTC 2008
// $Id: JetToMuAssMaker.h,v 1.2 2008/09/13 08:07:22 jmuelmen Exp $
//
//
#ifndef CMS2_JETTOMUASSMAKER_H
#define CMS2_JETTOMUASSMAKER_H

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

//
// class declaration
//

class JetToMuAssMaker : public edm::EDProducer {
public:
  explicit JetToMuAssMaker (const edm::ParameterSet&);

private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
      
  // ----------member data ---------------------------
  double m_minDR;
  
};

#endif
