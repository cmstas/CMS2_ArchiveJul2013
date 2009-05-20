import FWCore.ParameterSet.Config as cms

from CMS2.NtupleMaker.jetMaker_cfi import *
from CMS2.NtupleMaker.jetCollectionPruner_cfi import *

cms2CaloJetSequence = cms.Sequence(prunedUncorrectedCMS2Jets*jetMaker)
