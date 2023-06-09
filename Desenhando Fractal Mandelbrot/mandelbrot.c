#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <gl/gl.h>
#include <complex.h>
#include "mandelbrot.h"

// Tamanho da matriz
#define TAM_GRID 500
// Nro m�ximo de itera��es
#define maxIterations 400
// Tamanho de cada bloco da matriz na tela
#define TAM_BLOCO (2.0 / TAM_GRID)
//Fun��es que convertem a linha e coluna da matriz em uma coordenada de [-1,1]
#define MAT2X(j) ((j) * (TAM_BLOCO) - 1)
#define MAT2Y(i) (1 - (i) * (TAM_BLOCO))

struct mandelbrot_set{
    double x_ini;
    double y_ini;
    double passo;
    double incr;
    int contador[TAM_GRID][TAM_GRID];
    int menor, maior;
    int cor, coord;
};

MANDELBROT* inicializa(){
    MANDELBROT* M = malloc(sizeof(MANDELBROT));
    if(M != NULL){
        M->cor = 0;
        M->coord = 1;
        M->x_ini = -0.3787675;
        M->y_ini = +0.6212361;
        M->passo = 5e-7;
        M->incr = M->passo / TAM_GRID;
    }
    return M;
}

void muda_cor(MANDELBROT* M){
    M->cor = !M->cor;
}

void muda_coordenada(MANDELBROT* M){
    M->coord = !M->coord;
    if(M->coord){
        M->x_ini = -0.3787675;
        M->y_ini = +0.6212361;
        M->passo = 5e-7;
    }else{
        M->x_ini = -2;
        M->y_ini = -2;
        M->passo = 4;
    }
    M->incr = M->passo / TAM_GRID;
    calcula(M);
}

double cores1[64][3]={{0.0000,0.0000,0.0052},
                    {0.0139,0.0139,0.0243},
                    {0.0278,0.0278,0.0434},
                    {0.0417,0.0417,0.0625},
                    {0.0556,0.0556,0.0816},
                    {0.0694,0.0694,0.1007},
                    {0.0833,0.0833,0.1198},
                    {0.0972,0.0972,0.1389},
                    {0.1111,0.1111,0.1580},
                    {0.1250,0.1250,0.1771},
                    {0.1389,0.1389,0.1962},
                    {0.1528,0.1528,0.2153},
                    {0.1667,0.1667,0.2344},
                    {0.1806,0.1806,0.2535},
                    {0.1944,0.1944,0.2726},
                    {0.2083,0.2083,0.2917},
                    {0.2222,0.2222,0.3108},
                    {0.2361,0.2361,0.3299},
                    {0.2500,0.2500,0.3490},
                    {0.2639,0.2639,0.3681},
                    {0.2778,0.2778,0.3872},
                    {0.2917,0.2917,0.4062},
                    {0.3056,0.3056,0.4253},
                    {0.3194,0.3194,0.4444},
                    {0.3333,0.3385,0.4583},
                    {0.3472,0.3576,0.4722},
                    {0.3611,0.3767,0.4861},
                    {0.3750,0.3958,0.5000},
                    {0.3889,0.4149,0.5139},
                    {0.4028,0.4340,0.5278},
                    {0.4167,0.4531,0.5417},
                    {0.4306,0.4722,0.5556},
                    {0.4444,0.4913,0.5694},
                    {0.4583,0.5104,0.5833},
                    {0.4722,0.5295,0.5972},
                    {0.4861,0.5486,0.6111},
                    {0.5000,0.5677,0.6250},
                    {0.5139,0.5868,0.6389},
                    {0.5278,0.6059,0.6528},
                    {0.5417,0.6250,0.6667},
                    {0.5556,0.6441,0.6806},
                    {0.5694,0.6632,0.6944},
                    {0.5833,0.6823,0.7083},
                    {0.5972,0.7014,0.7222},
                    {0.6111,0.7205,0.7361},
                    {0.6250,0.7396,0.7500},
                    {0.6389,0.7587,0.7639},
                    {0.6528,0.7778,0.7778},
                    {0.6745,0.7917,0.7917},
                    {0.6962,0.8056,0.8056},
                    {0.7179,0.8194,0.8194},
                    {0.7396,0.8333,0.8333},
                    {0.7613,0.8472,0.8472},
                    {0.7830,0.8611,0.8611},
                    {0.8047,0.8750,0.8750},
                    {0.8264,0.8889,0.8889},
                    {0.8481,0.9028,0.9028},
                    {0.8698,0.9167,0.9167},
                    {0.8915,0.9306,0.9306},
                    {0.9132,0.9444,0.9444},
                    {0.9349,0.9583,0.9583},
                    {0.9566,0.9722,0.9722},
                    {0.9783,0.9861,0.9861},
                    {1.0000,1.0000,1.0000}};


