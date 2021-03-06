#include <math.h>
#include <stdio.h>
#include "hmm-fp.h"
#include "models.c"
#include <string.h>

int last_bit = 1 << FP_BITS-1;

//Called with each accelerometer reading
void input_reading(fp_t *acc){
  fp_t ord = 0, tmp;
  if (filter(acc)){
    printf("%f, %f, %f\n", fp2d(acc[0]), fp2d(acc[1]), fp2d(acc[2]));
    for (int i = 0; i < n_models; i++){
      int group = derive_group(models[i], acc);
      //printf("group= %d\n", group);
      ord |= forward_proc_inc(models[i], group);
    }

    //counts the number of bits we can shift left by - the leading zeros
    // : count 0
    // : count1 over if 2* over 1 . + count1 ; then drop ;
    int n = 0;
    while (ord && !(ord & last_bit)){
      n += 1;
      ord = ord << 1;
    }
    if (n>3){
      n-=3;
      for (int i = 0; i < n_models; i++){
        m = models[i];
        for (int l = 0; l < m->numStates; l++){
          m->s[l] = m->s[l] << n;
        }
      }
    }
  }//else{
  //  printf("99999\n");
  //}
}

//Called at the end of the gesture,
//Returns the id of the recognized gesture or -1 if none.
int input_end(){
  fp_t prob;
  model *m;
  fp_t* s;
  int recognized = -1; // which gesture has been recognized
  fp_t recogprob = -1; // probability of this gesture
  fp_t tmpgesture;

  for (int i = 0; i < n_models; i++){
    prob = d2fp(0.0);
    m = models[i];
    m->started = false;
    s = m->s;
    // add probabilities
    for (int j = 0; j < m->numStates; j++){
      prob = fp_add(prob, s[j]);
    }
    if (fp_cmp(prob, recogprob)==1) {
      recogprob = prob;
      recognized = i;
    }
  }
  printf("m->prob = %.30f\n", fp2d(recogprob));
  dir_filter_ref = dir_filter_ref_initial; //reset for next time
  return recognized;
}


//The quantizer, maps accelerometer readings to a set of integers.
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
    d = fp_add(fp_add(fp_mul(a,a), fp_mul(b,b)), fp_mul(c,c));
    if (fp_cmp(d, minDist) == -1){
      minDist = d;
      minGroup = i;
    }
  }
  return minGroup;
}



//Performs the next iteration of the HMM forward algorithm
fp_t forward_proc_inc(model *m, int o){
  fp_t *pi = m->PI;
  fp_t **a = m->A;
  //fp_t **b = m->B;
  fp_t *b = m->B;
  fp_t *f = m->f;
  fp_t *s = m->s;
  int numStates = m->numStates;
  fp_t ord = 0;
  fp_t sum;
  fp_t** tmp;

  if (m->started == false){
    for (int l = 0; l < numStates; l++){
      //printf("b[%d] = %d, %f\n", (o<<3) + l, (int)b[(o<<3) + l], fp2d(b[(o<<3) + l]));
      s[l] = fp_mul(pi[l], b[(o<<3) + l]);
      //printf("s[l]= %d, %f\n", (int)s[l], fp2d(s[l]));
    }
    m->started = true;
    return 0;
  }else{
    for (int k = 0; k < numStates; k++){
      sum = 0;
      for (int l = 0; l < numStates; l++){
        sum = fp_add(sum, fp_mul(s[l], a[l][k]));
      }
      f[k] = fp_mul(sum, b[(o<<3)+k]);
      //printf("f[k]= %d, %f\n", (int)f[k], fp2d(f[k]));
      ord |= f[k];
    }
    m->f = s;
    m->s = f;
  }
  //  printf("\n");
  //  for (int i = 0; i < 8; i++){
  //    printf("s[%d]= %d, %f\n", i, (int)s[i], fp2d(s[i]));
  //  }
  return ord;
}

//apply various filters to accelerometer reading ACC
// returning true if the ACC passes, else false
int filter(fp_t* acc){
  fp_t abs;
  fp_t idle_sensitivity;
  fp_t def_sensitivity;

  if (!acc){
    return false;
  }

  abs = d2fp(sqrt(fp2d(fp_add(fp_add(fp_mul(acc[0], acc[0]),
                                     fp_mul(acc[1], acc[1])),
                              fp_mul(acc[2], acc[2])))));
  //==>  0.240234, 0.009277, -0.347168
  //printf("acc =  (%f, %f, %f)\n", fp2d(acc[0]), fp2d(acc[1]), fp2d(acc[2]));
  //printf("acc[0]**2 = %f\n", fp2d(fp_mul(acc[0], acc[0])));
  //printf("acc[1]**2 = %f\n", fp2d(fp_mul(acc[1], acc[1])));
  //printf("acc[2]**2 = %f\n", fp2d(fp_mul(acc[2], acc[2])));
  //printf("acc[0]**2 + acc[1]**2 = %f\n", fp_add(fp_mul(acc[0], acc[0]),
  //                                              fp_mul(acc[1], acc[1])));
  //

  // printf("abs = %f\n", fp2d(abs));
  ////////////////////////////////////////////////////////////////////////////////
  //idle state filter
  idle_sensitivity = d2fp(0.1);//d2fp(0.3);
  int ret = 0;

  //if (fp_cmp(abs, fp_add(d2fp(1), idle_sensitivity))==1){
  if (fp_cmp(abs, fp_add(d2fp(0.2), idle_sensitivity))==1){
    ret = 1;
  }
  //if (fp_cmp(abs, fp_sub(d2fp(1), idle_sensitivity))==-1){
  if (fp_cmp(abs, fp_sub(d2fp(0.2), idle_sensitivity))==-1){
    ret = 1;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // def = directional equivalence filter
  def_sensitivity = d2fp(0.2);//d2fp(0.5);
  if (ret){
    ret = 0;
    if (fp_cmp(acc[0], fp_sub(dir_filter_ref[0], def_sensitivity))==-1){
      ret = 1;
    }else{
      if (fp_cmp(acc[0], fp_add(dir_filter_ref[0], def_sensitivity))== 1){
        ret = 1;
      }else{
        if (fp_cmp(acc[1], fp_sub(dir_filter_ref[1], def_sensitivity))==-1){
          ret = 1;
        }else{
          if (fp_cmp(acc[1], fp_add(dir_filter_ref[1], def_sensitivity))== 1){
            ret = 1;
          }else{
            if (fp_cmp(acc[2], fp_sub(dir_filter_ref[2], def_sensitivity))==-1){
              ret = 1;
            }else{
              if (fp_cmp(acc[2], fp_add(dir_filter_ref[2], def_sensitivity))==1){
                ret = 1;
              }}}}}}}

  if (ret){
    dir_filter_ref = acc;
  }
  return ret;
}

#include "main.c"
