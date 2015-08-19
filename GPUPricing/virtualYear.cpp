#include "virtualYear.h"
#include <cctype> // for toupper
#include <algorithm>
#include <functional>

namespace VCAPS
{

void VirtualYear::addVirtualEvent(int sequenceId, VirtualEvent e, double factor, 
                                  VLONG iterId, bool addhead)
{
  this->iterId = iterId;

  string newRG = e.riskGroup;
  e *= factor;

  VirtualEvent::Iterator iE = _events.find(sequenceId);
  if (iE == _events.end()){
    e.sequenceId = sequenceId;
    if (addhead)
      _head_events.push_back(e);
    _events[sequenceId] = e;
  }
  else {
    string existingRG = iE->second.riskGroup;
    if(newRG.find("Noncat-") == string::npos || existingRG == newRG) 
      iE->second += e; 
    else {
      int newSeqId;
      int c = 500;
      do {
        c++;
        newSeqId = sequenceId - (sequenceId % 1000) + c;
      } while( _events.find(newSeqId) != _events.end() && c != 999);
      if(c != 999)
        _events[newSeqId] = e;
      else
        exit(0);
    }
  }
}

void VirtualYear::addVirtualEvents(VirtualYear& y)
{
  for (VirtualEvent::VEC::iterator ie = y._head_events.begin(); ie != y._head_events.end(); ++ie)
    addVirtualEvent(ie->sequenceId, *ie, 1.0, y.iterId);
  y._head_events.clear();
}

VirtualYear& VirtualYear::operator+=(const VirtualYear& newVirtualYear)
{
  const VirtualEvent::MAP & newEvents = newVirtualYear._events;
  for(VirtualEvent::ConstIterator iE = newEvents.begin(); iE != newEvents.end(); iE++)
    addVirtualEvent(iE->first, iE->second, 1, 0, false);
  return *this;
}

VirtualYear& VirtualYear::operator-=(const VirtualYear& newVirtualYear)
{
  const VirtualEvent::MAP & newEvents = newVirtualYear._events;
  for(VirtualEvent::ConstIterator iE = newEvents.begin(); iE != newEvents.end(); iE++)
    addVirtualEvent(iE->first, iE->second, -1);
  return *this;
}

VirtualYear VirtualYear::operator-()
{
  VirtualYear res;
  for(VirtualEvent::Iterator iE = _events.begin(); iE != _events.end(); iE++)
    res.addVirtualEvent(iE->first, iE->second, -1);
  res.iterId = iterId;
  return res;
}

void VirtualYear::operator*=(double factor)
{
  for(VirtualEvent::Iterator i = _events.begin(); i != _events.end(); i++)
    i->second *= factor;
}

double VirtualYear::GetTotalLoss(bool includeReinstatePrem)
{
  double totalLoss = 0.0;
  for (VirtualEvent::Iterator iE = _events.begin(); iE != _events.end(); iE ++) {
    if(!includeReinstatePrem)
      totalLoss += iE->second.loss;
    else
      totalLoss += iE->second.get_lossNetOfReinstatePrem();
  }

  return totalLoss;
}


}
