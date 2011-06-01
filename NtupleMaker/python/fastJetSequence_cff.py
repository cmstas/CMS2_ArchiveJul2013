import FWCore.ParameterSet.Config as cms

from RecoJets.JetProducers.kt4PFJets_cfi import *
from CMS2.NtupleMaker.fastJetMaker_cfi import *


kt6PFJetsDeterministicJEC = kt4PFJets.clone(
    rParam = 0.6,
    doAreaFastjet = True,
    doRhoFastjet = True,
    voronoiRfact = 0.9,
    Rho_EtaMax = 5.0,
    Ghost_EtaMax = 5.0
)

kt6PFJetsDeterministicIso = kt4PFJets.clone(
    rParam = 0.6,
    doAreaFastjet = True,
    doRhoFastjet = True,
    voronoiRfact = 0.9,
    Rho_EtaMax = 2.5,
    Ghost_EtaMax = 2.5
)




fastJetSequence = cms.Sequence( kt6PFJetsDeterministicJEC * kt6PFJetsDeterministicIso * fastJetMaker )