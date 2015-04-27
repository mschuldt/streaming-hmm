#ifndef HMM_H
#define HMM_H

#include "stdlib.h"
#include "fixed-point.h"

#define true 1
#define false 0

#define double fp_t

typedef struct model{
  int started;
  fp_t prob;

  int numStates;
  int numObservations;
  fp_t defaultProbability;
  fp_t quantizerRadius;
  fp_t** quantizerMap;
  fp_t* PI;
  fp_t** A;
  fp_t** B;

  //used by the hmm forward algorithm
  fp_t* f;
  fp_t* s;
} model;

model** models;

int _count = 0;
model* new_model(){
  model* m = models[_count++] = (model*)malloc(sizeof(model));
  return m;
}

fp_t* dir_filter_ref;
fp_t* dir_filter_ref_initial;

int derive_group(model *m, fp_t *acc);
void forward_proc_inc(model *m, int o);
int filter(fp_t* acc);

#endif
