#include <stdio.h>
#include <stdlib.h>
#include "Grafo.h" //inclui os Protótipos

//Definição do tipo Grafo
struct grafo{
    int nro_vertices;
    int grau_max;
    int** arestas;
    int** pesos;
    int* grau;
};

Grafo* cria_Grafo(int nro_vertices, int grau_max){
    Grafo *gr;
    gr = (Grafo*) malloc(sizeof(struct grafo));
    if(gr != NULL){
        int i;
        gr->nro_vertices = nro_vertices;
        gr->grau_max = grau_max;
        gr->grau = (int*) calloc(nro_vertices,sizeof(int));

        gr->arestas = (int**) malloc(nro_vertices * sizeof(int*));
        gr->pesos = (int**) malloc(nro_vertices * sizeof(int*));
        for(i=0; i<nro_vertices; i++){
            gr->arestas[i] = (int*) malloc(grau_max * sizeof(int));
            gr->pesos[i] = (int*) malloc(grau_max * sizeof(int));
        }

    }
    return gr;
}

void libera_Grafo(Grafo* gr){
    if(gr != NULL){
        int i;
        for(i=0; i<gr->nro_vertices; i++){
            free(gr->arestas[i]);
            free(gr->pesos[i]);
        }
        free(gr->arestas);
        free(gr->pesos);
        free(gr->grau);
        free(gr);
    }
}

int insereAresta(Grafo* gr, int orig, int dest, int eh_digrafo, int peso){
    if(gr == NULL)
        return 0;
    if(orig < 0 || orig >= gr->nro_vertices)
        return 0;
    if(dest < 0 || dest >= gr->nro_vertices)
        return 0;

    gr->arestas[orig][gr->grau[orig]] = dest;
    gr->pesos[orig][gr->grau[orig]] = peso;
    gr->grau[orig]++;

    if(eh_digrafo == 0)
        insereAresta(gr,dest,orig,1,peso);
    return 1;
}

void arvoreGeradoraMinimaPRIM_Grafo(Grafo *gr, int orig, int *pai){
    int i, j, dest, NV, primeiro;
    int menorPeso;
    NV = gr->nro_vertices;
    for(i=0; i < NV; i++)
        pai[i] = -1;// sem pai

    pai[orig] = orig;
    while(1){
        primeiro = 1;
        for(i=0; i < NV; i++){//percorre todos os vértices
            if(pai[i] != -1){//achou vértices já visitado
                for(j=0; j<gr->grau[i]; j++){ // percorre os vizinhos do vértice visitado
                    if(pai[gr->arestas[i][j]] == -1){//achou vértice vizinho não visitado
                         if(primeiro){//procura aresta de menor custo
                            menorPeso = gr->pesos[i][j];
                            orig = i;
                            dest = gr->arestas[i][j];
                            primeiro = 0;
                         }else{
                             if(menorPeso > gr->pesos[i][j]){
                                menorPeso = gr->pesos[i][j];
                                orig = i;
                                dest = gr->arestas[i][j];
                             }
                         }
                    }
                }
            }
        }
        if(primeiro == 1)
            break;

        pai[dest] = orig;
    }
}

