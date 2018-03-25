#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc,char **argv)
{
	int n = 0, myid, numProcs, i,j,tamanio_matriz,nn;
	
	MPI_Init(&argc,&argv);
/*  Se inicia el trabajo con MPI */
           
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
/*  MPI almacena en numprocs el número total de procesos que se pusieron a correr */
 
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
/*  MPI almacena en myid la identificación del proceso actual */

    //MPI_Get_processor_name(processor_name,&namelen);

	//fprintf(stdout,"Proceso %d de %d en %s\n", myid, numprocs, processor_name);
/*  Cada proceso despliega su identificación y el nombre de la computadora en la que corre*/

	if (myid == 0)
	{
		//The seed to generate the random numbers
		srand((unsigned)time(NULL));
		n = askForN(numProcs);
		
		nn = n*n;
		int* matriz_m = (int *)malloc(nn * sizeof(int));
		int* vector_v = malloc(sizeof(int)*n);
	
		// int* matriz_m = malloc(sizeof(int)*nn);
		// int vector_v = malloc(sizeof(int)*n);
		fillMatrixAndVector(matriz_m,vector_v,n);
		
		printMatrix(matriz_m,n);
		printf("\n");
		printArray(vector_v,n);
	}
	
	MPI_Finalize();
}

int randomNumber(int modNum)
{
	int returnVal = 0;
	
	returnVal = rand() % modNum;
	return returnVal;
}

int askForN(int numProcs)
{
	int n = 0,tamanio_correcto = 0;
	while(tamanio_correcto == 0)
	{
		printf("Por favor digite el tamanio de la matriz cuadrada ");
		scanf("%d",&n);
		if(numProcs % n == 0)
		{
			tamanio_correcto = 1;
		}
		else
		{
			fprintf(stdout,"El tamanio de la matriz debe ser multiplo de %d\n",numProcs);
		}
	}
	return n;	
}


void fillMatrixAndVector(int * matriz_m,int* vector_v,int n)
{
	int i=0,j=0;
	 for(i = 0; i < n; i++)
	 {
		 vector_v[i] = randomNumber(5);//i; //random 0-5
		 for(j = 0; j < n ; j++)
		{
			 matriz_m[i*n+j] = randomNumber(10);//j;//random 0-9;
		 }
	 }
}

void printMatrix(int* array_n,int n)
{
	int i=0,j=0;
	fprintf(stdout,"Matriz %d x %d \n",n,n);
	for(i = 0; i < n; i++)
	{
		for(j = 0; j < n ; j++)
		{
			fprintf(stdout,"%d ",array_n[i*n+j]);
		}
		printf("\n");
	}
}

void printArray(int* array_n,int n)
{
	int j=0;
	fprintf(stdout,"Vector %d \n",n,n);

	for(j = 0; j < n ; j++)
	{
		fprintf(stdout,"%d ",array_n[j]);
	}
	printf("\n");

}

