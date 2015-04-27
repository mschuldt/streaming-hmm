#ifndef FP_H
#define FP_H

#define FP_BITS 64ull             /* Total bits per fixed-point number. */
#define FP_P 32ull                /* Number of integer bits. */
#define FP_Q 32ull                /* Number of fractional bits. */
#define FP_F (1ull << FP_Q)      /* pow(2, FP_Q). */

#define FP_MIN_INT (-FP_MAX_INT)      /* Smallest representable integer. */
#define FP_MAX_INT ((1ull << FP_P) - 1ull)  /* Largest representable integer. */


typedef /*unsigned*/ long long fp_t;

// Returns a fixed-point number with F as its internal value.
inline fp_t
_fp_fix(long long f) 
{
  return (fp_t)f;
}

//double => fixed point
inline fp_t
d2fp(double n){
  return n*FP_P;
}

//fixed point => double
inline double
fp2d(fp_t n){
  return (double)n/FP_P;
}


// Returns fixed-point number corresponding to integer N.
inline fp_t
fp_int(long long n) 
{
  return _fp_fix (n * FP_F);
}

// Returns X + Y. 
inline fp_t
fp_add(fp_t x, fp_t y) 
{
  return _fp_fix (x + y);
}

// Returns X - Y. 
inline fp_t
fp_sub(fp_t x, fp_t y) 
{
  return _fp_fix (x - y);
}

// Returns X * Y. 
inline fp_t
fp_mul(fp_t x, fp_t y) 
{
  return _fp_fix ((long long) x * y / FP_F);
}

// Returns X * N. 
inline fp_t
fp_scale(fp_t x, int n) 
{
  return _fp_fix (x * n);
}

// Returns X / Y. 
inline fp_t
fp_div(fp_t x, fp_t y) 
{
  if (y == 0){
    printf("ERROR: division by 0\n");
    return 0;
  }
  return _fp_fix ((long long) x * FP_F / y);
}

// Returns -1 if X < Y, 0 if X == Y, 1 if X > Y. * 
inline int
fp_cmp(fp_t x, fp_t y) 
{
  return x < y ? -1 : x > y;
}

#endif //FP
