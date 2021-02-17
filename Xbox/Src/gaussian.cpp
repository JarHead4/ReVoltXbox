//-----------------------------------------------------------------------------
// File: gaussian.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "Gaussian.h"
#include "main.h"
#include "Geom.h"

int SolveLinearEquations(BIGMAT *a, BIGVEC *b, REAL resTol, REAL coefTol, int *origRow, int *origCol, BIGVEC *z, BIGVEC *x);
void ClearBigMat(BIGMAT *a);
void ClearBigVec(BIGVEC *b);
void CopyBigMat(BIGMAT *src, BIGMAT *dest);
void BigMatMulVec(BIGMAT *matLeft, BIGVEC *vecRight, BIGVEC *vecOut);
REAL BigVecDotVec(BIGVEC *vLeft, BIGVEC *vRight);
void BigVecPlusScalarVec(BIGVEC *vLeft, REAL s, BIGVEC *vRight, BIGVEC *vOut);
void ConjGrad(BIGMAT *A, BIGVEC *b, REAL tol, int maxIts, BIGVEC *x, REAL *res, int *nIts);

#ifdef DEBUG_SOLVER
void TestSolver();
void TestConjGrad();
#endif


/////////////////////////////////////////////////////////////////////
//
// SolveLinearEquations: modified gaussian elimination from Barrodale & Stuart,
// ACM transactions on mathematical software, Sept. 1981.
//
// Inputs:
//      a:      NxN matrix of equation coefficients
//      b:      N vector of residuals
//      resTol: residuals tolerance
//      coefTol:    coefficients tolerance
//      
// Work variables:
//      origRow:    store for keeping track of row interchanges
//                  (origRow[i] is original row of current row i)
//      origCol:    store for keeping track of column interchanges
//                  (origRow[i] is original column of current column i)
//      z:          work array of reals to calculate results before
//                  reordering back into x
//
// Outputs:
//      x:      N vector with solution
//      return: number of equations solved
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
int SolveLinearEquations(BIGMAT *a, BIGVEC *b, REAL resTol, REAL coefTol, int *origRow, int *origCol, BIGVEC *z, BIGVEC *x)
{
    int     ii, jj, kk;
    int     nSolvable, tInt;
    int     maxRow, maxCol;
    REAL    maxCoef, maxRes;
    REAL    tReal;
    
    
    // Make sure there is something to solve
    if (NRows(a) == 0) return 0;

    // Initialise row and column permutation vectors
    for (ii = 0; ii < NRows(a); ii++) {
        origRow[ii] = origCol[ii] = ii;
    }

    // Main elimination loop 
    // (on exit, kk is the number of solvable equations)
    for (kk = 0; kk < NRows(a); kk++) {

        // Find the equation with the largest residual
        maxRow = kk;
        maxRes = abs(b->v[maxRow]);
        for (ii = kk + 1; ii < NRows(a); ii++) {
            if (abs(b->v[ii]) > maxRes) {
                maxRes = abs(b->v[ii]);
                maxRow = ii;
            }
        }

        // If the largest resudual is less than tolerance, start back substitution
        // since all further variables must be zero or undetermined
        if (maxRes < resTol) {
            break;
        }

        // Determine the pivot column
        maxCol = kk;
        maxCoef = abs(a->m[maxRow][maxCol]);
        for (ii = kk + 1; ii < NRows(a); ii++) {
            if (abs(a->m[maxRow][ii]) > maxCoef) {
                maxCoef = abs(a->m[maxRow][ii]);
                maxCol = ii;
            }
        }

        // If pivot less than tolerance, perform full pivot on
        // lower square of matrix and determine a new pivot
        if (maxCoef < coefTol) {
            maxCoef = ZERO;
            for (ii = kk; ii < NRows(a); ii++) {
                for (jj = kk; jj < NRows(a); jj++) {
                    if (abs(a->m[ii][jj]) > maxCoef) {
                        maxCoef = abs(a->m[ii][jj]);
                        maxRow = ii;
                        maxCol = jj;
                    }
                }
            }

            // If pivot still below tolerance, start back substitution
            if (maxCoef < coefTol) {
                break;
            }
        }

        // If pivot row is not the kth row, perform permutations to make it so
        // and record the change 
        if (maxRow != kk) {
            for (jj = 0; jj < NRows(a); jj++) {
                tReal = a->m[kk][jj];
                a->m[kk][jj] = a->m[maxRow][jj];
                a->m[maxRow][jj] = tReal;
            }
            tReal = b->v[kk];
            b->v[kk] = b->v[maxRow];
            b->v[maxRow] = tReal;
            tInt = origRow[kk];
            origRow[kk] = origRow[maxRow];
            origRow[maxRow] = tInt;
        }

        // If pivot column is not kth column, perform permutations to make it so
        // and record the change
        if (maxCol != kk) {
            for (jj = 0; jj < NRows(a); jj++) {
                tReal = a->m[jj][kk];
                a->m[jj][kk] = a->m[jj][maxCol];
                a->m[jj][maxCol] = tReal;
            }
            tInt = origCol[kk];
            origCol[kk] = origCol[maxCol];
            origCol[maxCol] = tInt;
        }

        // Do the elimination for this step
        tReal = - ONE / a->m[kk][kk];
        //tReal = - a->m[kk][kk];
        for (ii = kk + 1; ii < NRows(a); ii++) {
            a->m[ii][kk] *= tReal;
        }
        // a matrix
        for (jj = kk + 1; jj < NRows(a); jj++) {
            tReal = a->m[kk][jj];
            for (ii = kk + 1; ii < NRows(a); ii++) {
                a->m[ii][jj] += tReal * a->m[ii][kk];
            }
        }
        // b vector
        tReal = b->v[kk];
        for (ii = kk + 1; ii < NRows(a); ii++) {
            b->v[ii] += tReal * a->m[ii][kk];
        }
        /*tReal = ONE / a->m[kk][kk];
        for (jj = kk + 1; jj < NRows(a); jj++) {
            b->v[jj] -= b->v[kk] * a->m[jj][kk] * tReal; // / a->m[kk][kk];
            for (ii = NRows(a) - 1; ii >= kk; ii--) {
                a->m[jj][ii] -= a->m[kk][ii] * a->m[jj][kk] * tReal; // / a->m[kk][kk];
            }
        }*/



    }

    // kk is the number of solvable, non-trivial equations 
    // (unless last b element is less than tolerance)
    if (abs(b->v[kk - 1]) < resTol) {
        nSolvable = kk -1;
    } else {
        nSolvable = kk;
    }

    // Make sure there are some solvable and non-trivial equations
    if (nSolvable <= 0) {
        return 0;
    }

    // Perform back-substitution to solve the solvable equations
    if (nSolvable > 1) {
        for (jj = nSolvable - 1; jj > 0; jj--) {
            z->v[jj] = b->v[jj] / a->m[jj][jj];
            tReal = - z->v[jj];
            for (ii = 0; ii < jj; ii++) {
                b->v[ii] += tReal * a->m[ii][jj];
            }
        }
    }
    z->v[0] = b->v[0] / a->m[0][0];

    // Zero the unused variables
    for (ii = nSolvable; ii < NRows(a); ii++) {
        z->v[ii] = ZERO;
    }

    // Reorder variables to correspond the the input order
    for (ii = 0; ii < NRows(a); ii++) {
        x->v[origCol[ii]] = z->v[ii];
    }

    return nSolvable;
}
#endif


