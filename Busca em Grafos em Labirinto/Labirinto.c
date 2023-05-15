#include "Labirinto.h"
#include "Grafo.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

struct labirinto{
    int *arv;
    int *ant;
    int dist;
    ARESTA *ar_largura;
    ARESTA *ar;
    AR_PILHA *ph;
    int idx;
};

//=========================================================
// Tamanho da matriz do labirinto
#define N 30
// Tamanho de cada bloco da matriz na tela
#define TAM (2.0f / N)
//Funções que convertem a linha e coluna da matriz em uma coordenada de [-1,1]
#define MAT2X(j) ((j)* TAM - 1.0)
//#define MAT2Y(i) (1.0 - (i) * TAM)
#define MAT2Y(i) (1.0 - (i) * TAM)
//=========================================================

void desenha_linha(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, int cor){
    switch(cor){
        case 0: glColor3f(0.0,0.0,1.0); glLineWidth(5); break;
        case 1: glColor3f(1.0,0.0,0.0); glLineWidth(3); break;
        case 2: glColor3f(1.0,1.0,1.0); glLineWidth(5); break;
    }
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
}

void desenha_circulo(GLfloat x, GLfloat y, GLfloat radius){
	int i, triangleAmount = 20; //# of triangles used to draw circle
	GLfloat twicePi = 2.0f * acos(-1.0);
    glColor3f(0.0,0.0,0.0);
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y); // center of circle
		for(i = 0; i <= triangleAmount;i++) {
			glVertex2f(x + (radius * cos(i *  twicePi / triangleAmount)),
                       y + (radius * sin(i * twicePi / triangleAmount)));
		}
	glEnd();
}


LABIRINTO* monta_labirinto(){
    LABIRINTO* lab = malloc(sizeof(LABIRINTO));
    if(lab == NULL)
        return NULL;

    int i,j;
    Grafo* gr = cria_Grafo(N*N, 4);

    for(i=0; i<N; i++){
        for(j=0; j<N; j++){
            int v1 = i * N + j;
            if(j+1 < N){
                int v2 = i * N + (j+1);
                if(!insereAresta(gr,v1,v2,0,rand()%9 + 1))
                    printf("Erro 1: %d %d",i,j);
            }

            if(i+1 < N){
                int v2 = (i+1) * N + j;
                if(!insereAresta(gr,v1,v2,0,rand()%9 + 1))
                    printf("Erro 2: %d %d",i,j);
            }
        }
    }

    lab->arv = malloc(N*N*sizeof(int));
    lab->ant = malloc(N*N*sizeof(int));
    int *dist = malloc(N*N*sizeof(int));

    arvoreGeradoraMinimaPRIM_Grafo(gr, 0, lab->arv);
    libera_Grafo(gr);
    // =============================================
    //criar um novo grafo com a árvore geradora...
    gr = cria_Grafo(N*N, 4);
    int N2 = N*N;
    for(i=1; i < N2; i++){
        if(!insereAresta(gr,i,lab->arv[i],0,1))
            printf("Erro ARV: %d %d",i,lab->arv[i]);
    }

    lab->ar = menorCaminho_Grafo(gr, 0, lab->ant, dist);
    lab->dist = dist[N*N-1];
    free(dist);

    lab->ph = buscaProfundidade_Grafo(gr, 0, N*N-1);
    lab->ar_largura = buscaLargura_Grafo(gr, 0);

    libera_Grafo(gr);
    return lab;
}

void destroi_labirinto(LABIRINTO* lab){
    if(lab == NULL)
        return;
    free(lab->arv);
    free(lab->ant);
    free(lab->ar);
    free(lab->ar_largura);
    free(lab->ph);
    free(lab);
}

void desenha_arvore_geradora(int *arv){
    int i, j;

    int N2 = N*N;
    int i1, i2, j1, j2;
    for(i=1; i < N2; i++){
        i1 = i / N;
        j1 = i % N;

        i2 = arv[i] / N;
        j2 = arv[i] % N;

        desenha_linha(MAT2X(j1+0.5), MAT2Y(i1+0.5),
                      MAT2X(j2+0.5), MAT2Y(i2+0.5),1);
    }

    for(i=0; i<N; i++){
        for(j=0; j<N; j++){
            desenha_circulo(MAT2X(j+0.5), MAT2Y(i+0.5),0.0175);
        }
    }
}

void desenha_paredes_labirinto(int *arv){
    int i,j;
    desenha_linha(MAT2X(0.025), MAT2Y(0.025),MAT2X(0.025), MAT2Y(N-0.025),0);
    desenha_linha(MAT2X(0.025), MAT2Y(N-0.025),MAT2X(N-0.025), MAT2Y(N-0.025),0);
    desenha_linha(MAT2X(N-0.025), MAT2Y(0.025),MAT2X(N-0.025), MAT2Y(N-0.025),0);
    desenha_linha(MAT2X(0.025), MAT2Y(0.025),MAT2X(N-0.025), MAT2Y(0.025),0);

    for(i=0; i<N; i++){
        for(j=0; j<N; j++){
            int v1 = i * N + j;
            if(j+1 < N){
                int v2 = i * N + (j+1);
                if(arv[v2] != v1 && arv[v1] != v2){
                    desenha_linha(MAT2X(j+1), MAT2Y(i),
                                  MAT2X(j+1), MAT2Y(i+1),0);
                }
            }
            if(i+1 < N){
                int v2 = (i+1) * N + j;
                if(arv[v2] != v1 && arv[v1] != v2){
                    desenha_linha(MAT2X(j), MAT2Y(i+1),
                                  MAT2X(j+1), MAT2Y(i+1),0);
                }
            }
        }
    }
}

