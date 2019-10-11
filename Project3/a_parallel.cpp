#include "lib.h"
#include "functions.hpp"
#include <mpi.h>
#include <math.h>
#include <iomanip>
#include <iostream>

using namespace std;
int main(int argc, char** argv){
  int N;
  N=atoi(argv[1]);
  double *x = new double[N];
  double *w=new double[N];
  double lambda=2;
  double add_var;
  gauleg(-lambda,lambda,x,w,N); // Calls the lib.cpp function gauleg // THIS  LOOKS LIKE GULAG
  int local_n,numprocs,my_rank;
  double time_start,time_end,total_time;
  double local_sum;
  double total_sum;
  long long int * val;
  int i,j,k,l,m,n,counter;
  MPI_Init(&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD,&my_rank);
  time_start=MPI_Wtime();
  val=getParallelizationCoefficients(N,my_rank,numprocs,6);
  n=val[0]; m=val[1];l=val[2];k=val[3];j=val[4];i=val[5];counter=val[6];
  total_sum=0;
  while(counter>0){
    if(n>=N){
      n=0; m++;
      if(m>=N){
        m=0;l++;
      }
      if(l>=N){
        l=0;k++;
      }
      if(k>=N){
        k=0;j++;
      }
      if(j>=N){
        j=0;i++;
      }
    }
    local_sum+=w[i]*w[j]*w[k]*w[l]*w[m]*w[n]*int_func(x[i],x[j],x[k],x[l],x[m],x[n]);
    n++;
    counter--;
  }
  MPI_Reduce(&local_sum,&total_sum,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  time_end=MPI_Wtime();
  total_time=time_end-time_start;
  if (my_rank==0){
    cout<<"total sum:" << total_sum << endl;
    cout << "Time = " <<  total_time << " on number of processors: "  << numprocs  << endl;
  }
  MPI_Finalize ();
}
