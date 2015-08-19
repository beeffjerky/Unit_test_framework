#ifndef PRICING3_H
#define PRICING3_H

#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <string>

#include "Simulation.h"

using namespace std;
using namespace VCAPS;

class Pricing
{
public:
  Pricing();

  void execution();

protected:
  void _loadSimulationFromFile(Simulation& sim, string fileNames, string delim, bool isTerror);
};

#endif
