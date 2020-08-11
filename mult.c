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

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id_proceso);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

	if(id_proceso==0){

		// ****************************Lectura de los datos de las matrices A y B****************************************


		while(fgets(c, 100, stdin) != NULL){ //leemos una linea completa a la vez hasta el final del texto para saber las dimensiones de las matrices
			col=0;    // numero de columnas
			k=0;
			if (columnasTotal+1==filasTotal){ //identifica que hemos llegado a la mitad del archivo y envia el aviso
				fil=0;			//reseteamos el valor de fila para llenar la matriz b
				llenarMatrizb=1;  //aviso
			}
			while(c[k]!='\0'){        // leeamos la fila de texto que tenemos en la variable c
				l=0;
				char aux[10] = "";
				while(c[k]!=' ' && c[k]!='\0'){      //hasta el espacio en blanco para diferenciar cada numero
					aux[l]=c[k];       	// guardamos cada digito en una variable auxiliar
					l++;
					k++;
				}
				if (llenarMatrizb==1){    // recibe aviso que llegamos a la mitad del archivo por lo que los siguientes valores corresponden a la matriz b
					matriz_b[fil][col] =atoi(aux); 	// ponemos el numero obtenido en la matriz b
					
				}else
				{
					matriz_a[fil][col] =atoi(aux); // ponemos el numero obtenido en la matriz a
					//printf("%d\n",atoi(aux));
				}
				
				if(c[k]=='\0'){  			//si encuentra el final de la linea
					columnasTotal=col; 		// obtenemos la cantidad de columnas total de la matriz
					fil++;
					break;
				}
				col++;
				k++;
			}
			filasTotal++;
		}

	num_tareas = num_procesos-1;	//	2
	offset=0;

	columnas = N/num_tareas;	//	1

	if(N%num_tareas==1){
		filas_faltantes = N - columnas*num_tareas;
	}

	filas_calculadas=0;

	//printf("Las filas que se calculan son:  %d\n",num_tareas*columnas);


	for (destino=1; destino<=num_tareas;destino++){
		if(filas_faltantes>0){
			columnas++;
		}
		MPI_Send(&offset, 1, MPI_INT, destino, 1, MPI_COMM_WORLD);
		MPI_Send(&columnas, 1, MPI_INT, destino, 1, MPI_COMM_WORLD);
		MPI_Send(&matriz_a[offset][0], columnas*N, MPI_INT, destino, 1, MPI_COMM_WORLD);
		MPI_Send(&matriz_b, N*N, MPI_INT, destino, 1, MPI_COMM_WORLD);
		offset = offset + columnas;
		filas_faltantes--;
		if(filas_faltantes==0){
			columnas--;
		}

	}


	if(N%num_tareas==1){
		filas_faltantes = N - columnas*num_tareas;
	}

	for(i=1;i<=num_tareas;i++){
		if(filas_faltantes>0){
			columnas++;
		}
		fuente=i;
		MPI_Recv(&offset, 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
		MPI_Recv(&columnas, 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
		MPI_Recv(&matriz_c[offset][0], columnas*N, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
		MPI_Recv(&maximos[offset], 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
		if(filas_faltantes==0){
			columnas--;
		}
	}

	//printf("Offset vale: %d\n",offset);
	printf("Coeficiente mayor = ");
	max=maximos[0];
	for (i=0;i<=offset;++i){
		if(maximos[i]>max)
			max=maximos[i];
		//printf("[%d]\n", maximos[i]);
	}
	//printf("\n");*/
	printf("%d\n",max);

	/*for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			printf("[%d]", matriz_c[i][j]);
		}
		printf("\n");
	}
	printf("\n");*/


	
	}




	if(id_proceso>0){
		fuente=0;
		MPI_Recv(&offset, 1, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&columnas, 1, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&matriz_a, columnas*N, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&matriz_b, N*N, MPI_INT, fuente, 1, MPI_COMM_WORLD, &status);
		for(k=0; k<N; k++){
			for(i=0;i<columnas; i++){
				matriz_c[i][k] = 0;
				for(j=0;j<N;j++){
					matriz_c[i][k] = matriz_c[i][k] + matriz_a[i][j]*matriz_b[j][k];
				}
			}
		}

		max=matriz_c[0][0];
		//printf("El maximo inicial del proceso %d es: %d\n", id_proceso, max);
		for(k=0;k<N;k++){
			for(i=0;i<columnas;i++){
				if(matriz_c[i][k]>max){
					max=matriz_c[i][k];
				}
				//printf("[%d]", matriz_c[i][k]);
			}
		}
		//printf("\n");
		//printf("Soy el proceso %d y realicé un calculo\n", id_proceso);

		//printf("El maximo de la fila del proceso %d es %d\n",id_proceso,max);
		//printf("offset es %d\n", offset);

		MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&columnas, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&matriz_c, columnas*N, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&max, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
	}

	MPI_Finalize();


}

// mpicc mult.c -o mult (COMPILAR)
//mpirun -np N ./mult < test.txt || (EJECUTAR - N= Numero Procesos)