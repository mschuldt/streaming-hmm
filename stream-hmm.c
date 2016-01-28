#include <math.h>
#include <stdio.h>
#include "hmm.h"
#include "models.c"
#include <string.h>

//Called with each accelerometer reading
void input_reading(double *acc){
  double sum = 0, tmp;
  model *m;
  if (filter(acc)){
    for (int i = 0; i < n_models; i++){
      sum += forward_proc_inc(models[i], derive_group(models[i], acc));
    }
    if (sum){
      for (int i = 0; i < n_models; i++){
        m = models[i];
        for (int l = 0; l < m->numStates; l++){
          m->s[l] = m->s[l]/sum;
        }
      }
    }
  }
}

//Called at the end of the gesture,
//Returns the id of the recognized gesture or -1 if none.
int input_end(){
  double prob;
  model *m;
  double* s;
  double sum = 0;
  int recognized = -1; // which gesture has been recognized
  double recogprob = -1; // probability of this gesture
  double probgesture = 0; // temporal value for bayes algorithm
  double probmodel = 0; // temporal value for bayes algorithm

  for (int i = 0; i < n_models; i++){
    prob = 0.0;
    m = models[i];
    m->started = false;
    s = m->s;
    // add probabilities
    for (int j = 0; j < m->numStates; j++) {
      prob += s[j];
    }
    m->prob = prob;
    sum += (m->defaultProbability)*prob;
  }
  printf("m->prob = %.30f\n", m->prob);
  for (int i=0; i < n_models; i++) {
    m = models[i];
    double tmpgesture = m->prob;
    double tmpmodel = m->defaultProbability;
    //    if (((tmpmodel * tmpgesture) / sum) > recogprob) {
    if (tmpgesture > recogprob) {
      probgesture = tmpgesture;
      /////probmodel = tmpmodel;
      recogprob = tmpgesture;//(tmpmodel*tmpgesture)/sum;
      recognized = i;
    }
  }
  dir_filter_ref = dir_filter_ref_initial; //reset for next time
  return recognized;
}

//The quantizer, maps accelerometer readings to a set integers.
int derive_group(model *m, double *acc){
  double a, b, c, d;
  double minDist = 0x3ffff;
  int minGroup=0;
  double *ref;
  double max=0;
  for (int i = 0; i < m->numObservations; i++){
    ref = m->quantizerMap[i];
    a = ref[0] - acc[0];
    b = ref[1] - acc[1];
    c = ref[2] - acc[2];
    d = a*a + b*b + c*c;
    if (d > max){
      max = d;
      //printf("max = %f\n", max);
    }
    if (d < minDist){
      minDist = d;
      minGroup = i;

    }
  }
  //printf("\n");
  return minGroup;
}

//Performs the next iteration of the HMM forward algorithm
double forward_proc_inc(model *m, int o){
  double *pi = m->PI;
  double **a = m->A;
  double *b = m->B;
  double *f = m->f;
  double *s = m->s;
  int numStates = m->numStates;
  double total = 0;

  if (m->started == false){
    for (int l = 0; l < numStates; l++){
      s[l] = pi[l] * b[(o<<3) + l];//b[l][o];
    }
    m->started = true;
    return 0;
  }else{
    double** tmp;
    for (int k = 0; k < numStates; k++){
      double sum = 0;
      for (int l = 0; l < numStates; l++){
        sum += s[l] * a[l][k];
      }
      f[k] = sum * b[(o<<3)+k];//b[k][o];
      total += f[k];
    }
    m->f = s;
    m->s = f;
  }
  return total;
}

//apply various filters to accelerometer reading ACC
// returning true if the ACC passes, else false
int filter(double* acc){
  if (!acc){
    return false;
  }
  double abs = sqrt((acc[0]*acc[0])+(acc[1]*acc[1])+(acc[2]*acc[2]));

  ////////////////////////////////////////////////////////////////////////////////
  //idle state filter
  double idle_sensitivity = 0.1;//0.3;
  if (!(abs > 1 + idle_sensitivity ||
        abs < 1 - idle_sensitivity)) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // def = directional equivalence filter
  double  def_sensitivity = 0.4;//0.5;
  if (acc[0] < dir_filter_ref[0] - def_sensitivity ||
      acc[0] > dir_filter_ref[0] + def_sensitivity ||
      acc[1] < dir_filter_ref[1] - def_sensitivity ||
      acc[1] > dir_filter_ref[1] + def_sensitivity ||
      acc[2] < dir_filter_ref[2] - def_sensitivity ||
      acc[2] > dir_filter_ref[2] + def_sensitivity) {
    dir_filter_ref = acc;
    return true;
  }
  return false;
}

#define fp_t double //because main.c uses fixed point numbers
#include "main.c"
