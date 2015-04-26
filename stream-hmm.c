#include <math.h>
#include <stdio.h>
#include <float.h>
#include "hmm.h"
#include "models.c"
#include "gestures.c"

//Called with each accelerometer reading
void input_reading(double *acc){
  if (filter_ok(acc)){
    for (int i = 0; i < n_models; i++){
      forwardProc_helper(models[i], deriveGroup(models[i], acc));
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
  //printf("sum = %.*e,\n", sum);

  for (int i=0; i < n_models; i++) {
    m = models[i];
    double tmpgesture = m->prob;
    double tmpmodel = m->defaultProbability;

    if (((tmpmodel * tmpgesture) / sum) > recogprob) {
      probgesture = tmpgesture;
      probmodel = tmpmodel;
      recogprob = (tmpmodel*tmpgesture)/sum;
      recognized = i;
    }
  }

  //reset for next time
  def_ref = def_ref_initial;

  return recognized;
}

int classify_gesture(gesture *g){
  for (int i = 0; i < g->len; i++){
    input_reading(g->data[i]);
  }
  return input_end();
}

int deriveGroup(model *m, double *acc){
  double a, b, c, d;
  double minDist = 0x3ffff;
  int minGroup=0;
  double *ref;
  for (int i = 0; i < m->numObservations; i++){
    ref = m->quantizerMap[i];
    a = ref[0] - acc[0];
    b = ref[1] - acc[1];
    c = ref[2] - acc[2];
    d = sqrt(a*a + b*b + c*c);
    if (d < minDist){
      minDist = d;
      minGroup = i;
    }
  }
  return minGroup;
}

void forwardProc_helper(model *m, int o){
  //  printf("%p ", m->s);
  double *pi = m->PI;
  double **a = m->A;
  double **b = m->B;
  double *f = m->f;
  double *s = m->s;
  int numStates = m->numStates;

  if (m->started == false){
    for (int l = 0; l < numStates; l++){
      s[l] = pi[l] * b[l][o];
    }
    m->started = true;
  }else{
    double** tmp;
    //printf("\nforwardProc_helper(%d)\n", o);
    for (int k = 0; k < numStates; k++){
      double sum = 0;
      for (int l = 0; l < numStates; l++){
        sum += s[l] * a[l][k];
        //printf("[%f, %f], ", s[l], a[l][k]);
      }
      f[k] = sum * b[k][o];
      //printf("%f, ", b[k][o]);
      //printf("\nsum,f[k]=: %f, %f\n", sum, f[k]);
      //printf("%f\n", sum, f[k]);
    }
    //printf("\n");
    m->f = s;
    m->s = f;
  }
}

int _count = 0;
model* new_model(){
  model* m = models[_count++] = (model*)malloc(sizeof(model));
  return m;
}

double* idle_state_filter(double* acc){

}

int filter_ok(double* acc){
  if (!acc){
    return false;
  }
  double abs = sqrt((acc[0]*acc[0])+(acc[1]*acc[1])+(acc[2]*acc[2]));

  ////////////////////////////////////////////////////////////////////////////////
  //idle state filter
  double idle_sensitivity = 0.1;
  if (!(abs > 1 + idle_sensitivity ||
        abs < 1 - idle_sensitivity)) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////
  //motion detect filter
  /*
    if (in_motion  && (System.currentTimeMillis() - motion_start_time)
    >= motion_change_time) {
    in_motion=false;
    this.device.fireMotionStopEvent();
    }

    motion_start_time=System.currentTimeMillis();
    if (!in_motion) {
    in_motion=true;
    motion_start_time=System.currentTimeMillis();
    this.device.fireMotionStartEvent();
    }
  */
  ////////////////////////////////////////////////////////////////////////////////
  // def = directional equivalence filter
  double def_sensitivity = 0.4;
  if (acc[0] < def_ref[0] - def_sensitivity ||
      acc[0] > def_ref[0] + def_sensitivity ||
      acc[1] < def_ref[1] - def_sensitivity ||
      acc[1] > def_ref[1] + def_sensitivity ||
      acc[2] < def_ref[2] - def_sensitivity ||
      acc[2] > def_ref[2] + def_sensitivity) {
    def_ref = acc;
    return true;
  }
  return false;
}

int main(){
  init_models();
  init_gestures();

  def_ref = def_ref_initial = (double*)malloc(sizeof(double)*3);

  int n_states = models[0]->numStates;//assume they are all the same

  for (int i = 0; i < n_models; i++){
    models[i]->f = (double*)calloc(n_states, sizeof(double));
    models[i]->s = (double*)calloc(n_states, sizeof(double));
    models[i]->started = false;
  }

  for (int i = 0; i < n_gestures; i++){
    printf("%d\n", classify_gesture(gestures[i]));
  }
}
