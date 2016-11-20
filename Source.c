#include <ilcplex/cplex.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*
*   minimize  c*x 
*   subject to  Hx <= d
*               Ax = b
*               l <= x <= u
*   where
*
*   H = (  5.0  0.0  0.0 )  d = ( 25.0 )
*       (  0.0  3.0  0.0 )	    ( 20.0 )
*       (  0.0  0.0  1.0 )      ( 15.0 )
*
*   c = (  1.0  1.0  1.0 )
*   l = (  25   20   15 )
*   u = (  INF  INF  INF )
*/

#define  COLUMNS  6 
#define  ROWS   6
#define  NZTOT     (COLUMNS * ROWS)
#define demand 144826.25
#define length_of_wood 245

int
main()
{
	FILE *fp,*fp_log;										/// program data for cutting stock
	fp = fopen("output.txt", "w");
	fp_log = fopen("log.txt" , "w");

	double  Hrhs[ROWS] =  { 337.0 , 415.0 , 263.0, 736.0, 393.0, 140.0 };
	double  Hlb[COLUMNS] =  { 337, 415 , 263, 736, 393, 140 }; 
	double  Hub[COLUMNS] = { INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX };
	double  Hcost[COLUMNS] = { 1.0,  1.0,  1.0, 1.0,  1.0,  1.0};
	char    Hsense[ROWS] = { 'G', 'G', 'G', 'G', 'G', 'G'};
	int     Hmatbeg[COLUMNS] = { 0, 1, 2, 3, 4, 5};
	int     Hmatcnt[COLUMNS] = { 1, 1, 1, 1, 1, 1};
	int     Hmatind[NZTOT] = { 0, 1, 2, 3, 4 ,5};
	double  Hmatval[NZTOT] = { 2.0, 3.0, 3.0, 4.0, 5.0, 11.0};

	double        x[COLUMNS];

CPXENVptr     env = NULL;
	CPXLPptr      lp = NULL;

	int           j;
	int           status, lpstat;
	double        objval;

	//open cplex environment
	env = CPXopenCPLEX(&status);
		if (env == NULL) {
		fprintf(fp_log, "CAN NOT OPEN ENVIRONMENT :( \n");
		goto TERMINATE;
	}

	// check if ouput can be seen on screen or not
	status = CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
	if (status) {
		fprintf(fp_log, "can not see output :( %d \n", status);
		goto TERMINATE;
	}

	lp = CPXcreateprob(env, &status, "cutting_stock");

	if (lp == NULL) {
		fprintf(fp_log, "failed to create lp for the problem :( \n");
		goto TERMINATE;
	}

	status = CPXcopylp(env, lp, COLUMNS, ROWS, CPX_MIN, Hcost, Hrhs, Hsense, Hmatbeg, Hmatcnt, Hmatind, Hmatval, Hlb, Hub, NULL);

	if (status) {
		fprintf(fp_log, "could not copy problem data :( \n");
		goto TERMINATE;
	}

	status = CPXsetintparam(env, CPXPARAM_LPMethod, CPX_ALG_NET);
	if (status) {
		fprintf(fp_log, "Optimization failed :( %d \n", status);
		goto TERMINATE;
	}

	status = CPXlpopt(env, lp);
	if (status) {
		fprintf(fp_log, "Failed to optimize LP :( \n");
		goto TERMINATE;
	}

	status = CPXgetobjval(env, lp, &objval);
	if (status) {
		fprintf(fp_log, "could not get objective value :( \n");
		goto TERMINATE;
	}

	printf("Optimized objective is %lf\n", length_of_wood * objval - demand);
	status = CPXsolution(env, lp, &lpstat, &objval, x, NULL, NULL, NULL);
	if (status) {
		fprintf(fp_log, "couldn't got solution \n");
		goto TERMINATE;
	}

	fprintf(fp, "Objective value %lf\n Output is : \n", length_of_wood * objval - demand );
	for (j = 0; j < COLUMNS; j++) {
		fprintf(fp,"x[%d] = %d\n", j + 1, (int)x[j]);
	}

	status = CPXwriteprob(env, lp, "cutting_stock.txt", NULL);
	if (status) {
		fprintf(fp_log, "can not write problem to cutting_stock.lp \n");
		goto TERMINATE;
	}


TERMINATE:

	if (lp != NULL) {
		status = CPXfreeprob(env, &lp);
		if (status) {
			fprintf(fp_log, " couldnt free the cutting stock prblem \n", status);
		}
	}

	if (env != NULL) {
		status = CPXcloseCPLEX(&env);
		if (status) {
			fprintf(fp_log, "couldnt free the cplex environment.\n");
		}
	}
	getchar();
	return (status);
}
