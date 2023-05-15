#include "labirinto3D.h"
#include <stdio.h>
#include <math.h>
//=======================================================
#define GIRO_VAL 300
#define GIRO_ANG (90.0/GIRO_VAL)
#define MAX_DIST_PAREDE 0.01f
#define incr 0.01

//=======================================================
struct coord{
    int tipo; //0 horizontal
    double x1, y1, x2, y2;
};
int Nparedes = 4;
struct coord paredes[4] = {{0,-0.75f, 0.75f, 0.75f, 0.75f},
                           {0,-0.75f,-0.75f, 0.75f,-0.75f},
                           {1, 0.75f, 0.75f, 0.75f,-0.75f},
                           {1,-0.75f, 0.75f,-0.75f,-0.75f}};


double posXY[2] = {0.0f,0.0f};
int anguloZTopo[4] = {0,90,180,270};
int anguloZPersp[4] = {180,270,0,90};
int idx_anguloZ = 0;
int idx_anguloZ_ant = 0;
int giro = 0, giro_dir = 1;

int Npasso = 21, idx_passo = 10;
const double passo[21] = {-0.0167f, -0.0165f, -0.0160f, -0.0153f, -0.0144f, -0.0133f,
                          -0.0123f, -0.0114f, -0.0106f, -0.0102f, -0.0100f, -0.0102f,
                          -0.0106f, -0.0114f, -0.0123f, -0.0133f, -0.0144f, -0.0153f,
                          -0.0160f, -0.0165f, -0.0167};



//=======================================================
void viraEsquerda(){
    if(giro == 0){
        giro = GIRO_VAL;
        giro_dir = -1;
        idx_anguloZ_ant = idx_anguloZ;

        idx_anguloZ--;
        if(idx_anguloZ < 0)
            idx_anguloZ = 3;
    }
}

void viraDireita(){
    if(giro == 0){
        giro = GIRO_VAL;
        giro_dir = +1;
        idx_anguloZ_ant = idx_anguloZ;

        idx_anguloZ = (idx_anguloZ + 1) % 4;
    }
}

void caminhaXY(double xk, double yk, int frente){
    int i, caminha = 1;
    int tipo_parede = idx_anguloZ % 2;
    double numerador, denominador;

    for(i = 0; i < Nparedes; i++){
        if(paredes[i].tipo == tipo_parede){
            numerador = (xk - paredes[i].x1) * (paredes[i].y2 - paredes[i].y1) -
                        (yk - paredes[i].y1) * (paredes[i].x2 - paredes[i].x1);
            numerador = numerador * numerador;

            denominador = (paredes[i].x1 - paredes[i].x2) * (paredes[i].x1 - paredes[i].x2) +
                          (paredes[i].y1 - paredes[i].y2) * (paredes[i].y1 - paredes[i].y2);

            if(fabs(numerador / denominador) < MAX_DIST_PAREDE){
                caminha = 0;
                break;
            }
        }
    }

    if(caminha){
        posXY[0] = xk;
        posXY[1] = yk;
        if(frente)
            idx_passo = (idx_passo + 1) % Npasso;
        else{
            idx_passo--;
            if(idx_passo < 0)
                idx_passo = Npasso - 1;
        }

    }
}

void caminhaPraFrente(){
    if(giro > 0)
        return;

    switch(idx_anguloZ){
        case 0: caminhaXY(posXY[0],posXY[1]-incr, 1); break;
        case 1: caminhaXY(posXY[0]-incr,posXY[1], 1); break;
        case 2: caminhaXY(posXY[0],posXY[1]+incr, 1); break;
        case 3: caminhaXY(posXY[0]+incr,posXY[1], 1); break;
    }
}
void caminhaPraTras(){
    if(giro > 0)
        return;

    switch(idx_anguloZ){
        case 0: caminhaXY(posXY[0],posXY[1]+incr, 0); break;
        case 1: caminhaXY(posXY[0]+incr,posXY[1], 0); break;
        case 2: caminhaXY(posXY[0],posXY[1]-incr, 0); break;
        case 3: caminhaXY(posXY[0]-incr,posXY[1], 0); break;
    }

}

