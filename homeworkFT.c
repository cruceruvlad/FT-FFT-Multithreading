#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

double PI;
double complex *x_k;
double *x_n;
int N;
int numThreads;

int min (int a, int b) {
    return a <= b ? a : b;
}

void ft_serial(){
	for(int k = 0; k < N; k++)
		for(int i = 0; i < N; i++){
			double complex arg = cexp(-I * 2 * PI * k * i/ N);
			x_k[k] = x_k[k] + ((double complex)x_n[i]) * arg;
		}
	return NULL;
}

void* ft_parallel(void *var){
	int id = *(int*) var;
	int start = id * ceil((double) N / (double) numThreads);
	int end = min(((id + 1) * ceil((double) N / (double) numThreads)), N);

	for(int k = start; k < end; k++)
		for(int i = 0; i < N; i++){
			double complex arg = cexp(-I * 2 * PI * k * i/ N);
			x_k[k] = x_k[k] + ((double complex)x_n[i]) * arg;
		}
	return NULL;
}

int main(int argc, char * argv[]) {
	PI = atan2(1, 1) * 4;
	int i;

	FILE *in = fopen(argv[1], "r");
  	FILE *out = fopen(argv[2], "w");
	numThreads = atoi(argv[3]);

	fscanf(in,"%d",&N);
	x_k = (double complex*) calloc(N, sizeof(double complex));
	x_n = (double*) 	malloc(N * sizeof(double));
	for(i = 0; i < N; i++)
		fscanf(in,"%lf",&x_n[i]);

	if(numThreads == 1)
		ft_serial();
	else{
		pthread_t tid[numThreads];
		int thread_id[numThreads];
		for(i = 0;i < numThreads; i++)
			thread_id[i] = i;
		
		for(i = 0; i < numThreads; i++)
			pthread_create(&(tid[i]), NULL, ft_parallel ,&thread_id[i]);
		for(i = 0; i < numThreads;i++)
			pthread_join(tid[i], NULL);
	}
					

	fprintf(out, "%d\n", N);
	for(int k = 0; k < N; k++)
		fprintf(out,"%lf %lf\n", creal(x_k[k]), cimag(x_k[k]));
	
	fclose(in);
	fclose(out);
	return 0;
}
