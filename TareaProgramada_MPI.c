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
	//vector que guardara multiplicación m*v
	int* vector_q;
	//vector que calculara cierta parte de M*V
	int* sub_q;
	//total de primos en M
	int tp;
	//vector con número de primos por columna
	int* vector_p;
	//matriz que tendra 1 donde habia un primo y 0 si no primo
	int* primos;
	//vector fila que tendra 1 donde habia un primo y 0 si no primo
	int* primosFila;
	//hora inicial y final
	double startwtime, endwtime;
	
	
	MPI_Init(&argc,&argv);
           
	MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
 
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	

	if (myid == 0)
	{
		//Semilla para generar numeros aleatorios
		srand((unsigned)time(NULL));
		n = askForN(numProcs);
		
		//* =) empieza a tomarse el tiempo
		startwtime = MPI_Wtime();

		nn = n*n;

		matriz_m = (int *)malloc(nn * sizeof(int));
		vector_v = (int *)malloc(sizeof(int)*n);

		// * =)
		primos = (int *)malloc(nn * sizeof(int));
		
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

		// * =) Envia vector_v a todos los procesos
		MPI_Bcast(vector_v, n, MPI_INT,myid,MPI_COMM_WORLD);		 
	}

	else	//Todos los procesos excepto el 0
	{
		//Este primer broadcast nos lo podriamos volar...
		MPI_Bcast(&numberOfIntsToAssignToSendCounts, 1, MPI_INT,myid,MPI_COMM_WORLD);
		MPI_Bcast(&n, 1, MPI_INT,myid,MPI_COMM_WORLD);
		sub_matriz_m = (int *)malloc(sizeof(int)*numberOfIntsToAssignToSendCounts);
		rows_sub_matriz_m = (n/numProcs)+2;

		// * =) Recibe vector_v.   NO BORRAR ESTE
		vector_v = (int *)malloc(sizeof(int)*n);
		MPI_Bcast(vector_v, n, MPI_INT,0,MPI_COMM_WORLD);
	}

	if(myid == numProcs-1) //último proceso
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


	// * =) Calcula q y primos por Filas
	vector_q = (int *)malloc(sizeof(int)*n);
	int numFilas = n/numProcs;
	sub_q = (int *)calloc(numFilas,sizeof(int)); //numero de filas que le tocan (n/numProcs)

	primosFila = (int *)calloc(numFilas*n,sizeof(int)); //numero de columnas
	
	if (vector_q != NULL && sub_q != NULL && primosFila != NULL)
	{
		if (myid == 0)
		{
			int i = 0;
			for (i; i < numFilas; i++)
			{
				int j = 0;
				for (j; j < n; j++)
				{
					sub_q[i] += sub_matriz_m[i*n + j] * vector_v[j];
					if (sub_matriz_m[i*n + j] % 2 != 0)
					{
						if ( sub_matriz_m[i*n + j] != 9 )
						{
							primosFila[i*n + j] = 1;
						}
					}
				}
			}
		}
		else
		{
			int i = 1;
			for (i; i <= numFilas; i++)
			{
				int j = 0;
				for (j; j < n; j++)
				{
					sub_q[i-1] += sub_matriz_m[i*n + j] * vector_v[j];
					if (sub_matriz_m[i*n + j] % 2 != 0 && sub_matriz_m[i*n + j] != 9)
					{
						int temp = i-1;
						primosFila[temp*n + j] = 1;
					}
				}
			}
		}
		
	}
	//TERMINA * =) Calcula q y primos por Filas

	// * =) proceso 0 recibe vector q y primos por Fila
	MPI_Gather(sub_q, n/numProcs, MPI_INT, vector_q, n/numProcs, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(primosFila, numFilas*n, MPI_INT, primos, numFilas*n, MPI_INT, 0, MPI_COMM_WORLD);

	if(myid == 0)
	{

		// * =) Calcula tp = total de primos y vector_p = primos por columna
		tp = 0;
		vector_p = (int *)calloc(n, sizeof(int));
		if (primos != NULL && vector_p != NULL)
		{
			int i = 0;
			for (i; i < n; i++)
			{
				int j = 0;
				for (j; j < n; j++)
				{
					if (primos[i*n + j] == 1)
					{ 
						tp++;
						vector_p[j] += 1;
					}
				}
			}
		}

		// * =) Despliegue Final de Datos
		
		//* =) termina de tomarse el tiempo
		endwtime = MPI_Wtime();
		// * =) Despliega en pantalla y/o archivo los resultados finales del programa
		printf("\n *** RESULTADOS FINALES *** \n\nValor de n = %d\nCantidad de procesos que corrieron = %d\nTotal de primos en M (tp) = %d\n", n, numProcs, tp);

		if(n < 100)
		{
			printf("\nMatriz M = ");
			printMatrix(matriz_m, n, NULL);

			printf("\nVector V = ");
			printNonSquareMatrix(vector_v, 1, n, NULL);

			printf("\nVector Q (Q = M*V) = ");
			printNonSquareMatrix(vector_q, 1, n, NULL);

			printf("\nVector P (primos por columna) = ");
			printNonSquareMatrix(vector_p, 1, n, NULL);

			printf("\nMatriz B = ");
			printMatrix(b, n, NULL);

			//printf("\nFIIIIIIIIIN ");
			printf("\n  Duración del programa = %f\n", endwtime-startwtime);
		}
		else
		{
			FILE *salida;
			salida = fopen ("Resultados Finales.txt", "w");
			
			if (salida != NULL)
			{
				fprintf(salida, "*** RESULTADOS FINALES ***\n\n\nMatriz M = ");
				printMatrix(matriz_m, n, salida);

				fprintf(salida, "\n\n\nVector V = ");
				printNonSquareMatrix(vector_v, 1, n, salida);

				fprintf(salida, "\n\n\nVector Q (Q = M*V) = ");
				printNonSquareMatrix(vector_q, 1, n, salida);

				fprintf(salida, "\n\n\nVector P (primos por columna) = ");
				printNonSquareMatrix(vector_p, 1, n, salida);

				fprintf(salida, "\n\n\nMatriz B = ");
				printMatrix(b, n, salida);

				fclose (salida);
			}
			else
			{
				printf("\nERROR: Ocurrió un error al abrir archivo.\n");
			}
		}


		//Libera memoria que SOLO asigno proceso 0
		free(b);
		free(matriz_m);
		free(vector_v);
		free(sendcounts);
		free(displs);
		free(sub_matriz_b);
		free(primos);
		free(vector_p);
		free(vector_q);
		free(sub_q);
	}
	
	MPI_Finalize();

	//Libera memoria que asignaron TODOS los procesos y ya no se ocupa
	free(sub_matriz_m);
	free(primosFila);
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
void printNonSquareMatrix(int* array_n,int rows,int columns,FILE* salida)
{
	if (salida == NULL)
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
	else
	{
		int i=0,j=0;
		fprintf(salida,"Matriz NO CUADRADA %d x %d \n",rows,columns);
		for(i = 0; i < rows; i++)
		{
			for(j = 0; j < columns ; j++)
			{
				fprintf(salida,"%d ",array_n[i*columns+j]);
			}
			fprintf(salida,"\n");
		}

	}
}

//Imprime en pantalla una matriz cuadrada
void printMatrix(int* array_n,int n,FILE* salida)
{
	if (salida == NULL)
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
	else
	{
		int i=0,j=0;
		fprintf(salida,"Matriz %d x %d \n",n,n);
		for(i = 0; i < n; i++)
		{
			for(j = 0; j < n ; j++)
			{
				fprintf(salida,"%d ",array_n[i*n+j]);
			}
			fprintf(salida,"\n");
		}
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

