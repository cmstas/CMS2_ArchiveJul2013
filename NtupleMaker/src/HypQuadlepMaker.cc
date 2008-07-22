// -*- C++ -*-
//
// Package:    HypQuadlepMaker
// Class:      HypQuadlepMaker
// 
/**\class HypQuadlepMaker HypQuadlepMaker.cc CMS2/NtupleMaker/src/HypQuadlepMaker.cc

Description: create quadlepton hypothesis branches

Implementation:
- combine muons and electrons after preselection
- correct jets and store index vectors
- correct met

Hypothesis:
- lepton type: 1:m+, -1:m-, 2:e+, -2:e-
- quadlepton 20 buckets:
m+m+m+m+: 0
m+m+m+m-: 1
m+m+m-m-: 2
m+m-m-m-: 3
m-m-m-m-: 4
m+m+m+e+: 5
m+m+m+e-: 6
m+m+m-e+: 7
m+m+m-e-: 8
m+m-m-e+: 9
m+m-m-e-: 10
m-m-m-e+: 11
m-m-m-e-: 12
m+m+e+e+: 13
m+m+e+e-: 14
m+m+e-e-: 15
m+m-e+e+: 16
m+m-e+e-: 17
m+m-e-e-: 18
m-m-e+e+: 19
m-m-e+e-: 20
m-m-e-e-: 21
m+e+e+e+: 22
m+e+e+e-: 23
m+e+e-e-: 24
m+e-e-e-: 25
m-e+e+e+: 26
m-e+e+e-: 27
m-e+e-e-: 28
m-e-e-e-: 29
e+e+e+e+: 30
e+e+e+e-: 31
e+e+e-e-: 32
e+e-e-e-: 33
e-e-e-e-: 34

- quadlepton hypothesis first/second/third/fourth follows bucket definition. In the case of the same type, ordering follows descending index number within muon and electron collection.

*/
//
// Original Author:  Oliver Gutsche
//         Created:  Sat Jul 19 00:16:28 UTC 2008
// $Id: HypQuadlepMaker.cc,v 1.2 2008/07/22 18:54:56 gutsche Exp $
//
//

// system include files
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CMS2/NtupleMaker/interface/HypQuadlepMaker.h"


#include "CMS2/NtupleMaker/interface/MatchUtilities.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

//
// class decleration
//

//
// constructors and destructor
//
HypQuadlepMaker::HypQuadlepMaker(const edm::ParameterSet& iConfig)
{
  // product of this EDProducer
  // 
  // quadlepton hyptothesis
  //
  produces<std::vector<unsigned int> > ("hypquadlepbucket").setBranchAlias("hyp_quadlep_bucket");            // quadlepton bucket
  produces<std::vector<int> >          ("hypquadlepfirsttype").setBranchAlias("hyp_quadlep_first_type");     // type of the first lepton in the quadlepton hypothesis (1: muon, 2: electron)
  produces<std::vector<unsigned int> > ("hypquadlepfirstindex").setBranchAlias("hyp_quadlep_first_index");   // index of first lepton in lepton collection
  produces<std::vector<int> >          ("hypquadlepsecondtype").setBranchAlias("hyp_quadlep_second_type");   // type of the second lepton in the quadlepton hypothesis (1: muon, 2: electron)
  produces<std::vector<unsigned int> > ("hypquadlepsecondindex").setBranchAlias("hyp_quadlep_second_index"); // index of second lepton in lepton collection
  produces<std::vector<int> >          ("hypquadlepthirdtype").setBranchAlias("hyp_quadlep_third_type");     // type of the third lepton in the quadlepton hypothesis (1: muon, 2: electron)
  produces<std::vector<unsigned int> > ("hypquadlepthirdindex").setBranchAlias("hyp_quadlep_third_index");   // index of third lepton in lepton collection
  produces<std::vector<int> >          ("hypquadlepfourthtype").setBranchAlias("hyp_quadlep_fourth_type");   // type of the fourth lepton in the quadlepton hypothesis (1: muon, 2: electron)
  produces<std::vector<unsigned int> > ("hypquadlepfourthindex").setBranchAlias("hyp_quadlep_fourth_index"); // index of fourth lepton in lepton collection

  // parameters from configuration
  muonsInputTag = iConfig.getParameter<edm::InputTag>("muonsInputTag");
  electronsInputTag = iConfig.getParameter<edm::InputTag>("electronsInputTag");

}


