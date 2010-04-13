
#ifndef PLOTRESULTS_H
#define PLOTRESULTS_H

#include "TString.h"

class TCanvas;
class HistogramUtilities;
class THStack;
class TArrow;

void plotStack(HistogramUtilities &h1, TString name, TString titleX, TString saveName, TString det, int rebin = 1, float cutValEB = -1.0, float cutValEE = -1.0);
void plotDataRefOverlayStack(HistogramUtilities &hData, HistogramUtilities &hRef, TString name, TString titleX, TString saveName, TString det, int rebin,  float cutValEB = -1.0, float cutValEE = -1.0);
void plotElectronIDStack(HistogramUtilities &hData, HistogramUtilities &hRef, TString name, TString titleX, TString saveName, TString det, int rebin);
TArrow *getArrow(THStack *st, TString det, float cutValEB, float cutValEE, float max = -1.0);

void plotResultsW(TString det, TString fileStamp, TString refFileStamp, const float &luminorm);
void plotResultsQCDStudy(TString det, TString fileStamp, TString refFileStamp, const float &luminorm);

#endif

