#include "labirinto3D.h"
#include "Grafo.h"
#include "SOIL.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//=======================================================
// Tamanho da matriz do labirinto
#define N 5
// Tamanho de cada bloco da matriz na tela
#define TAM (2.0f / N)
//Funções que convertem a linha e coluna da matriz em uma coordenada de [-1,1]
#define MAT2X(j) ((j)* TAM - 1.0)
#define MAT2Y(i) (1.0 - (i) * TAM)

//=======================================================
#define GIRO_VAL 30
#define GIRO_ANG (18.0/GIRO_VAL)
#define MAX_DIST_PAREDE 0.01f
#define incr 0.01f
#define espessura 0.005f

#define NanguloZ 20
const int anguloZTopo[20] = {0, 18, 36, 54, 72,
                             90, 108, 126, 144, 162,
                             180, 198, 216, 234, 252,
                             270, 288, 306, 324, 342};

const int anguloZPersp[20] = {180, 198, 216, 234, 252,
                              270, 288, 306, 324, 342,
                              0, 18, 36, 54, 72,
                              90, 108, 126, 144, 162};

const double incrX[20] = {-0.0000,-0.0031,-0.0059,-0.0081,-0.0095,
                          -0.0100,-0.0095,-0.0081,-0.0059,-0.0031,
                           0.0000, 0.0031, 0.0059, 0.0081, 0.0095,
                           0.0100, 0.0095, 0.0081, 0.0059, 0.0031};

const double incrY[20] = {-0.0100,-0.0095,-0.0081,-0.0059,-0.0031,
                           0.0000, 0.0031, 0.0059, 0.0081, 0.0095,
                           0.0100, 0.0095, 0.0081, 0.0059, 0.0031,
                           0.0000,-0.0031,-0.0059,-0.0081,-0.0095};

#define Npasso 21
const double passo[21] = {-0.01836f, -0.01820f, -0.01780f, -0.01724f, -0.01652f, -0.01564f,
                          -0.01484f, -0.01412f, -0.01348f, -0.01316f, -0.01300f, -0.01316f,
                          -0.01348f, -0.01412f, -0.01484f, -0.01564f, -0.01652f, -0.01724f,
                          -0.01780f, -0.01820f, -0.01836f};

//=======================================================
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

void viewport_topo(labirinto3D* lab3d);
void viewport_perspectiva(labirinto3D* lab3d);

void draw_obj();
void draw_topo(struct lista_paredes *paredes);
void draw_perspectiva(struct lista_paredes *paredes);

double distanciaSegmentoReta(double xk, double yk, struct coord *p);
int pontoDentroSegmentoReta(double xk, double yk, struct coord *p, int tipo_parede);
struct lista_paredes *monta_labirinto();

static GLuint carregaArqTextura(char *str);
void desenha_retangulo_paredes(struct coord *p);

void desenhaBlocosParede(double x1, double y1, double x2, double y2);
void desenhaParedeTextura(double x1, double y1, double x2, double y2);

//==============================================================
// Texturas
//==============================================================
int idx_textura_chao = 0;
int idx_textura_parede = 1;

GLuint paredeTex2d[5];
GLuint chaoTex2d[3];
GLuint tetoTex2d;

void troca_textura(){
    idx_textura_chao = rand() % 3;
    idx_textura_parede = rand() % 5;
}


void carregaTexturas(){
    paredeTex2d[0] = carregaArqTextura(".//Texturas//parede0.png");
    paredeTex2d[1] = carregaArqTextura(".//Texturas//parede1.png");
    paredeTex2d[2] = carregaArqTextura(".//Texturas//parede2.png");
    paredeTex2d[3] = carregaArqTextura(".//Texturas//parede3.png");
    paredeTex2d[4] = carregaArqTextura(".//Texturas//parede4.png");

    chaoTex2d[0] = carregaArqTextura(".//Texturas//chao0.png");
    chaoTex2d[1] = carregaArqTextura(".//Texturas//chao1.png");
    chaoTex2d[2] = carregaArqTextura(".//Texturas//chao2.png");

    tetoTex2d = carregaArqTextura(".//Texturas//teto1.png");
}

static GLuint carregaArqTextura(char *str){
    // http://www.lonesock.net/soil.html
    GLuint tex = SOIL_load_OGL_texture
        (
            str,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y |
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );

    /* check for an error during the load process */
    if(0 == tex){
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }

    return tex;
}

