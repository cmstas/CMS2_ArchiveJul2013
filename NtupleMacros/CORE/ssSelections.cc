#include <assert.h>
#include <algorithm>
#include "Math/LorentzVector.h"
#include "Math/VectorUtil.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TDatabasePDG.h"
#include "electronSelections.h"
#include "electronSelectionsParameters.h"
#include "muonSelections.h"
#include "metSelections.h"
#include "triggerUtils.h"
#include "eventSelections.h"
#include "CMS2.h"

#include "ssSelections.h"

using namespace tas;

/******************************************************************************************/     
// good lepton (either mu or electron, no isolation cuts)
/******************************************************************************************/
bool isGoodLeptonNoIsoSS (int id, int lepIdx, bool applyAlignmentCorrection, bool removedEtaCutInEndcap)
{
	 //electrons
	 if (abs(id) == 11)
	 {  
		  const cuts_t elIDcuts =		(1ll<<ELEIP_200)                |                                                                  
										(1ll<<ELENOMUON_010)            |
										(1ll<<ELENOTCONV_HITPATTERN)    |
										(1ll<<ELENOTCONV_DISTDCOT002)   |
										(1ll<<ELESCET_010)              |
										(1ll<<ELEPT_010)				|
										(1ll<<ELEETA_250)				|
										(1ll<<ELENOSPIKE_SWISS005)		|
										(1ll<<ELESEED_ECAL);
    
		  unsigned int answer_vbtf = electronId_VBTF(lepIdx, VBTF_35X_80, applyAlignmentCorrection, removedEtaCutInEndcap);
		  bool elsvbtf80_ = ( ( answer_vbtf & (1ll<<ELEID_ID) ) == (1ll<<ELEID_ID) );
		  return (pass_electronSelection(lepIdx, elIDcuts, applyAlignmentCorrection, removedEtaCutInEndcap) && elsvbtf80_);
	 }

	 //muons
	 if (abs(id) == 13)
	 {
		  if (muonIdNotIsolated(lepIdx, Nominal)) 
			   return true;

		  return false;
	 }

	 return false;
}


/******************************************************************************************/     
// isolated lepton (either mu or electron)
/******************************************************************************************/
bool isGoodLeptonwIsoSS (int id, int lepIdx, bool applyAlignmentCorrection, bool removedEtaCutInEndcap)
{
	 if(!isGoodLeptonNoIsoSS(id, lepIdx, applyAlignmentCorrection, removedEtaCutInEndcap))
		  return false;

	 // 11 is a electron
	 if (abs(id)== 11)
	 {
		  const cuts_t elISOcuts =   (1ll<<ELEISO_REL010);
		  if (!pass_electronSelection(lepIdx, elISOcuts, applyAlignmentCorrection, removedEtaCutInEndcap))
			   return false;
	 }

	 // 13 is a muon
	 if(abs(id) == 13) 
		  if(muonIsoValue(lepIdx) > 0.10)  return false;
        
	 return true;
}


/******************************************************************************************/     
// are the leptons in the hypothesis good (all cuts but isolation?)
/******************************************************************************************/
bool isGoodHypNoIsoSS (int hypIdx, bool applyAlignmentCorrection, bool removedEtaCutInEndcap)
{
	 if(!isGoodLeptonNoIsoSS(hyp_lt_id()[hypIdx], hyp_lt_index()[hypIdx], applyAlignmentCorrection, removedEtaCutInEndcap))//, used0wrtPV)
		  return false;
	 if(!isGoodLeptonNoIsoSS(hyp_ll_id()[hypIdx], hyp_ll_index()[hypIdx], applyAlignmentCorrection, removedEtaCutInEndcap))//, used0wrtPV)
		  return false;

	 return true;
}


/******************************************************************************************/     
// are the leptons in the hypothesis isolated?
/******************************************************************************************/     
bool isGoodHypwIsoSS (int hypIdx, bool applyAlignmentCorrection, bool removedEtaCutInEndcap)
{
	 if(!isGoodLeptonwIsoSS(hyp_lt_id()[hypIdx], hyp_lt_index()[hypIdx], applyAlignmentCorrection, removedEtaCutInEndcap))
		  return false;
	 if(!isGoodLeptonwIsoSS(hyp_ll_id()[hypIdx], hyp_ll_index()[hypIdx], applyAlignmentCorrection, removedEtaCutInEndcap))
		  return false;

	 return true;
}

