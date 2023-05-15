#include "labirinto3D.h"
#include "Grafo.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//=========================================================
// Tamanho da matriz do labirinto
#define N 10
// Tamanho de cada bloco da matriz na tela
#define TAM (2.0f / N)
//Funções que convertem a linha e coluna da matriz em uma coordenada de [-1,1]
#define MAT2X(j) ((j)* TAM - 1.0)
#define MAT2Y(i) (1.0 - (i) * TAM)

//=========================================================
#define GIRO_VAL 100//600
#define GIRO_ANG (90.0/GIRO_VAL)
#define MAX_DIST_PAREDE 0.01f
#define incr 0.01

#define NanguloZ 4
const int anguloZTopo[4] = {0,90,180,270};
const int anguloZPersp[4] = {180,270,0,90};

#define Npasso 21
const double passo[21] = {-0.0167f, -0.0165f, -0.0160f, -0.0153f, -0.0144f, -0.0133f,
                          -0.0123f, -0.0114f, -0.0106f, -0.0102f, -0.0100f, -0.0102f,
                          -0.0106f, -0.0114f, -0.0123f, -0.0133f, -0.0144f, -0.0153f,
                          -0.0160f, -0.0165f, -0.0167};

struct coord{
    int tipo; //0 horizontal
    double x1, y1, x2, y2;
};

struct lista_paredes{
    int MAX, qtd;
    struct coord *CO;
};

struct labirinto3D{
    double posX, posY;
    int idx_anguloZ;
    int idx_anguloZ_ant;
    int giro;
    int giro_dir;
    int idx_passo;
    struct lista_paredes *paredes;
};

//=======================================================
void viewport_topo(labirinto3D* lab3d);
void viewport_perspectiva(labirinto3D* lab3d);

void draw_obj();
void draw_topo(struct lista_paredes *paredes);
void draw_perspectiva(struct lista_paredes *paredes);

double distanciaSegmentoReta(double xk, double yk, struct coord *p);
int pontoDentroSegmentoReta(double xk, double yk, struct coord *p, int tipo_parede);
struct lista_paredes *monta_labirinto();

//=======================================================
labirinto3D* cria_labirinto3D(){
    labirinto3D* lab3d = malloc(sizeof(labirinto3D));
    if(lab3d != NULL){
        lab3d->posX = 0.0f;
        lab3d->posY = 0.0f;
        lab3d->idx_anguloZ = 0;
        lab3d->idx_anguloZ_ant = 0;
        lab3d->giro = 0;
        lab3d->giro_dir = 1;
        lab3d->idx_passo = 10;

        lab3d->paredes = monta_labirinto();
    }
    return lab3d;
}

void destroi_labirinto3D(labirinto3D* lab3d){
    free(lab3d->paredes->CO);
    free(lab3d->paredes);
    free(lab3d);
}

//=======================================================
void viraEsquerda(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    if(lab3d->giro == 0){
        lab3d->giro = GIRO_VAL;
        lab3d->giro_dir = -1;
        lab3d->idx_anguloZ_ant = lab3d->idx_anguloZ;

        lab3d->idx_anguloZ--;
        if(lab3d->idx_anguloZ < 0)
            lab3d->idx_anguloZ = NanguloZ - 1;
    }
}

void viraDireita(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    if(lab3d->giro == 0){
        lab3d->giro = GIRO_VAL;
        lab3d->giro_dir = +1;
        lab3d->idx_anguloZ_ant = lab3d->idx_anguloZ;

        lab3d->idx_anguloZ = (lab3d->idx_anguloZ + 1) % NanguloZ;
    }
    //printf("verificar parede %d\n",lab3d->idx_anguloZ %2);
}

double distanciaSegmentoReta(double xk, double yk, struct coord *p){
    double numerador = (xk - p->x1) * (p->y2 - p->y1) -
                       (yk - p->y1) * (p->x2 - p->x1);
    numerador = numerador * numerador;

    double denominador = (p->x1 - p->x2) * (p->x1 - p->x2) +
                         (p->y1 - p->y2) * (p->y1 - p->y2);

    double t = fabs(numerador / denominador);
    return t;
}

int pontoDentroSegmentoReta(double xk, double yk, struct coord *p, int tipo_parede){
    if(tipo_parede == 0){
        ///parede horizontal
        if((p->x1 <= xk && xk <= p->x2) || (p->x2 <= xk && xk <= p->x1))
           return 1;
    }else{
        ///parede vertical
        if((p->y1 <= yk && yk <= p->y2) || (p->y2 <= yk && yk <= p->y1))
            return 1;
    }

    return 0;
}

