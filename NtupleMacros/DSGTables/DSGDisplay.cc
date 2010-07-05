#include <assert.h>
#include <math.h>
#include <ncurses.h>
#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "THStack.h"
#include "DSGTable.h"

/*  cheat sheet ...
i   Z/notZ     major row
j   MET        major column
jj  sumJetEt   alt major row
k   njets      minor column
l   bucket     minor row

*/

enum layout {ZMET, JETMET};

static const DSGTable **dsgs_;
static int n_dsgs_;

int nBucketsGroups[4] = {10, 6, 4, 3};
int nBucketsPerGroup[4][10] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
  {2, 2, 2, 2, 1, 1},
  {4, 2, 2, 2},
  {6, 2, 2}
};
int buckets[4][10][10] = {
  { {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9} },
  { {0,1},    {2,3},    {4,5},    {6,7},    {8}, {9} },   
  { {0,1,2,3},          {4,5},    {6,7},    {8,9} },    
  { {0,1,2,3,4,5},    {6,7},    {8,9} },   
};
const static char bucketStrs[4][10][1280] = {
  {"e+e+", "e-e-", "m+m+", "m-m-", "e+m+", "e-m-", "e+m-", "m+e-", "e+e-", "m+m-"},
  {"e+e+ e-e-",    "m+m+ m-m-",    "e+m+ e-m-",    "e+m- m+e-",    "e+e-", "m+m-"},
  {"SS SF",    "SS OF",    "OS OF",    "OS SF"},
  {"SS *F",    "OS OF",    "OS SF"},
};

static void printNumbers (int i, int j, int k, int l, enum layout ZJ, int whichBucketGrouping)
{
  double sm = 0;
  double smw2 = 0;
  double data = 0; 
  double dataw2 = 0;
  for (int n = 0 ; n < nBucketsPerGroup[whichBucketGrouping][l]; n++) {
    for (int m = 0; m < n_dsgs_ - 1; ++m) {
      if (ZJ == ZMET) {
	sm += dsgs_[m]->events_[i][j][0][k][buckets[whichBucketGrouping][l][n]];
	smw2 += dsgs_[m]->w2s_[i][j][0][k][buckets[whichBucketGrouping][l][n]];
      } else {
	sm += dsgs_[m]->events_[2][j][i][k][buckets[whichBucketGrouping][l][n]];
	smw2 += dsgs_[m]->w2s_[2][j][i][k][buckets[whichBucketGrouping][l][n]];
      }
    }
    if (ZJ == ZMET) {
      data += dsgs_[n_dsgs_ - 1]->events_[i][j][0][k][buckets[whichBucketGrouping][l][n]];
      dataw2 += dsgs_[n_dsgs_ - 1]->w2s_[i][j][0][k][buckets[whichBucketGrouping][l][n]];
    } else {
      data += dsgs_[n_dsgs_ - 1]->events_[2][j][i][k][buckets[whichBucketGrouping][l][n]];
      dataw2 += dsgs_[n_dsgs_ - 1]->w2s_[2][j][i][k][buckets[whichBucketGrouping][l][n]];
    }
  }
  double sig = (data - sm) / sqrt(smw2 + dataw2);
  if (sig > 3)
    attron(COLOR_PAIR(1));
  mvprintw(3 + i * 25 + (l + 1) * 2,
	   10 + j * 50 + (k + 1) * 10,
	   "%6.2f ", sm);
  mvprintw(4 + i * 25 + (l + 1) * 2,
	   10 + j * 50 + (k + 1) * 10,
	   "%6.2f", data);
  if (sig > 3)
    attroff(COLOR_PAIR(1));
}

