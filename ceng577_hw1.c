#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseInputs(int iArgCnt, char* sArrArgs[]);
void initArrays();

int GiVectorLength = 10000000, GiIterationCnt = 100, GiSubVectorLength = 0;
int GiProcessRank = 0, GiProcessCnt = 0;
double *GdArrX, *GdArrY, *GdArrSubX, *GdArrSubY;

int main(int iArgCnt, char* sArrArgs[])
{
   int iIterationNo = 0;
   int iArrayIndex = 0;
   double dSubResult = 0, dTotalResult = 0;
   double dTime0 = 0, dTime1 = 0, dTimeDiff = 0, dMinTimeDiff = 1000, dMaxTimeDiff = 0;
   MPI_Status statusX, statusY;

   parseInputs(iArgCnt, sArrArgs);

   MPI_Init(&iArgCnt, &sArrArgs);
   MPI_Comm_size(MPI_COMM_WORLD, &GiProcessCnt);
   MPI_Comm_rank(MPI_COMM_WORLD, &GiProcessRank);

   initArrays();

   for(iIterationNo = 0; iIterationNo < GiIterationCnt; iIterationNo++)
   {
      dSubResult = 0;

      MPI_Barrier(MPI_COMM_WORLD);
      dTime0 = MPI_Wtime();
      if(GiProcessCnt > 1)
      {
         if(GiProcessRank == 0)
         {
            for(iArrayIndex = 1; iArrayIndex < GiProcessCnt; iArrayIndex++)
            {
               MPI_Send((GdArrX + (GiSubVectorLength * iArrayIndex)), GiSubVectorLength, MPI_DOUBLE, iArrayIndex, 0, MPI_COMM_WORLD);
               MPI_Send((GdArrY + (GiSubVectorLength * iArrayIndex)), GiSubVectorLength, MPI_DOUBLE, iArrayIndex, 0, MPI_COMM_WORLD);

               /*printf("Process0: Subarrays are sent to Process%d!\n", iArrayIndex);*/
            }

            for(iArrayIndex = 0; iArrayIndex < GiSubVectorLength; iArrayIndex++)
            {
               dSubResult += (GdArrX[iArrayIndex] * GdArrY[iArrayIndex]);
            }
            /*printf("Process0: I did my job and I will be waiting for the subresults!\n");*/
         }
         else
         {
            MPI_Recv(GdArrSubX, GiSubVectorLength, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &statusX);
            MPI_Recv(GdArrSubY, GiSubVectorLength, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &statusY);

            for(iArrayIndex = 0; iArrayIndex < GiSubVectorLength; iArrayIndex++)
            {
               dSubResult += (GdArrSubX[iArrayIndex] * GdArrSubY[iArrayIndex]);
            }

            /*printf("Process%d: I did my job and I will be sending my result to Process0!\n", GiProcessRank);*/
         }
      }
      else
      {
         for(iArrayIndex = 0; iArrayIndex < GiVectorLength; iArrayIndex++)
         {
            dSubResult += (GdArrX[iArrayIndex] * GdArrY[iArrayIndex]);
         }
      }

      MPI_Barrier(MPI_COMM_WORLD);
      MPI_Reduce(&dSubResult, &dTotalResult, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

      dTime1 = MPI_Wtime();
      dTimeDiff = (dTime1 - dTime0);

      if(dTimeDiff > dMaxTimeDiff)
         dMaxTimeDiff = dTimeDiff;
      if(dTimeDiff < dMinTimeDiff)
         dMinTimeDiff = dTimeDiff;
   }

   if(GiProcessRank == 0)
      printf("Result=%f\nMin Time=%f uSec\nMax Time=%f uSec\n", dTotalResult, (1.e6 * dMinTimeDiff), (1.e6 * dMaxTimeDiff));
   
   MPI_Finalize();
}

void parseInputs(int iArgCnt, char* sArrArgs[])
{
   int iArgNo = 0;

   for (iArgNo = 0; iArgNo < iArgCnt; iArgNo++)
   {
      if(strcmp("-s", sArrArgs[iArgNo]) == 0)
         GiVectorLength = atoi(sArrArgs[++iArgNo]);
      else if(strcmp("-i", sArrArgs[iArgNo]) == 0)
         GiIterationCnt = atoi(sArrArgs[++iArgNo]);
   }
}

void initArrays()
{
   const int ciSizeOfDouble = sizeof(double);
   int iArrayIndex = 0;

   GiSubVectorLength = GiVectorLength/GiProcessCnt;
   GdArrSubX = (double*)malloc(GiSubVectorLength * ciSizeOfDouble);
   GdArrSubY = (double*)malloc(GiSubVectorLength * ciSizeOfDouble);

   if(GiProcessRank == 0)
   {
      GdArrX = (double*)malloc(GiVectorLength * ciSizeOfDouble);
      GdArrY = (double*)malloc(GiVectorLength * ciSizeOfDouble);

      for(iArrayIndex = 0; iArrayIndex < GiVectorLength; iArrayIndex++)
      {
         GdArrX[iArrayIndex] = 0.1 + (0.1 * (iArrayIndex % 20));
         GdArrY[iArrayIndex] = 2.0 - (0.1 * (iArrayIndex % 20));
      }

      /*printf("Process0: Arrays are initialized!\n");*/
   }
}