void caminhaXY(labirinto3D* lab3d, double dx, double dy, int frente){
    if(lab3d == NULL)
        return;

    double xk = lab3d->posX + dx;
    double yk = lab3d->posY + dy;
    double dist_antes, dist_depois;

    int i, caminha = 1;
    int tipo_parede = lab3d->idx_anguloZ % 2;

    struct lista_paredes *paredes = lab3d->paredes;

    for(i = 0; i < paredes->qtd; i++){
        if(paredes->CO[i].tipo == tipo_parede){

            if(!pontoDentroSegmentoReta(xk,yk,&paredes->CO[i],tipo_parede)){
                continue;
            }

            dist_antes = distanciaSegmentoReta(lab3d->posX,lab3d->posY,&paredes->CO[i]);
            dist_depois = distanciaSegmentoReta(xk,yk,&paredes->CO[i]);

            if(dist_depois < dist_antes && dist_depois < MAX_DIST_PAREDE){
                caminha = 0;
                break;
            }
        }
    }

    if(caminha){
        lab3d->posX = xk;
        lab3d->posY = yk;
        if(frente)
            lab3d->idx_passo = (lab3d->idx_passo + 1) % Npasso;
        else{
            lab3d->idx_passo--;
            if(lab3d->idx_passo < 0)
                lab3d->idx_passo = Npasso - 1;
        }
    }
}

void caminhaPraFrente(labirinto3D* lab3d){
    if(lab3d == NULL){
        return;
    }

    if(lab3d->giro > 0)
        return;

    switch(lab3d->idx_anguloZ){
        case 0: caminhaXY(lab3d, 0.0f, -incr, 1); break;
        case 1: caminhaXY(lab3d, -incr, 0.0f, 1); break;
        case 2: caminhaXY(lab3d, 0.0f, +incr, 1); break;
        case 3: caminhaXY(lab3d, +incr, 0.0f, 1); break;
    }
}

void caminhaPraTras(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    if(lab3d->giro > 0)
        return;

    switch(lab3d->idx_anguloZ){
        case 0: caminhaXY(lab3d, 0.0f, +incr, 0); break;
        case 1: caminhaXY(lab3d, +incr, 0.0f, 0); break;
        case 2: caminhaXY(lab3d, 0.0f, -incr, 0); break;
        case 3: caminhaXY(lab3d, -incr, 0.0f, 0); break;
    }
}

void desenha_labirinto3d(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    viewport_topo(lab3d);
    viewport_perspectiva(lab3d);
}

//=======================================================
void viewport_perspectiva(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    glViewport(0,0,tamanho,tamanho);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    gluPerspective(45.0,1.0,0.01,200.0);// Calculate The Aspect Ratio Of The Window

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glRotatef(-90, 1.0f, 0.0f, 0.0f); //ajusta a visão

    if(lab3d->giro > 0){
        lab3d->giro--;
        int var_ang = lab3d->giro_dir * (GIRO_VAL - lab3d->giro) * GIRO_ANG;
        glRotatef(var_ang + anguloZPersp[lab3d->idx_anguloZ_ant], 0.0f, 0.0f, 1.0f); //vira nos corredores
    }else{
        glRotatef(anguloZPersp[lab3d->idx_anguloZ], 0.0f, 0.0f, 1.0f); //vira nos corredores
    }

    glPushMatrix();
        glTranslatef(-lab3d->posX,-lab3d->posY,passo[lab3d->idx_passo]);
        draw_perspectiva(lab3d->paredes);
    glPopMatrix();
}

void draw_perspectiva(struct lista_paredes *paredes){
    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);     // plane
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();


    int i;
    glColor3f(0.0,0.0,1.0);
    glLineWidth(2);
    for(i=0; i< paredes->qtd; i++){
        switch(i % 4){
            case 0: glColor3f(1.0f, 0.0f, 0.0f); break;
            case 1: glColor3f(0.0f, 1.0f, 0.0f); break;
            case 2: glColor3f(0.0f, 0.0f, 1.0f); break;
            case 3: glColor3f(1.0f, 1.0f, 0.0f); break;
        }

        glBegin(GL_QUADS);
            glVertex3f(paredes->CO[i].x1, paredes->CO[i].y1, 0.00f);
            glVertex3f(paredes->CO[i].x2, paredes->CO[i].y2, 0.00f);
            glVertex3f(paredes->CO[i].x2, paredes->CO[i].y2, 0.05f);
            glVertex3f(paredes->CO[i].x1, paredes->CO[i].y1, 0.05f);
        glEnd();
    }
}

