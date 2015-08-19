#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

namespace VCAPS
{

typedef long long VLONG;

struct VirtualEvent
{
  double ripBase;
  double loss, reinstatementPrem, fullRip;
  string riskGroup;
  int eventId;
  int sequenceId;
  bool noncat;

  typedef vector<VirtualEvent> VEC;
  typedef map<int, VirtualEvent> MAP;
  typedef map<int, VirtualEvent>::iterator Iterator;
  typedef map<int, VirtualEvent>::const_iterator ConstIterator;
  typedef pair<int, VirtualEvent> Pair;

  VirtualEvent()
    : ripBase(0), loss(0), reinstatementPrem(0), 
    fullRip(0), riskGroup("NA"), eventId(0), sequenceId(0), noncat(false)
  {}

  VirtualEvent(int eId, double l, double rip)
    : ripBase(0), loss(loss), reinstatementPrem(rip), 
    fullRip(0), riskGroup("NA"), eventId(eId), noncat(false)
  {}

  VirtualEvent(int eId, double l, double rip, string rg)
    : ripBase(0), loss(l), reinstatementPrem(rip), 
    fullRip(0), riskGroup(rg), eventId(eId), noncat(false)
  {}
  
  VirtualEvent(int eId, double l, double rip, string rg, double fullrip)
    : ripBase(0), loss(l), reinstatementPrem(rip), 
    fullRip(fullrip), riskGroup(rg), eventId(eId), noncat(false)
  {}

  double get_lossNetOfReinstatePrem() { return loss - reinstatementPrem; }
  double get_lossNetOfFullRip() { return loss - fullRip; }

  void operator*=(double factor) {
    loss *= factor;
    reinstatementPrem *= factor;
  }

  void scale(double factor, string rg) {
    if(rg == "ALL" || rg == riskGroup) {
      loss *= factor;
      reinstatementPrem *= factor;
    }
  }
  
  void scale(double factor, vector<string> rgs) {
    if(std::find(rgs.begin(), rgs.end(), riskGroup)!=rgs.end()) {
      loss *= factor;
      reinstatementPrem *= factor;
    }
  }

  void scaleRP(double factor)
  {
    reinstatementPrem *= factor;
    if(abs(reinstatementPrem) > abs(loss)) reinstatementPrem = loss; 
  }

  void scaleFullRip(double factor)
  {
    fullRip *= factor;
    if(abs(fullRip) > abs(loss)) fullRip = loss; 
  }

  void combineReinstatePremToLoss()
  {
    loss -= reinstatementPrem;
    reinstatementPrem = 0.;
  }

  void add_loss(double newLoss) { loss += newLoss; }
  void add_reinstatementPrem(double newRp) { reinstatementPrem += newRp; }
  void add_fullRip(double newFullRip) { fullRip += newFullRip; }

  void operator+=(VirtualEvent& newEvent)
  {
	  if(eventId != newEvent.eventId)
	  {
      string rgTmp =newEvent.riskGroup;
		  transform(rgTmp.begin(), rgTmp.end(), rgTmp.begin(), (int(*)(int)) toupper);
		  if( rgTmp.substr(rgTmp.size()>4?rgTmp.size()-4:0) != "TERR") {
			  cerr << "Error: attempting to add two events with different event ids: "
				  << eventId << " in " << riskGroup << " != " << newEvent.eventId << " in " << rgTmp << endl;
		  }
	  }

	  loss += newEvent.loss;
	  reinstatementPrem += newEvent.reinstatementPrem;
	  fullRip += newEvent.fullRip;
  }
};

}