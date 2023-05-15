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

int procuraMenorDistancia(int *dist, int *visitado, int NV){
    int i, menor = -1, primeiro = 1;
    for(i=0; i < NV; i++){
        if(dist[i] >= 0 && visitado[i] == 0){
            if(primeiro){
                menor = i;
                primeiro = 0;
            }else{
                if(dist[menor] > dist[i])
                    menor = i;
            }
        }
    }
    return menor;
}

ARESTA* menorCaminho_Grafo(Grafo *gr, int ini, int *ant, int *dist){
    int i, pos, cont, NV, ind, *visitado, vert;
    cont = NV = gr->nro_vertices;
    visitado = (int*) malloc(NV * sizeof(int));
    for(i=0; i < NV; i++){
        ant[i] = -1;
        dist[i] = -1;
        visitado[i] = 0;
    }

    ARESTA *ar = (ARESTA*) malloc(4*NV * sizeof(ARESTA));
    for(i=0; i < 4*NV; i++){
        ar[i].orig = -1;
        ar[i].dest = -1;
    }


    dist[ini] = 0;
    pos = 0;
    while(cont > 0){
        vert = procuraMenorDistancia(dist, visitado, NV);
        if(vert == -1)
            break;

        visitado[vert] = 1;
        cont--;
        for(i=0; i<gr->grau[vert]; i++){
            ind = gr->arestas[vert][i];

            ar[pos].orig = vert;
            ar[pos].dest = ind;
            pos++;

            if(dist[ind] < 0){
               dist[ind] = dist[vert] + 1;//ou peso da aresta
               ant[ind] = vert;
            }else{
                if(dist[ind] > dist[vert] + 1){
                    dist[ind] = dist[vert] + 1;//ou peso da aresta
                    ant[ind] = vert;
                }
            }
        }
    }

    free(visitado);
    return ar;
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


void buscaProfundidade(Grafo *gr, int ini, int dest, int *visitado, AR_PILHA* ar, int *cont, int *achou){
    int i;
    visitado[ini] = 1;
    for(i=0; i<gr->grau[ini]; i++){
        if(*achou == 1)
            return;

        if(!visitado[gr->arestas[ini][i]]){
            ar[*cont].orig = ini;
            ar[*cont].dest = gr->arestas[ini][i];
            ar[*cont].tipo = 0;
            (*cont)++;

            if(gr->arestas[ini][i] == dest){
                *achou = 1;
                return;
            }
            buscaProfundidade(gr,gr->arestas[ini][i],dest,visitado,ar,cont,achou);
            if(*achou == 0){
                ar[*cont].orig = ini;
                ar[*cont].dest = gr->arestas[ini][i];
                ar[*cont].tipo = 1;
                (*cont)++;
            }
            if(*cont > 8*gr->nro_vertices)
                printf("ERRO!!!\n");
        }
    }
}

AR_PILHA* buscaProfundidade_Grafo(Grafo *gr, int ini, int dest){
    int i, cont = 0, achou = 0;
    int NV = gr->nro_vertices;

    int *visitado = (int*) malloc(NV * sizeof(int));
    for(i=0; i < NV; i++)
        visitado[i] = 0;

    AR_PILHA *ar = (AR_PILHA*) malloc(8*NV * sizeof(AR_PILHA));
    for(i=0; i < 8*NV; i++){
        ar[i].orig = -1;
        ar[i].dest = -1;
        ar[i].tipo = 0;
    }

    buscaProfundidade(gr,ini,dest,visitado,ar,&cont,&achou);
    free(visitado);
    return ar;
}

ARESTA* buscaLargura_Grafo(Grafo *gr, int ini){
    int i, vert, NV, cont = 1;
    int *fila, IF = 0, FF = 0;
    int ind, pos = 0;
    NV = gr->nro_vertices;

    int *visitado = (int*) malloc(NV * sizeof(int));
    for(i=0; i < NV; i++)
        visitado[i] = 0;

    ARESTA *ar = (ARESTA*) malloc(4*NV * sizeof(ARESTA));
    for(i=0; i < 4*NV; i++){
        ar[i].orig = -1;
        ar[i].dest = -1;
    }

    fila = (int*) malloc(NV * sizeof(int));
    FF++;
    fila[FF] = ini;
    visitado[ini] = cont;
    while(IF != FF){
        IF = (IF + 1) % NV;
        vert = fila[IF];
        cont++;
        for(i=0; i<gr->grau[vert]; i++){
            ind = gr->arestas[vert][i];
            if(!visitado[ind]){

                ar[pos].orig = vert;
                ar[pos].dest = ind;
                pos++;
                //printf("pos = %d\n",pos);

                FF = (FF + 1) % NV;
                fila[FF] = ind;
                visitado[ind] = cont;
            }
        }
    }
    free(fila);

    return ar;
}

