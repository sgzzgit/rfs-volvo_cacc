/* Title: matrix.c
 * Author: Tom Kuhn
 *
 * A very simple library of 2x2 and 3x3 double-valued matrix
 * operations.  Note that all matrices in this package are arrays of
 * arrays (not square arrays).  Memory is allocated for all matrix
 * return values and the original arguments are not modified.
 *
 * No numeric sensitivity testing is done (except for checking if the
 * determinant is _near_ 0.0, not == 0.0 before inversion)
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

double **zero2()
{
  double *r0 = calloc(2,sizeof(double));
  double *r1 = calloc(2,sizeof(double));
  double **Z = calloc(2,sizeof(double *));

  if ((NULL == r0) || (NULL == r1) || (NULL == Z)) return NULL;

  r0[0] = r0[1] = 0.0;
  r1[0] = r1[1] = 0.0;

  Z[0] = r0;
  Z[1] = r1;

  return Z;
}

double **zero3()
{
  double *r0 = calloc(3,sizeof(double));
  double *r1 = calloc(3,sizeof(double));
  double *r2 = calloc(3,sizeof(double));
  double **Z = calloc(3,sizeof(double *));

  if ((NULL == r0) || (NULL == r1) || (NULL == r2) || (NULL == Z)) return NULL;

  r0[0] = r0[1] = r0[2] = 0.0;
  r1[0] = r1[1] = r1[2] = 0.0;
  r2[0] = r2[1] = r2[2] = 0.0;

  Z[0] = r0;
  Z[1] = r1;
  Z[2] = r2;

  return Z;
}

void free_mat2(double **A)
{
  if (NULL != A) {
    free(A[0]);
    free(A[1]);

    free(A);
    A = NULL;
  }
}

void free_mat3(double **A)
{
  if (NULL != A) {
    free(A[0]);
    free(A[1]);
    free(A[2]);

    free(A);
    A = NULL;
  }
}

double det2(double **A)
{
  return A[0][0]*A[1][1] - A[0][1]*A[1][0];
}

double det3(double **A)
{
  return A[0][0]*A[1][1]*A[2][2] -
         A[0][0]*A[1][2]*A[2][1] -
         A[0][1]*A[1][0]*A[2][2] +
         A[0][1]*A[1][2]*A[2][0] +
         A[0][2]*A[1][0]*A[2][1] -
         A[0][2]*A[1][1]*A[2][0];
}

double **inverse2(double **A)
{
  double detA = det2(A);
  double **invA;

  if (1.0e-15 > fabs(detA)) return NULL;
 
  invA = zero2();

  invA[0][0] = A[1][1]/detA;
  invA[0][1] = - A[0][1]/detA;

  invA[1][0] = - A[1][0]/detA;
  invA[1][1] = A[0][0]/detA;
 
  return invA;  
}

/*
 *1/(detA)*
 *[
 *  |a_(22) a_(23)| |a_(13) a_(12)| |a_(12) a_(13)|
 *  |a_(32) a_(33)| |a_(33) a_(32)| |a_(22) a_(23)|
 *
 *  |a_(23) a_(21)| |a_(11) a_(13)| |a_(13) a_(11)|
 *  |a_(33) a_(31)| |a_(31) a_(33)| |a_(23) a_(21)|
 *
 *  |a_(21) a_(22)| |a_(12) a_(11)| |a_(11) a_(12)|
 *  |a_(31) a_(32)| |a_(32) a_(31)| |a_(21) a_(22)|
 *]
 */
double **inverse3(double **A)
{
  double detA = det3(A);
  double **invA;

  if (1.0e-15 > fabs(detA)) return NULL;

  invA = zero3();

  invA[0][0] = (A[1][1]*A[2][2] - A[1][2]*A[2][1])/detA;
  invA[0][1] = (A[0][2]*A[2][1] - A[0][1]*A[2][2])/detA;
  invA[0][2] = (A[0][1]*A[1][2] - A[0][2]*A[1][1])/detA;

  invA[1][0] = (A[1][2]*A[2][0] - A[1][0]*A[2][2])/detA;
  invA[1][1] = (A[0][0]*A[2][2] - A[0][2]*A[2][0])/detA;
  invA[1][2] = (A[0][2]*A[1][0] - A[0][0]*A[1][2])/detA;

  invA[2][0] = (A[1][0]*A[2][1] - A[1][1]*A[2][0])/detA;
  invA[2][1] = (A[0][1]*A[2][0] - A[0][0]*A[2][1])/detA;
  invA[2][2] = (A[0][0]*A[1][1] - A[0][1]*A[1][0])/detA;
 
  return invA;  
}

