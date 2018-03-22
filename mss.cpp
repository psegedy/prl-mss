// Merge-splitting sort algorithm
// Author: Patrik Segedy <xseged00@vutbr.cz>

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[])
{
    int numprocs;               // number of processors
    int rank;                   // rank
    int numbers = atoi(argv[1]); // number of numbers on input
    MPI_Status stat;            // struct- obsahuje kod- source, tag, error

    //MPI INIT
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numbers % numprocs) {
        cerr << "Number of numbers is not divisible by number of CPUs" << endl;
        return 1;
    }

    int *arr = NULL;
    // int myarr[numbers/numprocs];
    int *my_arr = (int*) malloc((numbers/numprocs) * sizeof(int));
    int *neigh_arr = (int*) malloc((numbers/numprocs) * sizeof(int));
    int *merged_arr = (int*) malloc(2*(numbers/numprocs) * sizeof(int));

    // processor with rank 0 read all values
    if(rank == 0) {
        int i;
        arr = (int*) malloc(numbers * sizeof(int) );
        fstream f;
        f.open("numbers", ios::in);

        // read file
        while(f.good()) {
            arr[i] = f.get();
            if(!f.good()) break;  // break on EOF
            i++;
        }
        f.close();
        for (int i = 0; i < numbers; ++i) {
            cout << arr[i] << " ";
        }
        cout << endl;
    }

    // send data to all processors
    MPI_Scatter(arr,(numbers/numprocs),MPI_INT,my_arr,(numbers/numprocs),MPI_INT,0,MPI_COMM_WORLD);
    
    // index limits
    int odd_lim = 2*(numprocs/2)-1;
    int even_lim = 2*((numprocs-1)/2);
    // Merge-splitting sort algorithm
    // sequential sort for each processor
    sort(my_arr, my_arr + (numbers/numprocs));

    // merging
    for (int i = 0; i < numprocs/2; ++i) {
        // odd
        if (rank%2 == 0 && rank < odd_lim) {
            // send my array to right neighbor
            // wait for arr from right neighbor
            MPI_Send(my_arr, (numbers/numprocs), MPI_INT, rank+1, 0, MPI_COMM_WORLD);
            MPI_Recv(my_arr, (numbers/numprocs), MPI_INT, rank+1, 0, MPI_COMM_WORLD, &stat);
        }
        else if (rank <= odd_lim) {
            // get data from left neighbor
            MPI_Recv(neigh_arr, (numbers/numprocs), MPI_INT, rank-1, 0, MPI_COMM_WORLD, &stat);
            // merge mine and neighbors data
            merge(my_arr, my_arr+(numbers/numprocs), neigh_arr, neigh_arr+(numbers/numprocs), merged_arr);
            memcpy(my_arr, merged_arr, (numbers/numprocs)*sizeof(int));
            memcpy(neigh_arr, merged_arr + (numbers/numprocs), (numbers/numprocs)*sizeof(int));
            // send lower to neighbor and swap mine for neighbor's
            MPI_Send(my_arr, (numbers/numprocs), MPI_INT, rank-1, 0, MPI_COMM_WORLD);
            memcpy(my_arr, neigh_arr, (numbers/numprocs)*sizeof(int));
        }

        // even
        if (rank%2 && rank < even_lim) {
            MPI_Send(my_arr, (numbers/numprocs), MPI_INT, rank+1, 0, MPI_COMM_WORLD);
            MPI_Recv(my_arr, (numbers/numprocs), MPI_INT, rank+1, 0, MPI_COMM_WORLD, &stat);
        }
        else if (rank <= even_lim && rank > 0) {
            // get data from left neighbor
            MPI_Recv(neigh_arr, (numbers/numprocs), MPI_INT, rank-1, 0, MPI_COMM_WORLD, &stat);
            // merge mine and neighbors data
            merge(my_arr, my_arr+(numbers/numprocs), neigh_arr, neigh_arr+(numbers/numprocs), merged_arr);
            memcpy(my_arr, merged_arr, (numbers/numprocs)*sizeof(int));
            memcpy(neigh_arr, merged_arr + (numbers/numprocs), (numbers/numprocs)*sizeof(int));
            // send lower to neighbor and swap mine for neighbor's
            MPI_Send(my_arr, (numbers/numprocs), MPI_INT, rank-1, 0, MPI_COMM_WORLD);
            memcpy(my_arr, neigh_arr, (numbers/numprocs)*sizeof(int));
        }
    }

    // Gather values from all processors to processor 0
    MPI_Gather(my_arr, (numbers/numprocs), MPI_INT, arr, (numbers/numprocs), MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
        for (int i = 0; i < numbers; ++i)
            cout << arr[i] << endl;

    // free memory
    if (rank == 0)
        free(arr);

    free(my_arr);
    free(neigh_arr);
    free(merged_arr);

    MPI_Finalize(); 
    return 0;
}
