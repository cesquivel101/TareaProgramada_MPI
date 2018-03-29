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
	int* sub_matriz_m;
	MPI_Status  status;        /* return status para receptor  */
	int         tag = 0;       /* etiqueta para mensajes */
	int numberOfIntsToAssignToSendCounts;
	int rows_sub_matriz_m;
	
	
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
		
		numberOfIntsToAssignToSendCounts = ((n/numProcs)+2)*n;
		MPI_Bcast(&numberOfIntsToAssignToSendCounts, 1, MPI_INT,myid,MPI_COMM_WORLD);
		MPI_Bcast(&n, 1, MPI_INT,myid,MPI_COMM_WORLD);
		sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSendCounts);
		rows_sub_matriz_m = (n/numProcs)+1;
		//This assigns the max amount of memory to the sub matrix of every process.
		
		//sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSubMatrixM);

		sendcounts = (int *)malloc(sizeof(int)*numProcs);
		displs = (int *)malloc(sizeof(int)*numProcs);
		
		int i = 0;
		
		for(i = 0; i < numProcs;i++)
		{
			sendcounts[i] = numberOfIntsToAssignToSendCounts;
			
		}
		//MPI_Bcast(sendcounts, numProcs, MPI_INT,myid,MPI_COMM_WORLD);
		// displs[0] = 0;
		// displs[numProcs-1] = numProcs*(n/2+1);
		
		fillMatrixAndVector(matriz_m,vector_v,n);
		
		fillDispls(displs,n,numProcs);
		

		 //MPI_Scatterv(x, sendcounts, displs, MPI_INT, local_x, 100, MPI_INT, 0, MPI_COMM_WORLD);

		 //MPI_Scatterv(matriz_m, sendcounts, displs, MPI_INT, local_x, 100, MPI_INT, 0, MPI_COMM_WORLD);
		 
	}
	else
	{
		//Este primer broadcast nos lo podriamos volar...
		MPI_Bcast(&numberOfIntsToAssignToSendCounts, 1, MPI_INT,myid,MPI_COMM_WORLD);
		MPI_Bcast(&n, 1, MPI_INT,myid,MPI_COMM_WORLD);
		sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSendCounts);
		rows_sub_matriz_m = (n/numProcs)+2;
		//fprintf(stdout,"HOla yo soy %d y tengo el valor %d \n",myid,numberOfIntsToAssignToSendCounts);
	}
	if(myid == numProcs-1)
	{
		rows_sub_matriz_m = (n/numProcs)+1;
	}
	//fprintf(stdout,"ANTES: HOla yo soy %d y tengo el valor %d \n",myid,numberOfIntsToAssignToSendCounts);
			//ScatterV here...
	
	MPI_Scatterv(matriz_m, sendcounts, displs, MPI_INT,sub_matriz_m, numberOfIntsToAssignToSendCounts, MPI_INT,0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD); 
	if(myid == 0)
	{
		fprintf(stdout,"HOla yo soy %d\n",myid);
		printNonSquareMatrix(sub_matriz_m,rows_sub_matriz_m,15);
	}
	MPI_Barrier(MPI_COMM_WORLD); 
	//Magic calculations...
	if(myid == 1){
		fprintf(stdout,"HOla yo soy %d\n",myid);
		printNonSquareMatrix(sub_matriz_m,rows_sub_matriz_m,15);
	}
	MPI_Barrier(MPI_COMM_WORLD); 
	if(myid == 2){
		fprintf(stdout,"HOla yo soy %d\n",myid);
		printNonSquareMatrix(sub_matriz_m,rows_sub_matriz_m,15);
	}
	MPI_Barrier(MPI_COMM_WORLD); 
	if(myid == 0)
	{
		//Information gathering probably gatherv...
		
		// printMatrix(matriz_m,n);
		// printf("\n");
		// printArray(vector_v,n);
		// printf("displs: ");
		// printarray(displs,numprocs);
		// printf("sendcounts: ");
		// printarray(sendcounts,numprocs);
		// printf("vector_v: ");
		// printarray(vector_v,numprocs);
		printf("matriz_m: ");
		printMatrix(matriz_m,n);
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

int scatterVMiddleDisplacement(int n, int numProcs)
{
	return ((n*n)/numProcs);
}

int fillDispls(int * displs, int n, int numProcs)
{
	int i;
	displs[1] = scatterVFirstMiddle(n,numProcs);
	int displacement = scatterVMiddleDisplacement(n,numProcs);
	for(i = 2; i < numProcs-1;i++)
	{
		displs[i] = displs[i-1] + displacement;
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
		 vector_v[i] = i;//randomNumber(5);//i; //random 0-5
		 for(j = 0; j < n ; j++)
		{
			 matriz_m[i*n+j] = j;//randomNumber(10);//j;//random 0-9;
		 }
	 }
}

void printNonSquareMatrix(int* array_n,int rows,int columns)
{
	int i=0,j=0;
	fprintf(stdout,"Matriz NO CUADRADA %d x %d \n",rows,columns);
	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < columns ; j++)
		{
			fprintf(stdout,"%d ",array_n[i*columns+j]);
		}
		printf("\n");
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