double **transpose2(double **A)
{
  double **tranA = zero2();

  tranA[0][0] = A[0][0];
  tranA[0][1] = A[1][0];

  tranA[1][0] = A[0][1];
  tranA[1][1] = A[1][1];

  return tranA;
}

double **transpose3(double **A)
{
  double **tranA = zero3();

  tranA[0][0] = A[0][0];
  tranA[0][1] = A[1][0];
  tranA[0][2] = A[1][0];

  tranA[1][0] = A[0][1];
  tranA[1][1] = A[1][1];
  tranA[1][2] = A[1][1];

  tranA[2][0] = A[0][2];
  tranA[2][1] = A[1][2];
  tranA[2][2] = A[2][2];

  return tranA;
}

double **mat_mult2(double **A, double **B)
{
  double **AB = zero2();

  AB[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
  AB[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];

  AB[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
  AB[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];

  return AB;
}

double **mat_mult3(double **A, double **B)
{
  double **AB = zero3();

  AB[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0];
  AB[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1] + A[0][2]*B[2][1];
  AB[0][2] = A[0][0]*B[0][2] + A[0][1]*B[1][2] + A[0][2]*B[2][2];

  AB[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0] + A[1][2]*B[2][0];
  AB[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1] + A[1][2]*B[2][1];
  AB[1][2] = A[1][0]*B[0][2] + A[1][1]*B[1][2] + A[1][2]*B[2][2];

  AB[2][0] = A[2][0]*B[0][0] + A[2][1]*B[1][0] + A[2][2]*B[2][0];
  AB[2][1] = A[2][0]*B[0][1] + A[2][1]*B[1][1] + A[2][2]*B[2][1];
  AB[2][2] = A[2][0]*B[0][2] + A[2][1]*B[1][2] + A[2][2]*B[2][2];

  return AB;
}

double **mat_add2(double **A, double **B)
{
  double **AB = zero2();

  AB[0][0] = A[0][0] + B[0][0];
  AB[0][1] = A[0][1] + B[0][1];

  AB[1][0] = A[1][0] + B[1][0];
  AB[1][1] = A[1][1] + B[1][1];

  return AB;
}

double **mat_add3(double **A, double **B)
{
  double **AB = zero3();

  AB[0][0] = A[0][0] + B[0][0];
  AB[0][1] = A[0][1] + B[0][1];
  AB[0][2] = A[0][2] + B[0][2];

  AB[1][0] = A[1][0] + B[1][0];
  AB[1][1] = A[1][1] + B[1][1];
  AB[1][2] = A[1][2] + B[1][2];

  AB[2][0] = A[2][0] + B[2][0];
  AB[2][1] = A[2][1] + B[2][1];
  AB[2][2] = A[2][2] + B[2][2];

  return AB;
}

double **mat_sub2(double **A, double **B)
{
  double **AB = zero2();

  AB[0][0] = A[0][0] - B[0][0];
  AB[0][1] = A[0][1] - B[0][1];

  AB[1][0] = A[1][0] - B[1][0];
  AB[1][1] = A[1][1] - B[1][1];

  return AB;
}

double **mat_sub3(double **A, double **B)
{
  double **AB = zero3();

  AB[0][0] = A[0][0] - B[0][0];
  AB[0][1] = A[0][1] - B[0][1];
  AB[0][2] = A[0][2] - B[0][2];

  AB[1][0] = A[1][0] - B[1][0];
  AB[1][1] = A[1][1] - B[1][1];
  AB[1][2] = A[1][2] - B[1][2];

  AB[2][0] = A[2][0] - B[2][0];
  AB[2][1] = A[2][1] - B[2][1];
  AB[2][2] = A[2][2] - B[2][2];

  return AB;
}


void print_mat2(double **mat)
{
  if (NULL == mat) {
    printf("NULL\n\n");
  } else {
    printf("%6.3g %6.3g\n", mat[0][0],mat[0][1]);
    printf("%6.3g %6.3g\n\n", mat[1][0],mat[1][1]);
  }
}

void print_mat3(double **mat)
{
  if (NULL == mat) {
    printf("NULL\n\n");
  } else {
    printf("%6.3g %6.3g %6.3g\n", mat[0][0],mat[0][1],mat[0][2]);
    printf("%6.3g %6.3g %6.3g\n", mat[1][0],mat[1][1],mat[1][2]);
    printf("%6.3g %6.3g %6.3g\n\n", mat[2][0],mat[2][1],mat[2][2]);
  }
}
