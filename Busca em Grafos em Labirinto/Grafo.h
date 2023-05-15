//Arquivo Grafo.h

typedef struct grafo Grafo;

struct aresta{
    int orig, dest;
};
typedef struct aresta ARESTA;

struct aresta_pilha{
    int orig, dest, tipo;
};
typedef struct aresta_pilha AR_PILHA;

Grafo* cria_Grafo(int nro_vertices, int grau_max);
void libera_Grafo(Grafo* gr);
int insereAresta(Grafo* gr, int orig, int dest, int eh_digrafo, int peso);

void arvoreGeradoraMinimaPRIM_Grafo(Grafo *gr, int ini, int *pai);
void arvoreGeradoraMinimaKruskal_Grafo(Grafo *gr, int orig, int *pai);

ARESTA* menorCaminho_Grafo(Grafo *gr, int ini, int *ant, int *dist);
int procuraMenorDistancia(int *dist, int *visitado, int NV);

AR_PILHA* buscaProfundidade_Grafo(Grafo *gr, int ini, int dest);

ARESTA* buscaLargura_Grafo(Grafo *gr, int ini);