/////////////////////////////////////////////////////////////////////
//
// ClearBigMat: Set all active elements of matrix to zero
//
/////////////////////////////////////////////////////////////////////

void ClearBigMat(BIGMAT *a)
{
    int i,j;

    for (i = 0; i < NRows(a); i++) {
        for (j = 0; j < NCols(a); j++) {
            a->m[i][j] = ZERO;
        }
    }
}

void ClearBigVec(BIGVEC *b)
{
    int i;

    for (i = 0; i < NSize(b); i++) {
        b->v[i] = ZERO;
    }
}

#ifdef _PC
void CopyBigMat(BIGMAT *src, BIGMAT *dest) 
{
    int i, j;

    SetBigMatSize((dest), NRows(src), NCols(src));
    for (i = 0; i < NRows(src); i++) {
        for (j = 0; j < NCols(src); j++) {
            (dest)->m[i][j] = (src)->m[i][j];
        }
    }
}
#endif

void CopyBigVec(BIGVEC *src, BIGVEC *dest) 
{
    int i;

    SetBigVecSize((dest), NSize(src));
    for (i = 0; i < NSize(src); i++) {
        (dest)->v[i] = (src)->v[i];
    }
}

void BigMatMulVec(BIGMAT *matLeft, BIGVEC *vecRight, BIGVEC *vecOut)
{
    int ii, jj;

    if (NCols(matLeft) != NSize(vecRight)) return;

    for (ii = 0; ii < NRows(matLeft); ii++) {
        vecOut->v[ii] = ZERO;
        for (jj = 0; jj < NCols(matLeft); jj++) {
            vecOut->v[ii] += MulScalar(matLeft->m[ii][jj], vecRight->v[jj]);
        }
    }
}

