#ifndef FP_H
#define FP_H

#define FP_BITS 32ull             /* Total bits per fixed-point number. */
#define FP_P 2ull                /* Number of integer bits. */
#define FP_Q 30ull                /* Number of fractional bits. */
#define FP_F (1ull << FP_Q)      /* pow(2, FP_Q). */

#define FP_MIN_INT (-FP_MAX_INT)      /* Smallest representable integer. */
#define FP_MAX_INT ((1ull << FP_P) - 1ull)  /* Largest representable integer. */


typedef /*unsigned*/ long long fp_t;

//double => fixed point
inline fp_t
d2fp(double n){
  return n*FP_F;
}

//fixed point => double
inline double
fp2d(fp_t n){
  return (double)n/FP_F;
}

// Returns X + Y. 
inline fp_t
fp_add(fp_t x, fp_t y) 
{
  return x + y;
}

// Returns X - Y. 
inline fp_t
fp_sub(fp_t x, fp_t y) 
{
  return x - y;
}

// Returns X * Y. 
inline fp_t
fp_mul(fp_t x, fp_t y) 
{
  //return d2fp(fp2d(x)*fp2d(y));
  fp_t _x = (x >= 0 ? x : -x);
  fp_t _y = (y >= 0 ? y : -y);
  fp_t xy = (long long) _x * _y / FP_F;
  if ((x >= 0) != (y >= 0)){
    xy = -xy;
  }
  return xy;
}

// Returns -1 if X < Y, 0 if X == Y, 1 if X > Y. * 
inline int
fp_cmp(fp_t x, fp_t y) 
{
  return x < y ? -1 : (x > y ? 1 : 0);
}

#endif //FP
