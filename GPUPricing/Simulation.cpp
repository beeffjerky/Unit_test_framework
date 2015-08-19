#include "Simulation.h"

#include <cctype> // for toupper
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <sstream>
#include <iostream>

#include "csvReader.h"

using namespace chrono;

namespace VCAPS
{

Simulation::Simulation(int numIter)
{
  _numIter = numIter;
}
  
const int workers = 12;
csv_io::CSVReader<7, workers, '\t'> thread_input;
VirtualYear::MAP thread_iterations[workers];
Simulation::RGMAP thread_riskGroupMap[workers];

void readFileThread(int idx, int memSize, double minLossToInclude, string mfid, 
                    bool ignoreOrdering, bool hasRG, bool hasFullRip, double fullRipScale)
{
  high_resolution_clock::time_point _start = high_resolution_clock::now();
  int tt = 0;

  VirtualYear::MAP& thread_iters = thread_iterations[idx];

  VirtualYear::MAP iters;
  thread_iters.swap(iters);

  Simulation::RGMAP& thread_rgMap = thread_riskGroupMap[idx];
  thread_rgMap.clear();

  char* data = new char[memSize];

  int nTotalEvents = 0;
  bool filterRipAndRG = (mfid.size()>0);
  VLONG iterId = 0;
  int eventId = 0, seqId = 0;
  double loss = 0, reinstatementPrem = 0, fullRip = 0;
  string riskGroup = "NA";
  bool firstIter = true;
  while (thread_input.read_row((void*)data,idx)) {
    double loss1 = 0, rip = 0;

    char* p = (char*)data;
    iterId = *((int*)p); p += sizeof(int);
    seqId = *((int*)p); p += sizeof(int);
    eventId = *((int*)p); p += sizeof(int);
    loss = *((double*)p); p += sizeof(double);
    reinstatementPrem = *((double*)p); p += sizeof(double);
    if (hasRG){
      riskGroup = p; p += riskGroup.length() + 1;
      if (filterRipAndRG && riskGroup == "Noncat")
        riskGroup = "Noncat-" + mfid;
    }

    if (hasFullRip)
      fullRip = *((double*)p);
    else if (fullRipScale == 1)
      fullRip = loss;
    else if (fullRipScale == 0)
      fullRip = 0;
    else
      fullRip = loss * fullRipScale;
    if (filterRipAndRG && fabs(reinstatementPrem) < 1)
      reinstatementPrem = 0.0;
    if (loss >= minLossToInclude) {
      string rgTmp = riskGroup;
      transform(rgTmp.begin(), rgTmp.end(), rgTmp.begin(), (int(*)(int)) toupper);
      if (rgTmp == "NONCAT")
        riskGroup = "Noncat";
      thread_rgMap[riskGroup] = 1;
        
      VLONG oldIterId = iterId;
      if (ignoreOrdering)
        iterId = ((oldIterId+1) << 32) | seqId;
      
      high_resolution_clock::time_point _start = high_resolution_clock::now();
      VirtualEvent v(eventId, loss, reinstatementPrem, riskGroup, fullRip);
      thread_iters[iterId].addVirtualEvent(seqId, v, 1.0, iterId, true);

      nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now() - _start);
      
      tt += ns.count();
      nTotalEvents++;
      if (nTotalEvents % 1000000 == 0)
        cerr << "Thread" << idx << " : " << nTotalEvents << " events read" << endl;
    }
  }
  delete[] data;
}

