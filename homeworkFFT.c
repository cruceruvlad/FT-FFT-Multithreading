#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

double PI;
double complex *out;
double complex *buf;
int N;
int Npad;
int numThreads;
pthread_barrier_t barrier;

void fft_serial(double complex buf[], double complex out[], int n, int step)
{
	if (step < n) {
		fft_serial(out, buf, n, step * 2);
		fft_serial(out + step, buf + step, n, step * 2);

		for (int i = 0; i < n; i += 2 * step) {
			double complex t = cexp(-I * PI * i / n) * out[i + step];
			buf[i / 2]     = out[i] + t;
			buf[(i + n)/2] = out[i] - t;
		}
	}
}

void* fft_parallel(void *var) {
	int start = *(int*)var;

    int i, step, padding, exchange = (((int)log2(Npad)) - 1) % 2;

    for (step = Npad / 2; step >= 1; step >>= 1) {

	if(start  < step){
		if(exchange == 0)
			for(padding = start; padding < step; padding += numThreads)
				for (int i = 0; i < Npad; i += 2 * step) {
					//printf("Solving %d & %d in %d & step %d\n", padding + i / 2, padding + (i + N)/2, padding, step);
					double complex t = cexp(-I * PI * i / Npad) * out[padding + i + step];
					buf[padding + i / 2]     = out[padding + i] + t;
					buf[padding + (i + Npad)/2] = out[padding + i] - t;
				}
		else
			for(padding = start; padding < step; padding += numThreads)
				for (int i = 0; i < Npad; i += 2 * step) {
					//printf("Solving %d & %d in %d & step %d\n", padding + i / 2, padding + (i + N)/2, padding, step);
					double complex t = cexp(-I * PI * i / Npad) * buf[padding + i + step];
					out[padding + i / 2]     = buf[padding + i] + t;
					out[padding + (i + Npad)/2] = buf[padding + i] - t;
				}
		}
	exchange = (exchange + 1) % 2;
	pthread_barrier_wait(&barrier);
	}
}

int main(int argc, char * argv[]) {
	PI = atan2(1, 1) * 4;
	int i;

	FILE *fin   = fopen(argv[1], "r");
  	FILE *fout  = fopen(argv[2], "w");
	numThreads = atoi(argv[3]);

	fscanf(fin,"%d",&N);
	for(Npad = 1; Npad < N; Npad <<= 1);
	out = (double complex*) malloc(Npad * sizeof(double complex));
	buf = (double complex*) malloc(Npad * sizeof(double complex));
	for(i = 0; i < N; i++){
		fscanf(fin,"%lf",&buf[i]);
		out[i] = buf[i];
	}

	for(Npad = 1; Npad < N; Npad <<= 1);

	//if(numThreads == 1)
		//fft_serial(buf,out,Npad,1);
	//else{
		pthread_t tid[numThreads];
		int thread_id[numThreads];
		for(i = 0;i < numThreads; i++)
			thread_id[i] = i;

		pthread_barrier_init(&barrier, NULL, numThreads);
		for(i = 0; i < numThreads; i++)
			pthread_create(&(tid[i]), NULL, fft_parallel ,&thread_id[i]);
		for(i = 0; i < numThreads;i++)
			pthread_join(tid[i], NULL);
	//}

	fprintf(fout, "%d\n", N);
	for(i = 0; i < N; i++)
		fprintf(fout,"%lf %lf\n", creal(buf[i]), cimag(buf[i]));

	fclose(fin);
	fclose(fout);
	return 0;
}
