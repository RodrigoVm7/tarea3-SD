#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <mpi.h>
#define N 3   

MPI_Status status;

int main(int argc, char *argv[]){
	int matriz_a[N][N];
	int matriz_b[N][N];
	int matriz_c[N][N];
	int num_procesos, id_proceso, num_tareas, fuente, destino, columnas, offset, i, j, k, l, x;
	char c[100];
	int llenarMatrizb=0;
	int filasTotal=0, fil=0, col, columnasTotal;
	int maximos[N];
	int max;
	int filas_calculadas, filas_faltantes;

	MPI_Init(&argc, &argv);                   		 //se inicializa el proceso actual
	MPI_Comm_rank(MPI_COMM_WORLD, &id_proceso); 	//se obtiene el id del proceso
	MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);  //se obtiene el numero de procesos con el que se trabajara

	if(id_proceso==0){

		// ****************************Lectura de los datos de las matrices A y B****************************************


		while(fgets(c, 100, stdin) != NULL){ //leemos una linea completa a la vez hasta el final del texto para saber las dimensiones de las matrices
			col=0;    						// numero de columnas
			k=0;
			if (columnasTotal+1==filasTotal){ //identifica que hemos llegado a la mitad del archivo y envia el aviso
				fil=0;						//reseteamos el valor de fila para llenar la matriz b
				llenarMatrizb=1;  		//aviso	
			}
			while(c[k]!='\0'){        // leeamos la fila de texto que tenemos en la variable c
				l=0;
				char aux[10] = "";
				while(c[k]!=' ' && c[k]!='\0'){    	 //hasta el espacio en blanco para diferenciar cada numero
					aux[l]=c[k];       				// guardamos cada digito en una variable auxiliar
					l++;
					k++;
				}
				if (llenarMatrizb==1){   	 // recibe aviso que llegamos a la mitad del archivo por lo que los siguientes valores corresponden a la matriz b
					matriz_b[fil][col] =atoi(aux); 	// ponemos el numero obtenido en la matriz b
					
				}else
				{
					matriz_a[fil][col] =atoi(aux); // ponemos el numero obtenido en la matriz a
					
				}
				
				if(c[k]=='\0'){  			//si encuentra el final de la linea
					columnasTotal=col; 		// obtenemos la cantidad de columnas total de la matriz
					fil++;
					break;                  //se termino de leer la fila...
				}
				col++;
				k++;
			}
			filasTotal++;
		}

	num_tareas = num_procesos-1;	//  define el numero de tareas a ejecutar
	offset=0;                       // indica el numero de fila que se procesara

	columnas = N/num_tareas;	//	Obtiene el numero de columnas que procesara el hilo

	if(N%num_tareas==1){
		filas_faltantes = N - columnas*num_tareas;
	}

	filas_calculadas=0;         

	for (destino=1; destino<=num_tareas;destino++){
		if(filas_faltantes>0){              //permite conocer cuantos columnas le quedan por procesar al hilo
			columnas++;
		}
		MPI_Send(&offset, 1, MPI_INT, destino, 1, MPI_COMM_WORLD);      // indica el numero de fila que se procesara
		MPI_Send(&columnas, 1, MPI_INT, destino, 1, MPI_COMM_WORLD);    //envia la direccion de memoria del valor de columnas para saber cual se procesara
		MPI_Send(&matriz_a[offset][0], columnas*N, MPI_INT, destino, 1, MPI_COMM_WORLD);  //envia la direccion de memoria de la fila de la matriz A que se procesara (multiplicacion)
		MPI_Send(&matriz_b, N*N, MPI_INT, destino, 1, MPI_COMM_WORLD);   //envia la direccion de memoria de la matriz B para realizar la multiplicacion
		offset = offset + columnas;                                      //calcula la nueva fila que se debera enviar
		filas_faltantes--;
		if(filas_faltantes==0){											//permite conocer cuantos columnas le quedan por procesar al hilo
			columnas--;
		}

	}


	if(N%num_tareas==1){									// para el caso en el que el numero de tareas es impar
		filas_faltantes = N - columnas*num_tareas;          // se calcula la cantidad de filas por procesar         
	}

	for(i=1;i<=num_tareas;i++){						// se procede a la recepcion de los calculos (los coeficientes obtenidos de la multiplicacion de AxB) para_  
		if(filas_faltantes>0){						// posteriormente obtener el coeficiente de valor maximo de la matriz resultante (C)
			columnas++;
		}
		fuente=i;
		MPI_Recv(&offset, 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);     //se recibe el valor de fila que calculó
		MPI_Recv(&columnas, 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);   // se recibe el valor de las columnas que se calcularon
		MPI_Recv(&matriz_c[offset][0], columnas*N, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);   //se obtienen los coeficientes de la multiplicacion que se calcularon
		MPI_Recv(&maximos[offset], 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);              //se recibe el array de maximos para obtener el mayor valor entre ellos
		if(filas_faltantes==0){
			columnas--;
		}
	}

	printf("Coeficiente mayor = ");
	max=maximos[0];                    
	for (i=0;i<=offset;++i){			//se busca en el array de maximos el mayor valor obtenido y se muestra por pantalla.
		if(maximos[i]>max)
			max=maximos[i];
	}	
	printf("%d\n",max);
	}


	if(id_proceso>0){
		fuente=0;															//Se recepciona los valores enviados correspondientes a:
		MPI_Recv(&offset, 1, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);  			// Numero de fila a procesar en matriz A
		MPI_Recv(&columnas, 1, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);  			// El numero de columnas a procesar 
		MPI_Recv(&matriz_a, columnas*N, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);	//la direccion de las columnas a procesar en matriz A
		MPI_Recv(&matriz_b, N*N, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);			//la direccion de la matriz B		
		for(k=0; k<N; k++){                                                //se realiza la multiplicacion de las filas y columnas que corresponda al hilo..
			for(i=0;i<columnas; i++){
				matriz_c[i][k] = 0;
				for(j=0;j<N;j++){
					matriz_c[i][k] = matriz_c[i][k] + matriz_a[i][j]*matriz_b[j][k];    //guardamos el resultado parcial en matriz_c
				}
			}
		}

		max=matriz_c[0][0];
		
		for(k=0;k<N;k++){					//Obtenemos el maximo valor de las filas calculadas de la matriz resultante...
			for(i=0;i<columnas;i++){
				if(matriz_c[i][k]>max){
					max=matriz_c[i][k];     //se guarda en la variable max para posteriormente ser enviado al proceso cero y ser insertado en el array de maximos
				}
				
			}
		}
		
		//se envia los datos (al proceso cero) correspondientes a los calculos realizados por el hilo..
		MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);    //Se envia el numero de fila que se proceso
		MPI_Send(&columnas, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);	//se envia el numero de columnas que se proceso
		MPI_Send(&matriz_c, columnas*N, MPI_INT, 0, 2, MPI_COMM_WORLD);    //se envia la matriz C (que contiene los coeficientes calculados por el hilo)
		MPI_Send(&max, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);        //se envia el valor de max para ser insertado en el array de maximos del proceso cero
	}

	MPI_Finalize();

}

// mpicc mult.c -o mult (COMPILAR)
//mpirun -np N ./mult < test.txt || (EJECUTAR - N= Numero Procesos)