#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <assert.h>
#include "errno.h"
#include "string.h"
#include <unistd.h> 

//#define NDEBUG

#define ISIZE 1000
#define JSIZE 1000
#define IBEGIN 3
#define JEND 2
// 0 - master; 1,2,3 - external cycle processors (externals); 5, ... - internal cycle processors (internals)
#define EXTERNAL_PROCS 4
#define FIRST_INTERNAL 4
#define DEF_TAG 0xd1
#define USE_TAG 0xf1
#define BLOCK_SIZE (sizeof(double) * JSIZE)

#define INTERNAL_WINDOW (60)
#define DELAY 0

int tmp0 = 0;

double function(const double x) {
    for (int i = 0; i < DELAY; ++i) {
    	tmp0++;
    } 
    return sin(0.00001 * x);
}

// однопоточный режим вычислений
void sequential(double **a) {
    // D = (3, -2); d = (<, >) = (истинная по внеш., анти по внутр.)
    // можем распараллелить внутренний с копированием и барьерной синхронизацией
    // внешний - на 3 потока (расстояние = 3)
    assert(a != NULL);
    int i, j;
    for (i = IBEGIN; i < ISIZE; i++) {
        for (j = 0; j < JSIZE - 2; j++) {
            a[i][j] = function(a[i - IBEGIN][j + 2]);
        }
    }
}

void sequential_internal(double* def_row, double* use_row) {
    for (int j = 0; j < JSIZE - 2; j++) {
        def_row[j] = function(use_row[j + 2]);
    }
}

void parallel_internal(double* row_def, double* row_use, const int row_idx, int num_procs) {
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // printf("parallel internal %d\n", rank);
    MPI_Status status;
    // procs for processing internal cycle (all without first 4 procs)
    const int num_internals = num_procs - EXTERNAL_PROCS;
    assert(num_internals > 0);

    //dprintf(STDERR_FILENO, "internals: %d\n", num_internals);
    
    // relative row idx in block M size of 3 - starts from 0
    // external cycle paralls on 3 procs --> on every iteration 3 rows can be internaly parallelized independantly
    // next 3-block can not be executed until the previous one is under execution --> it is useful to parall internally 3-block only
    int r = (row_idx - IBEGIN) % (INTERNAL_WINDOW);
    
    // num of partitions in row r
    // firstly, every row from M divides into 2 blocks (num of divided blocks is defined by num_internals)
    // if num_internals > 3 then continue divide rows from start into 3 blocks and so on
    // on the next iteration operations 
    const int partitions = (INTERNAL_WINDOW - r + num_internals - 1) / (INTERNAL_WINDOW);
    //dprintf(STDERR_FILENO, "row %d (r=%d) [rank=%d]: %d\n", row_idx, r, rank, partitions);
    // printf("partitions in rank %d: %d\n", rank, partitions);

    // if r in not divided
    if (partitions == 0) {
        sequential_internal(row_def, row_use);
        return;
    }

    const int block_size = (JSIZE - JEND) / (partitions + 1);

    const size_t block_size_bytes = block_size * sizeof(double);
    // double* buffer = (double*)calloc(block_size + JEND, sizeof(double));

    size_t offset = 0;
    int internal_id = FIRST_INTERNAL + r;
    
    // send all blocks except last to internal procs
    while (offset < block_size * partitions) {
        //dprintf(STDERR_FILENO, "(%d)iid|%d\n", row_idx, internal_id);
        assert(internal_id < num_procs);
        // printf("[%d] send(%d)]\n", rank, internal_id);
        // memcpy(buffer, row_def + offset, block_size_bytes);
        double* tmp = row_def + offset;
        int result = MPI_Send(tmp, block_size, MPI_DOUBLE, internal_id, DEF_TAG, MPI_COMM_WORLD);
        assert(result == MPI_SUCCESS);

        // memcpy(buffer, row_use + offset, block_size_bytes + JEND * sizeof(double));
        tmp = row_use + offset;
        result = MPI_Send(tmp, block_size + JEND, MPI_DOUBLE, internal_id, USE_TAG, MPI_COMM_WORLD);
        assert(result == MPI_SUCCESS);
        
        offset += block_size;
        // increment proc id on num of rows
        internal_id += INTERNAL_WINDOW;
    }
    //dprintf(STDERR_FILENO, "\n");

    // calculate the last block, can be a bit bigger than block_size
    // sdprintf(STDERR_FILENO, "offset:%lu\n", offset);
    for (size_t j = offset; j + 2 < ISIZE; ++j) {
        row_def[j] = function(row_use[j + 2]);
    }

    offset = 0;
    internal_id = FIRST_INTERNAL + r;
    // printf("max_offset: %d", block_size * partitions);
    // recieve calculated blocks in the same order
    // dprintf(STDERR_FILENO, "proc %d recieves\n", rank);
    while (offset < block_size * partitions) {
        // dprintf(STDERR_FILENO, "%d Wait recv from %d\n", rank, internal_id);
        MPI_Recv(row_def + offset, block_size, MPI_DOUBLE, internal_id, DEF_TAG, MPI_COMM_WORLD, &status);
        // dprintf(STDERR_FILENO, "%d Recieved from %d\n", rank, internal_id);
        //printf("[%d] recv(%d)]\n", rank, internal_id);
        assert(status._ucount == block_size_bytes);
        assert(status.MPI_ERROR == MPI_SUCCESS);
        // printf("R[%d]: %d\n", rank, p);
        
        // memcpy(row_def + offset, buffer, block_size_bytes);
        
        offset += block_size;
        // increment proc id on num of rows
        internal_id += INTERNAL_WINDOW;
    }
}

