#include "labirinto3D.h"
#include <stdio.h>


//=======================================================
//=======================================================
double posXY[2] = {0.0f,0.0f};
int anguloZ[4] = {0,90,180,270};
int anguloZPersp[4] = {180,270,0,90};
int idx_anguloZ = 0;

void viraEsquerda(){
    idx_anguloZ--;
    if(idx_anguloZ < 0)
        idx_anguloZ = 3;
}

void viraDireita(){
    idx_anguloZ = (idx_anguloZ + 1) % 4;
}

void caminhaPraFrente(){
    switch(idx_anguloZ){
        case 0: posXY[1] -= incr; break;
        case 1: posXY[0] -= incr; break;
        case 2: posXY[1] += incr; break;
        case 3: posXY[0] += incr; break;
    }
    printf("Angulo %d) %f %f\n",idx_anguloZ,posXY[0],posXY[1]);
}

void caminhaPraTras(){
    switch(idx_anguloZ){
        case 0: posXY[1] += incr; break;
        case 1: posXY[0] += incr; break;
        case 2: posXY[1] -= incr; break;
        case 3: posXY[0] -= incr; break;
    }
    printf("Angulo %d) %f %f\n",idx_anguloZ,posXY[0],posXY[1]);
}

//=======================================================
//=======================================================

void viewport_topo(){
    //glViewport(3*tamanho/4.0 - 15,0,tamanho/4,tamanho/4);
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
        glRotatef(-anguloZ[idx_anguloZ], 0.0f, 0.0f, 1.0f); //vira nos corredores
        draw_obj();
    glPopMatrix();
}

void viewport_perspectiva(){
    glViewport(0,0,tamanho,tamanho);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    gluPerspective(45.0,1.0,0.01,200.0);// Calculate The Aspect Ratio Of The Window

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glRotatef(-90, 1.0f, 0.0f, 0.0f); //ajusta a visão
    glRotatef(anguloZPersp[idx_anguloZ], 0.0f, 0.0f, 1.0f); //vira nos corredores
    glPushMatrix();
        glTranslatef(-posXY[0],-posXY[1],-0.01f);
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
        glVertex3f(-sizeY,sizeY,sizeX);
        glVertex3f(sizeY,sizeY,sizeX);
        glVertex3f(sizeY,sizeY,0.0f);
        glVertex3f(-sizeY,sizeY,0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 1.0f);     //Blue
        glVertex3f(-sizeY,-sizeY,sizeX);
        glVertex3f(sizeY,-sizeY,sizeX);
        glVertex3f(sizeY,-sizeY,0.0f);
        glVertex3f(-sizeY,-sizeY,0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);     //Green
        glVertex3f(sizeY,sizeY,sizeX);
        glVertex3f(sizeY,-sizeY,sizeX);
        glVertex3f(sizeY,-sizeY,0.0f);
        glVertex3f(sizeY,sizeY,0.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);     //Yellow
        glVertex3f(-sizeY,sizeY,sizeX);
        glVertex3f(-sizeY,-sizeY,sizeX);
        glVertex3f(-sizeY,-sizeY,0.0f);
        glVertex3f(-sizeY,sizeY,0.0f);
    glEnd();
}
void draw_obj(){
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);     // plane gray
        glVertex3f( sizeX, sizeX, -sizeX);
        glVertex3f( -sizeX,sizeX, -sizeX);
        glVertex3f( 0.0f,-2*sizeX, -sizeX);
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
        glVertex3f(-sizeY,sizeY,-sizeX);
        glVertex3f(sizeY,sizeY,-sizeX);
        glVertex3f(sizeY,sizeY-sizeX,-sizeX);
        glVertex3f(-sizeY,sizeY-sizeX,-sizeX);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 1.0f);     //Blue
        glVertex3f(-sizeY,-sizeY,-sizeX);
        glVertex3f(sizeY,-sizeY,-sizeX);
        glVertex3f(sizeY,-sizeY+sizeX,-sizeX);
        glVertex3f(-sizeY,-sizeY+sizeX,-sizeX);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.0f, 1.0f, 0.0f);     //Green
        glVertex3f(sizeY,sizeY,-sizeX);
        glVertex3f(sizeY,-sizeY,-sizeX);
        glVertex3f(sizeY-sizeX,-sizeY,-sizeX);
        glVertex3f(sizeY-sizeX,sizeY,-sizeX);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 0.0f);     //Yellow
        glVertex3f(-sizeY,sizeY,-sizeX);
        glVertex3f(-sizeY,-sizeY,-sizeX);
        glVertex3f(-sizeY+sizeX,-sizeY,-sizeX);
        glVertex3f(-sizeY+sizeX,sizeY,-sizeX);
    glEnd();

}