REAL BigVecDotVec(BIGVEC *vLeft, BIGVEC *vRight)
{
    int ii;
    REAL dot;

    Assert(NSize(vLeft) == NSize(vRight));

    dot = ZERO;
    for (ii = 0; ii < NSize(vLeft); ii++) {
        dot += MulScalar(vLeft->v[ii], vRight->v[ii]);
    }

    return dot;
}

void BigVecPlusScalarVec(BIGVEC *vLeft, REAL s, BIGVEC *vRight, BIGVEC *vOut)
{
    int ii;

    Assert(NSize(vLeft) == NSize(vRight));

    for (ii = 0; ii < NSize(vLeft); ii++) {
        vOut->v[ii] = vLeft->v[ii] + MulScalar(s, vRight->v[ii]);
    }

}



/////////////////////////////////////////////////////////////////////
//
// TestSolver: do a few tests on the linear problem solver
//
/////////////////////////////////////////////////////////////////////
#if DEBUG_SOLVER
static BIGMAT   Coef, OrigCoef;
static BIGVEC   Res, NewRes, OrigRes;
static BIGVEC   Soln;
static BIGVEC   Work;
static int      OrigRow[BIG_NMAX];
static int      OrigCol[BIG_NMAX];


void TestSolver()
{
    int     ii, kk;
    REAL    xx;

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

    SolveLinearEquations(&Coef, &Res, 0.0001f, 0.01f, OrigRow, OrigCol, &Work, &Soln);


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

    SolveLinearEquations(&Coef, &Res, 0.0001f, 0.01f, OrigRow, OrigCol, &Work, &Soln);


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
    SolveLinearEquations(&Coef, &Res, 0.0f, 0.0001f, OrigRow, OrigCol, &Work, &Soln);

    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

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
    SolveLinearEquations(&Coef, &Res, 0.0f, 0.0001f, OrigRow, OrigCol, &Work, &Soln);

    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

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
    SolveLinearEquations(&Coef, &Res, 0.0f, 0.0001f, OrigRow, OrigCol, &Work, &Soln);

    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

}


