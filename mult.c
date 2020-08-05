#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#define N 3

MPI_Status status;

int main(int argc, char *argv[]){
	int matriz_a[N][N] = {{4,2,4},{3,1,5},{2,5,1}};
	int matriz_b[N][N] = {{2,1,3},{5,1,2},{5,5,1}};
	int matriz_c[N][N];
	int num_procesos, id_proceso, num_tareas, fuente, destino, columnas, offset, i, j, k;
	struct timeval start, stop;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id_proceso);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procesos);

	if(id_proceso==0){

	num_tareas = num_procesos-1;
	offset=0;

	columnas = N/num_tareas;

	for (destino=1; destino<=num_tareas;destino++){
		MPI_Send(&offset, 1, MPI_INT, destino, 1, MPI_COMM_WORLD);
		MPI_Send(&columnas, 1, MPI_INT, destino, 1, MPI_COMM_WORLD);
		MPI_Send(&matriz_a[offset][0], columnas*N, MPI_INT, destino, 1, MPI_COMM_WORLD);
		MPI_Send(&matriz_b, N*N, MPI_INT, destino, 1, MPI_COMM_WORLD);
		offset = offset + columnas;
	}

	for(i=1;i<=num_tareas;i++){
		fuente=i;
		MPI_Recv(&offset, 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
		MPI_Recv(&columnas, 1, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
		MPI_Recv(&matriz_c[offset][0], columnas*N, MPI_INT, fuente, 2, MPI_COMM_WORLD, &status);
	}


	for(i=0;i<N;i++){
		for(j=0;j<N;j++){
			printf("[%d]",matriz_c[i][j]);
		}
		printf("\n");
	}
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
		MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&columnas, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&matriz_c, columnas*N, MPI_INT, 0, 2, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}

// mpicc mult.c -o mult (COMPILAR)
//mpirun -np N ./mult (EJECUTAR - N= Numero Procesos)