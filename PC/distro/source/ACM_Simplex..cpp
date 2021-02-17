#include "ReVolt.h"
#include "Geom.h"
#include "Gaussian.h"

/*C     ALGORITHM 478 COLLECTED ALGORITHMS FROM ACM.
C     ALGORITHM APPEARED IN COMM. ACM, VOL. 17, NO. 06,
C     P. 319.
C THIS SUBROUTINE USES A MODIFICATION OF THE SIMPLEX METHOD
C OF LINEAR PROGRAMMING TO CALCULATE AN L1 SOLUTION TO AN
C OVER-DETERMINED SYSTEM OF LINEAR EQUATIONS.
C DESCRIPTION OF PARAMETERS.
C M      NUMBER OF EQUATIONS.
C N      NUMBER OF UNKNOWNS (M.GE.N).
C M2     SET EQUAL TO M+2 FOR ADJUSTABLE DIMENSIONS.
C N2     SET EQUAL TO N+2 FOR ADJUSTABLE DIMENSIONS.
C A      TWO DIMENSIONAL REAL ARRAY OF SIZE (M2,N2).
C        ON ENTRY, THE COEFFICIENTS OF THE MAT MUST BE
C        STORED IN THE FIRST M ROWS AND N COLUMNS OF A.
C        THESE VALUES ARE DESTROYED BY THE SUBROUTINE.
C B      ONE DIMENSIONAL REAL ARRAY OF SIZE M. ON ENTRY, B
C        MUST CONTAIN THE RIGHT HAND SIDE OF THE EQUATIONS.
C        THESE VALUES ARE DESTROYED BY THE SUBROUTINE.
C TOLER  A SMALL POSITIVE TOLERANCE. EMPIRICAL EVIDENCE
C        SUGGESTS TOLER=10**(-D*2/3) WHERE D REPRESENTS
C        THE NUMBER OF DECIMAL DIGITS OF ACCURACY AVALABLE
C        (SEE DESCRIPTION).
C X      ONE DIMENSIONAL REAL ARRAY OF SIZE N. ON EXIT, THIS
C        ARRAY CONTAINS A SOLUTION TO THE L1 PROBLEM.
C E      ONE DIMENSIONAL REAL ARRAY OF SIZE M. ON EXIT, THIS
C        ARRAY CONTAINS THE RESIDUALS IN THE EQUATIONS.
C S      INTEGER ARRAY OF SIZE M USED FOR WORKSPACE.
C ON EXIT FROM THE SUBROUTINE, THE ARRAY A CONTAINS THE
C FOLLOWING INFORMATION.
C A(M+1,N+1)  THE MINIMUM SUM OF THE ABSOLUTE VALUES OF
C             THE RESIDUALS.
C A(M+1,N+2)  THE RANK OF THE MAT OF COEFFICIENTS.
C A(M+2,N+1)  EXIT CODE WITH VALUES.
C             0 - OPTIMAL SOLUTION WHICH IS PROBABLY NON-
C                 UNIQUE (SEE DESCRIPTION).
C             1 - UNIQUE OPTIMAL SOLUTION.
C             2 - CALCULATIONS TERMINATED PREMATURELY DUE TO
C                 ROUNDING ERRORS.
C A(M+2,N+2)  NUMBER OF SIMPLEX ITERATIONS PERFORMED.
*/