void TestConjGrad()
{
    int     ii, kk, nIts;
    REAL    res;
    float   xx;

    /////////////////////////////////////////////////////////////////////
    // Test 1: Simple solution
    //
    SetBigMatSize(&Coef, 4, 4);
    SetBigVecSize(&Res, 4);
    SetBigVecSize(&Soln, 4);

    Coef.m[0][0] = ONE;
    Coef.m[0][1] = ZERO;
    Coef.m[0][2] = ZERO;
    Coef.m[0][3] = ZERO;

    Coef.m[1][0] = ZERO;
    Coef.m[1][1] = ONE;
    Coef.m[1][2] = ZERO;
    Coef.m[1][3] = ZERO;

    Coef.m[2][0] = ZERO;
    Coef.m[2][1] = ZERO;
    Coef.m[2][2] = ZERO;
    Coef.m[2][3] = ONE;

    Coef.m[3][0] = ZERO;
    Coef.m[3][1] = ZERO;
    Coef.m[3][2] = ONE;
    Coef.m[3][3] = ZERO;

    Res.v[0] = Real(1.0);
    Res.v[1] = Real(5.0);
    Res.v[2] = Real(1.0);
    Res.v[3] = Real(2.0);

    ConjGrad(&Coef, &Res, Real(0.0001), 10, &Soln, &res, &nIts);

#ifdef _PSX
    printf("Solution:\n%d %d %d %d\n", Soln.v[0], Soln.v[1], Soln.v[2], Soln.v[3]);
    printf("Res: %d  Its: %d\n", res, nIts);
#endif

    /////////////////////////////////////////////////////////////////////
    // Test 2:
    //
    SetBigMatSize(&Coef, 2, 2);
    SetBigVecSize(&Res, 2);

    Coef.m[0][0] = ONE;
    Coef.m[0][1] = ZERO;
    Coef.m[1][0] = ONE;
    Coef.m[1][1] = ZERO;

    Res.v[0] = Real(2.0f);
    Res.v[1] = Real(1.0f);

    ConjGrad(&Coef, &Res, Real(0.0001), 10, &Soln, &res, &nIts);

#ifdef _PSX
    printf("Solution:\n%d %d\n", Soln.v[0], Soln.v[1]);
    printf("Res: %d  Its: %d\n", res, nIts);
#endif


    /////////////////////////////////////////////////////////////////////
    // Test 3:
    //
    SetBigMatSize(&Coef, 5, 5);
    SetBigVecSize(&Res, 5);
    SetBigVecSize(&Soln, 5);
    for (ii = 0; ii < NRows(&Coef); ii++) {
        xx = (ii + 1) * 0.25f;
        for (kk = 0; kk < NRows(&Coef); kk++) {
            Coef.m[ii][kk] = (REAL)pow(xx, kk + 1);
        }
        xx = xx  * (float)exp(-xx);
        Res.v[ii] = Real(xx);
    }
    CopyBigMat(&Coef, &OrigCoef);
    CopyBigVec(&Res, &OrigRes);
    ConjGrad(&Coef, &Res, Real(0.0001), 10, &Soln, &res, &nIts);

#ifdef _PSX
    printf("Solution:\n%d %d %d %d %d\n", Soln.v[0], Soln.v[1], Soln.v[2], Soln.v[3], Soln.v[4]);
    printf("Res: %d  Its: %d\n", res, nIts);
#endif


    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

    /////////////////////////////////////////////////////////////////////
    // Test 4: 
    //
    SetBigMatSize(&Coef, 9, 9);
    SetBigVecSize(&Res, 9);
    SetBigVecSize(&Soln, 9);
    ClearBigMat(&Coef);
    ClearBigVec(&Res);

    for (ii = 0; ii < NRows(&Coef); ii++) {
        xx = ii / 8.0f;
        Coef.m[ii][0] = ONE;
        Coef.m[ii][1] = Real(xx);
        Coef.m[ii][2] = Real(xx - 1.0f);
        Coef.m[ii][3] = Real(pow(xx, 2));
        Coef.m[ii][4] = Real(pow(xx, 2) - xx);
        Coef.m[ii][5] = Real(pow(xx, 3));
        Coef.m[ii][6] = Real(pow(xx, 3) - pow(xx, 2));
        Coef.m[ii][7] = Real(pow(xx, 4));
        Coef.m[ii][8] = Real(pow(xx, 4) - pow(xx, 3));
        Res.v[ii] = Real(exp(xx));
    }
    CopyBigMat(&Coef, &OrigCoef);
    CopyBigVec(&Res, &OrigRes);
    ConjGrad(&Coef, &Res, Real(0.0001), 100, &Soln, &res, &nIts);

#ifdef _PSX
    printf("Solution:\n%d %d %d %d\n%d %d %d %d\n", Soln.v[0], Soln.v[1], Soln.v[2], Soln.v[3], Soln.v[4], Soln.v[5], Soln.v[6], Soln.v[7]);
    printf("Res: %d  Its: %d\n", res, nIts);
#endif


    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

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
    Coef.m[1][0] = ONE;
    Coef.m[1][1] = Real(0.001);
    Res.v[0] = ONE;
    Res.v[1] = Real(1.001);

    CopyBigMat(&Coef, &OrigCoef);
    CopyBigVec(&Res, &OrigRes);
    ConjGrad(&Coef, &Res, Real(0.001), 10, &Soln, &res, &nIts);

#ifdef _PSX
    printf("Solution:\n%d %d\n", Soln.v[0], Soln.v[1]);
    printf("Res: %d  Its: %d\n", res, nIts);
#endif

    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

    /////////////////////////////////////////////////////////////////////
    // Test 6: 
    //
    SetBigMatSize(&Coef, 1, 1);
    SetBigVecSize(&Res, 1);
    SetBigVecSize(&Soln, 1);
    ClearBigMat(&Coef);
    ClearBigVec(&Res);

    Coef.m[0][0] = Real(0.908250);
    Res.v[0] = Real(0.002644);

    CopyBigMat(&Coef, &OrigCoef);
    CopyBigVec(&Res, &OrigRes);
    ConjGrad(&Coef, &Res, Real(0.001), 10, &Soln, &res, &nIts);

#ifdef _PSX
    printf("Solution:\n%d %d\n", Soln.v[0], Soln.v[1]);
    printf("Res: %d  Its: %d\n", res, nIts);
#endif

    BigMatMulVec(&OrigCoef, &Soln, &NewRes);

}
#endif

