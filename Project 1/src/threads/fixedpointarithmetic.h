#ifndef THREADS_FIXPARTH_H
#define THREADS_FIXPARTH_H

//int DECIMAL =  14 ;/*Setting value of q of p.q fixed-point format*/
int FPFORMAT = 1 << 14 ;/*Setting f = 1 << q*/
#define CONTOFP(N) ( N * FPFORMAT ) /*Convert from integer to fixed-point*/
#define CONTOINTRZ(X) ( X / FPFORMAT ) /*Convert from fixed-point to integer rounding to zero*/
#define CONTOINTRN(X) ( (X != 0 ? (X > 0 ? ((X + FPFORMAT / 2)/FPFORMAT) : ((X - FPFORMAT / 2)/FPFORMAT)) : 0 )) /*Convert from fixed-point to integer rounding to nearest*/
#define ADDFPFP(X,Y) ( X + Y ) /*Adding two fixed-point numbers*/
#define SUBFPFP(X,Y) ( X - Y ) /*Subtracting two fixed-point numbers*/
#define ADDFPINT(X,N) ( X + (N * FPFORMAT) ) /*Adding fixed-point and integer*/
#define SUBFPINT(X,N) ( X - N * FPFORMAT ) /*Subtracting integer from fixed-point*/
#define SUBINTFP(N,X) ( N * FPFORMAT - X ) /*Subtracting fixed-point from integer*/
#define MULFPFP(X,Y) ( ((int64_t) X) * Y / FPFORMAT ) /*Multiplying two fixed-point*/
#define MULFPINT(X,N) ( X * N ) /*Multiplying fixed-point and integer*/
#define DIVFPFP(X,Y) ( ((int64_t) X) * FPFORMAT / Y ) /*Dividing two fixed-point*/
#define DIVFPINT(X,N) ( X / N ) /*Dividing fixed-point and integer*/

#endif /* threads/fixedpointarithmetic.h */
