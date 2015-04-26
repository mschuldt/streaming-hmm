#include <math.h>
#include <stdio.h>
#include <float.h>
#include "hmm.h"
#include "models.c"
#include "gestures.c"

int classify_gesture(gesture *g) {
  //google translate: "Calculate value in the denominator, Bayesian"
  double sum = 0;
  double* matchProbs = (double*)malloc(sizeof(double)*n_models);
  double prob;
  for (int i=0; i<n_models; i++) {
    prob = matches(models[i], g);
    //printf("prob = %f\n", prob);
    matchProbs[i] = prob;
    sum += (models[i]->defaultProbability)*prob;
  }

  int recognized = -1; // which gesture has been recognized
  double recogprob = -111111; // probability of this gesture
  double probgesture = 0; // temporal value for bayes algorithm
  double probmodel = 0; // temporal value for bayes algorithm
  for (int i=0; i<n_models; i++) {
    //this.gesturemodel.elementAt(i).print(); // Debug
    double tmpgesture = matchProbs[i];
    double tmpmodel = models[i]->defaultProbability;

    if (((tmpmodel*tmpgesture)/sum)>recogprob) {
      probgesture=tmpgesture;
      probmodel=tmpmodel;
      recogprob=((tmpmodel*tmpgesture)/sum);
      recognized=i;
    }
  }

  // a gesture could be recognized
  if (recogprob>0 && probmodel>0 && probgesture>0 && sum>0) {
    //this.lastprob = recogprob; ;;???
    return recognized;
  } else {
    // no gesture could be recognized
    return -1;
  }
}

double matches(model *m, gesture *g) {
  g->o = getObservationSequence(m, g);

  if (false){
    printf("quantized: ");
    for (int i = 0; i < g->len; i++){
      printf("%d, ", g->o[i]);
    }
    printf("\n");
  }
  return getProbability(m, g);
}

int* getObservationSequence(model *m, gesture *g){
  double** data = g->data;
  int size = g->len;
  int last =0;
  if (size < m->numStates){
    size = m->numStates;
  }
  int* groups = (int*) malloc(sizeof(int)*g->len);
  int i;
  for (i = 0; i < g->len; i++) {
    groups[i] = last = deriveGroup(m, data[i]);
  }
  for (;i < size;i++){
    groups[i] = last;
  }
  return groups;
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

int started = false;

void forwardProc_helper(model *m, int o){
  //  printf("%p ", m->s);
  double *pi = m->PI;
  double **a = m->A;
  double **b = m->B;
  double *f = m->f;
  double *s = m->s;
  int numStates = m->numStates;

  if (started == false){
    for (int l = 0; l < numStates; l++){
      s[l] = pi[l] * b[l][o];
    }
    started = true;
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


double* forwardProc(model *m, gesture *g){
  //m->f = (double*)malloc(sizeof(double)*m->numStates);
  //m->s = (double*)malloc(sizeof(double)*m->numStates);

  /*
  printf("PI: %f, %f, %f\n", m->PI[0], m->PI[1], m->PI[2]);
  printf("A: %f, %f, %f\n", m->A[0][0], m->A[0][1], m->A[0][2]);
  printf("B: %f, %f, %f\n", m->B[0][0], m->B[0][1], m->B[0][2]);
  */

  m->f = (double*)calloc(m->numStates, sizeof(double));
  m->s = (double*)calloc(m->numStates, sizeof(double));

  started = false;
  for (int i = 0; i < g->len; i++) {
    //    printf("[%p, %p], ",m->f, m->s);
    forwardProc_helper(m, g->o[i]);
    /*
    printf("s: ");
    for (int i =0; i < m->numStates; i++){
      printf("%.*e,", 10, m->s[i]);
    }
    printf("\nf:");
    for (int i =0; i < m->numStates; i++){
      printf("%.*e, ", 10, m->f[i]);
    }
    printf("\n");
    */
  }
  
  return m->s;
}

double getProbability(model *m, gesture *g){
  double prob = 0.0;
  double *forward = forwardProc(m, g);
  // add probabilities
  for (int i = 0; i < m->numStates; i++) { // for every state
    prob += forward[i];
  }
  //printf("PROB = %f\n", prob);
  //printf("     => %.*e\n", 10, prob);
  return prob;
}
/*
Classifier.classifyGesture
 GestureModel.matches
   Quantizer.getObservationSequence
   HMM.getProbability
*/

int _count = 0;
model* new_model(){
  model* m = models[_count++] = (model*)malloc(sizeof(model));
  return m;
}

double* idle_state_filter(double* acc){

}

//for motion detect filter
int in_motion = false;
int motion_change_time = 190;
int motion_start_time;
double* def_ref;

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

void filter(gesture *g){
  int len = 0;
  for (int i = 0; i < g->len; i++){
    if (filter_ok(g->data[i])){
      g->data[len++] = g->data[i];
    }
  }
  g->len = len;
}

int main(){
  def_ref = (double*)malloc(sizeof(double)*3);
  init_models();
  init_gestures();
  for (int i = 0; i < n_gestures; i++){
    //printf("before filter ===> %d\n", gestures[i]->len);
    filter(gestures[i]);
    //printf("after filter ===> %d\n", gestures[i]->len);
    printf("%d\n", classify_gesture(gestures[i]));
  }
}
