#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define  dp 50.0f

void randX(int size, float _dp, int st, float *arr)
{
    int i;
    double x, max, min;
    srand(st);
    

    

    for(i = 0; i < size; i++)
    {
        x = (float)rand()/RAND_MAX*_dp*2.0f-_dp;
        arr[i] = x;
    }

}

void init_vector(int size, float *arr1)
{
    if(size != sizeof(arr1)/sizeof(arr1[0]))
    {
        for(int i = 0; i < size; i++)
            arr1[i] = (float)rand()/RAND_MAX*2.0f - dp;
    }
}

void print_vector(int size, float* arr1, char* zag)
{
    printf("\n%s\n",zag);

    for(int i = 0; i < size; i++)
    {
        printf (" %8.2f ",arr1[i]);
        
    }
    printf("\n");
}

void print_vector_rank(int size, float* arr1, char* zag, int rnk)
{
    printf("\n%s %d\n",zag,rnk);
    if(size == 1)
        printf ("%8.2f",arr1[0]);
    else{
    for(int i = 0; i < size; i++)
    {
       
        printf ("%8.2f",arr1[i]);

    }

    printf("\n");
    }
}


void scalarVector(int size_vec, float *vec_1, float*vec_2, float* gresult)
{
    float result = 0;
    for(int i = 0; i < size_vec; i++) // 1
    {
        gresult[0] = vec_1[i] * vec_2[i];
        result += gresult[0];
        
    }

    gresult[0] = result;
        
    
}

int main(int argc, char** argv)
{
    int size, rank;
    int nach, count, i, scol, ost;
    int size_vec;
    int genarg;
    int *adispls;
    int *acounts;
    int *vdispls;
    int *vcounts;

    float *sresult, *vresult;

    size_vec = atoi(argv[1]);

    double time1, time2;
    float* vec_1;
    float* vec_2;
    float *aresult;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Proccess %d of %d is started\n",rank,size);
    
    count = size_vec / size; 
    ost = size_vec % size;

    if (rank == 0)
    {
        
        sresult = (float *)calloc(size_vec, sizeof(float));
        vresult = (float *)calloc(1, sizeof(float));
        aresult = (float *)calloc(size_vec, sizeof(float));
        vec_1 = (float *)calloc(size_vec, sizeof(float));
        vec_2 = (float *)calloc(size_vec, sizeof(float));

        sresult[0] = 0;
        vresult[0] = 0;
        randX(size_vec, 1.0, 10, vec_1);
        print_vector(size_vec, vec_1, "Input Vector vec_1");

        randX(size_vec, 50.0, 10, vec_2);
        print_vector(size_vec, vec_2, "Input Vector vec_2");
        time1=MPI_Wtime();  //Time begining of parallel program

        scalarVector(size_vec, vec_1, vec_2, sresult);
        print_vector_rank(1, sresult, "Sequential scalar vector, rank", rank);

        time1 = MPI_Wtime();
        adispls = (int *)malloc(size * sizeof(int));
        acounts = (int *)malloc(size * sizeof(int));
        vdispls = (int *)malloc(size * sizeof(int));
        vcounts = (int *)malloc(size * sizeof(int));

        int* displs, * rcounts, * displs1, * rcounts1;
        int scol, nach;
        displs = (int*)malloc(size * sizeof(int));
        rcounts = (int*)malloc(size * sizeof(int));
        displs1 = (int*)malloc(size * sizeof(int));
        rcounts1 = (int*)malloc(size * sizeof(int));

	/* Creation auxiliary arrays for data communication */

        for(i=0;i < size;i++)
        { 
            scol = i < ost ? count+1 : count;  
                        acounts[i] = count; vcounts[i] = scol;
            nach = i*scol + (i >= ost ? ost : 0); 
                        adispls[i] = nach*ost;  vdispls[i] = nach;
        } 
            /*	output of auxilary arrays for test  */

            printf("\n\n");
        for (i=0; i<size; i++) 
            printf("%10d", acounts[i]); printf("  acounts\n");
        for (i=0; i<size; i++) 
            printf("%10d", adispls[i]); printf("  adispls\n");
        for (i=0; i<size; i++) 
            printf("%10d", vcounts[i]); printf("  vcounts\n");
        for (i=0; i<size; i++) 
            printf("%10d", vdispls[i]); printf("  vdispls\n");
     /* End of work process 0  */
    
    }

    scol = rank < ost ? count+1 : count;
    nach = rank*scol + (rank >= ost ? ost : 0); 
    printf("\nscol = %d, nach = %d", scol, nach);
    float *vresult_test = (float *)calloc(scol, sizeof(float));

    float *test;
    
    test = (float *)calloc(scol, sizeof(float));

    float *test_2;
    test_2 = (float *)calloc(scol, sizeof(float));
    
    MPI_Scatterv(vec_2, vcounts, vdispls, MPI_FLOAT, test_2, scol, MPI_FLOAT, 0, MPI_COMM_WORLD);

    MPI_Scatterv(vec_1, vcounts, vdispls, MPI_FLOAT, test, scol, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    printf("\nProcces %d\n", rank);
    print_vector_rank(scol, test, "Part vector vec_1", rank);
    print_vector_rank(scol, test_2, "\nPart vector vec_2", rank);
    
    
    scalarVector(scol, test, test_2, vresult_test);
    print_vector_rank(scol, vresult_test, "Scalar of vectors", rank);

    MPI_Gatherv(vresult_test, scol, MPI_FLOAT, aresult, vcounts, vdispls, MPI_FLOAT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Results of all. \n\n");
        for(int i = 0; i < size_vec; i++)
        {
            // aresult[0] += aresult[i + 1];
            printf(" %f ", aresult[i]);
        }
        float res = 0;
        for(int i = 0; i < size_vec; i++)
        {
            res += aresult[i];
        }
        printf("Result SCALAR of VECTORS = %f", res);

        time2 = MPI_Wtime();

        printf("\ntime = %f\n", time2-time1);

        free(vec_1); free(sresult); free(vresult); free(test); free(test_2);
        free(vec_2);
        free(adispls); free(acounts); free(vdispls); free(vcounts);
    }
    

    
    





    MPI_Finalize();
    return 0;
    
}