//=======================================================
void viewport_perspectiva(){
    glViewport(0,0,tamanho,tamanho);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    gluPerspective(45.0,1.0,0.01,200.0);// Calculate The Aspect Ratio Of The Window

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glRotatef(-90, 1.0f, 0.0f, 0.0f); //ajusta a visão

    if(giro > 0){
        giro--;
        int var_ang = giro_dir * (GIRO_VAL - giro) * GIRO_ANG;
        glRotatef(var_ang + anguloZPersp[idx_anguloZ_ant], 0.0f, 0.0f, 1.0f); //vira nos corredores
    }else
        glRotatef(anguloZPersp[idx_anguloZ], 0.0f, 0.0f, 1.0f);

    glPushMatrix();
        glTranslatef(-posXY[0],-posXY[1],passo[idx_passo]);
        draw_perspectiva();
    glPopMatrix();
}

void draw_perspectiva(){
    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);     // plane
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);     //Red
        glVertex3f(-0.75f,0.75f,0.05f);
        glVertex3f(0.75f,0.75f,0.05f);
        glVertex3f(0.75f,0.75f,0.0f);
        glVertex3f(-0.75f,0.75f,0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 1.0f);     //Blue
        glVertex3f(-0.75f,-0.75f,0.05f);
        glVertex3f(0.75f,-0.75f,0.05f);
        glVertex3f(0.75f,-0.75f,0.0f);
        glVertex3f(-0.75f,-0.75f,0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);     //Green
        glVertex3f(0.75f,0.75f,0.05f);
        glVertex3f(0.75f,-0.75f,0.05f);
        glVertex3f(0.75f,-0.75f,0.0f);
        glVertex3f(0.75f,0.75f,0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);     //Yellow
        glVertex3f(-0.75f,0.75f,0.05f);
        glVertex3f(-0.75f,-0.75f,0.05f);
        glVertex3f(-0.75f,-0.75f,0.0f);
        glVertex3f(-0.75f,0.75f,0.0f);
    glEnd();
}

//=======================================================
void viewport_topo(){
    glViewport(3*tamanho/4.0 - 20,5,tamanho/4,tamanho/4);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glPushMatrix();
        draw_topo();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(posXY[0],posXY[1],0.0f);
        if(giro > 0){
            giro--;
            int var_ang = giro_dir * (GIRO_VAL - giro) * GIRO_ANG;
            glRotatef(-(var_ang + anguloZTopo[idx_anguloZ_ant]), 0.0f, 0.0f, 1.0f); //vira nos corredores
        }else
            glRotatef(-anguloZTopo[idx_anguloZ], 0.0f, 0.0f, 1.0f); //vira nos corredores

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

void draw_topo(){
    glLineWidth(2);
    glColor3f(0.0f, 0.0f, 0.0f);     // contorno
    glBegin(GL_LINE_LOOP);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.95f, 0.95f, 0.95f);     // plane
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);     //Red
        glVertex3f(-0.75f,0.75f,-0.05f);
        glVertex3f(0.75f,0.75f,-0.05f);
        glVertex3f(0.75f,0.75f-0.05f,-0.05f);
        glVertex3f(-0.75f,0.75f-0.05f,-0.05f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 1.0f);     //Blue
        glVertex3f(-0.75f,-0.75f,-0.05f);
        glVertex3f(0.75f,-0.75f,-0.05f);
        glVertex3f(0.75f,-0.75f+0.05f,-0.05f);
        glVertex3f(-0.75f,-0.75f+0.05f,-0.05f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);     //Green
        glVertex3f(0.75f,0.75f,-0.05f);
        glVertex3f(0.75f,-0.75f,-0.05f);
        glVertex3f(0.75f-0.05f,-0.75f,-0.05f);
        glVertex3f(0.75f-0.05f,0.75f,-0.05f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);     //Yellow
        glVertex3f(-0.75f,0.75f,-0.05f);
        glVertex3f(-0.75f,-0.75f,-0.05f);
        glVertex3f(-0.75f+0.05f,-0.75f,-0.05f);
        glVertex3f(-0.75f+0.05f,0.75f,-0.05f);
    glEnd();

}