void Simulation::parallelFileReading(string filename, double minLossToInclude, string mfid,
                    bool ignoreOrdering, double fullRipScale)
{
  std::string cols[] = { "iterId", "seqId", "eventId", "loss", "reinstatementPrem", "riskGroup", "fullRip" };
  csv_io::ColumnType colTypes[] = { csv_io::Int, csv_io::Int, csv_io::Int,
                csv_io::Double, csv_io::Double, csv_io::String, csv_io::Double };
  thread_input.open(filename);

  string line1 = thread_input.bypass_row();
  std::stringstream ss(line1);
  string word1, word2;
  ss >> word1 >> word2 >> _numIter;
  if (word1 != "_numIter" || word2 != "=") {
    cerr << "Error: the first line of " + filename + " must be\n_numIter = <n>" << endl;
    exit(0);
  }
  if (_numIter == 0) {
    cerr << "programFinished" << endl
      << "Error:  " + filename + " must have nonzero _numIter." << endl;
    exit(0);
  }

  string line2 = thread_input.bypass_row();
  size_t numFields = std::count(line2.begin(), line2.end(), '\t') + 1;
  bool hasRG = numFields >= 6;
  bool hasFullRip = numFields == 7;

  cout << "hasRG = " << hasRG << "; hasFullRip = " << hasFullRip
    << "; ignoreOrdering=" << ignoreOrdering << endl;

  unsigned int numCol = 5 + (hasRG ? 1 : 0) + (hasFullRip ? 1 : 0);
  if (!hasRG)
    colTypes[5] = csv_io::Double;
  size_t memSize = thread_input.set_header(cols, colTypes, numCol);

  vector<std::thread*> pools;
  for (int i = 0; i < workers; i++) {
    std::thread* t = new thread(readFileThread, i, memSize, minLossToInclude, mfid, 
      ignoreOrdering, hasRG, hasFullRip, fullRipScale);
    pools.push_back(t);
  }

  std::this_thread::sleep_for(nanoseconds(1000)); // 1 micro-second

  for_each(pools.begin(), pools.end(), [](std::thread *t) { t->join(); delete t; });
  high_resolution_clock::time_point _start = high_resolution_clock::now();
  _iterations.swap(thread_iterations[0]);
  riskGroupMap.swap(thread_riskGroupMap[0]);
  for (int i = 1; i < workers; i++) {
    for (VirtualYear::Iterator it = thread_iterations[i].begin(); it != thread_iterations[i].end(); it++)
    {
      pair<VirtualYear::Iterator, bool> ret = _iterations.insert(VirtualYear::Pair(it->first, VirtualYear()));
      if (!ret.second)
        _iterations[it->first].addVirtualEvents(it->second);
      else
        _iterations[it->first].swap(it->second);
    }
    riskGroupMap.insert(thread_riskGroupMap[i].begin(), thread_riskGroupMap[i].end());
  }
  nanoseconds ns = duration_cast<nanoseconds>(high_resolution_clock::now() - _start);
  cerr << " merging years@ " << (ns.count() / 1000000) << " ms with " 
       << riskGroupMap.size() << " risk groups" << endl;

  thread_input.close();
}

void Simulation::readFromFile(string simulationFile, double minLossToInclude, string mfid, 
                              bool ignoreOrdering)
{
  string inFileName = simulationFile.substr(0, simulationFile.length() - 4) + ".vsm";
  cout << ToolBox::getAscTime() << "\t reading simulated data 1 from "
       << simulationFile << ": >=" << minLossToInclude << endl;

  if (!ToolBox::fileExists(simulationFile)) {
    cerr << "Error 1: text file " + simulationFile + " not openable " + mfid << endl;
    exit(0);
  }
  parallelFileReading(simulationFile, minLossToInclude, mfid, ignoreOrdering, 0);
  cout << ToolBox::getAscTime() << "-read " << countNumEvents() << " non-zero events" << endl;
}

Simulation::Simulation(Simulation& original, string riskGroupToInclude, bool isInclude)
  : _numIter(original.get_numIter())
{
  int nE = 0;
  for(VirtualYear::Iterator iI = original._iterations.begin();
    iI != original._iterations.end(); iI++)
  {
    const VirtualEvent::MAP & events = iI->second.get_events();
    for(VirtualEvent::ConstIterator iE = events.begin(); iE != events.end(); iE++)
    {
      string rg = iE->second.riskGroup;
      if (isInclude && rg == riskGroupToInclude || !isInclude && rg != riskGroupToInclude)
      {
        _iterations[iI->first].addVirtualEvent(iE->first, iE->second);
        nE++;
      }
    }
  }
  cout << nE << " simulated losses passed filtering for RG " 
       << riskGroupToInclude << endl; 
}

