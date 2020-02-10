#include <stdio.h>
#include <complex.h>
#include <tgmath.h>
#include <pthread.h>
#include <malloc.h>

FILE* in;
FILE* out;
int N;
int P;

double complex* vector ;
double complex* vector_out ;

void init() {

	int i;

	vector = (double complex*)malloc(sizeof(double complex) * N);
	vector_out = (double complex*)malloc(sizeof(double complex) * N);

	if(vector == NULL || vector_out== NULL) {
		printf("Eroare la alocarea vectorului\n");
		return;
	}

	for(i=0;i<N;i++) {
		fscanf(in,"%lf",&vector[i]);
		vector_out[i] = vector[i];
	}
}


//---------

void free_vectors(){

	free(vector);
	free(vector_out);
}



void* _fft(void *argv)
{
	int step = *(int*)argv;
	int step_aux = *(int*)(argv + 4);
	int arg1[2],arg2[2];
	arg1[0] = step * 2; 
	arg1[1] = step_aux;
	arg2[0] = step * 2;
	arg2[1] = step_aux + step;

	if (step < N) {
		
		if(step < P && P == 4){
			pthread_t tid;
			pthread_create(&(tid), NULL, _fft, arg1);
			_fft(arg2);
			pthread_join(tid, NULL);

		} else {
			
			_fft(arg1);
			_fft(arg2);

		}

		for (int i = 0; i < N; i += 2 * step) {
			if((int)log2(step) % 2 == 0) {

				double complex t = cexp(-I * M_PI * i / N) * vector_out[i + step + step_aux];
				vector[(i / 2) + step_aux]     = vector_out[i + step_aux] + t;
				vector[(i + N)/2 + step_aux] = vector_out[i + step_aux] - t;

			} else if((int)log2(step) % 2 == 1) {

				double complex t = cexp(-I * M_PI * i / N) * vector[i + step + step_aux];
				vector_out[i / 2 + step_aux]     = vector[i + step_aux] + t;
				vector_out[(i + N)/2 + step_aux] = vector[i + step_aux] - t;
			}

		}
	}
}

void* threadFunction(void *var)
{
	int thread_id = *(int*)var;
	int step = *(int*)(var + 4);
	int step_aux = *(int*)(var + 8);
	int arg[2];
	if(thread_id == 0){
		arg[0] = step * 2;
		arg[1] = step_aux;
		_fft(arg);
	} else {
		arg[0] = step * 2;
		arg[1] = step_aux + step;
		_fft(arg);
	}

}

int main(int argc, char * argv[]) {


	if(argc < 4) {
		printf("Eroare, nu s-a dat numarul corect de argumente\n");
		return -1;
	}


	in = fopen(argv[1],"r");
	out = fopen(argv[2],"w");

	if(in == NULL || out == NULL) {
		printf("Eroare la citirea sau scrierea din/in fisier\n");
	}

	int width,i;
	fscanf(in,"%d",&N);
	fprintf(out,"%d\n",N);
	P = atoi(argv[3]);

	init();


	if(P != 1){
		
		pthread_t tid[2];

		for(i = 0; i < 2; i++) {
			int *argv = calloc(3,sizeof(int));
			argv[0] = i;
			argv[1] = 1;
			argv[2] = 0;
			pthread_create(&(tid[i]), NULL, threadFunction, argv);
			
		}

		for(i = 0; i < 2; i++) {
			pthread_join(tid[i], NULL);
		}

		for (int i = 0; i < N; i += 2 ) {
			double complex t = cexp(-I * M_PI * i / N) * vector_out[i + 1];
			vector[i / 2]     = vector_out[i] + t;
			vector[(i + N)/2] = vector_out[i] - t;
		}
	} else {

		int argv[2];
		argv[0] = 1; 
		argv[1] = 0;
		_fft(argv);
	}
	

	for(i=0;i<N;i++) {
		fprintf(out,"%lf %lf\n",creal(vector[i]),cimag(vector[i]));
	}

	free_vectors();

	fclose(in);
	fclose(out);
	return 0;
}