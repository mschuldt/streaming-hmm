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

model** models;

int _count = 0;
model* new_model(){
  model* m = models[_count++] = (model*)malloc(sizeof(model));
  return m;
}

double* dir_filter_ref;
double* dir_filter_ref_initial;

int classify_gesture(gesture *g);
int derive_group(model *m, double *acc);
void forward_proc_inc(model *m, int o);
int filter(double* acc);
#endif
