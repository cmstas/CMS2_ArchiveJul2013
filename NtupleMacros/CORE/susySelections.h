#ifndef SUSYSELECTIONS_H
#define SUSYSELECTIONS_H

#include <vector>
#include "Math/LorentzVector.h"
#include "muonSelections.h"

typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<float> > LorentzVector;

int leptonOrTauIsFromW(int idx, int id, bool alsoSusy = false);

//---------------------------------------------
// single muon triggers for lljj bump search
//---------------------------------------------

bool passMuMuJJTrigger_v1( bool isData );

/*****************************************************************************************/
//print event info
/*****************************************************************************************/
void printEventInfo();

/*****************************************************************************************/
//veto Z->mumugamma events
/*****************************************************************************************/
bool vetoZmumuGamma( unsigned int hypIdx , float emax = 6. , 
                     float minmass = 76. , float maxmass = 106.);

/*****************************************************************************************/
//passes the SUSY 2011 trigger selections
/*****************************************************************************************/
bool passSUSYTrigger2011_v1( bool isData , int hypType , bool highpt );

/*****************************************************************************************/
//passes the single-lepton SUSY 2011 trigger selections
/*****************************************************************************************/
bool passSingleLepSUSYTrigger2011_v1( bool isData , int lepType );
bool passSingleLep2JetSUSYTrigger2011( bool isData , int lepType );

/*****************************************************************************************/
//passes the SUSY trigger selections
/*****************************************************************************************/
bool passSUSYTrigger_v1( bool isData , int hypType );

/*****************************************************************************************/
//passes the simplified version of the SUSY trigger selections
/*****************************************************************************************/
bool passSimpleSUSYTrigger_v1( bool isData );

/*****************************************************************************************/
//hypothesis disambiguation. Returns the hypothesis that has mass closest to MZ
/*****************************************************************************************/
unsigned int selectBestZHyp(const vector<unsigned int> &v_goodHyps);

/*****************************************************************************************/
//generalized Z veto
/*****************************************************************************************/
bool ZVetoGeneral( float ptcut = 20 , float minmass = 76 ,  float maxmass = 106 , SelectionType = OSGeneric_v3 );

//---------------------------------------------
// Check if trigger is unprescaled and passes
//---------------------------------------------
bool passUnprescaledHLTTriggerPattern(const char* arg);


#endif