HypQuadlepMaker::~HypQuadlepMaker()
{
 
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
HypQuadlepMaker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  //input collections

  // muon charge
  edm::InputTag mus_charge_tag(muonsInputTag.label(),"muscharge");
  edm::Handle<std::vector<int> > mus_charge_h;
  iEvent.getByLabel(mus_charge_tag, mus_charge_h);
  const std::vector<int> *mus_charge = mus_charge_h.product();

  // electron charge
  edm::InputTag els_charge_tag(electronsInputTag.label(),"elscharge");
  edm::Handle<std::vector<int> > els_charge_h;
  iEvent.getByLabel(els_charge_tag, els_charge_h);
  const std::vector<int> *els_charge = els_charge_h.product();

  // number of electrons
  unsigned int evt_nels = els_charge->size();

  // number of electrons
  unsigned int evt_nmus = mus_charge->size();

  // output collections
  std::auto_ptr<std::vector<unsigned int> > vector_hyp_quadlep_bucket(new std::vector<unsigned int>);
  std::auto_ptr<std::vector<int> > vector_hyp_quadlep_first_type(new std::vector<int>);
  std::auto_ptr<std::vector<unsigned int> > vector_hyp_quadlep_first_index(new std::vector<unsigned int>);
  std::auto_ptr<std::vector<int> > vector_hyp_quadlep_second_type(new std::vector<int>);
  std::auto_ptr<std::vector<unsigned int> > vector_hyp_quadlep_second_index(new std::vector<unsigned int>);
  std::auto_ptr<std::vector<int> > vector_hyp_quadlep_third_type(new std::vector<int>);
  std::auto_ptr<std::vector<unsigned int> > vector_hyp_quadlep_third_index(new std::vector<unsigned int>);
  std::auto_ptr<std::vector<int> > vector_hyp_quadlep_fourth_type(new std::vector<int>);
  std::auto_ptr<std::vector<unsigned int> > vector_hyp_quadlep_fourth_index(new std::vector<unsigned int>);

  // check for numbers of electrons/muons
  // if more than 99 electrons of 99 muons, skip event and fill empty vectors
  if ( evt_nels > 99 ) {
    edm::LogWarning("HypQuadlepMaker") << "more than 99 electrons, skipping event!!!";
    // put empty containers into event
    iEvent.put(vector_hyp_quadlep_bucket,"hypquadlepbucket");
    iEvent.put(vector_hyp_quadlep_first_type,"hypquadlepfirsttype");
    iEvent.put(vector_hyp_quadlep_first_index,"hypquadlepfirstindex");
    iEvent.put(vector_hyp_quadlep_second_type,"hypquadlepsecondtype");
    iEvent.put(vector_hyp_quadlep_second_index,"hypquadlepsecondindex");
    iEvent.put(vector_hyp_quadlep_third_type,"hypquadlepthirdtype");
    iEvent.put(vector_hyp_quadlep_third_index,"hypquadlepthirdindex");
    iEvent.put(vector_hyp_quadlep_fourth_type,"hypquadlepfourthtype");
    iEvent.put(vector_hyp_quadlep_fourth_index,"hypquadlepfourthindex");
    return;
  } else if ( evt_nmus > 99 ) {
    edm::LogWarning("HypQuadlepMaker") << "more than 99 muons, skipping event!!!";
    // put empty containers into event
    iEvent.put(vector_hyp_quadlep_bucket,"hypquadlepbucket");
    iEvent.put(vector_hyp_quadlep_first_type,"hypquadlepfirsttype");
    iEvent.put(vector_hyp_quadlep_first_index,"hypquadlepfirstindex");
    iEvent.put(vector_hyp_quadlep_second_type,"hypquadlepsecondtype");
    iEvent.put(vector_hyp_quadlep_second_index,"hypquadlepsecondindex");
    iEvent.put(vector_hyp_quadlep_third_type,"hypquadlepthirdtype");
    iEvent.put(vector_hyp_quadlep_third_index,"hypquadlepthirdindex");
    iEvent.put(vector_hyp_quadlep_fourth_type,"hypquadlepfourthtype");
    iEvent.put(vector_hyp_quadlep_fourth_index,"hypquadlepfourthindex");
    return;
  }

  // processed quadlepton candidates 
  // ordered: first by type (1st muon, 2nd electron), then by index number
  std::vector<unsigned int> cand;

  // arrays for sorts
  unsigned int sorter[4] = {0,0,0,0};
  unsigned int sorter2[4] = {0,0,0,0};

  // m
  for (unsigned int firstMuon = 0; firstMuon < evt_nmus; ++firstMuon) {
    // m
    for (unsigned int secondMuon = 0; secondMuon < evt_nmus; ++secondMuon) {
      if ( secondMuon == firstMuon ) continue;
      // m
      for (unsigned int thirdMuon = 0; thirdMuon < evt_nmus; ++thirdMuon) {
	if ( thirdMuon == firstMuon || thirdMuon == secondMuon ) continue;
	// m
	for (unsigned int fourthMuon = 0; fourthMuon < evt_nmus; ++fourthMuon) {
	  if ( fourthMuon == firstMuon || fourthMuon == secondMuon || fourthMuon == thirdMuon ) continue;
	  sorter[0] = firstMuon;
	  sorter[1] = secondMuon;
	  sorter[2] = thirdMuon;
	  sorter[3] = fourthMuon;
	  std::sort(sorter,sorter+4);
	  unsigned int candIndex = encodeQuadleptonCandidate(0,
							     sorter[0],
							     sorter[1],
							     sorter[2],
							     sorter[3]);
	  if ( find(cand.begin(),cand.end(), candIndex) == cand.end() ) {
	    cand.push_back(candIndex);
	    int charge = mus_charge->at(sorter[0]) + mus_charge->at(sorter[1]) + mus_charge->at(sorter[2]) + mus_charge->at(sorter[3]);
	    if ( charge == 4 ) {
	      // m+m+m+m+
	      vector_hyp_quadlep_bucket->push_back(0);
	    } else if ( charge == 2) {
	      // m+m+m+m-
	      vector_hyp_quadlep_bucket->push_back(1);
	    } else if ( charge == 0) {
	      // m+m+m-m-
	      vector_hyp_quadlep_bucket->push_back(2);
	    } else if ( charge == -2 ) {
	      // m+m-m-m-
	      vector_hyp_quadlep_bucket->push_back(3);
	    } else if ( charge == -4 ) {
	      // m-m-m-m-
	      vector_hyp_quadlep_bucket->push_back(4);
	    } else {
	      edm::LogError("HypQuadlepMaker") << "combineQuadleptons mmmm : charge combination could not be identified!!!";
	    }
	    vector_hyp_quadlep_first_type->push_back(mus_charge->at(sorter[0]));
	    vector_hyp_quadlep_first_index->push_back(sorter[0]);
	    vector_hyp_quadlep_second_type->push_back(mus_charge->at(sorter[1]));
	    vector_hyp_quadlep_second_index->push_back(sorter[1]);
	    vector_hyp_quadlep_third_type->push_back(mus_charge->at(sorter[2]));
	    vector_hyp_quadlep_third_index->push_back(sorter[2]);
	    vector_hyp_quadlep_fourth_type->push_back(mus_charge->at(sorter[3]));
	    vector_hyp_quadlep_fourth_index->push_back(sorter[3]);
	  }
	}
	// e
	for (unsigned int fourthElectron = 0; fourthElectron < evt_nels; ++fourthElectron) {
	  // order muon indices
	  sorter[0] = firstMuon;
	  sorter[1] = secondMuon;
	  sorter[2] = thirdMuon;
	  std::sort(sorter,sorter+3);
	  // add electron
	  sorter[3] = fourthElectron;
	  unsigned int candIndex = encodeQuadleptonCandidate(1,
							     sorter[0],
							     sorter[1],
							     sorter[2],
							     sorter[3]);
	  if ( find(cand.begin(),cand.end(), candIndex) == cand.end() ) {
	    cand.push_back(candIndex);
	    int charge = mus_charge->at(sorter[0]) + mus_charge->at(sorter[1]) + mus_charge->at(sorter[2]);
	    if ( charge == 3 ) {
	      if ( els_charge->at(sorter[3]) > 0 ) {
		// m+m+m+e+
		vector_hyp_quadlep_bucket->push_back(5);
	      } else {
		// m+m+m+e-
		vector_hyp_quadlep_bucket->push_back(6);
	      }
	    } else if ( charge == 1) {
	      if ( els_charge->at(sorter[3]) > 0 ) {
		// m+m+m-e+
		vector_hyp_quadlep_bucket->push_back(7);
	      } else {
		// m+m+m-e-
		vector_hyp_quadlep_bucket->push_back(8);
	      }
	    } else if ( charge == -1 ) {
	      if ( els_charge->at(sorter[3]) > 0 ) {
		// m+m-m-e-
		vector_hyp_quadlep_bucket->push_back(9);
	      } else {
		// m+m-m-e-
		vector_hyp_quadlep_bucket->push_back(10);
	      }
	    } else if ( charge == -3 ) {
	      if ( els_charge->at(sorter[3]) > 0 ) {
		// m-m-m-e-
		vector_hyp_quadlep_bucket->push_back(11);
	      } else {
		// m-m-m-e-
		vector_hyp_quadlep_bucket->push_back(12);
	      }
	    } else {
	      edm::LogError("HypQuadlepMaker") << "combineQuadleptons mmme: charge combination could not be identified!!!";
	    }
	    vector_hyp_quadlep_first_type->push_back(mus_charge->at(sorter[0]));
	    vector_hyp_quadlep_first_index->push_back(sorter[0]);
	    vector_hyp_quadlep_second_type->push_back(mus_charge->at(sorter[1]));
	    vector_hyp_quadlep_second_index->push_back(sorter[1]);
	    vector_hyp_quadlep_third_type->push_back(mus_charge->at(sorter[2]));
	    vector_hyp_quadlep_third_index->push_back(sorter[2]);
	    vector_hyp_quadlep_fourth_type->push_back(els_charge->at(sorter[3])*2);
	    vector_hyp_quadlep_fourth_index->push_back(sorter[3]);
	  }
	}
      }
      // e
      for (unsigned int thirdElectron = 0; thirdElectron < evt_nels; ++thirdElectron) {
	// e
	for (unsigned int fourthElectron = 0; fourthElectron < evt_nels; ++fourthElectron) {
	  if ( fourthElectron == thirdElectron ) continue;
	  // order electron indices
	  sorter2[0] = thirdElectron;
	  sorter2[1] = fourthElectron;
	  std::sort(sorter2,sorter2+2);
	  // sort muons
	  sorter[0] = firstMuon;
	  sorter[1] = secondMuon;
	  std::sort(sorter,sorter+2);
	  // add electrons
	  sorter[2] = sorter2[0];
	  sorter[3] = sorter2[1];
	  unsigned int candIndex = encodeQuadleptonCandidate(2,
							     sorter[0],
							     sorter[1],
							     sorter[2],
							     sorter[3]);
	  if ( find(cand.begin(),cand.end(), candIndex) == cand.end() ) {
	    cand.push_back(candIndex);
	    // check for charge to identify bucket number
	    int muCharge = mus_charge->at(sorter[0]) + mus_charge->at(sorter[1]);
	    int elCharge = els_charge->at(sorter[2]) + els_charge->at(sorter[3]);
	    if ( muCharge == 2 ) {
	      if ( elCharge == 2 ) {
		// m+m+e+e+
		vector_hyp_quadlep_bucket->push_back(13);
	      } else if ( elCharge == 0 ) {
		// m+m+e+e-
		vector_hyp_quadlep_bucket->push_back(14);
	      } else if ( elCharge == -2 ) {
		// m+m+e-e-
		vector_hyp_quadlep_bucket->push_back(15);
	      } else {
		edm::LogError("HypQuadlepMaker") << "combineQuadleptons m+m+ee: charge combination could not be identified!!!";
	      }
	    } else if ( muCharge == 0 ) {
	      if ( elCharge == 2 ) {
		// m+m-e+e+
		vector_hyp_quadlep_bucket->push_back(16);
	      } else if ( elCharge == 0 ) {
		// m+m-e+e-
		vector_hyp_quadlep_bucket->push_back(17);
	      } else if ( elCharge == -2 ) {
		// m+m-e-e-
		vector_hyp_quadlep_bucket->push_back(18);
	      } else {
		edm::LogError("HypQuadlepMaker") << "combineQuadleptons m+m-ee: charge combination could not be identified!!!";
	      }
	    } else if ( muCharge == -2 ) {
	      if ( elCharge == 2 ) {
		// m-m-e+e+
		vector_hyp_quadlep_bucket->push_back(19);
	      } else if ( elCharge == 0 ) {
		// m-m-e+e-
		vector_hyp_quadlep_bucket->push_back(20);
	      } else if ( elCharge == -2 ) {
		// m-m-e-e-
		vector_hyp_quadlep_bucket->push_back(21);
	      } else {
		edm::LogError("HypQuadlepMaker") << "combineQuadleptons m-m-ee: charge combination could not be identified!!!";
	      }
	    } else {
	      edm::LogError("HypQuadlepMaker") << "combineQuadleptons mmee: charge combination could not be identified!!!";
	    }
	    vector_hyp_quadlep_first_type->push_back(mus_charge->at(sorter[0]));
	    vector_hyp_quadlep_first_index->push_back(sorter[0]);
	    vector_hyp_quadlep_second_type->push_back(mus_charge->at(sorter[1]));
	    vector_hyp_quadlep_second_index->push_back(sorter[1]);
	    vector_hyp_quadlep_third_type->push_back(els_charge->at(sorter[2])*2);
	    vector_hyp_quadlep_third_index->push_back(sorter[2]);
	    vector_hyp_quadlep_fourth_type->push_back(els_charge->at(sorter[3])*2);
	    vector_hyp_quadlep_fourth_index->push_back(sorter[3]);
	  }
	}
      }
    }
    // e
    for (unsigned int secondElectron = 0; secondElectron < evt_nels; ++secondElectron) {
      // e
      for (unsigned int thirdElectron = 0; thirdElectron < evt_nels; ++thirdElectron) {
	if ( thirdElectron == secondElectron ) continue;
	// e
	for (unsigned int fourthElectron = 0; fourthElectron < evt_nels; ++fourthElectron) {
    	  if ( fourthElectron == thirdElectron || fourthElectron == secondElectron ) continue;
	  // sort electrons
	  sorter2[0] = secondElectron;
	  sorter2[1] = thirdElectron;
	  sorter2[2] = fourthElectron;
	  std::sort(sorter2,sorter2+3);
	  // add muon at first place
	  sorter[0] = firstMuon;
	  sorter[1] = sorter2[0];
	  sorter[2] = sorter2[1];
	  sorter[3] = sorter2[2];
	  unsigned int candIndex = encodeQuadleptonCandidate(3,
							     sorter[0],
							     sorter[1],
							     sorter[2],
							     sorter[3]);
	  if ( find(cand.begin(),cand.end(), candIndex) == cand.end() ) {
	    cand.push_back(candIndex);
	    int charge = els_charge->at(sorter[1]) + els_charge->at(sorter[2]) + els_charge->at(sorter[3]);
	    if ( mus_charge->at(sorter[0]) > 0 ) {
	      if ( charge == 3 ) {
		// m+e+e+e+
		vector_hyp_quadlep_bucket->push_back(22);
	      } else if ( charge == 1) {
		// m+e+e+e-
		vector_hyp_quadlep_bucket->push_back(23);
	      } else if ( charge == -1 ) {
		// m+e+e-e-
		vector_hyp_quadlep_bucket->push_back(24);
	      } else if ( charge == -3 ) {
		// m+e-e-e-
		vector_hyp_quadlep_bucket->push_back(25);
	      } else {
		edm::LogError("HypQuadlepMaker") << "combineQuadleptons m+eee: charge combination could not be identified!!!";
	      }
	    } else if ( mus_charge->at(sorter[0]) < 0 ) {
	      if ( charge == 3 ) {
		// m-e+e+e+
		vector_hyp_quadlep_bucket->push_back(26);
	      } else if ( charge == 1) {
		// m-e+e+e-
		vector_hyp_quadlep_bucket->push_back(27);
	      } else if ( charge == -1 ) {
		// m-e+e-e-
		vector_hyp_quadlep_bucket->push_back(28);
	      } else if ( charge == -3 ) {
		// m-e-e-e-
		vector_hyp_quadlep_bucket->push_back(29);
	      } else {
		edm::LogError("HypQuadlepMaker") << "combineQuadleptons m-eee: charge combination could not be identified!!!";
	      }
	    } else {
	      edm::LogError("HypQuadlepMaker") << "combineQuadleptons meee: charge combination could not be identified!!!";
	    }
	    vector_hyp_quadlep_first_type->push_back(mus_charge->at(sorter[0]));
	    vector_hyp_quadlep_first_index->push_back(sorter[0]);
	    vector_hyp_quadlep_second_type->push_back(els_charge->at(sorter[1])*2);
	    vector_hyp_quadlep_second_index->push_back(sorter[1]);
	    vector_hyp_quadlep_third_type->push_back(els_charge->at(sorter[2])*2);
	    vector_hyp_quadlep_third_index->push_back(sorter[2]);
	    vector_hyp_quadlep_fourth_type->push_back(els_charge->at(sorter[3])*2);
	    vector_hyp_quadlep_fourth_index->push_back(sorter[3]);
	  }
	}
      }
    }
  }
	  
  // e
  for (unsigned int firstElectron = 0; firstElectron < evt_nmus; ++firstElectron) {
    // e
    for (unsigned int secondElectron = 0; secondElectron < evt_nmus; ++secondElectron) {
      if ( secondElectron == firstElectron ) continue;
      // e
      for (unsigned int thirdElectron = 0; thirdElectron < evt_nmus; ++thirdElectron) {
	if ( thirdElectron == firstElectron || thirdElectron == secondElectron ) continue;
	// e
	for (unsigned int fourthElectron = 0; fourthElectron < evt_nmus; ++fourthElectron) {
	  if ( fourthElectron == firstElectron || fourthElectron == secondElectron || fourthElectron == thirdElectron ) continue;
	  sorter[0] = firstElectron;
	  sorter[1] = secondElectron;
	  sorter[2] = thirdElectron;
	  sorter[3] = fourthElectron;
	  std::sort(sorter,sorter+4);
	  unsigned int candIndex = encodeQuadleptonCandidate(4,
							     sorter[0],
							     sorter[1],
							     sorter[2],
							     sorter[3]);
	  if ( find(cand.begin(),cand.end(), candIndex) == cand.end() ) {
	    cand.push_back(candIndex);
	    int charge = els_charge->at(sorter[0]) + els_charge->at(sorter[1]) + els_charge->at(sorter[2]) + els_charge->at(sorter[3]);
	    if ( charge == 4 ) {
	      // e+e+e+e+
	      vector_hyp_quadlep_bucket->push_back(30);
	    } else if ( charge == 2) {
	      // e+e+e+e-
	      vector_hyp_quadlep_bucket->push_back(31);
	    } else if ( charge == 0) {
	      // e+e+e-e-
	      vector_hyp_quadlep_bucket->push_back(32);
	    } else if ( charge == -2 ) {
	      // e+e-e-e-
	      vector_hyp_quadlep_bucket->push_back(33);
	    } else if ( charge == -4 ) {
	      // e-e-e-e-
	      vector_hyp_quadlep_bucket->push_back(34);
	    } else {
	      edm::LogError("HypQuadlepMaker") << "combineQuadleptons eeee : charge combination could not be identified!!!";
	    }
	    vector_hyp_quadlep_first_type->push_back(els_charge->at(sorter[0])*2);
	    vector_hyp_quadlep_first_index->push_back(sorter[0]);
	    vector_hyp_quadlep_second_type->push_back(els_charge->at(sorter[1])*2);
	    vector_hyp_quadlep_second_index->push_back(sorter[1]);
	    vector_hyp_quadlep_third_type->push_back(els_charge->at(sorter[2])*2);
	    vector_hyp_quadlep_third_index->push_back(sorter[2]);
	    vector_hyp_quadlep_fourth_type->push_back(els_charge->at(sorter[3])*2);
	    vector_hyp_quadlep_fourth_index->push_back(sorter[3]);
	  }
	}
      }
    }
  }

  // put containers into event
  iEvent.put(vector_hyp_quadlep_bucket,"hypquadlepbucket");
  iEvent.put(vector_hyp_quadlep_first_type,"hypquadlepfirsttype");
  iEvent.put(vector_hyp_quadlep_first_index,"hypquadlepfirstindex");
  iEvent.put(vector_hyp_quadlep_second_type,"hypquadlepsecondtype");
  iEvent.put(vector_hyp_quadlep_second_index,"hypquadlepsecondindex");
  iEvent.put(vector_hyp_quadlep_third_type,"hypquadlepthirdtype");
  iEvent.put(vector_hyp_quadlep_third_index,"hypquadlepthirdindex");
  iEvent.put(vector_hyp_quadlep_fourth_type,"hypquadlepfourthtype");
  iEvent.put(vector_hyp_quadlep_fourth_index,"hypquadlepfourthindex");
}

// ------------ method called once each job just before starting event loop  ------------
void 
HypQuadlepMaker::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HypQuadlepMaker::endJob() {
}

unsigned int HypQuadlepMaker::encodeQuadleptonCandidate(unsigned int combination, unsigned int first, unsigned int second, unsigned int third, unsigned int fourth) {
  // encode quadlepton candidate according to
  //
  // quadlepton candidate is identified by coded unsigned integer: ABBCCDDEE
  //
  //  A: quad lepton combination: mmmm: 0, mmme: 1, mmee: 2, meee: 3, eeee: 4
  // BB: index of first lepton from els and mus collection
  // CC: index of second lepton from els and mus collection
  // DD: index of third lepton from els and mus collection
  // EE: index of fourth lepton from els and mus collection
  //
  // flavors are ordered: first mu, then el
  // same flavor leptons in candidate are ordered by increasing index
  //

  return combination * 10000000 + first * 1000000 + second * 10000 + third * 100 + fourth;
}

//define this as a plug-in
DEFINE_FWK_MODULE(HypQuadlepMaker);
