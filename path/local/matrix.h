/* Title: matrix.h
 * Author: Tom Kuhn
 *
 * A very simple library of 2x2 and 3x3 double-valued matrix
 * operations.  Note that all matrices in this package are arrays of
 * arrays (not square arrays).  Memory is allocated for all matrix
 * return values and the original arguments are not modified.  So be
 * sure to use the free_mat functions to clean up the extra matrices!
 *
 * No numeric sensitivity testing is done (except for checking if the
 * determinant is _near_ 0.0, not == 0.0 before inversion)
 */

#ifndef MATRIX_H
#define MATRIX_H

/* Define and allocate space for a 2x2 or 3x3 matrix with all entries
 * 0.0.
 */ 
extern double **zero2();
extern double **zero3();

/* Frees memory allocated by zero(), inverse(), transpose(), or
 * mat_mult() functions.
 */
extern void free_mat2(double **A);
extern void free_mat3(double **A);

/* Returns the determinant of the matrix (2x2 or 3x3).
 */
extern double det2(double **A);
extern double det3(double **A);

/* Returns the inverse of the matrix if defined, otherwise returns
 * NULL.  Argument is unchanged and new memory is allocated for the
 * return.
 */
extern double **inverse2(double **A);
extern double **inverse3(double **A);

/* Returns the transpose of the matrix.
 * Argument is unchanged and new memory is allocated for the return.
 */
extern double **transpose2(double **A);
extern double **transpose3(double **A);

/* Returns the result of the matrix multiplication AB (not BA).
 * Arguments are unchanged and new memory is allocated for the return.
 */
extern double **mat_mult2(double **A, double **B);
extern double **mat_mult3(double **A, double **B);

extern double **mat_add2(double **A, double **B);
extern double **mat_add3(double **A, double **B);

extern double **mat_sub2(double **A, double **B);
extern double **mat_sub3(double **A, double **B);

/* Simple formatted ASCII output for 2x2 and 3x3 matrices.
 */
extern void print_mat2(double **A);
extern void print_mat3(double **A);

#endif /* MATRIX_H */
