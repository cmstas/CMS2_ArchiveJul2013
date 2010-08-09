#include "monitor.h"
#include "CORE/CMS2.h"
#include <fstream>

MonitorEventId::MonitorEventId(CMS2& cms2){
  run = cms2.evt_run();
  event = cms2.evt_event();
  lumi = cms2.evt_lumiBlock();
}

MonitorEventId::MonitorEventId(){
  run = 0;
  event = 0;
  lumi = 0;
}

Entry::Entry()
{
  for (unsigned int i=0; i<4; ++i){
    nhyp[i] = 0;
    nevt[i] = 0;
    seen[i] = false;
  }
}

void hypo_monitor::count(CMS2& cms2, HypothesisType type, const char* name)
{
  std::vector<Entry>::iterator itr = counters.begin();
  while (itr != counters.end() && itr->name != name) itr++;
  Entry* entry(0);
  if ( itr == counters.end() ){
    counters.push_back(Entry());
    entry = &counters.back();
    entry->name = name;
  } else {
    entry = &*itr;
  }
  MonitorEventId id(cms2);
  entry->nhyp[type]++;
  entry->nhyp[ALL]++;
  if (id != entry->lastEvent){
    for (unsigned int i=0; i<4; ++i) entry->seen[i] = false;
    entry->nevt[type]++;
    entry->nevt[ALL]++;
    entry->seen[type] = true;
    entry->lastEvent = id;
  } else {
    if ( !entry->seen[type] ){
      entry->nevt[type]++;
      entry->seen[type] = true;
    }
  }
}

void hypo_monitor::print() const
{
  std::cout << "Total number of processed events: \t" << nEvtProcessed << std::endl;
  for ( unsigned int i=0; i<counters.size(); ++i ) 
    std::cout << Form("%-40s \thyps: %u/%u/%u/%u \tnevts: %u/%u/%u/%u", counters[i].name.c_str(),
		      counters[i].nhyp[EE],counters[i].nhyp[EM],counters[i].nhyp[MM],counters[i].nhyp[ALL],
		      counters[i].nevt[EE],counters[i].nevt[EM],counters[i].nevt[MM],counters[i].nevt[ALL]) 
	      << std::endl;
}