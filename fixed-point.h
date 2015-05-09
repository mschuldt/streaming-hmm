#ifndef FP_H
#define FP_H

#define FP_BITS 18ull       // Total bits per fixed-point number.
#define FP_P 2ull           // integer bits.
#define FP_Q 16ull          // fractional bits.

#define FP_F (1ull << FP_Q)  // pow(2, FP_Q).

typedef long long fp_t;

inline fp_t
d2fp(double n){ return n*FP_F; }

inline double
fp2d(fp_t n){ return (double)n/FP_F; }

inline fp_t
fp_add(fp_t x, fp_t y) { return x + y; }

inline fp_t
fp_sub(fp_t x, fp_t y) { return x - y; }

inline fp_t
fp_mul(fp_t x, fp_t y){
  fp_t _x = (x >= 0 ? x : -x);
  fp_t _y = (y >= 0 ? y : -y);
  fp_t xy = (fp_t) _x * _y / FP_F;
  if ((x >= 0) != (y >= 0)){
    xy = -xy;
  }
  return xy;
}

inline fp_t
__fp_div(fp_t x, fp_t y){
  fp_t _x = (x >= 0 ? x : -x);
  fp_t _y = (y >= 0 ? y : -y);
  fp_t xy = (fp_t) _x * FP_F / _y;
  if ((x >= 0) != (y >= 0)){
    xy = -xy;
  }
  return xy;
}

inline fp_t
fp_inv(fp_t n){ return __fp_div(d2fp(1.0), n);}

inline int // Return -1,0,1 if <,==,>
fp_cmp(fp_t x, fp_t y){
  return x < y ? -1 : (x > y ? 1 : 0);
}

#endif