/*****************************************************************************************/
//passes the EGamma triggers
/*****************************************************************************************/
bool passEGTriggerSS (unsigned int hypIdx, bool mc)
{
	 unsigned int hypType = cms2.hyp_type()[hypIdx];	 

	 if (mc)
	 {
		  if (passHLTTrigger("HLT_Ele10_LW_L1R"))
			   return true;

		  if (passHLTTrigger("HLT_Ele10_LW_EleId_L1R"))
			   return true;

		  if (passHLTTrigger("HLT_Ele15_LW_L1R"))
			   return true;

		  if (passHLTTrigger("HLT_DoubleEle5_SW_L1R"))
			   return true;
	 } 
	 else // data now
	 {
		  if(evt_run() < 138000)
		  {
			   if (passHLTTrigger("HLT_Ele10_LW_L1R"))
					return true;

			   if (passHLTTrigger("HLT_Ele15_LW_L1R"))
					return true;

			   if (hypType == 3)
					if (passHLTTrigger("HLT_DoubleEle5_SW_L1R"))
						 return true;
		  }

		  if(evt_run() >= 138000 && evt_run() < 141900)
		  {
			   if (passHLTTrigger("HLT_Ele10_LW_EleId_L1R"))
					return true;

			   if (passHLTTrigger("HLT_Ele15_LW_L1R"))
					return true;

			   if (hypType == 3)
					if (passHLTTrigger("HLT_DoubleEle5_SW_L1R"))
						 return true;
		  }

		  if (cms2.evt_run() >= 141900)
		  {
			   if (passHLTTrigger("HLT_Ele10_SW_EleId_L1R"))
					return true;

			   if (passHLTTrigger("HLT_Ele15_SW_CaloEleId_L1R"))
					return true;

			   if (passHLTTrigger("HLT_Ele15_SW_EleId_L1R"))
					return true;

			   if (hypType == 3)
					if (passHLTTrigger("HLT_DoubleEle10_SW_L1R"))
						 return true;
		  }
	 }

	 return false;
}

/*****************************************************************************************/
//passes the muon triggers
/*****************************************************************************************/
bool passMuTriggerSS(unsigned int hypIdx)
{
	 if (passHLTTrigger("HLT_Mu9"))
		  return true;

	 if (cms2.hyp_type()[hypIdx] == 0)
		  if (passHLTTrigger("HLT_DoubleMu3"))
			   return true;

	 return false;
}

/*****************************************************************************************/
// extra Z veto
/*****************************************************************************************/
bool makesExtraZ (int hypIdx, bool applyAlignmentCorrection, bool removedEtaCutInEndcap)
{
 	 LorentzVector ltp4	= cms2.hyp_lt_p4()[hypIdx];
	 int           ltid	= abs(cms2.hyp_lt_id()[hypIdx]);
	 int           ltch = cms2.hyp_lt_charge()[hypIdx];

	 LorentzVector llp4	= cms2.hyp_ll_p4()[hypIdx];
	 int           llid	= abs(cms2.hyp_ll_id()[hypIdx]);
	 int           llch = cms2.hyp_ll_charge()[hypIdx];

	 if (ltid == 13 || llid == 13)
	 {
		  for (unsigned int imu = 0; imu < cms2.mus_p4().size(); imu++)
		  {
			   if (cms2.mus_p4()[imu].pt() < 10.)
					continue;

			   if (!isGoodLeptonwIsoSS(13, imu, applyAlignmentCorrection, removedEtaCutInEndcap))
					continue;

			   if (ltid == 13 && ltch * cms2.mus_charge()[imu] < 0)
			   {
					LorentzVector temp = ltp4 + cms2.mus_p4()[imu];
					if (inZmassWindow(sqrt(temp.M2())))
						 return true;
			   }
		  
			   if (llid == 13 && llch * cms2.mus_charge()[imu] < 0)
			   {
					LorentzVector temp = llp4 + cms2.mus_p4()[imu];
					if (inZmassWindow(sqrt(temp.M2())))
						 return true;
			   }
		  }
	 }

	 if (ltid == 11 || llid == 11)
	 {
		  for (unsigned int iel = 0; iel < cms2.els_p4().size(); iel++)
		  {
			   if (cms2.els_p4()[iel].pt() < 10.)
					continue;

			   if (!isGoodLeptonwIsoSS(11, iel, applyAlignmentCorrection, removedEtaCutInEndcap))
					continue;

			   if (ltid == 11 && ltch * cms2.els_charge()[iel] < 0)
			   {
					LorentzVector temp = ltp4 + cms2.els_p4()[iel];
					if (inZmassWindow(sqrt(temp.M2())))
						 return true;
			   }
		  
			   if (llid == 11 && llch * cms2.els_charge()[iel] < 0)
			   {
					LorentzVector temp = llp4 + cms2.els_p4()[iel];
					if (inZmassWindow(sqrt(temp.M2())))
						 return true;
			   }
		  }
	 }

	 return false;
}