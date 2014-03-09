#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VECTOR_LENGTH   1048576
#define frand()         (((double)rand()) / RAND_MAX)

int main(int iArgCnt, char* sArrArgs[])
{
   const int ciSizeOfDouble = sizeof(double);
   int iSubArraySize = 0;
   int iProcessRank, iProcessCnt, iArrayIndex;
   double *dArrX, *dArrY, *dArrSubX, *dArrSubY;
   double dSubResult = 0, dTotalResult = 0;
   double dTime0, dTime1, dTimeDiff;
   MPI_Status status;

   MPI_Init(&iArgCnt, &sArrArgs);
   MPI_Comm_size(MPI_COMM_WORLD, &iProcessCnt);
   MPI_Comm_rank(MPI_COMM_WORLD, &iProcessRank);

   iSubArraySize = VECTOR_LENGTH/iProcessCnt;
   dArrSubX = (double*)malloc(iSubArraySize * ciSizeOfDouble);
   dArrSubY = (double*)malloc(iSubArraySize * ciSizeOfDouble);

   if(iProcessRank == 0)
   {
      dArrX = (double*)malloc(VECTOR_LENGTH * ciSizeOfDouble);
      dArrY = (double*)malloc(VECTOR_LENGTH * ciSizeOfDouble);

      for(iArrayIndex = 0; iArrayIndex < VECTOR_LENGTH; iArrayIndex++)
      {
         srand(time(0));
         dArrX[iArrayIndex] = frand();
         dArrY[iArrayIndex] = frand();
      }

      /*printf("Process0: Arrays are filled with random numbers!\n");*/
   }

   MPI_Barrier(MPI_COMM_WORLD);
   dTime0 = MPI_Wtime();
   if(iProcessCnt > 1)
   {
      if(iProcessRank == 0)
      {
         for(iArrayIndex = 1; iArrayIndex < iProcessCnt; iArrayIndex++)
         {
            MPI_Send((dArrX + (iSubArraySize * iArrayIndex)), iSubArraySize, MPI_DOUBLE, iArrayIndex, 0, MPI_COMM_WORLD);
            MPI_Send((dArrY + (iSubArraySize * iArrayIndex)), iSubArraySize, MPI_DOUBLE, iArrayIndex, 0, MPI_COMM_WORLD);

            /*printf("Process0: Subarrays are sent to Process%d!\n", iArrayIndex);*/
         }

         for(iArrayIndex = 0; iArrayIndex < iSubArraySize; iArrayIndex++)
         {
            dTotalResult += (dArrX[iArrayIndex] * dArrY[iArrayIndex]);
         }
         /*printf("Process0: I did my job and I will be waiting for the subresults!\n");*/

         for(iArrayIndex = 1; iArrayIndex < iProcessCnt; iArrayIndex++)
         {
            MPI_Recv(&dSubResult, 1, MPI_DOUBLE, iArrayIndex, 0, MPI_COMM_WORLD, &status);
            dTotalResult += dSubResult;

            /*printf("Process0: The subresult of Process%d has arrived!\n", iArrayIndex);*/
         }
      }
      else
      {
         MPI_Recv(dArrSubX, iSubArraySize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
         MPI_Recv(dArrSubY, iSubArraySize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

         for(iArrayIndex = 0; iArrayIndex < iSubArraySize; iArrayIndex++)
         {
            dSubResult += (dArrSubX[iArrayIndex] * dArrSubY[iArrayIndex]);
         }

         /*printf("Process%d: I did my job and I will be sending my result to Process0!\n", iProcessRank);*/

         MPI_Send(&dSubResult, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
      }
   }
   else
   {
      for(iArrayIndex = 0; iArrayIndex < VECTOR_LENGTH; iArrayIndex++)
      {
         dTotalResult += (dArrX[iArrayIndex] * dArrY[iArrayIndex]);
      }
   }
   dTime1 = MPI_Wtime();
   dTimeDiff = dTime1 - dTime0;

   if(iProcessRank == 0)
      printf("Result=%f Computation Time=%f\n", dTotalResult, dTimeDiff);

   MPI_Finalize();

   free(dArrSubX);
   free(dArrSubX);
   free(dArrX);
   free(dArrY);
}