double cores2[64][3]={{0.0000, 0.0000, 0.5625},
                    {0.0000, 0.0000, 0.6250},
                    {0.0000, 0.0000, 0.6875},
                    {0.0000, 0.0000, 0.7500},
                    {0.0000, 0.0000, 0.8125},
                    {0.0000, 0.0000, 0.8750},
                    {0.0000, 0.0000, 0.9375},
                    {0.0000, 0.0000, 1.0000},
                    {0.0000, 0.0625, 1.0000},
                    {0.0000, 0.1250, 1.0000},
                    {0.0000, 0.1875, 1.0000},
                    {0.0000, 0.2500, 1.0000},
                    {0.0000, 0.3125, 1.0000},
                    {0.0000, 0.3750, 1.0000},
                    {0.0000, 0.4375, 1.0000},
                    {0.0000, 0.5000, 1.0000},
                    {0.0000, 0.5625, 1.0000},
                    {0.0000, 0.6250, 1.0000},
                    {0.0000, 0.6875, 1.0000},
                    {0.0000, 0.7500, 1.0000},
                    {0.0000, 0.8125, 1.0000},
                    {0.0000, 0.8750, 1.0000},
                    {0.0000, 0.9375, 1.0000},
                    {0.0000, 1.0000, 1.0000},
                    {0.0625, 1.0000, 0.9375},
                    {0.1250, 1.0000, 0.8750},
                    {0.1875, 1.0000, 0.8125},
                    {0.2500, 1.0000, 0.7500},
                    {0.3125, 1.0000, 0.6875},
                    {0.3750, 1.0000, 0.6250},
                    {0.4375, 1.0000, 0.5625},
                    {0.5000, 1.0000, 0.5000},
                    {0.5625, 1.0000, 0.4375},
                    {0.6250, 1.0000, 0.3750},
                    {0.6875, 1.0000, 0.3125},
                    {0.7500, 1.0000, 0.2500},
                    {0.8125, 1.0000, 0.1875},
                    {0.8750, 1.0000, 0.1250},
                    {0.9375, 1.0000, 0.0625},
                    {1.0000, 1.0000, 0.0000},
                    {1.0000, 0.9375, 0.0000},
                    {1.0000, 0.8750, 0.0000},
                    {1.0000, 0.8125, 0.0000},
                    {1.0000, 0.7500, 0.0000},
                    {1.0000, 0.6875, 0.0000},
                    {1.0000, 0.6250, 0.0000},
                    {1.0000, 0.5625, 0.0000},
                    {1.0000, 0.5000, 0.0000},
                    {1.0000, 0.4375, 0.0000},
                    {1.0000, 0.3750, 0.0000},
                    {1.0000, 0.3125, 0.0000},
                    {1.0000, 0.2500, 0.0000},
                    {1.0000, 0.1875, 0.0000},
                    {1.0000, 0.1250, 0.0000},
                    {1.0000, 0.0625, 0.0000},
                    {1.0000, 0.0000, 0.0000},
                    {0.9375, 0.0000, 0.0000},
                    {0.8750, 0.0000, 0.0000},
                    {0.8125, 0.0000, 0.0000},
                    {0.7500, 0.0000, 0.0000},
                    {0.6875, 0.0000, 0.0000},
                    {0.6250, 0.0000, 0.0000},
                    {0.5625, 0.0000, 0.0000},
                    {0.5000, 0.0000, 0.0000}};

void finaliza(MANDELBROT* M){
    free(M);
}

void calcula(MANDELBROT* M){
    int x, y, iter;
    double py, px;
    _Complex double z0, z;
    py = M->y_ini;
    for(y = 0; y < TAM_GRID; y++){
        px = M->x_ini;
        for(x = 0; x < TAM_GRID; x++){
            //zera o contador
            M->contador[y][x] = 0;
            z0 = px + py * _Complex_I;
            z = z0;
            for(iter = 0; iter < maxIterations; iter++){
                z = z * z + z0;
                if(cabs(z) <= 2.0)
                    M->contador[y][x]++;
            }
            px += M->incr;
        }
        py += M->incr;
    }
    //calcula maior e menor valores
    M->menor = M->maior = M->contador[0][0];
    for(y = 0; y < TAM_GRID; y++){
        for(x = 0; x < TAM_GRID; x++){
            if(M->contador[y][x] > M->maior)
                M->maior = M->contador[y][x];

            if(M->contador[y][x] < M->menor)
                M->menor = M->contador[y][x];
        }
    }
}

void desenhaQuadrado(float coluna,float linha, double R, double G, double B){
    glColor3f(R, G, B);
    glBegin(GL_QUADS);
        glVertex2f(coluna, linha);
        glVertex2f(coluna+TAM_BLOCO, linha);
        glVertex2f(coluna+TAM_BLOCO, linha+TAM_BLOCO);
        glVertex2f(coluna, linha+TAM_BLOCO);
    glEnd();
}

void desenha(MANDELBROT* M){
    int x, y, idx_cor;
    for(y = 0; y < TAM_GRID; y++){
        for(x = 0; x < TAM_GRID; x++){
            idx_cor = 63 * ((M->contador[y][x] - M->menor) / (double)(M->maior - M->menor));
            if(idx_cor > 0)
                if(M->cor)
                    desenhaQuadrado(MAT2X(x),MAT2Y(y),
                                    cores1[idx_cor][0],cores1[idx_cor][1],cores1[idx_cor][2]);
                else
                    desenhaQuadrado(MAT2X(x),MAT2Y(y),
                                    cores2[idx_cor][0],cores2[idx_cor][1],cores2[idx_cor][2]);
        }
    }
}
