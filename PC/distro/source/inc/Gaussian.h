/////////////////////////////////////////////////////////////////////
//
// Gaussian elimination and variable sized matrices
//

#ifndef __GAUSSIAN_H__
#define __GAUSSIAN_H__

#ifdef _PC
#define DEBUG_SOLVER (TRUE)
#else
#define DEBUG_SOLVER (FALSE)
#endif

#define BIG_NMAX	32
#define BIG_MMAX	32

typedef struct {
	REAL m[BIG_NMAX][BIG_MMAX];		// matrix data
	long NRows, NCols;								// current matrix dimensions (rows, columns)
} BIGMAT;

typedef struct {
	REAL v[BIG_MMAX];
	long NElements;
} BIGVEC;

#define NRows(a) ((a)->NRows)					// Return number of rows in a matrix
#define NCols(a) ((a)->NCols)					// Return number of columns in a matrix
#define NSize(a) ((a)->NElements)				// Return number of elements in a vector
#define SetNRows(a, n) ((a)->NRows = (n))
#define SetNCols(a, m) ((a)->NCols = (m))
#define SetBigMatSize(a, n, m) { \
	(a)->NRows = (n); \
	(a)->NCols = (m); \
}
#define SetBigVecSize(v, n) { \
	(v)->NElements = (n); \
}


extern void ClearBigMat(BIGMAT *a);
extern void ClearBigVec(BIGVEC *b);
extern void CopyBigMat(BIGMAT *src, BIGMAT *dest);
extern void CopyBigVec(BIGVEC *src, BIGVEC *dest);
extern void BigVecPlusScalarVec(BIGVEC *vLeft, REAL s, BIGVEC *vRight, BIGVEC *vOut);

extern int SolveLinearEquations(BIGMAT *a, BIGVEC *b, REAL resTol, REAL coefTol, int *origRow, int *origCol, BIGVEC *z, BIGVEC *x);
extern int Eliminate(BIGMAT *a, BIGVEC *b, int *redundant, REAL pivotTol);
extern void Substitute(BIGMAT *a, BIGVEC *b, BIGVEC *x);
extern void L1(BIGMAT *a, BIGVEC *b, REAL toler, BIGVEC *x, BIGVEC *e, int *s);
extern void ConjGrad(BIGMAT *A, BIGVEC *b, REAL tol, int maxIts, BIGVEC *x, REAL *res, int *nIts);

#if DEBUG_SOLVER
extern void TestSolver();
extern bool CheckSolution(BIGMAT *origCoef, BIGVEC *origRes, BIGVEC *soln, BIGVEC *newRes, REAL error);
extern void TestConjGrad();
#endif

#endif
