#include <math.h>
#include <stdio.h>
#include <float.h>
#include "hmm-fp.h"
#include "models.c"
#include <string.h>

//Called with each accelerometer reading
void input_reading(fp_t *acc){
  fp_t max = 0, tmp;
  if (filter(acc)){
    for (int i = 0; i < n_models; i++){
      tmp = forward_proc_inc(models[i], derive_group(models[i], acc));
      if (tmp > max){
        max = tmp;
      }
    }
    if (fp_cmp(max, d2fp(0.01))==-1){
      for (int i = 0; i < n_models; i++){
        m = models[i];
        for (int l = 0; l < m->numStates; l++){
          m->s[l] = fp_mul(m->s[l], d2fp(10));
        }
      }
    }
    //    printf("max = %f\n", fp2d(max));
  }
}

//Called at the end of the gesture,
//Returns the id of the recognized gesture or -1 if none.
int input_end(){
  fp_t prob;
  model *m;
  fp_t* s;
  fp_t sum = 0;
  int recognized = -1; // which gesture has been recognized
  fp_t recogprob = -1; // probability of this gesture
  fp_t probgesture = 0; // temporal value for bayes algorithm
  fp_t probmodel = 0; // temporal value for bayes algorithm

  for (int i = 0; i < n_models; i++){
    prob = d2fp(0.0);
    m = models[i];
    m->started = false;
    s = m->s;
    // add probabilities
    for (int j = 0; j < m->numStates; j++){
      prob = fp_add(prob, s[j]);
    }
    m->prob = prob;
    sum = fp_add(sum, fp_mul(m->defaultProbability, prob));
  }
  //  printf("m->prob = %.*e,\n", m->prob);
  printf("m->prob = %f\n", fp2d(m->prob));
  for (int i=0; i < n_models; i++) {
    m = models[i];
    fp_t tmpgesture = m->prob;
    fp_t tmpmodel = m->defaultProbability;
    //    if (fp_cmp(fp_div(fp_mul(tmpmodel, tmpgesture), sum), recogprob)==1) {
    if (fp_cmp(tmpgesture, recogprob)==1) {
      probgesture = tmpgesture;
      probmodel = tmpmodel;
      recogprob = tmpgesture;//fp_div(fp_mul(tmpmodel,tmpgesture), sum);
      recognized = i;
    }
  }
  dir_filter_ref = dir_filter_ref_initial; //reset for next time
  return recognized;
}

//The quantizer, maps accelerometer readings to a set integers.
int derive_group(model *m, fp_t *acc){
  fp_t a, b, c, d;
  fp_t minDist = d2fp(0x3ffff);
  int minGroup=0;
  fp_t *ref;
  for (int i = 0; i < m->numObservations; i++){
    ref = m->quantizerMap[i];
    a = fp_sub(ref[0], acc[0]);
    b = fp_sub(ref[1], acc[1]);
    c = fp_sub(ref[2], acc[2]);
    //TODO: eliminate sqrt
    d = d2fp(sqrt(fp2d(fp_add(fp_add(fp_mul(a,a), fp_mul(b,b)), fp_mul(c,c)))));
    if (fp_cmp(d, minDist) == -1){
      minDist = d;
      minGroup = i;
    }
  }
  return minGroup;
}

//Performs the next iteration of the HMM forward algorithm
double forward_proc_inc(model *m, int o){
  fp_t *pi = m->PI;
  fp_t **a = m->A;
  fp_t **b = m->B;
  fp_t *f = m->f;
  fp_t *s = m->s;
  int numStates = m->numStates;
  double max = 0;

  if (m->started == false){
    for (int l = 0; l < numStates; l++){
      s[l] = fp_mul(pi[l], b[l][o]);
    }
    m->started = true;
  }else{
    fp_t** tmp;
    for (int k = 0; k < numStates; k++){
      fp_t sum = 0;
      for (int l = 0; l < numStates; l++){
        sum = fp_add(sum, fp_mul(s[l], a[l][k]));
      }
      f[k] = fp_mul(sum, b[k][o]);
      if (f[k] > max){
        max = f[k];
      }
    }
    m->f = s;
    m->s = f;
  }
  return max;
}

//apply various filters to accelerometer reading ACC
// returning true if the ACC passes, else false
int filter(fp_t* acc){
  if (!acc){
    return false;
  }
  fp_t abs = d2fp(sqrt(fp2d(fp_add(fp_add(fp_mul(acc[0], acc[0]),
                                          fp_mul(acc[1], acc[1])),
                                   fp_mul(acc[2], acc[2])))));
  ////////////////////////////////////////////////////////////////////////////////
  //idle state filter
  fp_t idle_sensitivity = d2fp(0.1);//d2fp(0.3);
  if (!(fp_cmp(abs, fp_add(d2fp(1), idle_sensitivity))==1 ||
        fp_cmp(abs, fp_sub(d2fp(1), idle_sensitivity))==-1)) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // def = directional equivalence filter
  fp_t def_sensitivity = d2fp(0.4);//d2fp(0.5);
  if (fp_cmp(acc[0], fp_sub(dir_filter_ref[0], def_sensitivity))==-1 ||
      fp_cmp(acc[0], fp_add(dir_filter_ref[0], def_sensitivity))== 1 ||
      fp_cmp(acc[1], fp_sub(dir_filter_ref[1], def_sensitivity))==-1 ||
      fp_cmp(acc[1], fp_add(dir_filter_ref[1], def_sensitivity))== 1 ||
      fp_cmp(acc[2], fp_sub(dir_filter_ref[2], def_sensitivity))==-1 ||
      fp_cmp(acc[2], fp_add(dir_filter_ref[2], def_sensitivity))==1) {
    dir_filter_ref = acc;
    return true;
  }
  return false;
}

fp_t *acc_reading;

fp_t* read_line(char* line){
  char* tok;
  tok = strtok(line, ",");//throw out time
  acc_reading = (fp_t*)malloc(sizeof(fp_t)*3);//mem leak
  for (int i = 0; i < 3; i++){
    tok = strtok(NULL, ",\n");
    acc_reading[i] = d2fp(strtod(tok, NULL));
  }

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

  dir_filter_ref_initial = (fp_t*)malloc(sizeof(fp_t)*3);
  dir_filter_ref = dir_filter_ref_initial;

  int n_states = models[0]->numStates;//assume they are all the same

  acc_reading = (fp_t*)malloc(sizeof(fp_t)*3);

  for (int i = 0; i < n_models; i++){
    models[i]->f = (fp_t*)calloc(n_states, sizeof(fp_t));
    models[i]->s = (fp_t*)calloc(n_states, sizeof(fp_t));
    models[i]->started = false;
  }

  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/square2/14.csv"));
  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/up_down/14.csv"));
  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/z/14.csv"));
  printf("%d\n", classify_csv_file("../we-g/gesture_recordings/roll_flip/14.csv"));

}
