#Contains the core CMS2 makers. Does not contain Gen or PAT makers
import FWCore.ParameterSet.Config as cms

from CMS2.NtupleMaker.aSkimFilter_cfi              import *
from CMS2.NtupleMaker.beamSpotMaker_cfi            import *
from CMS2.NtupleMaker.beamHaloSequence_cff         import *
from CMS2.NtupleMaker.bTaggingSequence_cfi         import *
from CMS2.NtupleMaker.bTagJPTSequence_cfi          import *
from CMS2.NtupleMaker.bTagPFSequence_cfi           import *
from CMS2.NtupleMaker.bTaggingTrkSequence_cfi      import *
from CMS2.NtupleMaker.bTagMaker_cfi                import *
from CMS2.NtupleMaker.bTagJPTJetMaker_cfi          import *
from CMS2.NtupleMaker.bTagTrkMaker_cfi             import *
from CMS2.NtupleMaker.conversionMaker_cfi          import *
from CMS2.NtupleMaker.elCaloIsoSequence_cff        import *
from CMS2.NtupleMaker.elTkJuraIsoMaker_cfi         import *
from CMS2.NtupleMaker.electronMaker_cfi            import *
from CMS2.NtupleMaker.electronSequence_cfi         import *
from CMS2.NtupleMaker.elToJetAssMaker_cfi          import *
from CMS2.NtupleMaker.elToMuAssMaker_cfi           import *
from CMS2.NtupleMaker.eventMaker_cfi               import *
from CMS2.NtupleMaker.gsfTrackMaker_cfi            import *
from CMS2.NtupleMaker.hcalNoiseSummaryMaker_cfi    import *
from CMS2.NtupleMaker.hltMaker_cff                 import *
from CMS2.NtupleMaker.hypDilepMaker_cfi            import *
from CMS2.NtupleMaker.hypDilepVertexMaker_cfi      import *
from CMS2.NtupleMaker.hypTrilepMaker_cfi           import *
from CMS2.NtupleMaker.hypQuadlepMaker_cfi          import *
from CMS2.NtupleMaker.hypIsoMaker_cfi              import *
from CMS2.NtupleMaker.jetSequence_cff              import *
from CMS2.NtupleMaker.jetMaker_cfi                 import *
from CMS2.NtupleMaker.jetToElAssMaker_cfi          import *
from CMS2.NtupleMaker.jetToMuAssMaker_cfi          import *
from CMS2.NtupleMaker.jptSequence_cff              import *
from CMS2.NtupleMaker.jptMaker_cfi                 import *
from CMS2.NtupleMaker.l1Maker_cfi                  import *
from CMS2.NtupleMaker.metSequence_cff              import *
from CMS2.NtupleMaker.metMaker_cfi                 import *
from CMS2.NtupleMaker.muonMaker_cfi                import *
from CMS2.NtupleMaker.muToElsAssMaker_cfi          import *
from CMS2.NtupleMaker.muToJetAssMaker_cfi          import *
from CMS2.NtupleMaker.photonMaker_cfi              import *
from CMS2.NtupleMaker.scMaker_cfi                  import *
from CMS2.NtupleMaker.tcmetSequence_cff            import *
from CMS2.NtupleMaker.trackMaker_cfi               import *
from CMS2.NtupleMaker.trackToElsAssMaker_cfi       import *
from CMS2.NtupleMaker.trackToMuonAssMaker_cfi      import *
from CMS2.NtupleMaker.trkJetMaker_cfi              import *
from CMS2.NtupleMaker.trkToVtxAssMaker_cfi         import *
from CMS2.NtupleMaker.vertexMaker_cfi import *


CMS2Reco         = cms.Sequence(egammaElectronIDCMS2 * JPTCorrections * cms2CaloJetSequence * metCorSequence  * CMS2Btagging  * CMS2JPTBtagging * CMS2PFBtagging * CMS2TrkBtagging * cms2beamHaloSequence)
                 
eventmakers      = cms.Sequence(beamSpotMaker * vertexMaker * eventMaker * hcalNoiseSummaryMaker)
                 
trigmakers       = cms.Sequence(l1Maker * hltMakerSequence)
                 
makers           = cms.Sequence(trackMaker * muonMaker * scMaker * electronMaker * photonMaker * jetMaker  *  jptMaker * metMaker * tcmetSequence * trkJetMaker * gsfTrackMaker)
                 
assmakers        = cms.Sequence(jetToMuAssMaker * jetToElAssMaker * muToElsAssMaker * muToJetAssMaker * elToMuAssMaker * elToJetAssMaker * trackToMuonAssMaker * trackToElsAssMaker * trkToVtxAssMaker)
                 
hypmakers        = cms.Sequence(hypDilepMaker * hypDilepVertexMaker * hypTrilepMaker * hypQuadlepMaker * hypIsoMaker)
                 
othermakers      = cms.Sequence(elCaloIsoSequence * elTkJuraIsoMaker * bTagMaker *  bTagJPTJetMaker * bTagTrkMaker * conversionMaker)

cms2CoreSequence = cms.Sequence(CMS2Reco * eventmakers * trigmakers * makers * assmakers * othermakers * hypmakers)

