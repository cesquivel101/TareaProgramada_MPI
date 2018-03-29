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
	int* recvcounts;
	int* recvdispls;
	int* sub_matriz_m;
	int* sub_matriz_b;
	MPI_Status  status;        /* return status para receptor  */
	int         tag = 0;       /* etiqueta para mensajes */
	int numberOfIntsToAssignToSendCounts;
	int rows_sub_matriz_m;
	int* b;
	
	
	
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
		recvcounts = (int *)malloc(sizeof(int)*numProcs);;
		recvdispls = (int *)malloc(sizeof(int)*numProcs);;
		
		int i = 0;
			
		for(i = 0; i < numProcs;i++)
		{
			sendcounts[i] = numberOfIntsToAssignToSendCounts;
			recvcounts[i] = numberOfIntsToAssignToSendCounts;
			recvdispls[i] = i*((n*n)/numProcs);
		}

		//MPI_Bcast(sendcounts, numProcs, MPI_INT,myid,MPI_COMM_WORLD);
		// displs[0] = 0;
		// displs[numProcs-1] = numProcs*(n/2+1);
		
		fillMatrixAndVector(matriz_m,vector_v,n);
		
		fillDispls(displs,n,numProcs);
		
		printArray(recvdispls,numProcs);
		printArray(recvcounts,numProcs);
		//fprintf(stdout,"tengo el valor %d \n",myid,numberOfIntsToAssignToSendCounts);
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
	
	b = (int *)malloc(n*n * sizeof(int));
	sub_matriz_b = (int *)malloc(rows_sub_matriz_m*n * sizeof(int));
	//fprintf(stdout,"ANTES: HOla yo soy %d y tengo el valor %d \n",myid,numberOfIntsToAssignToSendCounts);
			//ScatterV here...
	MPI_Scatterv(matriz_m, sendcounts, displs, MPI_INT,sub_matriz_m, numberOfIntsToAssignToSendCounts, MPI_INT,0, MPI_COMM_WORLD);
	
	calculateCross(sub_matriz_m,rows_sub_matriz_m,n,sub_matriz_b);
	
	if(myid != 0)
	{
		sub_matriz_b = &sub_matriz_b[n];
	}
	else
	{
		fprintf(stdout,"mjm cuantos es numberOfIntsToAssignToSendCounts %d\n",numberOfIntsToAssignToSendCounts);
	}

	MPI_Gather(sub_matriz_b, (n*n)/numProcs, MPI_INT, b, (n*n)/numProcs, MPI_INT, 0, MPI_COMM_WORLD);

	if(myid == 0)
	{
		printf("matriz b: ");
		printMatrix(b,n);
	}
	
	MPI_Finalize();
}

int isSubtractionLessThanZero(int value)
{
	int result = 0;
	if((value - 1) < 0)
	{
		result = 1;
	}
	return result;
}

int isAdditionGreaterThanN(int value,int n)
{
	int result = 0;
	if((value + 1) >= n)
	{
		result = 1;
	}
	return result;
}

// in this case, columns is n
void calculateCross(int* array_n,int rows,int columns,int* sub_matriz_b)
{
	int i=0,j=0;
	//fprintf(stdout,"Calculo de sub matriz b %d x %d \n",rows,columns);
	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < columns ; j++)
		{
			sub_matriz_b[i*columns+j] = array_n[i*columns+j];
			//M[i,j-1]
			if(isSubtractionLessThanZero(j) == 0)
			{
				sub_matriz_b[i*columns+j] += array_n[i*columns+(j-1)];
			}
			//M[i-1,j]
			if(isSubtractionLessThanZero(i) == 0)
			{
				sub_matriz_b[i*columns+j] += array_n[(i-1)*columns+j];
			}
			//M[i+1,j]
			if(isAdditionGreaterThanN(i,rows) == 0)
			{
				sub_matriz_b[i*columns+j] += array_n[(i+1)*columns+j];
			}
			//M[i,j+1]
			if(isAdditionGreaterThanN(j,columns) == 0 )
			{
				sub_matriz_b[i*columns+j] += array_n[i*columns+(j+1)];
			}
			//fprintf(stdout,"%d ",array_n[i*columns+j]);
		}
		//printf("\n");
	}
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
		 vector_v[i] = i+1;/*randomNumber(5);*//*i; *///random 0-5
		 for(j = 0; j < n ; j++)
		{
			 matriz_m[i*n+j] = j+1;/*randomNumber(10);*//*j;*///random 0-9;
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

