#include "AnnualLoss.h"

#include <iostream>
#include <omp.h>
#include <algorithm>
#include <functional>

namespace VCAPS
{


void AnnualLoss::addAnnualLoss(AnnualLoss& newAnnualLoss)
{
  if(newAnnualLoss.get_numIter() != _numIter) {
    if(_numIter == 0 || newAnnualLoss.get_numIter() ==0) {
      int numCorrect = _numIter;
      if(numCorrect < newAnnualLoss.get_numIter())
        numCorrect = newAnnualLoss.get_numIter();
      _numIter = numCorrect;
      newAnnualLoss.set_numIter(numCorrect);
    } else {
      exit(0);
    }
  }
  for(int k = 0; k < _numIter; k++)
    _annualLoss[k] += newAnnualLoss.getAnnualLoss(k);
  _sortedInAllocatedTVaRSeries=false;
}

void AnnualLoss::setAnnualLoss(VECTOR& losses)
{
  for (int y = 0; y<(int)losses.size(); y++)
    _annualLoss[y] = losses[y];
  _sortedInAllocatedTVaRSeries = false;
}

void AnnualLoss::setAnnualLoss(VECTOR& losses, VECTOR& grosses)
{
  for (int y = 0; y < _numIter; y++) {
    _annualLoss[y] = losses[y];
    _annualLossGross[y] = grosses[y];
  }
  _sortedInAllocatedTVaRSeries = false;
}

double AnnualLoss::getAnnualLoss(int iterId)
{
  Iterator i = _annualLoss.find(iterId);
  if(i == _annualLoss.end()) return 0.; 
  return i->second;
}

double AnnualLoss::get_expectedLoss()
{
  double expectedLoss = 0.;
  for (Iterator it = _annualLoss.begin(); it != _annualLoss.end(); it++)
    expectedLoss += it->second;
 
  expectedLoss /= (double)_numIter;
  return expectedLoss;
}

bool UDgreater(double elem1, double elem2){return elem1 > elem2; }
double AnnualLoss::getAllocatedTVaRSeries(AnnualLoss& contributor,
            vector<double> probs, bool removeMean)
{
  double meanContributing = 0;
  if(removeMean) {
    if (!_sortedInAllocatedTVaRSeries)
      _meanBase = get_expectedLoss();
    meanContributing = contributor.get_expectedLoss();
  }

  int N = (int)_annualLoss.size(),
      j = 0;
  if (!_sortedInAllocatedTVaRSeries) {
    _sortedAnnualLoss.resize(N);
    for(Iterator i = _annualLoss.begin(); i != _annualLoss.end(); i++, j++)
      _sortedAnnualLoss[j] = pair<double, int>( - i->second, i->first);

    sort(_sortedAnnualLoss.begin(), _sortedAnnualLoss.end());
    _baseWeightedTVaR = 0;
  }

  double contributingWeightedTVaR = 0;
  // make big prob first to reuse the calculated sum, it will not rewrite
  //    the input probs after return
  sort(probs.begin(), probs.end(), UDgreater);
  int i=0;
  double baseTVaR = 0, contributedTVaR = 0;
  for(int k = 0; k < (int)probs.size(); k++) {
    double aep = 0;
    int nPos = probabilityToIndex(_numIter, probs[k]);
    if(nPos < 1) nPos = 1;
    if(nPos <= (int)_annualLoss.size())
      aep = - _sortedAnnualLoss[nPos-1].first;

    // the sum for the next prob start from the end of the first
    for(/*int i = 0*/; i< N; i++) {
      if( - _sortedAnnualLoss[i].first < aep-0.00000001)
        break;
      if (!_sortedInAllocatedTVaRSeries)
        baseTVaR += - _sortedAnnualLoss[i].first - _meanBase;
      double contributor_loss = contributor.getAnnualLoss(_sortedAnnualLoss[i].second);
      //if (i < 10) {
      //  cerr << i << ":" << contributor_loss << ":" 
      //    << _sortedAnnualLoss[i].second << ":" << _sortedAnnualLoss[i].first << endl;
      //}
      contributedTVaR += contributor_loss - meanContributing;
    }
    if (!_sortedInAllocatedTVaRSeries)
      _baseWeightedTVaR += baseTVaR/double(nPos) * probs[k];
    contributingWeightedTVaR += contributedTVaR/double(nPos) * probs[k];
  }

  _sortedInAllocatedTVaRSeries=true;

  cout << "AnnualLoss::getAllocatedTVaRSeries " 
    << contributingWeightedTVaR << "/" << _baseWeightedTVaR 
    << ":" << meanContributing << ":" << _meanBase << endl;

  return (abs(_baseWeightedTVaR) <= 0.00001) ? 0 : contributingWeightedTVaR/_baseWeightedTVaR;
}

void AnnualLoss::scale(double scaleFactor)
{
  for(Iterator i = _annualLoss.begin(); i != _annualLoss.end(); i++)
    i->second *= scaleFactor;
  _sortedInAllocatedTVaRSeries=false;
}

pair<double, double> AnnualLoss::get_expected_sd()
{
  double totalLoss = 0., t2 = 0.;
  for (Iterator it = _annualLoss.begin(); it != _annualLoss.end(); it++) {
    totalLoss += it->second;
    t2 += it->second * it->second;
  }
  double expectedLoss = totalLoss / _numIter;
  double variance = (std::max)(0.0, t2 / _numIter - expectedLoss*expectedLoss);
  double sd = sqrt(variance);
  return pair<double, double>(expectedLoss, sd);
}

}
