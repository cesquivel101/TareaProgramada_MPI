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
	//arreglo de tamanio numProcs que contiene cuantos enteros a dar a cada proceso
	int* sendcounts;
	//arreglo de tamanio numProcs que contiene a partir de que entero repartir el sendcounts correspondiente
	int* displs;
	//la matriz con la que va a trabajar cada proceso
	int* sub_matriz_m;
	//Sub resultado de cada proceso para el calculo de B
	int* sub_matriz_b;
	int tag = 0;       /* etiqueta para mensajes */
	//El numero de enteros a asignar a cada proceso.
	int numberOfIntsToAssignToSendCounts;
	//Cuantas filas tiene la submatriz m
	int rows_sub_matriz_m;
	int* b;
	
	
	
	MPI_Init(&argc,&argv);
           
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
 
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	

	if (myid == 0)
	{
		//Semilla para generar numeros aleatorios
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
		
		fillMatrixAndVector(matriz_m,vector_v,n);
		
		fillDispls(displs,n,numProcs);
		 
	}
	else
	{
		//Este primer broadcast nos lo podriamos volar...
		MPI_Bcast(&numberOfIntsToAssignToSendCounts, 1, MPI_INT,myid,MPI_COMM_WORLD);
		MPI_Bcast(&n, 1, MPI_INT,myid,MPI_COMM_WORLD);
		sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSendCounts);
		rows_sub_matriz_m = (n/numProcs)+2;
	}
	if(myid == numProcs-1)
	{
		rows_sub_matriz_m = (n/numProcs)+1;
	}
	
	b = (int *)malloc(n*n * sizeof(int));
	sub_matriz_b = (int *)malloc(rows_sub_matriz_m*n * sizeof(int));
	MPI_Scatterv(matriz_m, sendcounts, displs, MPI_INT,sub_matriz_m, numberOfIntsToAssignToSendCounts, MPI_INT,0, MPI_COMM_WORLD);
	
	calculateCross(sub_matriz_m,rows_sub_matriz_m,n,sub_matriz_b);
	
	if(myid != 0)
	{
		sub_matriz_b = &sub_matriz_b[n];
	}

	MPI_Gather(sub_matriz_b, (n*n)/numProcs, MPI_INT, b, (n*n)/numProcs, MPI_INT, 0, MPI_COMM_WORLD);

	if(myid == 0)
	{
		printf("matriz b: ");
		printMatrix(b,n);
	}
	
	MPI_Finalize();
}

//Calcula que si al valor de entrada se le resta 1, el resultado es negativo.
//Se usa para calcular las casillas a sumar calculando la matriz b
int isSubtractionLessThanZero(int value)
{
	int result = 0;
	if((value - 1) < 0)
	{
		result = 1;
	}
	return result;
}

//Calcula que si el valor de entrada 'value' se le suma 1, el resultado es igual a n o mayor
//Se usa para calcular las casillas a sumar calculando la matriz b
int isAdditionGreaterThanN(int value,int n)
{
	int result = 0;
	if((value + 1) >= n)
	{
		result = 1;
	}
	return result;
}

//Calcula el valor de la sumatoria en cruz de una entrada en particular en la matriz m.
//Cada proceso hace este calculo para una sub matriz m (en este caso array_n)
//El resultado se guarda en la casilla correspondiente del parametro sub_matriz_b
void calculateCross(int* array_n,int rows,int columns,int* sub_matriz_b)
{
	int i=0,j=0;
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
		}
	}
}

//EL numero de enteros a asignar a la ultima particion de la matriz m
int scatterVLast(int n, int numProcs)
{
	return ((n*n)-(((n/numProcs)+1)*n));
}

//EL numero de enteros a asignar a la primera particion de los procesos 'medios' de la matriz m
int scatterVFirstMiddle(int n, int numProcs)
{

	return n*((n/numProcs)-1);
}

//El numero de enteros a moverme a partir de la segunda particion de los procesos medios
int scatterVMiddleDisplacement(int n, int numProcs)
{
	return ((n*n)/numProcs);
}

//Llena el arreglo displs que se usa en el scatterv
//Este arreglo contiene a partir de que entero el scatterv tiene que repartir la cantidad de enteros que esta
//en sendcounts
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

//Generador de numeros aleatorios de 0 a modNum
int randomNumber(int modNum)
{
	int returnVal = 0;
	
	returnVal = rand() % modNum;
	return returnVal;
}

// Procesa y pide hasta que este correcto, el numero filas y columnas de la matriz cuadrada de entrada m 
// N tiene que ser multiplo del numero de procesos
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

//Llena la matriz m y el vector v con valores aleatorios.
// M ==> entre 0 y 9
// V ==> entre 0 y 5
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

//Imprime en consola una matriz NO cuadrada
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

//Imprime en pantalla una matriz cuadrada
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
//Imprime un arreglo
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

