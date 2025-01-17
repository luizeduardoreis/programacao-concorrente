#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define N 20
#define NUMERO_DE_VAGAS 10
#define MAXIMO_DE_EXPERIENTES NUMERO_DE_VAGAS / 2

int experientes_estacionados = 0;
int vagas_ocupadas[NUMERO_DE_VAGAS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t iniciantes_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t experientes_cond = PTHREAD_COND_INITIALIZER;

bool iniciante_pode_estacionar() {
	for (int i = 1; i < NUMERO_DE_VAGAS - 1; i++) {
		if (!vagas_ocupadas[i-1] && !vagas_ocupadas[i] && !vagas_ocupadas[i+1]) {
			return true;
		}
	}
	return (!vagas_ocupadas[0] && !vagas_ocupadas[1]) || (!vagas_ocupadas[NUMERO_DE_VAGAS-2] && !vagas_ocupadas[NUMERO_DE_VAGAS-1]);
}

bool experiente_pode_estacionar() {
	for (int i = 0; i < NUMERO_DE_VAGAS; i++) {
		if (!vagas_ocupadas[i]) return true;
	}
	return false;
}

int iniciante_estaciona() {
	for (int i = 1; i < NUMERO_DE_VAGAS-1; i++) {
		if (!vagas_ocupadas[i-1] && !vagas_ocupadas[i] && !vagas_ocupadas[i+1]) {
			vagas_ocupadas[i] = 1;
			return i;
		}
	}
	
	if (!vagas_ocupadas[0] && !vagas_ocupadas[1]) {
		vagas_ocupadas[0] = 1;
		return 0;
	}

	if (!vagas_ocupadas[NUMERO_DE_VAGAS-2] && !vagas_ocupadas[NUMERO_DE_VAGAS-1]) {
		vagas_ocupadas[NUMERO_DE_VAGAS-1] = 1;
		return NUMERO_DE_VAGAS - 1;
	}
}

int experiente_estaciona() {
	for (int i = 0; i < NUMERO_DE_VAGAS; i++) {
		if (!vagas_ocupadas[i]) {
			vagas_ocupadas[i] = 2;
			return i;
		}
	}
}

void liberar_vaga(int vaga_idx) {
	vagas_ocupadas[vaga_idx] = 0;
}

void print_vagas() {
	printf("|");
	for (int i = 0; i < NUMERO_DE_VAGAS; i++) {
		if (vagas_ocupadas[i] == 1) printf("*|");
		else if (vagas_ocupadas[i] == 2) printf("^|");
		else printf(" |");
	}
	printf("\n\n");
}

void* iniciante(void* arg) {
	int id = *((int*) arg);
	while (1) {
		sleep(rand()%(10));
		pthread_mutex_lock(&mutex);
		while (!iniciante_pode_estacionar()) {
			pthread_cond_wait(&iniciantes_cond, &mutex);
		}
		int vaga = iniciante_estaciona();
		printf("Iniciante número %d estacionou na vaga: %d\n\n", id, vaga);
		print_vagas();
		pthread_mutex_unlock(&mutex);

		sleep(rand()%(10));
		

		pthread_mutex_lock(&mutex);
		printf("Iniciante número %d liberou a vaga: %d\n\n", id, vaga);
		liberar_vaga(vaga);
		print_vagas();
		pthread_cond_broadcast(&iniciantes_cond);
		pthread_cond_broadcast(&experientes_cond);
		pthread_mutex_unlock(&mutex);
	}
}

void* experiente(void * arg) {
	int id = *((int*) arg);
	while (1) {
		sleep(rand()%(10));
		pthread_mutex_lock(&mutex);
		while (!experiente_pode_estacionar() || experientes_estacionados >= MAXIMO_DE_EXPERIENTES) {
			pthread_cond_wait(&experientes_cond, &mutex);
		}
		experientes_estacionados++;
		int vaga = experiente_estaciona();
		printf("Experiente número %d estacionou na vaga: %d\n\n", id, vaga);
		print_vagas();
		pthread_mutex_unlock(&mutex);
		

		sleep(rand()%(10));
		

		pthread_mutex_lock(&mutex);
		printf("Experiente número %d liberou a vaga: %d\n\n", id, vaga);
		experientes_estacionados--;
		liberar_vaga(vaga);
		print_vagas();
		pthread_cond_broadcast(&iniciantes_cond);
		pthread_cond_broadcast(&experientes_cond);
		pthread_mutex_unlock(&mutex);
	}
}


int main() {
	pthread_t experientes[N], iniciantes[N];
	int i;
	int *id;
	
	for (i = 0; i < N; i++) {
		id = (int *) malloc(sizeof(int));
		*id = i;
		pthread_create(&iniciantes[i], NULL, &iniciante, (void*)id);
	}

	for (i = 0; i < N; i++) {
		id = (int *) malloc(sizeof(int));
		*id = i;
		pthread_create(&experientes[i], NULL, &experiente, (void*)id);
	}

	for (i = 0; i < N; i++) {
		pthread_join(experientes[i], NULL);
		pthread_join(iniciantes[i], NULL);
	}
}
