ceng577-hws
===========

Ceng577 Parallel Computing Course Homeworks

ceng577_hw1
-----------
In this homework, inner product of two vectors is computed. This application is executed using different number of processes on the cluster. The main aim is to observe the effect of the number of processes on the computing performance. The size of the vectors is selected large enough. Vectors are filled by a predefined pattern.

ceng577_hw2
-----------
In this homework, different approaches for broadcasting the data over the network are implemented. In the naive broadcast approach, one process sends its data to the other processes sequentially. In that approach, network usage remains constant. In recursive doubling broadcast, network usage is doubled at every step. Therefore, it's more effective than the naive approach. Also, the performance of these two approaches are compared with the performance of MPI_Bcast function.
