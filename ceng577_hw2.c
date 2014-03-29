#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define APPROACH_TYPE_NAIVE					0
#define APPROACH_TYPE_MPI_BCAST				1
#define APPROACH_TYPE_RECURSIVE_DOUBLING	2

void parseInputs(int iArgCnt, char* sArrArgs[]);
void printResults(int iApproachType, double dMinDiff, double dMaxDiff);
void getMinMaxTimeDiffs(double* dpMinTimeDiff, double* dpMaxTimeDiff, double* dArrTimeDiffs);
void recursiveDoublingBroadcast(double* dArrTransferred);
void naiveBroadcast(double* dArrTransferred);
int intlog2(int iNumber);

int GiVectorLength = 10000000, GiIterationCnt = 100;
int GiProcessRank = 0, GiProcessCnt = 0;

int main(int iArgCnt, char* sArrArgs[])
{
	const int ciSizeOfDouble = sizeof(double);
	int iIterationNo = 0;
	int iArrayIndex = 0;
	double dTimeAtStart = 0, dTimeAtFinish = 0;
	double dMinTimeDiff = 0, dMaxTimeDiff = 0;
	double *dArrTimeDiff;
	double *dArrTransferred;

	parseInputs(iArgCnt, sArrArgs);

	MPI_Init(&iArgCnt, &sArrArgs);
	MPI_Comm_size(MPI_COMM_WORLD, &GiProcessCnt);
	MPI_Comm_rank(MPI_COMM_WORLD, &GiProcessRank);

	printf("Process%d has started...\n", GiProcessRank);

	dArrTransferred = (double*)malloc(GiVectorLength * ciSizeOfDouble);
	dArrTimeDiff = (double*)malloc(GiIterationCnt * ciSizeOfDouble);

	if(GiProcessRank == 0)
	{
		srand(time(0));
		for(iArrayIndex = 0; iArrayIndex < GiVectorLength; iArrayIndex++)
		{
			dArrTransferred[iArrayIndex] = rand();
		}
	}

	/*** Naive Approach ***/
	for(iIterationNo = 0; iIterationNo < GiIterationCnt; iIterationNo++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		dTimeAtStart = MPI_Wtime();

		naiveBroadcast(dArrTransferred);

		MPI_Barrier(MPI_COMM_WORLD);
		dTimeAtFinish = MPI_Wtime();
		dArrTimeDiff[iIterationNo] = dTimeAtFinish - dTimeAtStart;
	}

	getMinMaxTimeDiffs(&dMinTimeDiff, &dMaxTimeDiff, dArrTimeDiff);
	if(GiProcessRank == 0)
	{
		printResults(APPROACH_TYPE_NAIVE, dMinTimeDiff, dMaxTimeDiff);
	}

	/*** MPI_Bcast Approach ***/
	for(iIterationNo = 0; iIterationNo < GiIterationCnt; iIterationNo++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		dTimeAtStart = MPI_Wtime();

		MPI_Bcast(dArrTransferred, GiVectorLength, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		dTimeAtFinish = MPI_Wtime();
		dArrTimeDiff[iIterationNo] = dTimeAtFinish - dTimeAtStart;
	}

	getMinMaxTimeDiffs(&dMinTimeDiff, &dMaxTimeDiff, dArrTimeDiff);
	if(GiProcessRank == 0)
	{
		printResults(APPROACH_TYPE_MPI_BCAST, dMinTimeDiff, dMaxTimeDiff);
	}

	/*** Recursive Doubling Approach ***/
	for(iIterationNo = 0; iIterationNo < GiIterationCnt; iIterationNo++)
	{
		MPI_Barrier(MPI_COMM_WORLD);
		dTimeAtStart = MPI_Wtime();

		recursiveDoublingBroadcast(dArrTransferred);

		MPI_Barrier(MPI_COMM_WORLD);
		dTimeAtFinish = MPI_Wtime();
		dArrTimeDiff[iIterationNo] = dTimeAtFinish - dTimeAtStart;
	}

	getMinMaxTimeDiffs(&dMinTimeDiff, &dMaxTimeDiff, dArrTimeDiff);
	if(GiProcessRank == 0)
	{
		printResults(APPROACH_TYPE_RECURSIVE_DOUBLING, dMinTimeDiff, dMaxTimeDiff);
	}

	MPI_Finalize();

	return 0;
}

void parseInputs(int iArgCnt, char* sArrArgs[])
{
	int iArgNo = 0;

	for (iArgNo = 1; iArgNo < iArgCnt; iArgNo++)
	{
		if(strcmp("-s", sArrArgs[iArgNo]) == 0)
			GiVectorLength = atoi(sArrArgs[++iArgNo]);
		else if(strcmp("-i", sArrArgs[iArgNo]) == 0)
			GiIterationCnt = atoi(sArrArgs[++iArgNo]);
	}
}

void printResults(int iApproachType, double dMinDiff, double dMaxDiff)
{
	char* sArrApproaches[3] = {"Naive", "MPI_BCast", "Recursive Doubling"};

	printf("%s Approach Results:\n", sArrApproaches[iApproachType]);
	printf("   Min Time=%f uSec\n", (1.e6 * dMinDiff));
	printf("   Max Time=%f uSec\n", (1.e6 * dMaxDiff));
	printf("\n\n\n");
}

void getMinMaxTimeDiffs(double* dpMinTimeDiff, double* dpMaxTimeDiff, double* dArrTimeDiffs)
{
	int iIterationNo = 0;

	*dpMinTimeDiff = dArrTimeDiffs[0];
	*dpMaxTimeDiff = dArrTimeDiffs[0];

	for(iIterationNo = 1; iIterationNo < GiIterationCnt; iIterationNo++)
	{
		if(dArrTimeDiffs[iIterationNo] > (*dpMaxTimeDiff))
			*dpMaxTimeDiff = dArrTimeDiffs[iIterationNo];

		if(dArrTimeDiffs[iIterationNo] < (*dpMinTimeDiff))
			*dpMinTimeDiff = dArrTimeDiffs[iIterationNo];
	}
}

void naiveBroadcast(double* dArrTransferred)
{
	int iReceivingProc = 0;
	MPI_Status status;

	if(GiProcessRank == 0)
	{
		for(iReceivingProc = 1; iReceivingProc < GiProcessCnt; iReceivingProc++)
		{
			MPI_Send(dArrTransferred, GiVectorLength, MPI_DOUBLE, iReceivingProc, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Recv(dArrTransferred, GiVectorLength, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
	}
}

void recursiveDoublingBroadcast(double* dArrTransferred)
{
	int iStepNo = 0;
	int iSendingProcOffset = 0;
	int iStepCnt = 0;
	int iSendingProc = 0;
	int iReceivingProc = 0;
	MPI_Status status;

	iStepCnt = intlog2(GiProcessCnt);
	iSendingProcOffset = GiProcessCnt;
	for(iStepNo = 0; iStepNo < iStepCnt; iStepNo++)
	{
		for(iSendingProc = 0; iSendingProc < GiProcessCnt; iSendingProc += iSendingProcOffset)
		{
			iReceivingProc = iSendingProc + (iSendingProcOffset >> 1);

			if(GiProcessRank == iSendingProc)
				MPI_Send(dArrTransferred, GiVectorLength, MPI_DOUBLE, iReceivingProc, 0, MPI_COMM_WORLD);
			else if(GiProcessRank == iReceivingProc)
				MPI_Recv(dArrTransferred, GiVectorLength, MPI_DOUBLE, iSendingProc, 0, MPI_COMM_WORLD, &status);
		}

		iSendingProcOffset /= 2;
	}
}

int intlog2(int iNumber)
{
	int iResult = 0;

	while(iNumber >>= 1)
		iResult++;

	return iResult;
}
