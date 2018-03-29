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
	int* sendcounts;
	int* displs;
	
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
		
		int numberOfIntsToAssignToSendCounts = ((n/numProcs)+2)*n;
		//This assigns the max amount of memory to the sub matrix of every process.
		
		//sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSubMatrixM);

		sendcounts = (int *)malloc(sizeof(int)*numProcs);
		displs = (int *)malloc(sizeof(int)*numProcs);
		
		int i = 0;
		
		for(i = 0; i < numProcs;i++)
		{
			sendcounts[i] = numberOfIntsToAssignToSendCounts;
			
		}
		
		// displs[0] = 0;
		// displs[numProcs-1] = numProcs*(n/2+1);
		
		fillMatrixAndVector(matriz_m,vector_v,n);
		
		fillDispls(displs,n,numProcs);
		
		//ScatterV here...
		 //MPI_Scatterv(x, sendcounts, displs, MPI_INT, local_x, 100, MPI_INT, 0, MPI_COMM_WORLD);

		 //MPI_Scatterv(matriz_m, sendcounts, displs, MPI_INT, local_x, 100, MPI_INT, 0, MPI_COMM_WORLD);
		 
	}
	else
	{
		//other processes receive the information
	}
	
	//Magic calculations...
	
	if(myid == 0)
	{
		//Information gathering probably gatherv...
		
		// printMatrix(matriz_m,n);
		// printf("\n");
		// printArray(vector_v,n);
		printArray(displs,numProcs);
	}
	
	MPI_Finalize();
}

int scatterVLast(int n, int numProcs)
{
	return ((n*n)-(((n/numProcs)+1)*n));
}

int scatterVFirstMiddle(int n, int numProcs)
{

	return n*((n/numProcs)-1);
}

int scatterVNextMiddle(int n, int numProcs)
{
	return ((n*n)/numProcs);
}

int fillDispls(int * displs, int n, int numProcs)
{
	
	int i;
	displs[1] = scatterVFirstMiddle(n,numProcs);
	for(i = 2; i < numProcs-1;i++)
	{
		displs[i] = displs[i-1] + scatterVNextMiddle(n,numProcs);
	}
	displs[0] = 0;
	displs[numProcs-1] = scatterVLast(n,numProcs);
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
	fprintf(stdout,"Vector %d \n",n);

	for(j = 0; j < n ; j++)
	{
		fprintf(stdout,"%d ",array_n[j]);
	}
	printf("\n");

}

