import FWCore.ParameterSet.Config as cms

prunedUncorrectedCMS2Jets = cms.EDFilter("JetCollectionPruner",
   inputUncorrectedJetCollection = cms.InputTag("ak5CaloJets"),
   uncorrectedJetPtCut           = cms.double(5.0) ##cut on the UNCORRECTED reco jets!!!!!
)

