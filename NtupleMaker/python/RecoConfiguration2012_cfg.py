# Import Python Modules
import FWCore.ParameterSet.Config as cms
from Configuration.EventContent.EventContent_cff        import *
from JetMETCorrections.Type1MET.MetType1Corrections_cff import *

# CMS2
process = cms.Process("CMS2")

# Version Control For Python Configuration Files
process.configurationMetadata = cms.untracked.PSet(
        version    = cms.untracked.string('$Revision: 1.7 $'),
        annotation = cms.untracked.string('CMS2'),
        name       = cms.untracked.string('CMS2 test configuration')
)

# load event level configurations
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load('Configuration/EventContent/EventContent_cff')
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.load('JetMETCorrections.Configuration.DefaultJEC_cff')
process.load("RecoJets.Configuration.RecoJPTJets_cff")
process.load("TrackPropagation.SteppingHelixPropagator.SteppingHelixPropagatorAny_cfi")
process.load("TrackingTools.TrackAssociator.DetIdAssociatorESProducer_cff")
process.load("CMS2.NtupleMaker.cms2CoreSequences_cff")
process.load("CMS2.NtupleMaker.cms2GENSequence_cff")
process.load('CMS2.NtupleMaker.pixelDigiMaker_cfi')
process.load("CMS2.NtupleMaker.cms2HFCleaningSequence_cff")
process.load("CMS2.NtupleMaker.cms2HcalCleaningSequence_cff")
process.load("CMS2.NtupleMaker.cms2PFSequence_cff")
process.load('RecoJets.Configuration.RecoPFJets_cff') # Import the Jet RECO modules
process.kt6PFJets.doRhoFastjet  = True                # Turn-on the FastJet density calculation
process.ak5PFJets.doAreaFastjet = True                # Turn-on the FastJet jet area calculation for your favorite algorithm


####################
# MET Filters 2012 #
####################

process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
#process.load('RecoMET.METAnalyzers.CSCHaloFilter_cfi')
process.load('RecoMET.METFilters.hcalLaserEventFilter_cfi')
process.load('RecoMET.METFilters.EcalDeadCellBoundaryEnergyFilter_cfi')
process.load('RecoMET.METFilters.EcalDeadCellDeltaRFilter_cfi')
process.load('RecoMET.METFilters.EcalDeadCellTriggerPrimitiveFilter_cfi')
process.load('RecoMET.METFilters.trackingFailureFilter_cfi')
process.load('RecoMET.METFilters.inconsistentMuonPFCandidateFilter_cfi')
process.load('RecoMET.METFilters.greedyMuonPFCandidateFilter_cfi')


#undo what's pulled in by including Reconstruction_cff
#it relies on transient steps introduced in PF in 44X (back-fill)
process.pfPileUp.PFCandidates = cms.InputTag("particleFlow")
process.pfNoPileUp.bottomCollection = cms.InputTag("particleFlow") 

#
metJESCorAK5CaloJet.inputUncorJetsLabel = cms.string("ak5CaloJets")

# Input
process.source = cms.Source("PoolSource",
    dropDescendantsOfDroppedBranches = cms.untracked.bool(False),
    skipEvents = cms.untracked.uint32(0),
    fileNames  = cms.untracked.vstring(
      #'file:/nfs-3/userdata/cms2/RelValProdTTbar_CMSSW_5_2_2-START52_V4-v2/82900610-FC74-E111-B01A-0018F3D09628.root'   # CMSSW_5_2_2
#      'file:/nfs-3/userdata/cms2/cms2_validation/RelValProdTTbar_CMSSW_5_2_3-START52_V5-v1/144D6226-2C7A-E111-8629-003048678B7C.root'   # CMSSW_5_2_3
#      'file:/nfs-3/userdata/cms2/cms2_validation/RelValTTbar_CMSSW_5_2_3_patch3-START52_V9_special_120410-v1/F8D46BF0-1083-E111-9B6A-001A92811728.root'   # CMSSW_5_2_3_patch3
#        'file:/nfs-3/userdata/fgolf/synchronization/isoSynchFile_DoubleMu191700.root' #muon synch
        'file:/nfs-3/userdata/fgolf/synchronization/DoubleElectron_Run2012_synchFile.root' #electron synch
   ),
    #--- Uncomment to emulate AOD with RECO --- #
    #inputCommands = process.AODEventContent.outputCommands,
)

# Speed up I/O from castor
process.AdaptorConfig = cms.Service (
  "AdaptorConfig",
  stats = cms.untracked.bool(True),
  enable = cms.untracked.bool(True),
  cacheHint = cms.untracked.string("lazy-download"),
  readHint = cms.untracked.string("auto-detect")
)

# Options
process.options                       = cms.untracked.PSet( Rethrow = cms.untracked.vstring('ProductNotFound') )
process.source.noEventSort            = cms.untracked.bool( True )
process.MessageLogger.cerr.threshold  = ''

# Number of Events to Process
process.maxEvents                     = cms.untracked.PSet( input = cms.untracked.int32(500) )

# Hypothesis cuts
process.hypDilepMaker.TightLepton_PtCut  = cms.double(20.0)
process.hypDilepMaker.LooseLepton_PtCut  = cms.double(10.0)
process.hypTrilepMaker.TightLepton_PtCut = cms.double(20.0)
process.hypTrilepMaker.LooseLepton_PtCut = cms.double(10.0)

# Event Maker
process.eventMaker.datasetName = cms.string("")
process.eventMaker.CMS2tag     = cms.string("")
