/*
Compile and run as
 mpic++ -c c.cpp
 mpic++ -o c.exe c.o VMC.o functions.o System.o
 mpirun -n 4 c.exe //same amount as "howmany"
*/
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
//#include "System.h"
#include "VMC.h"
#include "functions.h"
#include <fstream>
#include <mpi.h>
#include <iomanip>
using namespace std;
void energy_equilibrium(){
  /*Creates the file "energy_distribution.csv" that contains the average energies for each 1000th iteration*/
  ofstream outfile;
  outfile.open("../results/energy_distribution.csv");
  double energy=0,energysquared=0,time=0;
  double alpha=0.88;
  int skip=2e5;
  int amount=5e6;
  double dr=1.3;
  double beta=0;
  double omega=1.0;
  double distance=0;
  double **pos=createNMatrix(2,3);pos[0][0]=1;pos[1][1]=1;pos[1][2]=1;pos[0][0]=0;pos[0][1]=0;pos[0][2]=0; //placement not based on anything
  System1 *per;//=System1(0,0,0);
  VRMonteCarlo *vrc;//=VRMonteCarlo(&per, 0,0,0,0);;
  per=new System1(alpha,beta,omega); // alpha, beta (not relevant for system1) and omega
  vrc=new VRMonteCarlo(per, dr,amount,skip,0); //System, dr, amount of samplings, how many skips
  double *energies=vrc->sample_detailed(&energy,&energysquared,&distance,&time,pos);
  outfile << "Index,Energy,omega,alpha"<<endl;
  for(int i=1000;i<amount;i+=1000){
    outfile << i << ","<<energies[i]<<","<<omega<<","<<alpha<<endl;
  }
  outfile.close();
  delete per;
  delete vrc;
}
int main(int argc, char** argv){
  double alpha_start=0.2; //Start_value for alpha
  double alpha_end=1.4; //End_value for alpha
  double d_alpha=0.01; //Step length
  int n=(int)((alpha_end-alpha_start)/d_alpha)+1; //Amount of simulations
  int howmany=4; //How many omega values there are
  double omega[howmany]={0.01,0.5,1.0,5.0};
  double * energy_list=new double[howmany*n];
  double * sigma_list=new double[howmany*n];
  double *distance_list=new double[howmany*n];
  double * energy_list_final=new double[howmany*n]; //For use in MPI_reduce
  double * sigma_list_final=new double[howmany*n];
  double *distance_list_final=new double[howmany*n];
  int samplings=1e8;
  int skip=2e5;
  double dr=1.0;
  double beta=0;
  double **pos=createNMatrix(2,3);pos[1][0]=1;pos[1][1]=1;pos[1][2]=1;pos[0][0]=0;pos[0][1]=0;pos[0][2]=0; //placement not based on anything
  System1* per;
  VRMonteCarlo* vrc;
  double energy=0,energysquared=0,time=0,distance=0,V=0,sigma=0;
  int numprocs,j;
  MPI_Init(&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD,&j);
  if(j==0){ //Only one thread needs to do this.
    energy_equilibrium();
  }
    for (int i=0;i<n;i++){
      per=new System1(alpha_start+i*d_alpha,beta,omega[j]);
      vrc=new VRMonteCarlo(per, dr,samplings,skip,j);
      vrc->sample(&energy,&energysquared,&V,&distance,&time,pos);
      sigma=sqrt(energysquared-energy*energy);
      energy_list[j*n+i]=energy;
      sigma_list[j*n+i]=sigma;
      distance_list[j*n+i]=distance;
      cout <<"Omega: "<<omega[j]<<" alpha: " <<alpha_start+i*d_alpha<< " Energy: "<<energy<<" Energy squared: "<<energysquared<< " sigma: "<<sigma<< "distance: "<<distance<<endl;
      energy=energysquared=distance=0;
      delete per;
      delete vrc;
    }
  MPI_Reduce(energy_list,energy_list_final,n*howmany,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(sigma_list,sigma_list_final,n*howmany,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(distance_list,distance_list_final,n*howmany,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  if(j==0){
    ofstream outfile;
    outfile.open("../results/function1.csv",ios::out | ios::app); //Collected results (Numbers)
    for(int i=0;i<howmany;i++){
      for (int k=0;k<n;k++){
        outfile<< setprecision(8)<<omega[i]<<","<<alpha_start+k*d_alpha<<","<<energy_list_final[k+n*i]<<","<<sigma_list_final[k+n*i]<<","<<distance_list_final[k+n*i]<<endl;
      }
    }
    outfile.close();
  }
  MPI_Finalize();
}