static void plotTheDistribution (DSGTable::table_t DSGTable::* h, 
				 int i, int j, int k, int l, enum layout ZJ, int whichBucketGrouping)
{

  THStack *stack = new THStack((dsgs_[0]->*h)[0][0][0][0][0]->GetName(), (dsgs_[0]->*h)[0][0][0][0][0]->GetTitle() );
  
  for (int m = 0; m < n_dsgs_ - 1; ++m) {
    TH1F *hist;
    if (ZJ == ZMET) {
      hist = dynamic_cast<TH1F*> ((dsgs_[m]->*h)[i][j][0][k][buckets[whichBucketGrouping][l][0]]->Clone());
    } else {
      hist = dynamic_cast<TH1F*> ((dsgs_[m]->*h)[2][j][i][k][buckets[whichBucketGrouping][l][0]]->Clone());
    }
    for (int n = 1 ; n < nBucketsPerGroup[whichBucketGrouping][l]; n++) {
      if (ZJ == ZMET) {
	hist->Add((dsgs_[m]->*h)[i][j][0][k][buckets[whichBucketGrouping][l][n]]);
      } else {
	hist->Add((dsgs_[m]->*h)[2][j][i][k][buckets[whichBucketGrouping][l][n]]);
      }
    }
    stack->Add(hist);
  }
  TH1F *data;
  if (ZJ == ZMET) {
    data = dynamic_cast<TH1F*> ((dsgs_[n_dsgs_ - 1]->*h)[i][j][0][k][buckets[whichBucketGrouping][l][0]]->Clone());
  } else {
    data = dynamic_cast<TH1F*> ((dsgs_[n_dsgs_ - 1]->*h)[2][j][i][k][buckets[whichBucketGrouping][l][0]]->Clone());
  }
  for (int n = 1 ; n < nBucketsPerGroup[whichBucketGrouping][l]; n++) {
    if (ZJ == ZMET) {
      data->Add((dsgs_[n_dsgs_ - 1]->*h)[i][j][0][k][buckets[whichBucketGrouping][l][n]]);
    } else {
      data->Add((dsgs_[n_dsgs_ - 1]->*h)[2][j][i][k][buckets[whichBucketGrouping][l][n]]);
    }
  }
  data->SetFillStyle(0);
  data->SetMarkerStyle(30);
  data->Draw("pee");
  stack->Draw("same");
  data->Draw("same pee");
}

static void displayHistos (int i, int j, int k, int l, enum layout ZJ, int whichBucketGrouping)

{
     static TCanvas *c = 0;
     if (c == 0) {
	  c = new TCanvas("c", "c", 1200, 800);
// 	  c->Update();
// 	  c->SetWindowSize(c->GetWindowWidth() * 2, c->GetWindowHeight() * 2);
// 	  c->Update();
     }
     c->Clear();
     c->Divide(4, 2);
     int i_pad = 0;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hmet_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hmll_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hht_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hjsumet_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hmaxjetpt_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hmaxleppt_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->cd(++i_pad);
     plotTheDistribution (&DSGTable::hlepdphi_, i, j, k, l, ZJ, whichBucketGrouping) ;
     c->Update();
}


void printTable(enum layout ZJ, int whichBucketGrouping)
{
     
  clear();
  mvprintw(0, 20, "  MET > 35");
  mvprintw(0, 70, "  MET > 100");
  mvprintw(0, 120, "  MET > 175");
  if (ZJ == ZMET) {  
    mvprintw(15, 0, "no Z");
    mvprintw(40, 0, "Z");
    for (int i = 0; i < DSGTable::nZcat; ++i) {
      for (int j = 0; j < DSGTable::nMETcat; ++j) {
	mvprintw(2 + i * 25, 30 + j * 50, "Njet");
	mvprintw(3 + i * 25, 22 + j * 50, "<2        2-4       >4");
	for (int k = 0; k < DSGTable::nJetcat; ++k) { 
	  for (int l = 0; l < nBucketsGroups[whichBucketGrouping]; ++l) { // # of elements in current grp: 10, 5, 4 elements
	    printNumbers(i, j, k, l, ZJ, whichBucketGrouping);
	    if (k == 0) {
	      mvprintw(3 + i * 25 + (l + 1) * 2,
		       10 + j * 50, bucketStrs[whichBucketGrouping][l]);
	    }
	  }
	}
      }
    }
  } else {
    mvprintw(15, 0, "SJPT>0");
    mvprintw(40, 0, "SJPT>300");
    mvprintw(65, 0, "SJPT>500");
    for (int i = 0; i < DSGTable::nSumJetcat; ++i) {
      for (int j = 0; j < DSGTable::nMETcat; ++j) {
	mvprintw(2 + i * 25, 30 + j * 50, "Njet");
	mvprintw(3 + i * 25, 22 + j * 50, "<2        2-4       >4");
	for (int k = 0; k < DSGTable::nJetcat; ++k) { 
	  for (int l = 0; l < nBucketsGroups[whichBucketGrouping]; ++l) { // # of elements in current grp: 10, 5, 4 elements
	    printNumbers(i, j, k, l, ZJ, whichBucketGrouping);
	    if (k == 0) {
	      mvprintw(3 + i * 25 + (l + 1) * 2,
		       10 + j * 50, bucketStrs[whichBucketGrouping][l]);
	    }
	  }
	}
      }
    }
  }
  refresh();
}


