#pragma once

#include "VirtualEvent.h"

#include <map>
#include <set>
#include <iomanip>
#include <string>
#include <cmath>
#include <iterator>

#include <unordered_map>

//#define HASH_MAP unordered_map
#define HASH_MAP map

using namespace std;

namespace VCAPS
{

class VirtualYear
{
public:
  typedef HASH_MAP<VLONG, VirtualYear> MAP;
  typedef MAP::iterator Iterator;
  typedef MAP::const_iterator ConstIterator;
  typedef pair<VLONG, VirtualYear> Pair;

  bool less(VirtualYear::Pair& a, VirtualYear::Pair& b) { return a.first < b.first; }

public:
  VirtualYear() {}
  VirtualYear(VirtualEvent::MAP events) : _events(events) {}
  ~VirtualYear() { clear(); }

  void clear() {
    _events.clear();
    _head_events.clear();
  }
  void swap(VirtualYear& y) { 
    _events.swap(y._events); std::swap(y.iterId, iterId); 
    _head_events.swap(y._head_events);
  }

  void addVirtualEvent(int sequenceId, VirtualEvent e, double factor=1.0,
                       VLONG iterId=0, bool addhead=true);
  void addVirtualEvents(VirtualYear& events);
  void updateVirtualEvent(int sequenceId, VirtualEvent e);

  VirtualYear& operator+=(const VirtualYear& newVirtualYear);
  VirtualYear& operator-=(const VirtualYear& newVirtualYear);
  VirtualYear operator-();
  VirtualEvent& operator[](int seqId) { return _events[seqId]; }
  void operator*=(double factor);

  VirtualEvent::MAP & get_events() { return _events; }
  int size(){ return (int)_events.size(); };

  double GetTotalLoss(bool includeReinstatePrem);

  VLONG iterId;

private:
  // key = event sequence ID in a year
  VirtualEvent::MAP _events;
  VirtualEvent::VEC _head_events;
};

}
