#include <stdio.h>
#include <complex.h>
#include <tgmath.h>
#include <pthread.h>
#include <malloc.h>


FILE* in;
FILE* out;
int N;
int P;
double complex* vector;
double complex* vector_out;
int grp;
int rest;

void init() {

	vector = (double complex*)malloc(sizeof(double complex)*N);
	vector_out = (double complex*)malloc(sizeof(double complex) * N);

	if(vector == NULL || vector_out== NULL) {
		printf("Eroare la alocarea vectorului\n");
		return;
	}

	int i;
	for(i=0;i<N;i++) {
		fscanf(in,"%lf",&vector[i]);
	}
}

void free_vectors(){

	free(vector);
	free(vector_out);
}

void* fourier (void *var) {

	int thread_id = *(int*)var;
	int i,j;
	int ok=0;


	if(thread_id == P-1 ) {
		ok = rest;
	}

	for(i=grp*thread_id;i<grp*(thread_id+1) + ok;i++) {
		complex double sum = 0.0;
		for(j=0;j<N;j++) {
			double angle = 2*M_PI * i * j / N;
			sum += vector[j] * cexp(-angle * I);
		}
		vector_out[i] = sum;
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

	int i;
	fscanf(in,"%d",&N);
	init();
	P = atoi(argv[3]);

	grp = N/P;
	rest = N%P;

	pthread_t tid[P];
	int thread_id[P];

	fprintf(out,"%d\n",N);

	for(i = 0;i < P; i++)
		thread_id[i] = i;

	for(i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, fourier, &(thread_id[i]));
	}

	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	for(i=0;i<N;i++) {
		fprintf(out,"%lf %lf\n",creal(vector_out[i]),cimag(vector_out[i]));
	}


	free_vectors();

	fclose(in);
	fclose(out);
	return 0;
}
