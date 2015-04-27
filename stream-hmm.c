#include <math.h>
#include <stdio.h>
#include <float.h>
#include "hmm.h"
#include "models.c"
#include <string.h>

//Called with each accelerometer reading
void input_reading(double *acc){
  if (filter(acc)){
    for (int i = 0; i < n_models; i++){
      forward_proc_inc(models[i], derive_group(models[i], acc));
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
  dir_filter_ref = dir_filter_ref_initial; //reset for next time
  return recognized;
}

//The quantizer, maps accelerometer readings to a set integers.
int derive_group(model *m, double *acc){
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

//Performs the next iteration of the HMM forward algorithm
void forward_proc_inc(model *m, int o){
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
    for (int k = 0; k < numStates; k++){
      double sum = 0;
      for (int l = 0; l < numStates; l++){
        sum += s[l] * a[l][k];
      }
      f[k] = sum * b[k][o];
    }
    m->f = s;
    m->s = f;
  }
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
  double idle_sensitivity = 0.1;
  if (!(abs > 1 + idle_sensitivity ||
        abs < 1 - idle_sensitivity)) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // def = directional equivalence filter
  double def_sensitivity = 0.4;
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

double *acc_reading;

double* read_line(char* line){
  char* tok;
  tok = strtok(line, ",");//throw out time
  acc_reading = (double*)malloc(sizeof(double)*3);//mem leak
  for (int i = 0; i < 3; i++){
    tok = strtok(NULL, ",\n");
    acc_reading[i] = strtod(tok, NULL);
  }
  /*
  for (int i = 0; i < 3; i++){
    printf("%f ", acc_reading[i]);
  }
  printf("\n");
  */
  return acc_reading;
}

int classify_csv_file(const char* filename){
  FILE* f = fopen(filename, "r");
  char line[1024];
  fgets(line, 1024, f);
  while (fgets(line, 1024, f)){
    input_reading(read_line(line));
  }
  fclose(f);
  return input_end();
}


int main(){
  init_models();

  dir_filter_ref_initial = (double*)malloc(sizeof(double)*3);
  dir_filter_ref = dir_filter_ref_initial;

  int n_states = models[0]->numStates;//assume they are all the same

  acc_reading = (double*)malloc(sizeof(double)*3);

  for (int i = 0; i < n_models; i++){
    models[i]->f = (double*)calloc(n_states, sizeof(double));
    models[i]->s = (double*)calloc(n_states, sizeof(double));
    models[i]->started = false;
  }

  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/square2/14.csv"));
  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/up_down/14.csv"));
  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/z/14.csv"));
  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/roll_flip/14.csv"));

}
