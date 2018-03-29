/* ScatterV-Gather.c 
El proceso 0 crea el vector x con valores enteros en secuencia desde 0, 1, 2, ..., 99
los reparte (Scatterv) en partes desiguales y traslapadas a cada uno de los 10 procesos con los que debe correrse este ejemplillo, 
cada proceso despliega lo que recibio�

Y  si� funciona  :)   */


/*  por la clase de hoy Lunes 19 de Marzo de 2018 se introduce el uso de MPI_Scatterv el cual facilita grandemente la TP del curso.


(Esta descripci�n de Scatterv fue tomada del tutorial de MPI que aparece en la plataforma)

 MPI_Scatterv

Scatters a buffer in parts to all processes in a communicator
Synopsis

 MPI_Scatterv(sendbuf, sendcounts, displs, MPI_Datatype sendtype, recvbuf, recvcount, MPI_Datatype recvtype,root, MPI_Comm comm);

sendbuf:       address of send buffer (choice, significant only at root)
sendcounts:    integer array (of length group size) specifying the number of elements to send to each processor
displs:        integer array (of length group size). Entry i specifies the displacement (relative to sendbuf from 
                              which to take the outgoing data to process i
sendtype:      data type of send buffer elements (handle)
recvcount:     number of elements in receive buffer (integer)
recvtype:      data type of receive buffer elements (handle)
root:          rank of sending process (integer)
comm:          communicator (handle)
recvbuf:       address of receive buffer (choice)  */ 


#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

#define MAX 100  /* Comentarios sobre esto:
                  1. Ocurre que no pasa nada si el usuario indica una dimension mayor para los vectores
                  2. Deberia ser preferible dimensionar de manera dinamica, pero  
                     cuando se dimensiona dinamicamente la estructura de datos se crea en la memoria HEAP
                     y esta tiene capacidad finita.  
                     Si se hace estaticamente como aqui, la estructura se crea en la pila y "no hay limite ahi"
                     */
    
main(int argc, char* argv[]) {
    int  x[MAX];        /* Primer Vector, el cual se debe repartir entre todos los procesos*/ 
    int  local_x[MAX]; /*  Aca recibe cada proceso la parte de x que le corresponde*/
    int  p;            /*  Numero de procesos que corren */
    int  my_rank;      /*  Identificacion de cada proceso*/
    int  i;
    int  sendcounts[10];  /* nuevo para Scatterv  para 10 procesos */
    int  displs[10];      /* nuevo para Scatterv*/
    
    void Genera_vector(int v[], int m);   /* Genera valores aleatorios para  vector de enteros*/
   
    MPI_Init(&argc, &argv); /* Inicializacion del ambiente para MPI.
                            En C MPI_Init se puede usar para pasar los argumentos de la linea de comando
                            a todos los procesos, aunque no es requerido, depende de la implementacion */

    MPI_Comm_size(MPI_COMM_WORLD, &p); /* Se le pide al comunicador MPI_COMM_WORLD que 
                                          almacene en p el numero de procesos de ese comunicador.
                                          O sea para este caso, p indicara el num total de procesos que corren*/

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); /* Se le pide al comunicador MPI_COMM_WORL que devuelva en 
                                                la variable my_rank la identificacion del proceso "llamador" 
                                                la identificacion es un numero de 0 a p-1 */

    if (my_rank == 0) {   /* LO QUE SIGUE LO REALIZA UNICAMENTE EL PROCESO 0 (PROCESO RAIZ o PRINCIPAL) */
       /* inicializa el vector x  con valores, el sendcounts y el displs  */
        Genera_vector(x, 100);
        /*nuevo para Scatterv */
        for (i = 0; i < p; i++){
        sendcounts[i]= 10 + i;  /* al proceos 0 le quiero mandar 10 enteros, al 1 11, al 2 12, ... al 9 19*/
        displs[i] = i*10 - i;   /* al proceso 0 se le env�an sus 10 a partir de la entrada 0 de x, al 1 apartir de la entrada 9,
		                           al 2 a partir de la entrada 18, ... al 9 a partir de la entrada 81 */
        }
       /* Despliega  en pantalla los vectores iniciales*/
        for (i = 0; i < 100; i++) 
         printf("Proc ROOT = %d   x[%d] = %d\n", my_rank, i, x[i]);
        
        for (i = 0; i < 10; i++)
         printf("Proc ROOT = %d   sendcounts[%d] = %d\n", my_rank, i, sendcounts[i]); 
        
        for (i = 0; i < 10; i++)
         printf("Proc ROOT = %d   displs[%d] = %d\n", my_rank, i, displs[i]); 

      } /* FIN DE LO QUE REALIZA UNICAMENTE EL PROCESO CERO */
    
       
    MPI_Scatterv(x, sendcounts, displs, MPI_INT, local_x, 100, MPI_INT, 0, MPI_COMM_WORLD);
                  
    
    for (i = 0; i < 10 + my_rank; i++)                       /* Cada proceso despliega su parte de x */
    printf("Proc= %d local_x[%d] = %d\n", my_rank, i, local_x[i]); /*aparecera� desordenado porque no quise devolverle esto al proceso 0
	                                                                 para que lo desplegara en orden bonito*/

   
    MPI_Finalize();   /* Se termina el ambiente MPI */
	return 0;

}  /* main */

/*****************************************************************/
void Genera_vector(
         int    v[]        /* out */,
         int    m          /* in  */)
{
      int    i;
      for (i = 0; i < m; i++)
        v[i]= i; /* solo lo llena de enteros en secuencia del 0 a l 99*/
                
 }  /* Genera_vector */