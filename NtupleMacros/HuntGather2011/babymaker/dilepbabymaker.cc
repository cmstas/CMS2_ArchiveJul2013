#include "babymakercommon.h"
#include "dilepbabymaker.h" 

#include <algorithm>
#include <iostream>

#include "TChain.h"
#include "TCollection.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TObjArray.h"
#include "TTree.h"

// CMS2 CORE includes 
#include "CORE/MT2/MT2.h"
#include "CORE/CMS2.h"
#include "CORE/mcSelections.h"
#include "CORE/electronSelections.h"
#include "CORE/electronSelectionsParameters.h"
#include "CORE/metSelections.h"
#include "CORE/muonSelections.h"
#include "CORE/trackSelections.h"
#include "CORE/ssSelections.h"
#include "CORE/eventSelections.h"

// tools include is for
// the duplicate cleaning
#include "Tools/tools.cc"

void dilepbabymaker::ScanChain (const char *inputFilename, const char *babyFilename, int nEvents)
{
    TChain *chain = new TChain("Events");
    chain->Add(inputFilename);
    TObjArray *listOfFiles = chain->GetListOfFiles();    

    unsigned int nEventsChain=0;
    if (nEvents==-1) 
        nEvents = chain->GetEntries();
    nEventsChain = nEvents;
    unsigned int nEventsTotal = 0;

    // make a baby ntuple
    MakeBabyNtuple(babyFilename);

    // file loop
    TIter fileIter(listOfFiles);
    TFile *currentFile = 0;
    while ((currentFile = (TFile*)fileIter.Next()))
    {
        TFile f(currentFile->GetTitle());
        TTree *tree = (TTree*)f.Get("Events");
        cms2.Init(tree);

        //Event Loop
        unsigned int nEvents = tree->GetEntries();
        for(unsigned int event = 0; event < nEvents; ++event)
        {
            cms2.GetEntry(event);
            ++nEventsTotal;

            // call progress counted
            CMS2::progress(nEventsTotal, nEventsChain );

            // dilepton hypothesis stuff
            for (unsigned hypi = 0; hypi < cms2.hyp_p4().size(); ++hypi)
            {

                //
                // select a basic 10/10 hypothesis
                //

                if (min(cms2.hyp_lt_p4()[hypi].pt(), cms2.hyp_ll_p4()[hypi].pt()) < 10.)
                    continue;

                int index1 = cms2.hyp_lt_index()[hypi];
                int index2 = cms2.hyp_ll_index()[hypi];

                if (abs(cms2.hyp_lt_id()[hypi]) == 13 && !(cms2.mus_type()[index1] & 6))
                    continue;
                if (abs(cms2.hyp_ll_id()[hypi]) == 13 && !(cms2.mus_type()[index2] & 6))
                    continue;

                //
                // duplicate removal for data
                //

                if (cms2.evt_isRealData()) {
                    DorkyEventIdentifier id(cms2.evt_run(), cms2.evt_event(), cms2.evt_lumiBlock());
                    if (is_duplicate(id)) {
                        continue;
                    }
                }

                //
                // initialize baby quantities
                //

                InitBabyNtuple();

                //
                // Fill event level information
                //

                SetEventLevelInfo();

                // 
                // Fill MET information
                //

                pfmet_      = cms2.evt_pfmet();

                float theTCMetPhi     = 0.0;
                if (cms2.evt_CMS2tag() != "V03-06-14" && cms2.evt_dataset() != "/Mu/Run2010B-Nov4ReReco_v1/RECO") {
                    tcmet_          = cms2.evt_pf_tcmet();
                    theTCMetPhi     = cms2.evt_pf_tcmetPhi();
                }
                calotcmet_  = cms2.evt_tcmet();

                float thePFMetPhi     = cms2.evt_pfmetPhi();
                float theCaloTCMetPhi = cms2.evt_tcmetPhi();

                float metx  = tcmet_ * cos(theTCMetPhi);
                float mety  = tcmet_ * sin(theTCMetPhi);
                float cmetx = calotcmet_ * cos(theCaloTCMetPhi);
                float cmety = calotcmet_ * sin(theCaloTCMetPhi);
                float pfmetx = pfmet_ * cos(thePFMetPhi);
                float pfmety = pfmet_ * sin(thePFMetPhi);
                for (unsigned int muj = 0; muj < cms2.mus_p4().size(); ++muj) {
                    if (cms2.mus_p4()[muj].Pt() <= 10.0) continue;
                    if (!wasMetCorrectedForThisMuon(muj, usingTcMet) && muonIdNotIsolated(muj, NominalTTbarV2)) {
                        fixMetForThisMuon(muj, metx, mety, usingTcMet);
                        fixMetForThisMuon(muj, cmetx, cmety, usingTcMet);
                    }
                }
                tcmet_ = sqrt(metx * metx + mety * mety);
                theTCMetPhi = atan2(mety, metx);
                calotcmet_ = sqrt(cmetx * cmetx + cmety * cmety);
                theCaloTCMetPhi = atan2(cmety, cmetx);

                //
                // Fill lepton counters
                //

                //
                // note that this is also used to define what leptons the jets
                // will be cleaned for
                //

                ngoodlep_ = 0;
                ngoodmus_ = 0;
                ngoodels_ = 0;

                std::vector<unsigned int> goodElectronIndicesTTBarV2;
                std::vector<unsigned int> goodMuonIndicesTTBarV2;
                std::vector<unsigned int> goodElectronIndicesSSV2;
                std::vector<unsigned int> goodMuonIndicesSSV2;

                // muon loop
                for (unsigned muii = 0; muii < cms2.mus_p4().size(); ++muii) {
                    if (cms2.mus_p4()[muii].pt() <= 10.) continue;
                    // for SS
                    if (muonId(muii, NominalSSv2) && fabs(cms2.mus_p4()[muii].eta()) < 2.4) {
                        goodMuonIndicesSSV2.push_back(muii);
                    }
                    // for TTBarV2
                    if (cms2.mus_p4()[muii].pt() > 10. && muonId(muii, NominalTTbarV2)) {
                        goodMuonIndicesTTBarV2.push_back(muii);
                        ++ngoodlep_;
                        ++ngoodmus_;
                    }
                }

                // electron loop
                for (unsigned eli = 0; eli < cms2.els_p4().size(); ++eli) {
                    if (cms2.els_p4()[eli].pt() <= 10.) continue;
                    cuts_t cuts_passed = electronSelection(eli);
                    // for SS
                    if(pass_electronSelectionCompareMask(cuts_passed, electronSelection_ssV2) && fabs(cms2.els_p4()[eli].eta()) < 2.4) {
                        goodElectronIndicesSSV2.push_back(eli);
                    }
                    // for TTBarV2
                    if(cms2.els_p4()[eli].pt() > 10. && pass_electronSelectionCompareMask(cuts_passed, electronSelection_ttbarV2)) {
                        goodElectronIndicesTTBarV2.push_back(eli);
                        ++ngoodlep_;
                        ++ngoodels_;
                    }
                }

                //
                // hypothesis stuff
                //

                hyp_type_    = cms2.hyp_type()[hypi];
                pt1_         = cms2.hyp_lt_p4()[hypi].pt();
                pt2_         = cms2.hyp_ll_p4()[hypi].pt();
                eta1_        = cms2.hyp_lt_p4()[hypi].eta();
                eta2_        = cms2.hyp_ll_p4()[hypi].eta();
                phi1_        = cms2.hyp_lt_p4()[hypi].phi();
                phi2_        = cms2.hyp_ll_p4()[hypi].phi();
                d0corr1_     = cms2.hyp_lt_d0corr()[hypi];
                d0corr2_     = cms2.hyp_ll_d0corr()[hypi];		 
                eormu1_      = cms2.hyp_lt_id()[hypi];
                eormu2_      = cms2.hyp_ll_id()[hypi];
                dphipfmet1_  = deltaPhi(cms2.hyp_lt_p4()[hypi].phi(), thePFMetPhi);
                dphipfmet2_  = deltaPhi(cms2.hyp_ll_p4()[hypi].phi(), thePFMetPhi);
                dphitcmet1_  = deltaPhi(cms2.hyp_lt_p4()[hypi].phi(), theTCMetPhi);
                dphitcmet2_  = deltaPhi(cms2.hyp_ll_p4()[hypi].phi(), theTCMetPhi);
                mass_        = cms2.hyp_p4()[hypi].mass2() > 0 ? cms2.hyp_p4()[hypi].mass() : TMath::Sqrt(-1 * cms2.hyp_p4()[hypi].mass2());
                dilpt_       = cms2.hyp_p4()[hypi].pt();		 
                deltaphi_    = deltaPhi(phi1_, phi2_);

                // calculate projected pfmet
                float mindphi_pfmet = min(dphipfmet1_, dphipfmet2_);
                if (mindphi_pfmet < TMath::Pi()/2.)
                    proj_pfmet_ = pfmet_ * sin(mindphi_pfmet);
                else
                    proj_pfmet_ = pfmet_;
                // calculate projected pfmet
                float mindphi_tcmet = min(dphitcmet1_, dphitcmet2_);
                if (mindphi_tcmet < TMath::Pi()/2.)
                    proj_tcmet_ = tcmet_ * sin(mindphi_tcmet);
                else
                    proj_tcmet_ = tcmet_;				

                // initialize meff to 0
                pfmeff_ = 0.;
                tcmeff_ = 0.;

                // add MET to meff
                pfmeff_ += pfmet_;
                tcmeff_ += tcmet_;

                // add lepton pt to meff
                pfmeff_ += (pt1_ + pt2_);
                tcmeff_ += (pt1_ + pt2_);

                //
                // clean jets for ALL leptons
                //

                VofP4 theJets;
                std::vector<unsigned int> theJetIndices;
                sumjetpt_   = 0.;
                sumjetpt25_ = 0.;
                sumjetptSS_ = 0.;
                njets25_ = 0;
                njetsSS_ = 0;
                njets_ = 0;
        
                for (unsigned int jeti = 0; jeti < cms2.pfjets_p4().size(); ++jeti)
                {

                    LorentzVector vjet = cms2.pfjets_p4()[jeti]*cms2.pfjets_cor()[jeti];
                    bool jetIsLepTTBarV2 = false;
                    bool jetIsLepSSV2 = false;

                    // check if jet is a lepton that passes the ttbar selection
                    for (unsigned int muj = 0; muj < goodMuonIndicesTTBarV2.size(); ++muj) {
                        LorentzVector vlep = cms2.mus_p4()[goodMuonIndicesTTBarV2[muj]];
                        if (!jetIsLepTTBarV2 && dRbetweenVectors(vjet, vlep) < 0.4)
                            jetIsLepTTBarV2 = true;
                    }
                    for (unsigned int elj = 0; elj < goodElectronIndicesTTBarV2.size(); ++elj) {
                        LorentzVector vlep = cms2.els_p4()[goodElectronIndicesTTBarV2[elj]];
                        if (!jetIsLepTTBarV2 && dRbetweenVectors(vjet, vlep) < 0.4)
                            jetIsLepTTBarV2 = true;
                    }

                    // 30 GeV TTBarV2 cleaning
                    if (!jetIsLepTTBarV2 && vjet.Pt() > 30. && fabs(cms2.pfjets_p4()[jeti].eta()) < 2.5 && isGoodPFJet(jeti)) {
                        theJets.push_back(cms2.pfjets_p4()[jeti]);
                        theJetIndices.push_back(jeti);
                        sumjetpt_ += vjet.Pt();
                    }
                    // 25 GeV TTBarV2 cleaning
                    if (!jetIsLepTTBarV2 && vjet.Pt() > 25. && fabs(cms2.pfjets_p4()[jeti].eta()) < 2.5 && isGoodPFJet(jeti)) {
                        sumjetpt25_ += vjet.Pt();
                        njets25_ ++;
                    }

                    // check if jet is a lepton that passes the ss selection
                    for (unsigned int muj = 0; muj < goodMuonIndicesSSV2.size(); ++muj) {
                        LorentzVector vlep = cms2.mus_p4()[goodMuonIndicesSSV2[muj]];
                        if (!jetIsLepSSV2 && dRbetweenVectors(vjet, vlep) < 0.4)
                            jetIsLepSSV2 = true;
                    }
                    for (unsigned int elj = 0; elj < goodElectronIndicesSSV2.size(); ++elj) {
                        LorentzVector vlep = cms2.els_p4()[goodElectronIndicesSSV2[elj]];
                        if (!jetIsLepSSV2 && dRbetweenVectors(vjet, vlep) < 0.4)
                            jetIsLepSSV2 = true;
                    }
                    if (!jetIsLepSSV2 && vjet.Pt() > 30. && fabs(cms2.pfjets_p4()[jeti].eta()) < 2.5 && isGoodPFJet(jeti)) {
                        ++njetsSS_;
                        sumjetptSS_ += vjet.Pt();
                    }

                }
                std::sort(theJets.begin(), theJets.end(), sortByPt);
                std::sort(theJetIndices.begin(), theJetIndices.end(), sortByPFJetPt);

                //
                // jet kinematic quantities 
                // for TTBarV2 selection cleaning
                // 

                njets_        = theJetIndices.size();
                jet1pt_       = theJetIndices.size() > 0 ? cms2.pfjets_p4()[theJetIndices[0]].pt()*cms2.pfjets_cor()[theJetIndices[0]] : -999999.;
                jet1eta_      = theJetIndices.size() > 0 ? cms2.pfjets_p4()[theJetIndices[0]].eta() : -999999.;
                jet1phi_      = theJetIndices.size() > 0 ? cms2.pfjets_p4()[theJetIndices[0]].phi() : -999999.;
                jet2pt_       = theJetIndices.size() > 1 ? cms2.pfjets_p4()[theJetIndices[1]].pt()*cms2.pfjets_cor()[theJetIndices[1]] : -999999.;
                jet2eta_      = theJetIndices.size() > 1 ? cms2.pfjets_p4()[theJetIndices[1]].eta() : -999999.;
                jet2phi_      = theJetIndices.size() > 1 ? cms2.pfjets_p4()[theJetIndices[1]].phi() : -999999.;
                jet3pt_       = theJetIndices.size() > 2 ? cms2.pfjets_p4()[theJetIndices[2]].pt()*cms2.pfjets_cor()[theJetIndices[2]] : -999999.;
                jet3eta_      = theJetIndices.size() > 2 ? cms2.pfjets_p4()[theJetIndices[2]].eta() : -999999.;
                jet3phi_      = theJetIndices.size() > 2 ? cms2.pfjets_p4()[theJetIndices[2]].phi() : -999999.;

                LorentzVector dijetP4;
                jetmass_ = theJetIndices.size() > 1 ? sqrt((cms2.pfjets_p4()[theJetIndices[0]]*cms2.pfjets_cor()[theJetIndices[0]] 
                                    + cms2.pfjets_p4()[theJetIndices[1]]*cms2.pfjets_cor()[theJetIndices[1]]).M2()) : -999999.; 

                double mindphipfmet = 999999.;
                double mindphitcmet = 999999.;
                ntchelbtags_    = 0;
                nssvhembtags_   = 0;
                nssvhetbtags_   = 0;
                nssvhptbtags_   = 0;
                jet1isBtag_  = 0;
                jet2isBtag_  = 0;
                jet3isBtag_  = 0;

                //
                // loop on the jets
                // BTag WP are documented in:
                // https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagPerformanceOP
                //

                for (unsigned int jeti = 0; jeti < theJetIndices.size(); ++jeti)
                {

                    // TCHEL
                    if (cms2.pfjets_trackCountingHighEffBJetTag()[jeti] > 1.7) {
                        ++ntchelbtags_;
                        if (jeti == 0) jet1isBtag_ |= 0x1;
                        if (jeti == 1) jet2isBtag_ |= 0x1;
                        if (jeti == 2) jet3isBtag_ |= 0x1;                        
                    }

                    // SSVHEM
                    if (cms2.pfjets_simpleSecondaryVertexHighEffBJetTag()[theJetIndices[jeti]] > 1.74)
                    {
                        ++nssvhembtags_;
                        if (jeti == 0) jet1isBtag_ |= 0x2;
                        if (jeti == 1) jet2isBtag_ |= 0x2;
                        if (jeti == 2) jet3isBtag_ |= 0x2;
                    }

                    // SSVHET
                    if (cms2.pfjets_simpleSecondaryVertexHighEffBJetTag()[theJetIndices[jeti]] > 3.05)
                    {
                        ++nssvhetbtags_;
                        if (jeti == 0) jet1isBtag_ |= 0x4;
                        if (jeti == 1) jet2isBtag_ |= 0x4;
                        if (jeti == 2) jet3isBtag_ |= 0x4;
                    }

                    // SSVHPT
                    if (cms2.pfjets_simpleSecondaryVertexHighPurBJetTags()[theJetIndices[jeti]] > 2.)
                    {
                        ++nssvhptbtags_;
                        if (jeti == 0) jet1isBtag_ |= 0x8;
                        if (jeti == 1) jet2isBtag_ |= 0x8;
                        if (jeti == 2) jet3isBtag_ |= 0x8;

                    }

                    float currdphipfmet = deltaPhi(thePFMetPhi, cms2.pfjets_p4()[theJetIndices[jeti]].phi());
                    if (currdphipfmet < mindphipfmet)
                        mindphipfmet = currdphipfmet;

                    float currdphitcmet = deltaPhi(theTCMetPhi, cms2.pfjets_p4()[theJetIndices[jeti]].phi());
                    if (currdphitcmet < mindphitcmet)
                        mindphitcmet = currdphitcmet;

                    // add jet pt to meff
                    pfmeff_ += cms2.pfjets_p4()[theJetIndices[jeti]].pt() * cms2.pfjets_cor()[theJetIndices[jeti]];
                    tcmeff_ += cms2.pfjets_p4()[theJetIndices[jeti]].pt() * cms2.pfjets_cor()[theJetIndices[jeti]];
                }

                dphipfmetjet_ = mindphipfmet;
                dphitcmetjet_ = mindphitcmet;

                // now, find jet closest to each hyp lepton
                float mindrjet1 = 999999.;
                float mindrjet2 = 999999.;
                for (unsigned int jeti = 0; jeti < theJetIndices.size(); ++jeti)
                {
                    // for tight hyp lepton
                    float deta1 = cms2.hyp_lt_p4()[hypi].eta()-cms2.pfjets_p4()[theJetIndices[jeti]].eta();
                    float dphi1 = deltaPhi(cms2.hyp_lt_p4()[hypi].phi(), cms2.pfjets_p4()[theJetIndices[jeti]].phi());
                    float currdrjet1 = sqrt(deta1*deta1+dphi1*dphi1);
                    if (currdrjet1 < mindrjet1)
                        mindrjet1 = currdrjet1;

                    // for loose hyp lepton
                    float deta2 = cms2.hyp_ll_p4()[hypi].eta()-cms2.pfjets_p4()[theJetIndices[jeti]].eta();
                    float dphi2 = deltaPhi(cms2.hyp_ll_p4()[hypi].phi(), cms2.pfjets_p4()[theJetIndices[jeti]].phi());
                    float currdrjet2 = sqrt(deta2*deta2+dphi2*dphi2);
                    if (currdrjet2 < mindrjet2)
                        mindrjet2 = currdrjet2;
                }

                drjet1_ = mindrjet1;
                drjet2_ = mindrjet2;

                // mt2
                mt2_ = MT2(pfmet_, thePFMetPhi, cms2.hyp_ll_p4()[hypi], cms2.hyp_lt_p4()[hypi]);
                // mt2j
                if (theJets.size() > 1)
                    mt2j_ = MT2J(pfmet_, thePFMetPhi, cms2.hyp_ll_p4()[hypi], cms2.hyp_lt_p4()[hypi], theJets);

                // MT for Higgs

                float enell = TMath::Sqrt(dilpt_*dilpt_ + mass_*mass_);
                float pfenenn = TMath::Sqrt(pfmet_*pfmet_  + mass_*mass_);
                float pfenex  = cms2.hyp_p4()[hypi].Px() + pfmetx;
                float pfeney  = cms2.hyp_p4()[hypi].Py() + pfmety;
                float tcenenn = TMath::Sqrt(tcmet_*tcmet_  + mass_*mass_);
                float tcenex  = cms2.hyp_p4()[hypi].Px() + metx;
                float tceney  = cms2.hyp_p4()[hypi].Py() + mety;
                pfmth_ = sqrt(2.0*mass_*mass_ + 2.0*(enell*pfenenn - pfenex*pfenex - pfeney*pfeney));
                tcmth_ = sqrt(2.0*mass_*mass_ + 2.0*(enell*tcenenn - tcenex*tcenex - tceney*tceney));

                // check if hypothesis makes an extra Z
                extraZveto_ = makesExtraZ(hypi, false, false);

                //
                // Now fill the detailed electron/muon specific variables
                // for each leg of the hypothesis
                //

                //
                // First do LT
                //

                // if LT is a mu, fill mu info
                if (abs(cms2.hyp_lt_id()[hypi]) == 13) {
                    iso1_   = muonIsoValue(index1);
                    type1_  = cms2.mus_type()[index1];
                    mu1_muonidfull_   = muonId(index1, NominalTTbarV2);
                    mu1_muonid_       = muonIdNotIsolated(index1, NominalTTbarV2);
                    mu1_muonidfullV1_ = muonId(index1, NominalTTbar);
                    mu1_muonidV1_     = muonIdNotIsolated(index1, NominalTTbar);
                    mu1_goodmask_     = cms2.mus_goodmask()[index1];
                    mu1_gfitchi2_     = cms2.mus_gfit_chi2()[index1] < -9000. ? -999999. : cms2.mus_gfit_chi2()[index1]/cms2.mus_gfit_ndof()[index1];
                    mu1_cosmic_       = isCosmics(index1);
                    mu1_siHits_       = cms2.mus_validHits()[index1];
                    mu1_saHits_       = cms2.mus_gfit_validSTAHits()[index1];
                    mu1_emVetoDep_    = cms2.mus_iso_ecalvetoDep()[index1];
                    mu1_hadVetoDep_   = cms2.mus_iso_hcalvetoDep()[index1];
                    if (cms2.mus_pfmusidx()[index1] > -1) mu1_isPFmuon_ = 1;
                    int trkidx1 = cms2.mus_trkidx()[index1];
                    d0vtx1_ = cms2.trks_d0vtx()[trkidx1];
                }
                // if LT is an ele, fill ele info
                if (abs(cms2.hyp_lt_id()[hypi]) == 11) {
                    iso1_   = electronIsolation_rel(index1, true);
                    type1_  = cms2.els_type()[index1];
                    e1_cand01full_  = pass_electronSelection(index1, electronSelection_ttbar);
                    e1_cand01_      = electronId_cand(index1, CAND_01);
                    e1_vbtf90full_  = pass_electronSelection(index1, electronSelection_ttbarV2);
                    electronIdComponent_t answer_vbtf90 = electronId_VBTF(index1, VBTF_35X_90);
                    e1_vbtf90_      = (answer_vbtf90 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    electronIdComponent_t answer_vbtf85 = electronId_VBTF(index1, VBTF_35X_85);
                    e1_vbtf85_      = (answer_vbtf85 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    electronIdComponent_t answer_vbtf80 = electronId_VBTF(index1, VBTF_35X_80);
                    e1_vbtf80_      = (answer_vbtf80 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    electronIdComponent_t answer_vbtf70 = electronId_VBTF(index1, VBTF_35X_70);
                    e1_vbtf70_      = (answer_vbtf70 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    e1_scet_        = cms2.els_eSC()[index1] / cosh(cms2.els_etaSC()[index1]);
                    e1_eopin_       = cms2.els_eOverPIn()[index1];
                    e1_hoe_         = cms2.els_hOverE()[index1];
                    e1_dphiin_      = cms2.els_dPhiIn()[index1];
                    e1_detain_      = cms2.els_dEtaIn()[index1];
                    e1_e25Me55_     = cms2.els_e2x5Max()[index1] / cms2.els_e5x5()[index1];
                    e1_sigieie_     = cms2.els_sigmaIEtaIEta()[index1];
                    e1_eMe55_       = cms2.els_eMax()[index1] / cms2.els_e5x5()[index1];
                    e1_nmHits_      = cms2.els_exp_innerlayers()[index1];
                    e1_dcot_        = cms2.els_conv_dcot()[index1];
                    e1_dist_        = cms2.els_conv_dist()[index1];
                    e1_drmu_        = cms2.els_closestMuon()[index1] < 0 ? -999999. : cms2.els_musdr()[index1];
                    e1_isspike_     = isSpikeElectron(index1);
                    e1_scCharge_   = cms2.els_sccharge()[index1];
                    e1_gsfCharge_  = cms2.els_trk_charge()[index1];
                    e1_ctfCharge_  = cms2.els_trkidx()[index1] > -1 ? cms2.trks_charge()[cms2.els_trkidx()[index1]] : -999999;
                    int trkidx1 = cms2.els_trkidx()[index1];
                    if(trkidx1 >= 0) d0vtx1_ = cms2.trks_d0vtx()[trkidx1];
                }

                //
                // Now do LL...
                //
    
                // if LL is a mu, fill mu info
                if (abs(cms2.hyp_ll_id()[hypi]) == 13) {
                    iso2_   = muonIsoValue(index2);
                    type2_  = cms2.mus_type()[index2];
                    mu2_muonidfull_   = muonId(index2, NominalTTbarV2);
                    mu2_muonid_       = muonIdNotIsolated(index2, NominalTTbarV2);
                    mu2_muonidfullV1_ = muonId(index2, NominalTTbar);
                    mu2_muonidV1_     = muonIdNotIsolated(index2, NominalTTbar);
                    mu2_goodmask_     = cms2.mus_goodmask()[index2];
                    mu2_gfitchi2_     = cms2.mus_gfit_chi2()[index2] < -9000. ? -999999. : cms2.mus_gfit_chi2()[index2]/cms2.mus_gfit_ndof()[index2];
                    mu2_cosmic_       = isCosmics(index2);
                    mu2_siHits_       = cms2.mus_validHits()[index2];
                    mu2_saHits_       = cms2.mus_gfit_validSTAHits()[index2];
                    mu2_emVetoDep_    = cms2.mus_iso_ecalvetoDep()[index2];
                    mu2_hadVetoDep_   = cms2.mus_iso_hcalvetoDep()[index2];
                    if (cms2.mus_pfmusidx()[index2] > -1) mu2_isPFmuon_ = 1;
                    int trkidx2 = cms2.mus_trkidx()[index2];
                    d0vtx2_ = cms2.trks_d0vtx()[trkidx2];
                }
                // if LL is an ele, fill ele info
                if (abs(cms2.hyp_ll_id()[hypi]) == 11) {
                    iso2_   = electronIsolation_rel(index2, true);
                    type2_  = cms2.els_type()[index2];
                    e2_cand01full_  = pass_electronSelection(index2, electronSelection_ttbar);
                    e2_cand01_      = electronId_cand(index2, CAND_01);
                    e2_vbtf90full_  = pass_electronSelection(index2, electronSelection_ttbarV2);
                    electronIdComponent_t answer_vbtf90   = electronId_VBTF(index2, VBTF_35X_90);
                    e2_vbtf90_      = (answer_vbtf90 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    electronIdComponent_t answer_vbtf85   = electronId_VBTF(index2, VBTF_35X_85);
                    e2_vbtf85_      = (answer_vbtf85 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    electronIdComponent_t answer_vbtf80   = electronId_VBTF(index2, VBTF_35X_80);
                    e2_vbtf80_      = (answer_vbtf80 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    electronIdComponent_t answer_vbtf70   = electronId_VBTF(index2, VBTF_35X_70);
                    e2_vbtf70_      = (answer_vbtf70 & (1ll<<ELEID_ID)) == (1ll<<ELEID_ID);
                    e2_scet_        = cms2.els_eSC()[index2] / cosh(cms2.els_etaSC()[index2]);
                    e2_eopin_       = cms2.els_eOverPIn()[index2];
                    e2_hoe_         = cms2.els_hOverE()[index2];
                    e2_dphiin_      = cms2.els_dPhiIn()[index2];
                    e2_detain_      = cms2.els_dEtaIn()[index2]; 
                    e2_e25Me55_     = cms2.els_e2x5Max()[index2] / cms2.els_e5x5()[index2];
                    e2_sigieie_     = cms2.els_sigmaIEtaIEta()[index2];
                    e2_eMe55_       = cms2.els_eMax()[index2] / cms2.els_e5x5()[index2];
                    e2_nmHits_      = cms2.els_exp_innerlayers()[index2];
                    e2_dcot_        = cms2.els_conv_dcot()[index2];
                    e2_dist_        = cms2.els_conv_dist()[index2];
                    e2_drmu_        = cms2.els_closestMuon()[index2] < 0 ? -999999. : cms2.els_musdr()[index2];
                    e2_isspike_     = isSpikeElectron(index2);
                    e2_scCharge_    = cms2.els_sccharge()[index2];
                    e2_gsfCharge_   = cms2.els_trk_charge()[index2]; 
                    e2_ctfCharge_   = cms2.els_trkidx()[index2] > -1 ? cms2.trks_charge()[cms2.els_trkidx()[index2]] : -999999;
                    int trkidx2 = cms2.els_trkidx()[index2];
                    if (trkidx2 >= 0) d0vtx2_ = cms2.trks_d0vtx()[trkidx2];
                }

                //
                // Now write everything to the baby
                //

                FillBabyNtuple();
            }
        }

        f.Close();

    }

    if (nEventsChain != nEventsTotal)
        std::cout << "ERROR: number of events from files is not equal to total number of events" << std::endl;


    CloseBabyNtuple();
}

void dilepbabymaker::InitBabyNtuple ()
{
    // event stuff
    memset(dataset_, '\0', 200);
    run_          = -999999;
    ls_           = -999999;
    evt_          = -999999;
    isdata_       = 1;
    evt_clean082010_ = -999999;
    evt_clean102010_ = -999999;    
    nvtx_         = -999999;
    scale1fb_     = -999999.;
    pthat_        = -999999.;
    hyp_type_     = -999999;
    pfmet_        = -999999.;
    tcmet_        = -999999.;
    calotcmet_    = -999999.;
    proj_pfmet_   = -999999.;
    proj_tcmet_   = -999999.;
    ntrks_        = -999999;
    njets_        = -999999;
    njets25_      = -999999;
    njetsSS_      = -999999;
    jet1pt_       = -999999.;
    jet2pt_       = -999999.;
    jet3pt_       = -999999.;
    sumjetpt_     = -999999.;
    sumjetpt25_     = -999999.;
    sumjetptSS_   = -999999.;
    jet1eta_      = -999999.;
    jet2eta_      = -999999.;
    jet3eta_      = -999999.;
    jet1phi_      = -999999.;
    jet2phi_      = -999999.;
    jet3phi_      = -999999.;
    jetmass_      = -999999.;
    pfmth_          = -999999.;
    tcmth_          = -999999.;
    jet1isBtag_   = 0;
    jet2isBtag_   = 0;
    jet3isBtag_   = 0;
    dphipfmetjet_ = -999999.;
    dphitcmetjet_ = -999999.;
    deltaphi_     = -999999.;

    ntchelbtags_ = -999999;
    nssvhembtags_ = -999999;
    nssvhetbtags_ = -999999;
    nssvhptbtags_ = -999999;

    pfmeff_       = -999999.;
    tcmeff_       = -999999.;

    // lepton stuff
    ngoodlep_     = -999999;
    ngoodmus_     = -999999;
    ngoodels_     = -999999;
    ngenels_      = -999999;
    ngenmus_      = -999999;
    ngentaus_     = -999999;

    dilpt_        = -999999.;
    mass_         = -999999.;
    eormu1_       = -999999;
    type1_        = -999999;
    pt1_          = -999999.;
    eta1_         = -999999.;
    phi1_         = -999999.;
    iso1_         = -999999.;
    d0corr1_      = -999999.;
    d0vtx1_       = -999999.;
    dphipfmet1_   = -999999.;
    dphitcmet1_   = -999999.;
    drjet1_       = -999999.;
    eormu2_       = -999999;
    type2_        = -999999;
    pt2_          = -999999.;
    eta2_         = -999999.;
    phi2_         = -999999.;
    iso2_         = -999999.;
    d0corr2_      = -999999.;
    d0vtx2_       = -999999.;
    dphipfmet2_   = -999999.;
    dphitcmet2_   = -999999.;
    drjet2_       = -999999.;
    mt2_          = -999999.;
    mt2j_         = -999999.;
    extraZveto_   = 0;

    // muon stuff
    mu1_muonidfull_   = 0;
    mu1_muonid_       = 0;
    mu1_muonidfullV1_ = 0;
    mu1_muonidV1_     = 0;
    mu1_goodmask_     = -999999;
    mu1_gfitchi2_     = -999999.;
    mu1_cosmic_       = 0;
    mu1_siHits_       = -999999;
    mu1_saHits_       = -999999;
    mu1_emVetoDep_    = -999999.;
    mu1_hadVetoDep_   = -999999.;
    mu1_isPFmuon_     = 0;

    mu2_muonidfull_   = 0;
    mu2_muonid_       = 0;
    mu2_muonidfullV1_ = 0;
    mu2_muonidV1_     = 0;
    mu2_goodmask_     = -999999;
    mu2_gfitchi2_     = -999999.;
    mu2_cosmic_       = 0;
    mu2_siHits_       = -999999;
    mu2_saHits_       = -999999;
    mu2_emVetoDep_    = -999999.;
    mu2_hadVetoDep_   = -999999.;
    mu2_isPFmuon_     = 0;

    // electron stuff
    e1_cand01full_  = 0;
    e1_cand01_      = 0;
    e1_vbtf90full_  = 0;
    e1_vbtf90_      = 0;
    e1_vbtf85_      = 0;
    e1_vbtf80_      = 0;
    e1_vbtf70_      = 0;
    e1_scet_        = -999999.;
    e1_eopin_       = -999999.;
    e1_hoe_         = -999999.;
    e1_dphiin_      = -999999.;
    e1_detain_      = -999999.;
    e1_e25Me55_     = -999999.;
    e1_sigieie_     = -999999.;
    e1_eMe55_       = -999999.;
    e1_nmHits_      = -999999;
    e1_dcot_        = -999999.;
    e1_dist_        = -999999.;
    e1_drmu_        = -999999.;
    e1_isspike_     = 0;
    e1_ctfCharge_   = -999999;
    e1_gsfCharge_   = -999999;
    e1_scCharge_    = -999999;

    e2_cand01full_  = 0;
    e2_cand01_      = 0;
    e2_vbtf90full_  = 0;
    e2_vbtf90_      = 0;
    e2_vbtf85_      = 0;
    e2_vbtf80_      = 0;
    e2_vbtf70_      = 0;
    e2_scet_        = -999999.;
    e2_eopin_       = -999999.;
    e2_hoe_         = -999999.;
    e2_dphiin_      = -999999.;
    e2_detain_      = -999999.;
    e2_e25Me55_     = -999999.;
    e2_sigieie_     = -999999.;
    e2_eMe55_       = -999999.;
    e2_nmHits_      = -999999;
    e2_dcot_        = -999999.;
    e2_dist_        = -999999.;
    e2_drmu_        = -999999.;
    e2_isspike_     = 0;
    e2_ctfCharge_   = -999999;
    e2_gsfCharge_   = -999999;
    e2_scCharge_    = -999999;
}

void dilepbabymaker::MakeBabyNtuple(const char *babyFilename)
{
    TDirectory *rootdir = gDirectory->GetDirectory("Rint:");
    rootdir->cd();

    babyFile_ = new TFile(Form("%s", babyFilename), "RECREATE");
    babyFile_->cd();
    babyTree_ = new TTree("tree", "A Baby Ntuple");

    // event stuff
    babyTree_->Branch("dataset",      &dataset_,     "dataset[200]/C");
    babyTree_->Branch("run",          &run_,         "run/I"         );
    babyTree_->Branch("ls",           &ls_,          "ls/I"          );
    babyTree_->Branch("evt",          &evt_,         "evt/I"         );
    babyTree_->Branch("nvtx",         &nvtx_,        "nvtx/I"        );
    babyTree_->Branch("isdata",       &isdata_,      "isdata/I"      );
    babyTree_->Branch("evt_clean082010",       &evt_clean082010_,      "evt_clean082010/I"      );
    babyTree_->Branch("evt_clean102010",       &evt_clean102010_,      "evt_clean102010/I"      );
    babyTree_->Branch("scale1fb",     &scale1fb_,    "scale1fb/F"    );
    babyTree_->Branch("pthat",        &pthat_,       "pthat/F"       );
    babyTree_->Branch("hyp_type",     &hyp_type_,    "hyp_type/I"    );
    babyTree_->Branch("pfmet",        &pfmet_,       "pfmet/F"       );
    babyTree_->Branch("tcmet",        &tcmet_,       "tcmet/F"       );
    babyTree_->Branch("calotcmet",    &calotcmet_,   "calotcmet/F"   );
    babyTree_->Branch("proj_pfmet",   &proj_pfmet_,  "proj_pfmet/F"  );
    babyTree_->Branch("proj_tcmet",   &proj_tcmet_,  "proj_tcmet/F"  );
    babyTree_->Branch("ntrks",        &ntrks_,       "ntrks/I"       );
    babyTree_->Branch("njets",        &njets_,       "njets/I"       );
    babyTree_->Branch("njets25",        &njets25_,       "njets25/I"       ); 
    babyTree_->Branch("njetsSS",      &njetsSS_,     "njetsSS/I"     );
    babyTree_->Branch("jet1pt",       &jet1pt_,      "jet1pt/F"      );
    babyTree_->Branch("jet2pt",       &jet2pt_,      "jet2pt/F"      );
    babyTree_->Branch("jet3pt",       &jet3pt_,      "jet3pt/F"      );
    babyTree_->Branch("sumjetpt",     &sumjetpt_,    "sumjetpt/F"    );
    babyTree_->Branch("sumjetpt25",     &sumjetpt25_,    "sumjetpt25/F"    );
    babyTree_->Branch("sumjetptSS",   &sumjetptSS_,  "sumjetptSS/F"  );
    babyTree_->Branch("jet1eta",      &jet1eta_,     "jet1eta/F"     );
    babyTree_->Branch("jet2eta",      &jet2eta_,     "jet2eta/F"     );
    babyTree_->Branch("jet3eta",      &jet3eta_,     "jet3eta/F"     );
    babyTree_->Branch("jet1phi",      &jet1phi_,     "jet1phi/F"     );
    babyTree_->Branch("jet2phi",      &jet2phi_,     "jet2phi/F"     );
    babyTree_->Branch("jet3phi",      &jet3phi_,     "jet3phi/F"     );
    babyTree_->Branch("jetmass",      &jetmass_,     "jetmass/F"     );
    babyTree_->Branch("pfmth",          &pfmth_,         "pfmth/F"         );
    babyTree_->Branch("tcmth",          &tcmth_,         "tcmth/F"         );
    babyTree_->Branch("jet1isBtag",   &jet1isBtag_,  "jet1isBtag/I"  );
    babyTree_->Branch("jet2isBtag",   &jet2isBtag_,  "jet2isBtag/I"  );
    babyTree_->Branch("jet3isBtag",   &jet3isBtag_,  "jet3isBtag/I"  );
    babyTree_->Branch("dphipfmetjet", &dphipfmetjet_,"dphipfmetjet/F");
    babyTree_->Branch("dphitcmetjet", &dphitcmetjet_,"dphitcmetjet/F");
    babyTree_->Branch("deltaphi",     &deltaphi_,    "deltaphi/F"    );

    babyTree_->Branch("ntchelbtags",    &ntchelbtags_,      "ntchelbtags/I"   );
    babyTree_->Branch("nssvhembtags",   &nssvhembtags_,     "nssvhembtags/I"   );
    babyTree_->Branch("nssvhetbtags",   &nssvhetbtags_,     "nssvhetbtags/I" );
    babyTree_->Branch("nssvhptbtags",   &nssvhptbtags_,     "nssvhptbtags/I" );

    babyTree_->Branch("pfmeff",       &pfmeff_,      "pfmeff/F"      );
    babyTree_->Branch("tcmeff",       &tcmeff_,      "tcmeff/F"      );


    // lepton stuff
    babyTree_->Branch("ngoodlep",   &ngoodlep_,   "ngoodlep/I"  );
    babyTree_->Branch("ngoodmus",   &ngoodmus_,   "ngoodmus/I"  );
    babyTree_->Branch("ngoodels",   &ngoodels_,   "ngoodels/I"  );
    babyTree_->Branch("dilpt",      &dilpt_,      "dilpt/F"     );
    babyTree_->Branch("mass",       &mass_,       "mass/F"      );
    babyTree_->Branch("eormu1",     &eormu1_,     "eormu1/I"    );
    babyTree_->Branch("type1",      &type1_,      "type1/I"     );
    babyTree_->Branch("ngenels",    &ngenels_,    "ngenels/I"   );
    babyTree_->Branch("ngenmus",    &ngenmus_,    "ngenmus/I"   );
    babyTree_->Branch("ngentaus",   &ngentaus_,   "ngentaus/I"  );
    babyTree_->Branch("pt1",        &pt1_,        "pt1/F"       );
    babyTree_->Branch("eta1",       &eta1_,       "eta1/F"      );
    babyTree_->Branch("phi1",       &phi1_,       "phi1/F"      );
    babyTree_->Branch("iso1",       &iso1_,       "iso1/F"      );
    babyTree_->Branch("d0corr1",    &d0corr1_,    "d0corr1/F"   );
    babyTree_->Branch("d0vtx1",     &d0vtx1_,     "d0vtx1/F"    );
    babyTree_->Branch("dphipfmet1", &dphipfmet1_, "dphipfmet1/F");
    babyTree_->Branch("dphitcmet1", &dphitcmet1_, "dphitcmet1/F");
    babyTree_->Branch("drjet1",     &drjet1_,     "drjet1/F"    );
    babyTree_->Branch("eormu2",     &eormu2_,     "eormu2/I"    );
    babyTree_->Branch("type2",      &type2_,      "type2/I"     );
    babyTree_->Branch("pt2",        &pt2_,        "pt2/F"       );
    babyTree_->Branch("eta2",       &eta2_,       "eta2/F"      );
    babyTree_->Branch("phi2",       &phi2_,       "phi2/F"      );
    babyTree_->Branch("iso2",       &iso2_,       "iso2/F"      );
    babyTree_->Branch("d0corr2",    &d0corr2_,    "d0corr2/F"   );
    babyTree_->Branch("d0vtx2",     &d0vtx2_,     "d0vtx2/F"    );
    babyTree_->Branch("dphipfmet2", &dphipfmet2_, "dphipfmet2/F");
    babyTree_->Branch("dphitcmet2", &dphitcmet2_, "dphitcmet2/F");
    babyTree_->Branch("drjet2",     &drjet2_,     "drjet2/F"    );
    babyTree_->Branch("mt2",        &mt2_,        "mt2/F"       );
    babyTree_->Branch("mt2j",       &mt2j_,       "mt2j/F"      );
    babyTree_->Branch("extraZveto", &extraZveto_, "extraZveto/O");

    // muon stuff
    babyTree_->Branch("mu1_muonidfull",   &mu1_muonidfull_,   "mu1_muonidfull/O"  );
    babyTree_->Branch("mu1_muonid",       &mu1_muonid_,       "mu1_muonid/O"      );
    babyTree_->Branch("mu1_muonidfullV1", &mu1_muonidfullV1_, "mu1_muonidfullV1/O");
    babyTree_->Branch("mu1_muonidV1",     &mu1_muonidV1_,     "mu1_muonidV1/O"    );
    babyTree_->Branch("mu1_goodmask",     &mu1_goodmask_,     "mu1_goodmask/I"    );
    babyTree_->Branch("mu1_gfitchi2",     &mu1_gfitchi2_,     "mu1_gfitchi2/F"    );
    babyTree_->Branch("mu1_cosmic",       &mu1_cosmic_,       "mu1_cosmic/O"      );
    babyTree_->Branch("mu1_siHits",       &mu1_siHits_,       "mu1_siHits/I"      );
    babyTree_->Branch("mu1_saHits",       &mu1_saHits_,       "mu1_saHits/I"      );
    babyTree_->Branch("mu1_emVetoDep",    &mu1_emVetoDep_,    "mu1_emVetoDep/F"   );
    babyTree_->Branch("mu1_hadVetoDep",   &mu1_hadVetoDep_,   "mu1_hadVetoDep/F"  );
    babyTree_->Branch("mu1_isPFmuon",     &mu1_isPFmuon_,     "mu1_isPFmuon/O"    );

    babyTree_->Branch("mu2_muonidfull",   &mu2_muonidfull_,   "mu2_muonidfull/O"  );
    babyTree_->Branch("mu2_muonid",       &mu2_muonid_,       "mu2_muonid/O"      );
    babyTree_->Branch("mu2_muonidfullV1", &mu2_muonidfullV1_, "mu2_muonidfullV1/O");
    babyTree_->Branch("mu2_muonidV1",     &mu2_muonidV1_,     "mu2_muonidV1/O"    );
    babyTree_->Branch("mu2_goodmask",     &mu2_goodmask_,     "mu2_goodmask/I"    );
    babyTree_->Branch("mu2_gfitchi2",     &mu2_gfitchi2_,     "mu2_gfitchi2/F"    );
    babyTree_->Branch("mu2_cosmic",       &mu2_cosmic_,       "mu2_cosmic/O"      );
    babyTree_->Branch("mu2_siHits",       &mu2_siHits_,       "mu2_siHits/I"      );
    babyTree_->Branch("mu2_saHits",       &mu2_saHits_,       "mu2_saHits/I"      );
    babyTree_->Branch("mu2_emVetoDep",    &mu2_emVetoDep_,    "mu2_emVetoDep/F"   );
    babyTree_->Branch("mu2_hadVetoDep",   &mu2_hadVetoDep_,   "mu2_hadVetoDep/F"  );
    babyTree_->Branch("mu2_isPFmuon",     &mu2_isPFmuon_,     "mu2_isPFmuon/O"    );

    // electron stuff

    babyTree_->Branch("e1_cand01full", &e1_cand01full_, "e1_cand01full/O");
    babyTree_->Branch("e1_cand01",     &e1_cand01_,     "e1_cand01/O"    );
    babyTree_->Branch("e1_vbtf90full", &e1_vbtf90full_, "e1_vbtf90full/O");
    babyTree_->Branch("e1_vbtf90",     &e1_vbtf90_,     "e1_vbtf90/O"    );
    babyTree_->Branch("e1_vbtf85",     &e1_vbtf85_,     "e1_vbtf85/O"    );
    babyTree_->Branch("e1_vbtf80",     &e1_vbtf80_,     "e1_vbtf80/O"    );
    babyTree_->Branch("e1_vbtf70",     &e1_vbtf70_,     "e1_vbtf70/O"    );
    babyTree_->Branch("e1_scet",       &e1_scet_,       "e1_scet/F"      );
    babyTree_->Branch("e1_eopin",      &e1_eopin_,      "e1_eopin/F"     );
    babyTree_->Branch("e1_hoe",        &e1_hoe_,        "e1_hoe/F"       );
    babyTree_->Branch("e1_dphiin",     &e1_dphiin_,     "e1_dphiin/F"    );
    babyTree_->Branch("e1_detain",     &e1_detain_,     "e1_detain/F"    );
    babyTree_->Branch("e1_e25Me55",    &e1_e25Me55_,    "e1_e25Me55/F"   );
    babyTree_->Branch("e1_sigieie",    &e1_sigieie_,    "e1_sigieie/F"   );
    babyTree_->Branch("e1_eMe55",      &e1_eMe55_,      "e1_eMe55/F"     ); // for spikes
    babyTree_->Branch("e1_nmHits",     &e1_nmHits_,     "e1_nmHits/I"    );
    babyTree_->Branch("e1_dcot",       &e1_dcot_,       "e1_dcot/F"      );
    babyTree_->Branch("e1_dist",       &e1_dist_,       "e1_dist/F"      );
    babyTree_->Branch("e1_drmu",       &e1_drmu_,       "e1_drmu/F"      );
    babyTree_->Branch("e1_isspike",    &e1_isspike_,    "e1_isspike/O"   );
    babyTree_->Branch("e1_ctfCharge",  &e1_ctfCharge_,  "e1_ctfCharge/I" );
    babyTree_->Branch("e1_gsfCharge",  &e1_gsfCharge_,  "e1_gsfCharge/I" );
    babyTree_->Branch("e1_scCharge",   &e1_scCharge_,   "e1_scCharge/I"  );
    babyTree_->Branch("e2_cand01full", &e2_cand01full_, "e2_cand01full/O");
    babyTree_->Branch("e2_cand01",     &e2_cand01_,     "e2_cand01/O"    );
    babyTree_->Branch("e2_vbtf90full", &e2_vbtf90full_, "e2_vbtf90full/O");
    babyTree_->Branch("e2_vbtf90",     &e2_vbtf90_,     "e2_vbtf90/O"    );
    babyTree_->Branch("e2_vbtf85",     &e2_vbtf85_,     "e2_vbtf85/O"    );
    babyTree_->Branch("e2_vbtf80",     &e2_vbtf80_,     "e2_vbtf80/O"    );
    babyTree_->Branch("e2_vbtf70",     &e2_vbtf70_,     "e2_vbtf70/O"    );
    babyTree_->Branch("e2_scet",       &e2_scet_,       "e2_scet/F"      );
    babyTree_->Branch("e2_eopin",      &e2_eopin_,      "e2_eopin/F"     );
    babyTree_->Branch("e2_hoe",        &e2_hoe_,        "e2_hoe/F"       );
    babyTree_->Branch("e2_dphiin",     &e2_dphiin_,     "e2_dphiin/F"    );
    babyTree_->Branch("e2_detain",     &e2_detain_,     "e2_detain/F"    );
    babyTree_->Branch("e2_e25Me55",    &e2_e25Me55_,    "e2_e25Me55/F"   );
    babyTree_->Branch("e2_sigieie",    &e2_sigieie_,    "e2_sigieie/F"   );
    babyTree_->Branch("e2_eMe55",      &e2_eMe55_,      "e2_eMe55/F"     ); // for spikes
    babyTree_->Branch("e2_nmHits",     &e2_nmHits_,     "e2_nmHits/I"    );
    babyTree_->Branch("e2_dcot",       &e2_dcot_,       "e2_dcot/F"      );
    babyTree_->Branch("e2_dist",       &e2_dist_,       "e2_dist/F"      );
    babyTree_->Branch("e2_drmu",       &e2_drmu_,       "e2_drmu/F"      );
    babyTree_->Branch("e2_isspike",    &e2_isspike_,    "e2_isspike/O"   );
    babyTree_->Branch("e2_ctfCharge",  &e2_ctfCharge_,  "e2_ctfCharge/I" );
    babyTree_->Branch("e2_gsfCharge",  &e2_gsfCharge_,  "e2_gsfCharge/I" );
    babyTree_->Branch("e2_scCharge",   &e2_scCharge_,   "e2_scCharge/I"  );
}

void dilepbabymaker::SetEventLevelInfo ()
{

    strcpy(dataset_, cms2.evt_dataset().Data());
    run_        = cms2.evt_run();
    ls_         = cms2.evt_lumiBlock();
    evt_        = cms2.evt_event();
    isdata_     = cms2.evt_isRealData();

    evt_clean082010_ = cleaning_standardAugust2010(isdata_);
    evt_clean102010_ = cleaning_standardOctober2010();

    nvtx_ = 0;
    for (unsigned int i=0; i<cms2.vtxs_isFake().size(); i++) {
        // isGoodVertex is defined in CORE/eventSelections.cc
        // !isFake, ndof >= 4, rho <= 2, Z <= 24.
        if (isGoodVertex(i))
            ++nvtx_;
    }

    if (!isdata_) {
        int nlep  = leptonGenpCount_lepTauDecays(ngenels_, ngenmus_, ngentaus_);
        scale1fb_ = cms2.evt_scale1fb();
        pthat_    = cms2.genps_pthat();
    }

    ntrks_      = cms2.trks_trk_p4().size();

}

void dilepbabymaker::FillBabyNtuple()
{
    babyTree_->Fill();
}

void dilepbabymaker::CloseBabyNtuple()
{
    babyFile_->cd();
    babyTree_->Write();
    babyFile_->Close();
}