#ifdef _PC
bool CheckSolution(BIGMAT *origCoef, BIGVEC *origRes, BIGVEC *soln, BIGVEC *newRes, REAL error)
{
    int ii;
    REAL diff, diffSq;
    bool solnOkay = TRUE;

    //Calculate new residual from the supplied solution
    BigMatMulVec(origCoef, soln, newRes);

    for (ii = 0; ii < NRows(origCoef); ii++) {
        diff = newRes->v[ii] - origRes->v[ii];
        diffSq = diff * diff;
        if (abs(diff) > error) {
            solnOkay = FALSE;
        }
    }

    return solnOkay;
}
#endif


/////////////////////////////////////////////////////////////////////
//
// ConjGrad: Solve the equations Ax=b using conjugate gradients
// minimisation
//
/////////////////////////////////////////////////////////////////////

BIGVEC r, p, t;

void ConjGrad(BIGMAT *A, BIGVEC *b, REAL tol, int maxIts, BIGVEC *x, REAL *res, int *nIts)
{
    REAL alpha, beta;
    REAL rSq, rSqOld, rNorm, pDott;
    bool terminate = FALSE;;

    Assert((tol > ZERO) && (maxIts > 0));

    // Initialise
    SetBigVecSize(&r, NSize(b));
    SetBigVecSize(&t, NSize(b));
    SetBigVecSize(x, NSize(b));
    ClearBigVec(x);
    CopyBigVec(b, &p);
    CopyBigVec(b, &r);
    beta = ZERO;
    rSq = ZERO;
    rNorm = MulScalar( Real(2), tol );

    // Minimise - loop while not too many iterations and outside residual tolerance
    *nIts = 0;

    do {

        rSqOld = rSq;
        rSq = BigVecDotVec(&r, &r);

#ifdef _PSX
        rNorm = SquareRoot1616(rSq);
#else
        rNorm = (REAL)sqrt(rSq);
#endif

        if ((*nIts < maxIts) && ((*res = rNorm - tol) > ZERO)) {

            if (*nIts > 0) {
                /*if (rSqOld > SMALL_REAL) {
                    beta = DivScalar( rSq, rSqOld);
                    BigVecPlusScalarVec(&r, beta, &p, &p);
                } else {
                    CopyBigVec(&r, &p);
                    terminate = TRUE;
                }*/
                beta = DivScalar( rSq, rSqOld);
                BigVecPlusScalarVec(&r, beta, &p, &p);
            }

            BigMatMulVec(A, &p, &t);

            pDott = BigVecDotVec(&p, &t);
            if (abs(pDott) > SMALL_REAL) {
                alpha = DivScalar( rSq, pDott);
                BigVecPlusScalarVec(x, alpha, &p, x);

                BigMatMulVec(A, &p, &t);
                BigVecPlusScalarVec(&r, -alpha, &t, &r);
                (*nIts)++;
            } else {
                terminate = TRUE;
            }
        } else {
            terminate = TRUE;
        }

    } while(!terminate);
}


////////////////////////////////////////////////////////////////
//
// ScaleEquations:
//
////////////////////////////////////////////////////////////////
#if 0
void ScaleEquations(BIGMAT *A, BIGVEC *b, REAL min, BIGVEC *p)
{
    long ii, jj;
    register REAL tReal;

    // calculate the diagonal part of the precondition matrix in a vector
    for (ii = 0; ii < NSize(b); ii++) {

        if (b->v[ii] > min) {
            tReal = p->v[ii] = ONE / b->v[ii];
            b->v[ii] = ONE;

            for (jj = 0; jj < NCols(A); jj++) {
                A->m[ii][jj] *= tReal;
            }
        } else {
            p->v[ii] = ONE;
        }
    }
}
#endif

