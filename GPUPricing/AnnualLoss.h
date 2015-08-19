#pragma once

#include <map>
#include <vector>
#include <unordered_map>
#include <iterator>

using namespace std;

namespace VCAPS
{

/*
  this class is used primarily for calculating allocated TVaR to a contract
  in a computationally efficient way than using the Simulation class
*/
class AnnualLoss
{
public:
  typedef vector<double> VECTOR;
  typedef unordered_map<int, double> MAP;
  typedef unordered_map<int, double>::iterator Iterator;

public:
  AnnualLoss(int numIter=0)
    : _numIter(numIter), 
      _sortedInAllocatedTVaRSeries(false),
      _meanBase(0),
      _baseWeightedTVaR(0)
  { }
  AnnualLoss(MAP& annualLoss, int numIter)
    : _annualLoss(annualLoss), 
      _numIter(numIter),
      _sortedInAllocatedTVaRSeries(false),
      _meanBase(0),
      _baseWeightedTVaR(0)
  {}
  AnnualLoss(MAP& annualLoss, MAP& annualLossGross, int numIter)
    : _annualLoss(annualLoss), 
      _annualLossGross(annualLossGross),
      _numIter(numIter),
      _sortedInAllocatedTVaRSeries(false),
      _meanBase(0),
      _baseWeightedTVaR(0)
  {}
  void set_numIter(int x) { _numIter = x; }
  int get_numIter() { return _numIter; }
  int size() { return (int)_annualLoss.size(); }
  void addAnnualLoss(int iterId, double x) { 
    _annualLoss[iterId] += x; _sortedInAllocatedTVaRSeries=false;
  }
  void addAnnualLoss(int iterId, double x, double y) {
    _annualLoss[iterId] += x;
    _annualLossGross[iterId] += y;
    _sortedInAllocatedTVaRSeries=false;
  }
  void addAnnualLoss(AnnualLoss& newAnnualLoss);
  void setAnnualLoss(VECTOR& losses);
  void setAnnualLoss(VECTOR& losses, VECTOR& grosses);
  double getAnnualLoss(int iterId);
  double getQuantile(double prob);
  void clear() { _annualLoss.clear(); _sortedInAllocatedTVaRSeries=false;}
  void scale(double scaleFactor);

  void swap(AnnualLoss& other){
    _annualLoss.swap(other._annualLoss);
    _annualLossGross.swap(other._annualLossGross);
    _sortedLoss.swap(other._sortedLoss);
    std::swap(_numIter, other._numIter);
    _sortedAnnualLoss.swap(other._sortedAnnualLoss);
    std::swap(_sortedInAllocatedTVaRSeries, other._sortedInAllocatedTVaRSeries);
    std::swap(_meanBase, other._meanBase);
    std::swap(_baseWeightedTVaR, other._baseWeightedTVaR);
  }

  int probabilityToIndex(int numIter, double prob) {
    int nReverse = (int)(numIter * prob + 0.5);
    return numIter - (std::min)(1, nReverse);
  }

  /*
    calculate the contribution by the "contributor" to the TVaRs (speicified
    by probs) of this instance
  */
  double getAllocatedTVaRSeries(AnnualLoss& contributor, vector<double> probs,
        bool removeMean=1);

  double get_expectedLoss();
  void addConstant(double x);
  double get_sd();
  pair<double, double> get_expected_sd();
  double getTVaR(double prob);
  bool empty() { return _annualLoss.size()==0; }

  MAP& get_annualLoss() { return _annualLoss; }
  MAP& get_annualLossGross() { return _annualLossGross; }

private:

  /*
    key = iteration ID
    value = total annual loss
  */
  MAP _annualLoss;
  MAP _annualLossGross;
  /*
    key = annual loss
    value = iteration ID
  */
  multimap<double, int> _sortedLoss;
  int _numIter;

  vector< pair<double, int> > _sortedAnnualLoss;
  bool _sortedInAllocatedTVaRSeries;
  double _meanBase;
  double _baseWeightedTVaR;
};

}