//=======================================================
void viewport_topo(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    glViewport(3*tamanho/4.0 - 20,5,tamanho/4,tamanho/4);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glPushMatrix();
        draw_topo(lab3d->paredes);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(lab3d->posX,lab3d->posY,0.0f);
        if(lab3d->giro > 0){
            lab3d->giro--;
            int var_ang = lab3d->giro_dir * (GIRO_VAL - lab3d->giro) * GIRO_ANG;
            glRotatef(-(var_ang + anguloZTopo[lab3d->idx_anguloZ_ant]), 0.0f, 0.0f, 1.0f); //vira nos corredores
        }else
            glRotatef(-anguloZTopo[lab3d->idx_anguloZ], 0.0f, 0.0f, 1.0f); //vira nos corredores

        draw_obj();
    glPopMatrix();
}

void draw_obj(){
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);     // plane gray
        glVertex3f( 0.05f, 0.05f, -0.05f);
        glVertex3f( -0.05f,0.05f, -0.05f);
        glVertex3f( 0.0f,-2*0.05f, -0.05f);
    glEnd();
}

void draw_topo(struct lista_paredes *paredes){
    glLineWidth(2);
    glColor3f(0.0f, 0.0f, 0.0f);     // contorno
    glBegin(GL_LINE_LOOP);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

    int i;
    glColor3f(0.0,0.0,1.0);
    glLineWidth(2);

    for(i=0; i< paredes->qtd; i++){
        glBegin(GL_LINES);
            glVertex3f(paredes->CO[i].x1, paredes->CO[i].y1, 0.00f);
            glVertex3f(paredes->CO[i].x2, paredes->CO[i].y2, 0.00f);
        glEnd();
    }

    glBegin(GL_QUADS);
        glColor3f(0.95f, 0.95f, 0.95f);     // plane
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();
}

//=======================================================

void insere_parede(struct lista_paredes *li, double x1, double y1, double x2, double y2, int tipo){
    if(li->qtd == li->MAX){ /// aumenta o tamanho da lista quando necessário
        li->MAX = li->MAX * 1.5;
        struct coord *CO = realloc(li->CO,li->MAX * sizeof(struct coord));
        if(CO == NULL){
            printf("ERRO REALLOC\n");
            system("pause");
            exit(1);
        }

        li->CO = CO;
    }
    ///insere a parede na lista
    li->CO[li->qtd].tipo = tipo;
    li->CO[li->qtd].x1 = x1;
    li->CO[li->qtd].y1 = y1;
    li->CO[li->qtd].x2 = x2;
    li->CO[li->qtd].y2 = y2;
    li->qtd++;
}

struct lista_paredes *monta_labirinto(){
    int i,j;
    Grafo* gr = cria_Grafo(N*N, 4);

    ///insere as arestas no grafo
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

    ///calcula a árvore geradora
    int *arv = malloc(N*N*sizeof(int));
    arvoreGeradoraMinimaPRIM_Grafo(gr, 0, arv);
    libera_Grafo(gr);

    ///inicia a lista de paredes
    struct lista_paredes *li;
    li = malloc(sizeof(struct lista_paredes));
    li->qtd = 0;
    li->MAX = 500;
    li->CO = malloc(500*sizeof(struct coord));

    ///calcula as paredes
    for(i=0; i<N; i++){
        for(j=0; j<N; j++){
            int v1 = i * N + j;
            if(j+1 < N){
                int v2 = i * N + (j+1);
                if(arv[v2] != v1 && arv[v1] != v2){
                    ///parede vertical
                    insere_parede(li, MAT2X(j+1), MAT2Y(i), MAT2X(j+1), MAT2Y(i+1), 1);
                }
            }
            if(i+1 < N){
                int v2 = (i+1) * N + j;
                if(arv[v2] != v1 && arv[v1] != v2){
                    ///parede horizontal
                    insere_parede(li, MAT2X(j), MAT2Y(i+1), MAT2X(j+1), MAT2Y(i+1), 0);
                }
            }
        }
    }
    ///insere as paredes de fora
    insere_parede(li, MAT2X(0.025), MAT2Y(0.025),MAT2X(0.025), MAT2Y(N-0.025),1);
    insere_parede(li, MAT2X(0.025), MAT2Y(N-0.025),MAT2X(N-0.025), MAT2Y(N-0.025),0);
    insere_parede(li, MAT2X(N-0.025), MAT2Y(0.025),MAT2X(N-0.025), MAT2Y(N-0.025),1);
    insere_parede(li, MAT2X(0.025), MAT2Y(0.025),MAT2X(N-0.025), MAT2Y(0.025),0);

    free(arv);
    return li;
}

