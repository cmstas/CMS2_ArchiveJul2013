from CMS2.NtupleMaker.RecoConfiguration2011_cfg import *

# Global Tag
process.GlobalTag.globaltag = "START39_V8::All"

# Load Filters
process.load("CMS2.NtupleMaker.hypFilter_cfi")
process.load("CMS2.NtupleMaker.dilepGenFilter_cfi")

# Dilepton Filter
process.EventSelectionDilFilt = cms.PSet (
  SelectEvents = cms.untracked.PSet (
    SelectEvents = cms.vstring('pWithHyp', 'pWithGenHyp')
  )
)

# Output
process.out = cms.OutputModule(
        "PoolOutputModule",
        process.EventSelectionDilFilt,
        fileName     = cms.untracked.string('ntuple.root'),
        dropMetaData = cms.untracked.string("NONE")
)
process.outpath      = cms.EndPath(process.out)

# Branches
process.out.outputCommands = cms.untracked.vstring( 'drop *' )
process.out.outputCommands.extend(cms.untracked.vstring('keep *_*Maker*_*_CMS2*'))
process.out.outputCommands.extend(cms.untracked.vstring('drop *_cms2towerMaker*_*_CMS2*'))
process.out.outputCommands.extend(cms.untracked.vstring('drop CaloTowers*_*_*_CMS2*'))

#
process.cms2WithEverything = cms.Sequence( process.kt6PFJets * process.cms2CoreSequence * process.cms2PFNoTauSequence * process.cms2GENSequence )
process.p                  = cms.Path( process.cms2WithEverything )
process.pWithHyp           = cms.Path( process.cms2WithEverything * process.hypFilter )
process.pWithGenHyp        = cms.Path( process.cms2WithEverything * process.dilepGenFilter )

#
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.eventMaker.isData                        = cms.bool(False)
