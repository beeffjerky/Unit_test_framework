#pragma once

#include <map>
#include <string>
#include <sys/stat.h>

#ifdef _WINDOWS
#include "Windows.h"
#else
#include <sys/time.h>
#endif
#include "virtualYear.h"

using namespace std;

namespace VCAPS
{

class ToolBox {
public:
  static string getAscTime() {
    char tmp[100];
#ifdef _WINDOWS
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf(tmp, "%d-%d-%d %d:%d:%d.%03d ",
      st.wYear, st.wMonth, st.wDay,
      st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    //get timestamp
    time_t rawtime;
    struct tm* tminfo;
    time(&rawtime);
    tminfo = localtime(&rawtime);
    struct timeval st;
    gettimeofday(&st, NULL);
    sprintf(tmp, "%d-%d-%d %d:%d:%d.%03d ",
      tminfo->tm_year, tminfo->tm_mon, tminfo->tm_mday,
      tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec, (int)(st.tv_usec / 1000));
#endif
    string timeStr = tmp;

    return timeStr;
  }

  static bool fileExists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
  }
};

typedef pair<double, double> EL_SD;

class Simulation
{
public:
  typedef map<string, Simulation*> MAP;
  typedef map<string, int> RGMAP;

public:
  Simulation() : _numIter(0)
  {}

  Simulation(const Simulation& newSimu)
    : _numIter(newSimu._numIter), _iterations(newSimu._iterations), riskGroupMap(newSimu.riskGroupMap)
  {}

  Simulation(Simulation& original, string riskGroupToInclude, bool isInclude);

  ~Simulation() { clear(); }

  VirtualYear::MAP & getIterations()
  { return _iterations; }

  VirtualYear::MAP & getIterations() const
  { return const_cast<VirtualYear::MAP&>(_iterations); }

  Simulation(int numIter);
  
  void operator=(const Simulation& newSimu) { 
    _numIter = newSimu._numIter;
    _iterations = newSimu._iterations;
    riskGroupMap= newSimu.riskGroupMap;
  }
  void operator=(VirtualYear::MAP& ymap) {_iterations.swap(ymap);}
  VirtualYear& operator[](int iterId) { return _iterations[iterId]; }

  void operator*=(double factor); // For rg="ALL"
  void parallelFileReading(string filename, double minLossToInclude, string mfid, 
            bool ignoreOrdering, double fullRipScale);
  void readFromFile(string simulationFileName, double minLossToInclude, string mfid, 
            bool ignoreOrdering=false);
  void set_numIter(int numIter){_numIter = numIter; }
  int get_numIter() const { return _numIter; }
  bool empty() { return _iterations.size()==0; }

  int countNumEvents();

  pair<double, double> get_expected_sd(bool includeReinstatePrem=1);

  Simulation& operator+=(const Simulation& newSimulation);
  inline Simulation operator+(const Simulation& newSimulation) {
    Simulation lhs;
    lhs += newSimulation;
    return lhs;
  }
  Simulation& operator-=(const Simulation& newSimulation);

  void clear();

  void swap(Simulation& other) {
    (std::swap)(_numIter, other._numIter);
    _iterations.swap(other._iterations);
  }
public:
  RGMAP riskGroupMap;

private:
  // key = iteration ID
  //  only include the iterations with losses
  VirtualYear::MAP _iterations;

  // number of iterations including those with no losses
  int _numIter;
};

}