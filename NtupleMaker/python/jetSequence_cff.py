import FWCore.ParameterSet.Config as cms

from CMS2.NtupleMaker.jetCollectionPruner_cfi import *

cms2JetSequence = cms.Sequence(prunedUncorrectedCMS2Jets*cms2ak5JetID)