void inicia_animacao(LABIRINTO *lab){
    lab->idx = 0;
}

void desenha_procura_caminho(LABIRINTO *lab){
    int i1, i2, j1, j2;
    int cont = 0;
    while(cont <= lab->idx){
        i1 = lab->ar[cont].orig / N;
        j1 = lab->ar[cont].orig % N;

        i2 = lab->ar[cont].dest / N;
        j2 = lab->ar[cont].dest % N;

        desenha_linha(MAT2X(j1+0.5), MAT2Y(i1+0.5),
                      MAT2X(j2+0.5), MAT2Y(i2+0.5),1);

        desenha_circulo(MAT2X(j1+0.5), MAT2Y(i1+0.5),0.0175);
        desenha_circulo(MAT2X(j2+0.5), MAT2Y(i2+0.5),0.0175);

        cont++;
        if(lab->ar[cont].orig == -1)
            break;
    }
    if(lab->idx < 4*N*N)
        lab->idx++;

}


void desenha_menor_caminho(LABIRINTO *lab){
    int v1, v2 = N*N-1;
    int i1, i2, j1, j2;
    int cont = 0;
    while(cont < lab->idx){
        v1 = lab->ant[v2];
        i1 = v1 / N;
        j1 = v1 % N;
        i2 = v2 / N;
        j2 = v2 % N;

        desenha_linha(MAT2X(j1+0.5), MAT2Y(i1+0.5),
                      MAT2X(j2+0.5), MAT2Y(i2+0.5),1);

        desenha_circulo(MAT2X(j1+0.5), MAT2Y(i1+0.5),0.0175);
        desenha_circulo(MAT2X(j2+0.5), MAT2Y(i2+0.5),0.0175);

        cont++;
        v2 = v1;
        if(cont == lab->dist)
            break;

    }
    if(lab->idx < lab->dist)
        lab->idx++;
}

void desenha_busca_profundidade(LABIRINTO *lab){
    int v1, v2 = N*N-1;
    int i1, i2, j1, j2;
    int cont = 0;
    while(cont < lab->idx){
        v1 = lab->ph[cont].orig;
        v2 = lab->ph[cont].dest;

        i1 = v1 / N;
        j1 = v1 % N;

        i2 = v2 / N;
        j2 = v2 % N;
        if(lab->ph[cont].tipo == 0){
            desenha_linha(MAT2X(j1+0.5), MAT2Y(i1+0.5),
                          MAT2X(j2+0.5), MAT2Y(i2+0.5),1);
        }else{
            desenha_linha(MAT2X(j1+0.5), MAT2Y(i1+0.5),
                          MAT2X(j2+0.5), MAT2Y(i2+0.5),2);
        }

        desenha_circulo(MAT2X(j1+0.5), MAT2Y(i1+0.5),0.0175);
        desenha_circulo(MAT2X(j2+0.5), MAT2Y(i2+0.5),0.0175);

        cont++;
        v2 = v1;
        if(lab->ph[cont].orig == -1)
            break;
    }
    if(lab->idx < 8*N*N)
        lab->idx++;
}

void desenha_busca_largura(LABIRINTO *lab){
    int i1, i2, j1, j2;
    int cont = 0;
    //printf("busca_largura\n");
    while(cont <= lab->idx){
        i1 = lab->ar_largura[cont].orig / N;
        j1 = lab->ar_largura[cont].orig % N;

        i2 = lab->ar_largura[cont].dest / N;
        j2 = lab->ar_largura[cont].dest % N;

        desenha_linha(MAT2X(j1+0.5), MAT2Y(i1+0.5),
                      MAT2X(j2+0.5), MAT2Y(i2+0.5),1);

        desenha_circulo(MAT2X(j1+0.5), MAT2Y(i1+0.5),0.0175);
        desenha_circulo(MAT2X(j2+0.5), MAT2Y(i2+0.5),0.0175);

        cont++;
        //printf("cont = %d\n",cont);
        if(lab->ar_largura[cont].orig == -1)
            break;
    }
    if(lab->idx < 4*N*N)
        lab->idx++;

}

void desenha_labirinto(LABIRINTO *lab,int tipo){
    if(lab == NULL)
        return;

    switch(tipo){
        case 0: desenha_arvore_geradora(lab->arv);
                break;
        case 1: desenha_arvore_geradora(lab->arv);
                desenha_paredes_labirinto(lab->arv);
                break;
        case 2: desenha_paredes_labirinto(lab->arv);
                break;
        case 3: desenha_paredes_labirinto(lab->arv);
                desenha_procura_caminho(lab);
                break;
        case 4: desenha_paredes_labirinto(lab->arv);
                desenha_menor_caminho(lab);
                break;
        case 5: desenha_paredes_labirinto(lab->arv);
                desenha_busca_profundidade(lab);
                break;
        case 6: desenha_paredes_labirinto(lab->arv);
                desenha_busca_largura(lab);
                break;
        default: desenha_paredes_labirinto(lab->arv);
    }
}