void DSGDisplay ()
{       
     initscr();                       /* Start curses mode            */
     raw();                           /* Line buffering disabled      */
     keypad(stdscr, TRUE);            /* We get F1, F2 etc..          */
     noecho();                        /* Don't echo() while we do getch */
     // get the file with the tables
     TFile *f = TFile::Open("Results.root", "read");
     assert(f != 0);
     DSGTable *dsg_ww 		= dynamic_cast<DSGTable *>(f->Get("ww"));
     DSGTable *dsg_wz   	= dynamic_cast<DSGTable *>(f->Get("wz"));
     DSGTable *dsg_zz   	= dynamic_cast<DSGTable *>(f->Get("zz"));
     DSGTable *dsg_wjets	= dynamic_cast<DSGTable *>(f->Get("wjets"));
     DSGTable *dsg_dyee 	= dynamic_cast<DSGTable *>(f->Get("dyee"));
     DSGTable *dsg_dymm 	= dynamic_cast<DSGTable *>(f->Get("dymm"));
     DSGTable *dsg_dytt 	= dynamic_cast<DSGTable *>(f->Get("dytt"));
     DSGTable *dsg_ttbar	= dynamic_cast<DSGTable *>(f->Get("ttbar"));
//      DSGTable *dsg_tw   	= dynamic_cast<DSGTable *>(f->Get("tw"));
     DSGTable *dsg_data 	= dynamic_cast<DSGTable *>(f->Get("data"));
     const DSGTable *dsgs[] = { 
	  dsg_ww          ,
 	  dsg_wz          ,
 	  dsg_zz          ,
 	  dsg_wjets       ,
 	  dsg_dyee        ,
 	  dsg_dymm        ,
 	  dsg_dytt        ,
 	  dsg_ttbar       ,
// 	  dsg_tw          ,
  	  dsg_data        ,
     };
     const int n_dsgs = sizeof(dsgs) / sizeof(DSGTable *);
     dsgs_ = dsgs;
     n_dsgs_ = n_dsgs;

     // print table
     start_color();
     init_pair(1, COLOR_RED, COLOR_BLACK);
     printTable(JETMET, 0);

     refresh();
     int i = 0;
     int j = 0;
     int k = 0;
     int l = 0;
     enum layout ZJ = JETMET;
     int iBucketGrouping = 0;
     attron(A_REVERSE);
     printNumbers(i, j, k, l, ZJ, iBucketGrouping);
     while (1) {
	  // read input
	  int ch = getch();
	  if (tolower(ch) == 'q')
	       break;
	  // unhighlight current cell
	  attroff(A_REVERSE);
	  printNumbers(i, j, k, l, ZJ, iBucketGrouping );
	  switch (ch) {
	  case KEY_UP: case 'k': case 'K':
	       if (l == 0) {
		    if (i != 0) {
			 i--;
			 l = nBucketsGroups[iBucketGrouping] - 1;
		    }
	       } else {
		    l--;
	       }
	       break;
	  case KEY_DOWN: case 'j': case 'J':
	       if (l == nBucketsGroups[iBucketGrouping] - 1) {
		    if (ZJ == ZMET && i != DSGTable::nZcat - 1 ||
			ZJ == JETMET && i != DSGTable::nSumJetcat - 1) {
			 i++;
			 l = 0;
		    }
	       } else {
		    l++;
	       }
	       break;
	  case KEY_LEFT: case 'h': case 'H':
	       if (k == 0) {
		    if (j != 0) {
			 j--;
			 k = DSGTable::nJetcat - 1;
		    }
	       } else {
		    k--;
	       }
	       break;
	  case KEY_RIGHT: case 'l': case 'L':
	       if (k == DSGTable::nJetcat - 1) {
		    if (j != DSGTable::nMETcat - 1) {
			 j++;
			 k = 0;
		    }
	       } else {
		    k++;
	       }
	       break;
	  case 'b': case 'B':
	    iBucketGrouping++;
	    iBucketGrouping %= 4;
	    // for now, if we've gone beyond the end of the new
	    // grouping, we get put at the new end; ideally, we would
	    // be put into the bucket group that swallowed the old
	    // bucket group
	    if (l > nBucketsGroups[iBucketGrouping] - 1)
		 l = nBucketsGroups[iBucketGrouping] - 1;
	    printTable(ZJ, iBucketGrouping);
	    break;
	  case 'z': case 'Z':
	    if (ZJ == ZMET) {
	      ZJ = JETMET;
	    } else {
	      ZJ = ZMET;
	    }
	    // if we are in the 3rd major row, switch to 2 major rows.
	    if (ZJ == ZMET) {
	      if (i == DSGTable::nSumJetcat - 1) {
		i =  DSGTable::nZcat - 1;
	      }
	    }
	    printTable(ZJ, iBucketGrouping);
	    break;
	  case '\n':
	    displayHistos(i, j, k, l, ZJ, iBucketGrouping);
	    break;
	  default:
	       break;
	  }
	  // highlight new cell
	  attron(A_REVERSE);
	  printNumbers(i, j, k, l, ZJ, iBucketGrouping);
	  attroff(A_REVERSE);
	  refresh();
     } 
     endwin(); 
     return;
}