void print(double *row, int rank) {
    for (int j = 0; j < JSIZE; ++j) {
        printf("%f ", row[j]);

    }
    printf("\n proc[%d]\n\n", rank);
}

void dual(double** a, int rank) {
    MPI_Status status;
    
    // double* row_def = (double*)calloc(ISIZE, sizeof(double));
    // double* row_use = (double*)calloc(ISIZE, sizeof(double));
    double row_def[ISIZE];
    double row_use[ISIZE];
    
    // assert(row_def != NULL && row_use != NULL);
    
    for (int i = IBEGIN + rank*2; i < ISIZE; i += 3) {
        
        if (rank == 0) {
            assert(a != NULL);
            const int p = i + 2; // idx for parall proc
            
            if (p < ISIZE) {
                // send a[p] as def before calculation
                memcpy(row_def, a[p], BLOCK_SIZE);
                int result = MPI_Send(row_def, JSIZE, MPI_DOUBLE, 1, p, MPI_COMM_WORLD);
                assert(result == MPI_SUCCESS);
                // printf("S[%d]: %d\n", rank, p - IBEGIN);

                // send a[p-3] to calculate a[p]
                memcpy(row_use, a[p - IBEGIN], BLOCK_SIZE);
                result = MPI_Send(row_use, JSIZE, MPI_DOUBLE, 1, p - IBEGIN, MPI_COMM_WORLD);
                assert(result == MPI_SUCCESS);
                // printf("S[%d]: %d\n", rank, p - IBEGIN);
            }

            // 2 iters
            for (int n = i; n < p && n < ISIZE; ++n) {
                sequential_internal(a[n], a[n - IBEGIN]);
            }
            
            if (p >= ISIZE)
                return;
            
            // recv of calculted a[p]
            // printf("Wait R[%d]: %d\n", rank, p);
            MPI_Recv(row_def, JSIZE, MPI_DOUBLE, 1, p, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("R[%d]: %d\n", rank, p);
            
            memcpy(a[p], row_def, BLOCK_SIZE);

        } else {

            if (i >= ISIZE)
                return;
            
            // recv a[p] before calculation
            // printf("\tWait R[%d]: %d\n", rank, p);
            MPI_Recv(row_def, JSIZE, MPI_DOUBLE, 0, i, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("\tR[%d]: %d\n", rank, p);

            // recv a[p-3] before calculation
            // printf("\tWait R[%d]: %d\n", rank, p - 3);
            MPI_Recv(row_use, JSIZE, MPI_DOUBLE, 0, i - IBEGIN, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("\tR[%d]: %d\n", rank, p - 3);
            
            sequential_internal(row_def, row_use);
            
            int result = MPI_Send(row_def, JSIZE, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
            assert(result == MPI_SUCCESS);
            // printf("\tS[%d]: %d\n", rank, p);
        }
    }
    
    // free(row_use);
    // free(row_def);
}

void triple(double** a, int rank) {
    MPI_Status status;
    
    // double* row_def = (double*)calloc(ISIZE, sizeof(double));
    // double* row_use = (double*)calloc(ISIZE, sizeof(double));
    double row_def[ISIZE];
    double row_use[ISIZE];
    
    // assert(row_def != NULL && row_use != NULL);
    
    for (int i = IBEGIN + rank; i < ISIZE; i += 3) {
        
        if (rank == 0) {
            assert(a != NULL);

            const int p1 = i + 1; // idx for parall proc 1
            const int p2 = i + 2; // idx for parall proc 2
            
            if (p1 < ISIZE) {
                // send a[p] as def before calculation
                // to 1st proc
                memcpy(row_def, a[p1], BLOCK_SIZE);
                int result = MPI_Send(row_def, JSIZE, MPI_DOUBLE, 1, p1, MPI_COMM_WORLD);
                assert(result == MPI_SUCCESS);
                // to 2nd proc
                if (p2 < ISIZE) {
                    memcpy(row_def, a[p2], BLOCK_SIZE);
                    result = MPI_Send(row_def, JSIZE, MPI_DOUBLE, 2, p2, MPI_COMM_WORLD);
                    assert(result == MPI_SUCCESS);
                    // printf("S[%d]: %d\n", rank, p - IBEGIN);
                }

                // send a[p-3] to calculate a[p]
                // to 1st proc
                memcpy(row_use, a[p1 - IBEGIN], BLOCK_SIZE);
                result = MPI_Send(row_use, JSIZE, MPI_DOUBLE, 1, p1 - IBEGIN, MPI_COMM_WORLD);
                assert(result == MPI_SUCCESS);
                // to 2nd proc
                if (p2 < ISIZE) {
                    memcpy(row_use, a[p2 - IBEGIN], BLOCK_SIZE);
                    result = MPI_Send(row_use, JSIZE, MPI_DOUBLE, 2, p2 - IBEGIN, MPI_COMM_WORLD);
                    assert(result == MPI_SUCCESS);
                }
                // printf("S[%d]: %d\n", rank, p - IBEGIN);
            }

            // 1 iter
            sequential_internal(a[i], a[i - IBEGIN]);
            
            if (p1 >= ISIZE)
                return;
            
            // recv of calculted a[p] from 1 proc
            // printf("Wait R[%d]: %d\n", rank, p);
            MPI_Recv(row_def, JSIZE, MPI_DOUBLE, 1, p1, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("R[%d]: %d\n", rank, p);
            memcpy(a[p1], row_def, BLOCK_SIZE);

            if (p2 >= ISIZE)
                return;
            
            // recv of calculted a[p] from 2 proc
            MPI_Recv(row_def, JSIZE, MPI_DOUBLE, 2, p2, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("R[%d]: %d\n", rank, p);
            memcpy(a[p2], row_def, BLOCK_SIZE);

        } else {

            if (i >= ISIZE)
                return;
            
            // recv a[p] before calculation
            // printf("\tWait R[%d]: %d\n", rank, p);
            MPI_Recv(row_def, JSIZE, MPI_DOUBLE, 0, i, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("\tR[%d]: %d\n", rank, p);

            // recv a[p-3] before calculation
            // printf("\tWait R[%d]: %d\n", rank, p - 3);
            MPI_Recv(row_use, JSIZE, MPI_DOUBLE, 0, i - IBEGIN, MPI_COMM_WORLD, &status);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("\tR[%d]: %d\n", rank, p - 3);
            
            sequential_internal(row_def, row_use);
            
            int result = MPI_Send(row_def, JSIZE, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
            assert(result == MPI_SUCCESS);
            // printf("\tS[%d]: %d\n", rank, p);
        }
    }
    
    // free(row_use);
    // free(row_def);
}

void multi(double** a, int num_procs, int rank) {
    MPI_Status status;
    // printf("rank: %d\n", rank);
    
    double* row_def = NULL;
    double* row_use = NULL;
    // double row_def[ISIZE];
    // double row_use[ISIZE];
    if (rank != 0) {
        row_def = (double*)calloc(ISIZE, sizeof(double));
        row_use = (double*)calloc(ISIZE, sizeof(double));
        assert(row_def != NULL && row_use != NULL);
    }
    
    for (int i = IBEGIN; i < ISIZE; i += 3) {
        
        if (rank == 0) {
            assert(a != NULL);

            for (int k = i; k < i + IBEGIN && k < ISIZE; ++k) {
                // memcpy(row_def, a[k], BLOCK_SIZE);
                int result = MPI_Send(a[k], JSIZE, MPI_DOUBLE, k - i + 1, k, MPI_COMM_WORLD);
                // printf("S[%d to %d]: %d\n", rank, k - i + 1, k);
                assert(result == MPI_SUCCESS);

                //memcpy(row_use, a[k - IBEGIN], BLOCK_SIZE);
                result = MPI_Send(a[k - IBEGIN], JSIZE, MPI_DOUBLE, k - i + 1, k - IBEGIN, MPI_COMM_WORLD);
                // printf("S[%d to %d]: %d\n", rank, k - i + 1, k - IBEGIN);
                assert(result == MPI_SUCCESS);
            }

            for (int k = i; k < i + IBEGIN && k < ISIZE; ++k) {
                MPI_Recv(a[k], JSIZE, MPI_DOUBLE, k - i + 1, k, MPI_COMM_WORLD, &status);
                assert(status._ucount == BLOCK_SIZE);
                assert(status.MPI_ERROR == MPI_SUCCESS);
                // printf("R[%d from %d]: %d\n", rank, k - i + 1, k);
                // memcpy(a[k], row_def, BLOCK_SIZE);
            }

        } else if (rank <=3 ) {
            // row idx
            // proc 1 sends [3] row (start idx), proc 2 - [4] row, proc 3 - [5] row, ... 
            int k = i + (rank - 1);
            if (k >= ISIZE)
                return;
            //if (k + 3 >= ISIZE)
            //printf("[%d] last iteration\n", rank);
            // recv a[p] before calculation
            // printf("\tWait R[%d]: %d\n", rank, p);
            MPI_Recv(row_def, JSIZE, MPI_DOUBLE, 0, k, MPI_COMM_WORLD, &status);
            //printf("\tR[%d from %d]: %d\n", rank, 0, k);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("\tR[%d]: %d\n", rank, p);

            // recv a[p-3] before calculation
            // printf("\tWait R[%d]: %d\n", rank, p - 3);
            MPI_Recv(row_use, JSIZE, MPI_DOUBLE, 0, k - IBEGIN, MPI_COMM_WORLD, &status);
            //printf("\tR[%d from %d]: %d\n", rank, 0, k - IBEGIN);
            assert(status._ucount == BLOCK_SIZE);
            assert(status.MPI_ERROR == MPI_SUCCESS);
            // printf("\tR[%d]: %d\n", rank, p - 3);
            
            if (num_procs == 4) {
                sequential_internal(row_def, row_use);
            } else {
                parallel_internal(row_def, row_use, k, num_procs);
            }
            
            int result = MPI_Send(row_def, JSIZE, MPI_DOUBLE, 0, k, MPI_COMM_WORLD);
            //printf("\tS[%d to %d]: %d\n", rank, 0, k);
            assert(result == MPI_SUCCESS);
            // printf("\tS[%d]: %d\n", rank, p);
        } else {
            // procs for processing internal cycle (all without first 4 procs)
            const int num_internals = num_procs - EXTERNAL_PROCS;

            // calculate relative row idx from slave proc rank and its num of partitions
            const int r = (rank - EXTERNAL_PROCS) % INTERNAL_WINDOW; 
            const int partitions = (INTERNAL_WINDOW - r + num_internals - 1) / (INTERNAL_WINDOW);
            const int src_rank = (rank - EXTERNAL_PROCS) % IBEGIN + 1;

            // do nothing if block has not been separated
            if (partitions == 0) {
                return;
            }

            const int block_size = (JSIZE - JEND) / (partitions + 1);
            const size_t block_size_bytes = block_size * sizeof(double);
            
            size_t last_processed_row = r;

            while (last_processed_row < ISIZE - IBEGIN) {
                //printf("\tWait R[%d]\n", rank);
                MPI_Recv(row_def, block_size, MPI_DOUBLE, src_rank, DEF_TAG, MPI_COMM_WORLD, &status);
                // printf("\tRecieved R[%d]\n", rank);
                //printf("\tR[%d from %d]: %d\n", rank, 0, k - IBEGIN);
                assert(status._ucount == block_size_bytes);
                assert(status.MPI_ERROR == MPI_SUCCESS);

                // printf("\tWait R[%d]\n", rank);
                MPI_Recv(row_use, block_size + JEND, MPI_DOUBLE, src_rank, USE_TAG, MPI_COMM_WORLD, &status);
                // printf("[%d] recv(%d)]\n", rank, src_rank);
                //printf("\tRecieved R[%d] from %d\n", rank, status.MPI_SOURCE);
                //printf("\tR[%d from %d]: %d\n", rank, 0, k - IBEGIN);
                assert(status._ucount == block_size_bytes + JEND * sizeof(double));
                assert(status.MPI_ERROR == MPI_SUCCESS);

                
                // calculate blcok and send to the src proc
                for (size_t j = 0; j < block_size; ++j) {
                    row_def[j] = function(row_use[j + 2]);
                }

                // printf("[%d] send(%d)]\n", rank, status.MPI_SOURCE);
                int result = MPI_Send(row_def, block_size, MPI_DOUBLE, status.MPI_SOURCE, DEF_TAG, MPI_COMM_WORLD);
                //printf("\tS[%d to %d]: %d\n", rank, 0, k);
                assert(result == MPI_SUCCESS);
                //printf("processed row: %d\n", last_processed_row);
                last_processed_row += INTERNAL_WINDOW;
            }

            // every block maps to a single proc, therefore it should be terminated after processing of one block
            //printf("here\n");
            return;
        }
    }
    
    if (rank != 0) {
        free(row_use);
        free(row_def);
    }
}

void main_main(double **a, const int num_procs, const int rank) {
    // получить число потоков  в глобальной группе, т.е. всего
    
    if (num_procs == 1) {
        assert(a != NULL);
        sequential(a);
    } else if (num_procs == 2) {
        // int rank;
        // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        dual(a, rank);
    } else if (num_procs == 3) {
        // int rank;
        // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        triple(a, rank);
    } else {
        // int rank;
        // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        multi(a, num_procs, rank);
    }
}

int main(int argc, char **argv) {
    // инициализация MPI
    MPI_Init(&argc, &argv);
    // int i, j;

    int num_procs = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    // printf ("num procs: %d\n", num_procs);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //int res = 0;
    //for (int l = 0; l < rank * 100; ++l) {
    //	res++;
    //}

    double** a = NULL;
    if (rank == 0) {
        assert(num_procs < IBEGIN * (JSIZE - JEND - 1) + EXTERNAL_PROCS);
        a = malloc(ISIZE * sizeof(double*));
        assert(a != NULL);
        for (int i = 0; i < ISIZE; i++) {
            a[i] = malloc(JSIZE * sizeof(double));
            assert(a[i] != NULL);
        }

        // инициализация массива
        for (int i = 0; i < ISIZE; i++) {
            for (int j = 0; j < JSIZE; j++) {
                a[i][j] = 10 * i + j;
            }
        }
    }

    // основные вычисления
    double start = MPI_Wtime();
    main_main(a, num_procs, rank);
    double duration = MPI_Wtime() - start;

    // печать результата
    //printf("%d\n", res);

    if (rank == 0) {
        printf("total time: %lf\n", duration);
        printf ("num procs: %d\n", num_procs);

        char str[] = "result00.txt";
        str[7] = '0' + (num_procs % 10);
        str[6] = '0' + (num_procs / 10);

        FILE *ff = fopen(str, "w");
        if (ff == NULL) {
            perror("output file");
            exit(errno);
        }
        for (int i = 0; i < ISIZE; i++) {
            for (int j = 0; j < JSIZE; j++) {
                fprintf(ff, "%f\t", a[i][j]);
            }
            fprintf(ff, "\n");
        }
        fclose(ff);

        /*
        for (i = 0; i < ISIZE; i++) {
                free(a[i]);
        }
        free(a);
        */
    }

    // printf("Finalize\n");
    MPI_Finalize();
}
