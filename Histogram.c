#include <stdio.h>
#include <math.h>
#include <mpi.h>

/*
   Author: Ian Isely
   Date: 10/22/19
	This program reads a certain amount of doubles and spreads said doubles into equal sized
        portions across all processes using MPI_Scatter(). The processes then sort their scattered
        numbers and count (into a local array) the amount of their numbers that fall into a certain 
        interval. That array is then global summed with MPI_Reduce() and printed by process 0.
   Contributors: Ian Isely

   PS: Compile with the following: mpicc -g -Wall -std=c99 Ex1.c -lm -o Ex1
*/

// Prototypes
void Read_and_Scatter_Inputs(float*, int, int, int, int);
void Find_Histo(float*, int, int);

int main()
{
   int my_rank, comm_sz, input_sz;

   MPI_Init(NULL, NULL);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

   // Read input_sz and MPI_Bcast() it
   if(my_rank == 0)
   {
      printf("Enter the amount of number inputs: ");
      fflush(stdout);
      scanf("%d", &input_sz);
   }
   MPI_Bcast(&input_sz, 1, MPI_INT, 0, MPI_COMM_WORLD);
   
   // Preparation for MPI_Scatter()
   int local_n = input_sz/comm_sz;
   float local_nums[local_n];
   Read_and_Scatter_Inputs(local_nums, local_n, input_sz, my_rank, comm_sz);

   // Sorts local_nums for each process
   float temp;
   for(int i=0;i<local_n;i++){
      for(int j=i+1;j<local_n;j++){
         if(local_nums[i] > local_nums[j]){
            temp = local_nums[i];
            local_nums[i] = local_nums[j];
            local_nums[j] = temp;
         }
      }
   }

   Find_Histo(local_nums, local_n, my_rank);

   MPI_Finalize();
}


/*
   Function: Read_and_Scatter_Inputs
   Description: Process 0 reads floats entered by the user and scatters them
                in local_n blocks to each process.
*/
void Read_and_Scatter_Inputs(float* local_nums, int local_n, int input_sz, int my_rank, int comm_sz)
{
   float input_nums[input_sz];

   if(my_rank == 0)
   {
      printf("Enter %d floats:\n", input_sz);
      fflush(stdout);
      for(int i=0;i<input_sz;i++)
         scanf("%f", &input_nums[i]);
   }

   MPI_Scatter(input_nums, local_n, MPI_FLOAT, local_nums, local_n, MPI_FLOAT, 0, MPI_COMM_WORLD);
}


/*
   Function: Find_Histo
   Description: Maximum and minimum number within each process' numbers is calculated then the
                global maximum is found and sent to all processes using MPI_Allreduce(). 
                local_counts array is initialized with max_range size and holds the amount
                of numbers within each interval for each process. local_counts is then summed
                with MPI_Reduce() to process 0 which then prints the number of intervals and
                the corresponding amount of numbers located within said interval.
*/
void Find_Histo(float* local_nums, int local_n, int my_rank)
{
   int local_min, local_max, max_range, i, j;

   local_min = (int)floor(local_nums[0]);
   local_max = (int)ceil(local_nums[local_n-1]);

   MPI_Allreduce(&local_max, &max_range, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

   // local_counts[0] holds amount of nums from 0-1, etc.
   int local_counts[max_range];
   for(i=0;i<max_range;i++)
      local_counts[i] = 0;


   for(i=0;i<local_n;i++)
   {
      for(j=local_min;j<local_max;j++)
      {
         if(local_nums[i] >= j && local_nums[i] < j+1)
         {
            local_counts[j]++;
            break;
         }
      }
   }
   
   int max_count[max_range];
   if(my_rank == 0){
      MPI_Reduce(local_counts, max_count, max_range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

      for(int i = 0; i<max_range; i++)
         printf("Interval %d-%d has %d numbers\n", i, i+1, max_count[i]);
   }
   else
      MPI_Reduce(local_counts, max_count, max_range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
}
         

   




























