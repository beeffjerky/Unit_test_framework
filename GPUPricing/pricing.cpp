#include "pricing.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <values.h>
#include <getopt.h>

#include <algorithm>
#include <sys/stat.h>

#include <omp.h>

Pricing::Pricing()
{
}

void Pricing::execution()
{
}

void Usage()
{
}

int main(int argc, char** argv)
{
  if(argc<2) {
    Usage();
    exit(-1);
  }

  extern char* optarg;
  extern int optind;
  char c=0;
  while((c=getopt(argc,argv,":A:a:B:M:o:S:e:p:x:n:d:b:s:P:Z:L:l:T:F:C:D:"))!=EOF){
    cerr << "<" <<c<<">";
    switch(c)
    {
    case 'A':
      break;
    default:
      Usage();
      exit(-1);
    }
  }
  return 0; 
}
