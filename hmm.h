#ifndef HMM_H
#define HMM_H

#include "stdlib.h"
#define true 1
#define false 0

typedef struct model{
  int numStates;
  int numObservations;
  double defaultProbability;
  double quantizerRadius;
  double** quantizerMap;
  double* PI;
  double** A;
  double** B;

  //used by the hmm forward algorithm
  double* f;
  double* s;
} model;

typedef struct gesture{
  int len;  
  double** data;
  int *o;
} gesture;

model** models;

model* new_model();
int classify_gesture(gesture *g);
double matches(model *m, gesture *g);
int* getObservationSequence(model *m, gesture *g);
int deriveGroup(model *m, double *acc);
void forwardProc_helper(model *m, int o);
double* forwardProc(model *m, gesture *g);
double getProbability(model *m, gesture *g);
#endif
