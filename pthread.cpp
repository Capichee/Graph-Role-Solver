#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define N 26
#define ROLES 7
#define J 12

pthread_mutex_t lock;
int turno = -1;

//const int matriz_papeis[ROLES][ROLES] = {
//    {0, 1, 0, 1},
//    {1, 0, 1, 0},
//    {0, 1, 0, 1},
//    {1, 0, 1, 0},
//};

const int matriz_papeis[ROLES][ROLES] = {
    {0, 1, 0, 1, 0, 0, 0},
    {1, 0, 1, 0, 1, 0, 0},
    {0, 1, 0, 1, 0, 1, 0},
    {1, 0, 1, 0, 1, 0, 1},
    {0, 1, 0, 1, 0, 1, 1},
    {0, 0, 1, 0, 1, 0, 1},
    {0, 0, 0, 1, 1, 1, 1}
};

    typedef struct {
        int papel_min;
        int papel_max;
        int solucao_atribuicao[N];
    } ThreadContext;

    ThreadContext dados_threads[3];

    void obter_vizinhos(int v, int vizinhos[4]) {
        vizinhos[0] = (v + 1) % N;
        vizinhos[1] = (v - 1 + N) % N;
        vizinhos[2] = (v + J) % N;
        vizinhos[3] = (v - J + N) % N;
    }

    bool eh_valido(int vertice_atual, const int atribuicao[N]) {
        for (int v = 0; v <= vertice_atual; v++) {
            int papel_v = atribuicao[v];
            int vizinhos[4];
            obter_vizinhos(v, vizinhos);

            bool papeis_visto_na_vizinhanca[ROLES] = { false };
            bool vizinhanca_incompleta = false;

            for (int i = 0; i < 4; i++) {
                int vizinho = vizinhos[i];
                if (atribuicao[vizinho] != -1) {
                    int papel_vizinho = atribuicao[vizinho];
                    if (matriz_papeis[papel_v][papel_vizinho] == 0) {
                        return false;
                    }
                    papeis_visto_na_vizinhanca[papel_vizinho] = true;
                } else {
                    vizinhanca_incompleta = true; 
                }
            }

            if (!vizinhanca_incompleta) {
                for (int r = 0; r < ROLES; r++) {
                    if (matriz_papeis[papel_v][r] == 1 && !papeis_visto_na_vizinhanca[r]) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool backtracking(int vertice, int atribuicao[N]) {
        if (vertice == N) {
            return eh_valido(N - 1, atribuicao);
        }

        for (int papel = 0; papel < ROLES; papel++) {
            atribuicao[vertice] = papel;

            if (eh_valido(vertice, atribuicao)) {
                if (backtracking(vertice + 1, atribuicao)) {
                    return true;
                }
            }
            atribuicao[vertice] = -1;
        }
        return false;
    }

    void* printA(void* arg) {
        int atribuicao[N];
        for (int i = 0; i < N; i++) atribuicao[i] = -1;
        atribuicao[0] = 0;

        for (int r = dados_threads[0].papel_min; r <= dados_threads[0].papel_max; r++) {
            atribuicao[1] = r;
            if (eh_valido(1, atribuicao)) {
                if (backtracking(2, atribuicao)) {
                    pthread_mutex_lock(&lock);
                    if (turno == -1) {
                        turno = 0;
                        for (int i = 0; i < N; i++) dados_threads[0].solucao_atribuicao[i] = atribuicao[i];
                    }
                    pthread_mutex_unlock(&lock);
                    return NULL;
                }
            }
            atribuicao[1] = -1;
        }
        return NULL;
    }

    void* printB(void* arg) {
        int atribuicao[N];
        for (int i = 0; i < N; i++) atribuicao[i] = -1;
        atribuicao[0] = 0;

        for (int r = dados_threads[1].papel_min; r <= dados_threads[1].papel_max; r++) {
            atribuicao[1] = r;
            if (eh_valido(1, atribuicao)) {
                if (backtracking(2, atribuicao)) {
                    pthread_mutex_lock(&lock);
                    if (turno == -1) {
                        turno = 1;
                        for (int i = 0; i < N; i++) dados_threads[1].solucao_atribuicao[i] = atribuicao[i];
                    }
                    pthread_mutex_unlock(&lock);
                    return NULL;
                }
            }
            atribuicao[1] = -1;
        }
        return NULL;
    }

    void* printC(void* arg) {
        int atribuicao[N];
        for (int i = 0; i < N; i++) atribuicao[i] = -1;
        atribuicao[0] = 0;

        for (int r = dados_threads[2].papel_min; r <= dados_threads[2].papel_max; r++) {
            atribuicao[1] = r;
            if (eh_valido(1, atribuicao)) {
                if (backtracking(2, atribuicao)) {
                    pthread_mutex_lock(&lock);
                    if (turno == -1) {
                        turno = 2;
                        for (int i = 0; i < N; i++) dados_threads[2].solucao_atribuicao[i] = atribuicao[i];
                    }
                    pthread_mutex_unlock(&lock);
                    return NULL;
                }
            }
            atribuicao[1] = -1;
        }
        return NULL;
    }

    int main() {

        //pthread_t threads[3];
        //ThreadData t_data[3];

        pthread_t t1, t2, t3;
        pthread_mutex_init(&lock, NULL);

        dados_threads[0].papel_min = 0; dados_threads[0].papel_max = 2;
        dados_threads[1].papel_min = 3; dados_threads[1].papel_max = 4;
        dados_threads[2].papel_min = 5; dados_threads[2].papel_max = 6;

        pthread_create(&t1, NULL, printA, NULL);
        pthread_create(&t2, NULL, printB, NULL);
        pthread_create(&t3, NULL, printC, NULL);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        pthread_join(t3, NULL);

        if (turno == 0) {
            printf("\nThread A:\n");
            for (int i = 0; i < N; i++) printf("v%d:%d ", i, dados_threads[0].solucao_atribuicao[i]);
            printf("\n");
        } else if (turno == 1) {
            printf("\nThread B:\n");
            for (int i = 0; i < N; i++) printf("v%d:%d ", i, dados_threads[1].solucao_atribuicao[i]);
            printf("\n");
        } else if (turno == 2) {
            printf("\nThread C:\n");
            for (int i = 0; i < N; i++) printf("v%d:%d ", i, dados_threads[2].solucao_atribuicao[i]);
            printf("\n");
        } else {
            printf("\nNenhuma 7-atribuição válida.\n");
        }

        pthread_mutex_destroy(&lock);
        return 0;
    }