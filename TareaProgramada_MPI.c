#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int main(int argc,char **argv)
{
	int n = 0, myid, numProcs, nn;
	int* matriz_m;
	int* vector_v;
	//Every subprocess uses this to do its calculations
	int* sub_matriz_m;
	
	MPI_Init(&argc,&argv);
           
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
 
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	if (myid == 0)
	{
		//The seed to generate the random numbers
		srand((unsigned)time(NULL));
		n = askForN(numProcs);
		
		nn = n*n;

		matriz_m = (int *)malloc(nn * sizeof(int));
		vector_v = (int *)malloc(sizeof(int)*n);
		
		int numberOfIntsToAssignToSubMatrixM = ((n/numProcs)+2)*n;
		//This assigns the max amount of memory to the sub matrix of every process.
		
		sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSubMatrixM);

		fillMatrixAndVector(matriz_m,vector_v,n);
		
		//ScatterV here...
	}
	else
	{
		//other processes receive the information
	}
	
	//Magic calculations...
	
	if(myid == 0)
	{
		//Information gathering probably gatherv...
		
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
		if(n % numProcs == 0)
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