void Simulation::operator*=(double factor)
{
  if(fabs(factor-1) < 1e-5)
    return;
  for(VirtualYear::Iterator iI = _iterations.begin(); iI != _iterations.end(); iI++)
    iI->second *= factor;
}

pair<double, double> Simulation::get_expected_sd(bool includeReinstatePrem)
{
  if(_numIter==0)
    return pair<double, double>(0., 0.);
  
  double expectedLoss = 0, e2 = 0;
  for(VirtualYear::Iterator iI = _iterations.begin(); iI != _iterations.end(); iI++) {
    double annualLoss = iI->second.GetTotalLoss(includeReinstatePrem);
    expectedLoss += annualLoss;
    e2 += annualLoss*annualLoss;
  }

  expectedLoss /= (double)_numIter;
  e2 /= (double)_numIter;

  double sd = 0.0;
  if(e2-expectedLoss*expectedLoss >= 0. )
    sd = sqrt(e2 - expectedLoss*expectedLoss);
  else {
    cout<<"WARNNING: you get negative standard deviation: "<<endl
      <<"\t e2 = "<<e2<<endl
      <<"\t expectedLoss = "<<expectedLoss<<endl
      <<"\t expectedLoss * expectedLoss = "<<expectedLoss * expectedLoss<<endl;  
  }

  return pair<double, double>(expectedLoss, sd);
}

Simulation& Simulation::operator-=(const Simulation& newSimulation)
{
  if(_numIter != newSimulation._numIter) {
    if(_numIter == 0)
      _numIter = newSimulation._numIter;
    else if(!const_cast<Simulation&>(newSimulation).empty()) {
      cerr << "Error: to add two Simulation objects _numIter must be the" << " same" << endl
        << "But here are: " << _numIter << " and " << newSimulation._numIter << endl;
      exit(0);
    }
  }

  VirtualYear::MAP& newIters = newSimulation.getIterations();
  for(VirtualYear::Iterator iYear = newIters.begin(); iYear!= newIters.end(); iYear++) {
    VirtualYear::Iterator i = _iterations.find(iYear->first);
    if(i == _iterations.end())
      _iterations[iYear->first] = - iYear->second;
    else
      i->second -= iYear->second;
  }
  cout << " @@@@-= Now I have " << countNumEvents() << " events from gross " 
       << const_cast<Simulation&>(newSimulation).countNumEvents() << endl;
  return *this;
}

Simulation& Simulation::operator+=(const Simulation& newSimulation)
{
  if(_numIter != newSimulation._numIter) {
    if(_numIter == 0)
      _numIter = newSimulation._numIter;
    else if(!const_cast<Simulation&>(newSimulation).empty()) {
      cerr << "Error: to add two Simulation objects _numIter must be the" << " same" << endl
        << "But here are: " << _numIter << " and " << newSimulation._numIter << endl;
      exit(0);
    }
  }

  VirtualYear::MAP* newIters = const_cast<VirtualYear::MAP*>(&(newSimulation._iterations));
  for (VirtualYear::Iterator iI = newIters->begin(); iI != newIters->end(); iI++) {
    if (_iterations.find(iI->first) == _iterations.end())
      _iterations[iI->first] = VirtualYear();
  }
  for (VirtualYear::Iterator iI = _iterations.begin(); iI != _iterations.end(); iI++) {
    VirtualYear::Iterator iN = newIters->find(iI->first);
    if (iN != newIters->end())
      iI->second += iN->second;
  }
  riskGroupMap.insert(newSimulation.riskGroupMap.begin(), newSimulation.riskGroupMap.end());
  cout << " @@@@+= Now I have " << countNumEvents() << " events from gross "
       << const_cast<Simulation&>(newSimulation).countNumEvents()
       << " and " << riskGroupMap.size() << " riskGroups" <<  endl;

  return *this;
}

int Simulation::countNumEvents()
{
  int nTotalEvent = 0;
  for (VirtualYear::Iterator iI = _iterations.begin(); iI != _iterations.end(); ++iI)
    nTotalEvent += iI->second.size();
  return nTotalEvent;
}

void Simulation::clear()
{
  _iterations.clear();
  _numIter = 0;
}


}
