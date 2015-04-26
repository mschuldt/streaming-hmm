#ifndef HMM_H
#define HMM_H

#include "stdlib.h"
#define true 1
#define false 0

//#define double float

typedef struct model{
  int started;
  double prob;

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

//for motion detect filter
int in_motion = false;
int motion_change_time = 190;
int motion_start_time;
//
double* def_ref;
double* def_ref_initial;

model** models;

model* new_model();
int classify_gesture(gesture *g);
double matches(model *m, gesture *g);
int* getObservationSequence(model *m, gesture *g);
int deriveGroup(model *m, double *acc);
void forwardProc_helper(model *m, int o);
double* forwardProc(model *m, gesture *g);
double getProbability(model *m, gesture *g);
int filter_ok(double* acc);
#endif