void L1(BIGMAT *a, BIGVEC *b, REAL toler, BIGVEC *x, BIGVEC *e, int *s)
{
	int		m, m1, m2, n, n1, n2, out, i, j, k, l, kount, kr, kl, in;
	REAL	sum;
	REAL	min, max, d, pivot;
	bool	stage, test;

	// big must be set equal to any very large real constant.
    const REAL big = 1.0e7f;
	
	m = NRows(a);
	n = NCols(a);


	// initialization.
	m1 = m + 1;
	n1 = n + 1;
	m2 = m + 2;
	n2 = n + 2;
	
	for (j = 0; j < n; j++) {
		a->m[m1][j] = (REAL)j;
        x->v[j] = ZERO;
	}
	for (i = 0; i < m; i++) {
        a->m[i][n1] = (REAL)(n + i + 1);		// possible error (probably not)
		a->m[i][n] = b->v[i];				
		if (b->v[i] < ZERO) {
			for (j = 0; j < n2; j++) {
				a->m[i][j] = -a->m[i][j];
			}
		}
		e->v[i] = ZERO;
	}

	// compute the marginal costs.
	for (j = 0; j < n1; j++) {
		sum = ZERO;
		for (i = 0; i < m; i++) {
			sum += a->m[i][j];
		}
        a->m[m][j] = sum;
	}

	// stage i.
	// determine the vector to enter the basis.
	stage = TRUE;
	test = FALSE;
	kount = -1;						// possible error
	kr = 0;
	kl = 0;

l70:
	max = -ONE;
	for (j = kr; j < n; j++) {
		if (abs(a->m[m1][j]) <= (REAL)n) {
			d = abs(a->m[m][j]);
			if (d > max) {
				max = d;
				in = j;
			}
		}
	}
	if (a->m[m][in] < ZERO) {
		for (i=0; i < m2; i++) {
			a->m[i][in] = -a->m[i][in];
		}
	}

	// determine the vector to leave the basis.
l100:
	k = -1;
	for (i = kl; i < m; i++) {
		d = a->m[i][in];
		if (d > toler) {
			k++;
			b->v[k] = a->m[i][n1] / d;
			s[k] = i;
			test = TRUE;
		}
	}
l120:
	if (k >= 0) {
		min = big;
		for (i = 0; i <= k; i++) {
			if (b->v[i] < min) {
				j = i;
				min = b->v[i];
				out = s[i];
			}
		}
		b->v[j] = b->v[k];
		s[j] = s[k];
		k = k - 1;			//possible problem
	} else {
		test = FALSE;
	}

	// check for linear dependence in stage i.
	if (!(test || !stage)) {
		for (i = 0; i < m2; i++) {
			d = a->m[i][kr];
			a->m[i][kr] = a->m[i][in];
			a->m[i][in] = d;
		}
		kr++;					// possible problem
		goto l260;
	}
	if (!test) {
		a->m[m1][n] = Real(2);
		goto l350;
	}
	pivot = a->m[out][in];
	if (a->m[m][in] - pivot - pivot > toler) {
		for (j = kr; j < n1; j++) {
			d = a->m[out][j];
			a->m[m][j] = a->m[m][j] - d - d;
			a->m[out][j] = -d;
		}
		a->m[out][n1] = -a->m[out][n1];
		goto l120;
	}

	// pivot on a(out,in).
	for (j = kr; j < n1; j++) {
		if (j != in) {
			a->m[out][j] = a->m[out][j] / pivot;
		}
	}
	for (i = 0; i < m1; i++) {
		if (i != out) {
			d = a->m[i][in];
			for (j = kr; j < n1; j++) {
				if (j != in) {
					a->m[i][j] -= d * a->m[out][j];
				}
			}
		}
	}
	for (i = 0; i < m1; i++) {
		if (i != out) {
			a->m[i][in] = -a->m[i][in] / pivot;
		}
	}
	a->m[out][in] = ONE / pivot;
	d = a->m[out][n1];
	a->m[out][n1] = a->m[m1][in];
	a->m[m1][in] = d;
	kount++;						// possible error
	if (stage) {
	// interchange rows in stage i.
		kl++;
		for (j = kr; j < n2; j++) {
			d = a->m[out][j];
			a->m[out][j] = a->m[kount][j];
			a->m[kount][j] = d;
		}
l260:
		if (kount + kr != n1 - 2) goto l70;		// possible error
		// stage ii.
		stage = FALSE;
	}
	
	// determine the vector to enter the basis.
	max = -big;
	for (j = kr; j < n; j++) {
		d = a->m[m][j];
		if (d >= ZERO) goto l280;
		if (d > Real(-2)) goto l290;
        d = - (d + Real(2));
l280:
		if (d <= max) goto l290;
		max = d;
		in = j;
	}
l290:
	if (max > toler) {
		if (a->m[m][in] <= ZERO) {
			for (i = 0; i < m2; i++) {
				a->m[i][in] = -a->m[i][in];
			}
			a->m[m][in] = a->m[m][in] - Real(2);
		}
		goto l100;
	}

	//prepare output.
	l = kl - 1;				// possible error
	for (i = 0; i < l; i++) {
		if (a->m[i][n] < ZERO) {
			for (j = kr; j < n2; j++) {
				a->m[i][j] = -a->m[i][j];
			}
		}
	}
	a->m[m1][n] = ZERO;
	if (kr == 1) {
		for (j = 0; j < n; j++) {
			d = abs(a->m[m][j]);
			if ((d <= toler) || (Real(2 - d) <= toler)) goto l350;
		}
		a->m[m][n1] = ONE;
	}
l350:
	for (i = 0; i < m; i++) {
		k = NearestInt(a->m[i][n1]);
		d = a->m[i][n];
		if (k <= 0) {
			k = -k;
			d = -d;
		}
		if (i < kl) {
			x->v[k] = d;
		} else {
			k = k - n;
			e->v[k] = d;
		}
	}
	a->m[m1][n1] = (REAL)kount;
	a->m[m][n1] = (REAL)n1 - kr;
	sum = ZERO;
	for (i = kl; i < m; i++) {
		sum += a->m[i][n];
	}
	a->m[m][n] = sum;

}



