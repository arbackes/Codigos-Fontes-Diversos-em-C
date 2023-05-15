#include <gl/gl.h>

typedef struct labirinto LABIRINTO;

void desenha_circulo(GLfloat x, GLfloat y, GLfloat radius);
void desenha_linha(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, int cor);

LABIRINTO* monta_labirinto();
void destroi_labirinto(LABIRINTO* lab);

void desenha_arvore_geradora(int *arv);
void desenha_paredes_labirinto(int *arv);
void desenha_labirinto(LABIRINTO *lab,int tipo);

void inicia_animacao(LABIRINTO *lab);
void desenha_procura_caminho(LABIRINTO *lab);
void desenha_menor_caminho(LABIRINTO *lab);

void desenha_busca_largura(LABIRINTO *lab);