void desenhaParedeTextura(double x1, double y1, double x2, double y2){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, paredeTex2d[idx_textura_parede]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f); glVertex3f(x1, y1, 0.00f);
        glTexCoord2f(1.0f,0.0f); glVertex3f(x2, y2, 0.00f);
        glTexCoord2f(1.0f,1.0f); glVertex3f(x2, y2, 0.05f);
        glTexCoord2f(0.0f,1.0f); glVertex3f(x1, y1, 0.05f);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void desenhaTetoTextura(){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tetoTex2d);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double x1, y1, di = 0.05f;
    x1 = -1.0f;
    while(x1 < 1.0f){
        y1 = -1.0f;
        while(y1 < 1.0f){
            glBegin(GL_QUADS);
                glNormal3f(0.0f, 0.0f, -1.0f);
                glTexCoord2f(0.0f,0.0f); glVertex3f(x1, y1, 0.05f);
                glTexCoord2f(1.0f,0.0f); glVertex3f(x1, y1+di, 0.05f);
                glTexCoord2f(1.0f,1.0f); glVertex3f(x1+di, y1+di, 0.05f);
                glTexCoord2f(0.0f,1.0f); glVertex3f(x1+di, y1, 0.05f);
            glEnd();

            y1 += di;
        }
        x1 += di;
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void desenhaChaoTextura(){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, chaoTex2d[idx_textura_chao]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double x1, y1, di = 0.01f;
    x1 = -1.0f;
    while(x1 < 1.0f){
        y1 = -1.0f;
        while(y1 < 1.0f){
            glBegin(GL_QUADS);
                glNormal3f(0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f,0.0f); glVertex3f(x1, y1, 0.0f);
                glTexCoord2f(1.0f,0.0f); glVertex3f(x1, y1+di, 0.0f);
                glTexCoord2f(1.0f,1.0f); glVertex3f(x1+di, y1+di, 0.0f);
                glTexCoord2f(0.0f,1.0f); glVertex3f(x1+di, y1, 0.0f);
            glEnd();

            y1 += di;
        }
        x1 += di;
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void desenhaBlocosParede(double x1, double y1, double x2, double y2){
    double di = 0.05f;
    if(x2-x1 >= 0.01f){
        while(x1 < x2){
            if(x1+di > x2)
                desenhaParedeTextura(x1, y1, x2, y2);
            else
                desenhaParedeTextura(x1, y1, x1+di, y2);
            x1 = x1 + di;
        }
    }else{
        while(y1 < y2){
            if(y1+di > y2)
                desenhaParedeTextura(x1, y1, x2, y2);
            else
                desenhaParedeTextura(x1, y1, x2, y1+di);
            y1 = y1 + di;
        }
    }
}

//=========================================================

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
    if(lab3d == NULL || lab3d->giro > 0)
        return;

    caminhaXY(lab3d, incrX[lab3d->idx_anguloZ], incrY[lab3d->idx_anguloZ], 1);
}

void caminhaPraTras(labirinto3D* lab3d){
    if(lab3d == NULL || lab3d->giro > 0)
        return;

    caminhaXY(lab3d, -incrX[lab3d->idx_anguloZ], -incrY[lab3d->idx_anguloZ], 0);
}

void desenha_labirinto3d(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    glDisable(GL_LIGHTING);
    viewport_topo(lab3d);

    glEnable(GL_LIGHTING);
    viewport_perspectiva(lab3d);
    glDisable(GL_LIGHTING);
}

//=======================================================
void viewport_perspectiva(labirinto3D* lab3d){
    if(lab3d == NULL)
        return;

    glViewport(0,0,tamanho,tamanho);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0,1.0,0.01,200.0);

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
    desenhaChaoTextura();
    desenhaTetoTextura();
    int i;
    glColor3f(0.0,0.0,1.0);
    glLineWidth(2);
    for(i=0; i< paredes->qtd; i++){
        glColor3f(0.0f, 0.0f, 0.7f);
        desenha_retangulo_paredes(&(paredes->CO[i]));
    }
}

void desenha_retangulo_paredes(struct coord *p){
    if(p->tipo == 0){// parede horizontal
        double x1, x2;
        if(p->x1 < p->x2){
            x1 = p->x1 - espessura;
            x2 = p->x2 + espessura;
        }else{
            x1 = p->x1 + espessura;
            x2 = p->x2 - espessura;
        }

        glNormal3f(0.0f, 1.0f, 0.0f);
        desenhaBlocosParede(x1, p->y1+espessura, x2, p->y2+espessura);

        glNormal3f(0.0f, -1.0f, 0.0f);
        desenhaBlocosParede(x1,p->y1-espessura,x2,p->y2-espessura);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        desenhaBlocosParede(x1,p->y1-espessura,x1,p->y1+espessura);

        glNormal3f(1.0f, 0.0f, 0.0f);
        desenhaBlocosParede(x2,p->y2-espessura,x2,p->y2+espessura);

    }else{//parede vertical
        double y1, y2;
        if(p->y1 < p->y2){
            y1 = p->y1 - espessura;
            y2 = p->y2 + espessura;
        }else{
            y1 = p->y1 + espessura;
            y2 = p->y2 - espessura;
        }
        glNormal3f(1.0f, 0.0f, 0.0f);
        desenhaBlocosParede(p->x1+espessura,y1,p->x2+espessura,y2);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        desenhaBlocosParede(p->x1-espessura,y1,p->x2-espessura,y2);

        glNormal3f(0.0f, -1.0f, 0.0f);
        desenhaBlocosParede(p->x1-espessura,y1,p->x1+espessura,y1);

        glNormal3f(0.0f, 1.0f, 0.0f);
        desenhaBlocosParede(p->x2-espessura,y2,p->x2+espessura,y2);
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

    double temp;
    if(x1 > x2){
        temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if(y1 > y2){
        temp = y1;
        y1 = y2;
        y2 = temp;
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