/////////////////////////////////////////////////////////////////////
//
// TestSolver: do a few tests on the linear problem solver
//
/////////////////////////////////////////////////////////////////////

static BIGMAT	Coef, OrigCoef;
static BIGVEC	Res, NewRes, OrigRes;
static BIGVEC	Soln;
static int		Work[BIG_NMAX], OrigRow[BIG_NMAX], OrigCol[BIG_NMAX];


#ifdef _PC
void TestL1Solver()
{
	int		ii, kk;
	REAL	xx;

	/////////////////////////////////////////////////////////////////////
	// Test 1: Simple solution
	//
	SetBigMatSize(&Coef, 4, 4);
	SetBigVecSize(&Res, 4);

	Coef.m[0][0] = 1.0f;
	Coef.m[0][1] = 0.0f;
	Coef.m[0][2] = 1.0f;
	Coef.m[0][3] = 0.0f;

	Coef.m[1][0] = 1.0f;
	Coef.m[1][1] = 0.0f;
	Coef.m[1][2] = 1.0f;
	Coef.m[1][3] = 0.0f;

	Coef.m[2][0] = 0.0f;
	Coef.m[2][1] = 1.0f;
	Coef.m[2][2] = 0.0f;
	Coef.m[2][3] = 1.0f;

	Coef.m[3][0] = 0.0f;
	Coef.m[3][1] = 1.0f;
	Coef.m[3][2] = 1.0f;
	Coef.m[3][3] = 1.0f;

	Res.v[0] = 2.0f;
	Res.v[1] = 2.0f;
	Res.v[2] = 2.0f;
	Res.v[3] = 2.0f;

	CopyBigMat(&Coef, &OrigCoef);
	CopyBigVec(&Res, &OrigRes);

	L1(&Coef, &Res, 0.0001f, &Soln, &NewRes, Work);
	SolveLinearEquations(&OrigCoef, &OrigRes, 0.0001f, 0.0001f, OrigRow, OrigCol, &NewRes, &Soln);

	/////////////////////////////////////////////////////////////////////
	// Test 2:
	//
	SetBigMatSize(&Coef, 2, 2);
	SetBigVecSize(&Res, 2);

	Coef.m[0][0] = 1.0f;
	Coef.m[0][1] = 0.0f;
	Coef.m[1][0] = 1.01f;
	Coef.m[1][1] = 0.0f;

	Res.v[0] = 2.0f;
	Res.v[1] = 1.0f;

	CopyBigMat(&Coef, &OrigCoef);
	CopyBigVec(&Res, &OrigRes);

	L1(&Coef, &Res, 0.0001f, &Soln, &NewRes, Work);
	SolveLinearEquations(&OrigCoef, &OrigRes, 0.0001f, 0.0001f, OrigRow, OrigCol, &NewRes, &Soln);


	/////////////////////////////////////////////////////////////////////
	// Test 3:
	//
	SetBigMatSize(&Coef, 5, 5);
	SetBigVecSize(&Res, 5);
	SetBigVecSize(&Soln, 5);
	for (ii = 0; ii < NRows(&Coef); ii++) {
		xx = (ii + 1) * Real(0.25);
		for (kk = 0; kk < NRows(&Coef); kk++) {
			Coef.m[ii][kk] = (REAL)pow(xx, kk + 1);
		}
		Res.v[ii] = xx * (REAL)exp(-xx);
	}

	CopyBigMat(&Coef, &OrigCoef);
	CopyBigVec(&Res, &OrigRes);

	L1(&Coef, &Res, 0.0001f, &Soln, &NewRes, Work);
	SolveLinearEquations(&OrigCoef, &OrigRes, 0.0001f, 0.0001f, OrigRow, OrigCol, &NewRes, &Soln);

	/////////////////////////////////////////////////////////////////////
	// Test 4: 
	//
	SetBigMatSize(&Coef, 9, 9);
	SetBigVecSize(&Res, 9);
	SetBigVecSize(&Soln, 9);
	ClearBigMat(&Coef);
	ClearBigVec(&Res);

	for (ii = 0; ii < NRows(&Coef); ii++) {
		xx = ii / Real(8.0);
		Coef.m[ii][0] = ONE;
		Coef.m[ii][1] = xx;
		Coef.m[ii][2] = xx - ONE;
		Coef.m[ii][3] = (REAL)pow(xx, 2);
		Coef.m[ii][4] = (REAL)pow(xx, 2) - xx;
		Coef.m[ii][5] = (REAL)pow(xx, 3);
		Coef.m[ii][6] = (REAL)pow(xx, 3) - (REAL)pow(xx, 2);
		Coef.m[ii][7] = (REAL)pow(xx, 4);
		Coef.m[ii][8] = (REAL)pow(xx, 4) - (REAL)pow(xx, 3);
		Res.v[ii] = (REAL)exp(xx);
	}

	CopyBigMat(&Coef, &OrigCoef);
	CopyBigVec(&Res, &OrigRes);

	L1(&Coef, &Res, 0.0001f, &Soln, &NewRes, Work);
	SolveLinearEquations(&OrigCoef, &OrigRes, 0.0001f, 0.0001f, OrigRow, OrigCol, &NewRes, &Soln);

	/////////////////////////////////////////////////////////////////////
	// Test 5: 
	//
	SetBigMatSize(&Coef, 2, 2);
	SetBigVecSize(&Res, 2);
	SetBigVecSize(&Soln, 2);
	ClearBigMat(&Coef);
	ClearBigVec(&Res);

	Coef.m[0][0] = ONE;
	Coef.m[0][1] = ZERO;
	Coef.m[1][0] = ZERO;
	Coef.m[1][1] = 0.00001f;
	Res.v[0] = ONE;
	Res.v[1] = 1.00001f;

	CopyBigMat(&Coef, &OrigCoef);
	CopyBigVec(&Res, &OrigRes);

	L1(&Coef, &Res, 0.0001f, &Soln, &NewRes, Work);
	SolveLinearEquations(&OrigCoef, &OrigRes, 0.0001f, 0.0001f, OrigRow, OrigCol, &NewRes, &Soln);

}
#endif

