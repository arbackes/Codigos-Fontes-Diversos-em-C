#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <gl/gl.h>
#include "SOIL.h"
#include "pacman.h"

//=========================================================
// Tamanho de cada bloco da matriz do jogo
#define bloco 70
// Tamanho da matriz do jogo
#define N 20
// Tamanho de cada bloco da matriz do jogo na tela
#define TAM 0.1f
//Funções que convertem a linha e coluna da matriz em uma coordenada de [-1,1]
#define MAT2X(j) ((j)*0.1f-1)
#define MAT2Y(i) (0.9-(i)*0.1f)

//=========================================================
// Estruturas usadas para controlar o jogo
struct TPoint{
    int x,y;
};

const struct TPoint direcoes[4] = {{1,0},{0,1},{-1,0},{0,-1}};

struct TPacman{
    int status;
    int xi,yi,x,y;
    int direcao,passo,parcial;
    int pontos;
    int invencivel;
    int vivo;
    int animacao;
};

struct TPhantom{
    int status;
    int xi,yi,x,y;
    int direcao,passo,parcial;
	int decidiu_cruzamento,iniciou_volta;
	int indice_atual;
	int *caminho;
};

struct TVertice{
    int x,y;
    int vizinhos[4];
};

struct TCenario{
    int mapa[N][N];
    int nro_pastilhas;
    int NV;
    struct TVertice *grafo;
};

//==============================================================
// Texturas
//==============================================================

GLuint pacmanTex2d[12];
GLuint phantomTex2d[12];
GLuint mapaTex2d[14];

GLuint telaStart, telaGameOver;

static void desenhaSprite(float coluna,float linha, GLuint tex);
static GLuint carregaArqTextura(char *str);
static void desenhaTipoTela(float x, float y, float tamanho, GLuint tex);

// Função que carrega todas as texturas do jogo
void carregaTexturas(){
    int i;
    char str[50];
    for(i=0; i<12; i++){
        sprintf(str,".//Sprites//phantom%d.png",i);
        phantomTex2d[i] = carregaArqTextura(str);
    }

    for(i=0; i<12; i++){
        sprintf(str,".//Sprites//pacman%d.png",i);
        pacmanTex2d[i] = carregaArqTextura(str);
    }

    for(i=0; i<14; i++){
        sprintf(str,".//Sprites//mapa%d.png",i);
        mapaTex2d[i] = carregaArqTextura(str);
    }

    telaStart = carregaArqTextura(".//Sprites//start.png");
    telaGameOver = carregaArqTextura(".//Sprites//gameover.png");;

}

// Função que carrega um arquivo de textura do jogo
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

// Função que recebe uma linha e coluna da matriz e um código
// de textura e desenha um quadrado na tela com essa textura
void desenhaSprite(float coluna,float linha, GLuint tex){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f); glVertex2f(coluna, linha);
        glTexCoord2f(1.0f,0.0f); glVertex2f(coluna+TAM, linha);
        glTexCoord2f(1.0f,1.0f); glVertex2f(coluna+TAM, linha+TAM);
        glTexCoord2f(0.0f,1.0f); glVertex2f(coluna, linha+TAM);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

}

void desenhaTipoTela(float x, float y, float tamanho, GLuint tex){

    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,1.0f); glVertex2f(x - tamanho, y + tamanho);
        glTexCoord2f(1.0f,1.0f); glVertex2f(x + tamanho, y + tamanho);
        glTexCoord2f(1.0f,0.0f); glVertex2f(x + tamanho, y - tamanho);
        glTexCoord2f(0.0f,0.0f); glVertex2f(x - tamanho, y - tamanho);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void desenhaTela(int tipo){
    if(tipo == 0)
        desenhaTipoTela(0, 0, 1.0, telaStart);
    else
        desenhaTipoTela(0, 0, 1.0, telaGameOver);
}

//==============================================================
// Cenario
//==============================================================

static int cenario_EhCruzamento(int x, int y, Cenario* cen);
static int cenario_VerificaDirecao(int mat[N][N], int y, int x, int direcao);
static void cenario_constroiGrafo(Cenario* cen);

// Função que carrega os dados do cenário de um arquivo texto
Cenario* cenario_carrega(char *arquivo){
    int i,j;

    FILE *arq = fopen(arquivo,"r");

    if(arq == NULL){
        printf("Erro ao carregar cenario!\n");
        system("pause");
        exit(1);
    }

    Cenario* cen = malloc(sizeof(Cenario));
    cen->nro_pastilhas = 0;

    for(i=0; i<N; i++)
        for(j=0; j<N; j++){
            fscanf(arq,"%d",&cen->mapa[i][j]);
            if(cen->mapa[i][j] == 1 || cen->mapa[i][j] == 2)
                cen->nro_pastilhas++;
        }

    fclose(arq);

    cenario_constroiGrafo(cen);

    return cen;
}

// Libera os dados associados ao cenário
void cenario_destroy(Cenario* cen){
    free(cen->grafo);
    free(cen);
}

// Percorre a matriz do jogo desenhando os sprites
void cenario_desenha(Cenario* cen){
    int i,j;
    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            desenhaSprite(MAT2X(j),MAT2Y(i),mapaTex2d[cen->mapa[i][j]]);
}

// Verifica se uma posição (x,y) do cenário é um cruzamento e não uma parede
static int cenario_EhCruzamento(int x, int y, Cenario* cen){
    int i, cont = 0;
    int v[4];

    for(i=0; i<4; i++){
        if(cen->mapa[y + direcoes[i].y][x + direcoes[i].x] <=2){//não é parede...
            cont++;
            v[i] = 1;
        }else{
            v[i] = 0;
        }
    }
    if(cont > 1){
        if(cont == 2){
            if((v[0] == v[2] && v[0]) || (v[1] == v[3] && v[1]))
                return 0;
            else
                return 1;

        }else
            return 1;
    }else
        return 0;
}

// Função que verifica se é possivel andar em uma determinada direção
static int cenario_VerificaDirecao(int mat[N][N], int y, int x, int direcao){
    int xt = x;
    int yt = y;
    while(mat[yt + direcoes[direcao].y][xt + direcoes[direcao].x] == 0){ //não é parede...
        yt = yt + direcoes[direcao].y;
        xt = xt + direcoes[direcao].x;
    }

    if(mat[yt + direcoes[direcao].y][xt + direcoes[direcao].x] < 0)
        return -1;
    else
        return mat[yt + direcoes[direcao].y][xt + direcoes[direcao].x] - 1;
}

// Dado um cenário, monta o grafo que ajudará os fantasmas
// a voltarem para o ponto de início no jogo
static void cenario_constroiGrafo(Cenario* cen){
    int mat[N][N];
    int i,j,k, idx, cont = 0;

    for(i=1; i<N-1; i++){
        for(j=1; j<N-1; j++){
            if(cen->mapa[i][j] <= 2){//não é parede...
                if(cenario_EhCruzamento(j,i,cen)){
                    cont++;
                    mat[i][j] = cont;
                }else
                    mat[i][j] = 0;
            }else
                mat[i][j] = -1;
        }
    }

    for(i=0; i < N; i++){
        mat[0][i] = -1;
        mat[i][0] = -1;
        mat[N-1][i] = -1;
        mat[i][N-1] = -1;
    }

    cen->NV = cont;
    cen->grafo = malloc(cont * sizeof(struct TVertice));

    for(i=1; i<N-1; i++){
        for(j=1; j<N-1; j++){
            if(mat[i][j] > 0){
                idx = mat[i][j] - 1;
                cen->grafo[idx].x = j;
                cen->grafo[idx].y = i;
                for(k=0; k < 4; k++)
                    cen->grafo[idx].vizinhos[k] = cenario_VerificaDirecao(mat,i,j,k);
            }
        }
    }

}

//==============================================================
// Pacman
//==============================================================

static int pacman_eh_invencivel(Pacman *pac);
static void pacman_morre(Pacman *pac);
static void pacman_pontos_fantasma(Pacman *pac);
static void pacman_AnimacaoMorte(float coluna,float linha,Pacman* pac);

// Função que inicializa os dados associados ao pacman
Pacman* pacman_create(int x, int y){
    Pacman* pac = malloc(sizeof(Pacman));
    if(pac != NULL){
        pac->invencivel = 0;
        pac->pontos = 0;
        pac->passo = 4;
        pac->vivo = 1;
        pac->status = 0;
        pac->direcao = 0;
        pac->parcial = 0;
        pac->xi = x;
        pac->yi = y;
        pac->x = x;
        pac->y = y;
    }
    return pac;
}

// Função que libera os dados associados ao pacman
void pacman_destroy(Pacman *pac){
    free(pac);
}

// Função que verifica se o pacman está vivo ou não
int pacman_vivo(Pacman *pac){
    if(pac->vivo)
        return 1;
    else{
        if(pac->animacao > 60)
            return 0;
        else
            return 1;
    }
}

// Função que verifica se o pacman pode ir para uma nova direção escolhida
void pacman_AlteraDirecao(Pacman *pac, int direcao, Cenario *cen){
    if(cen->mapa[pac->y + direcoes[direcao].y][pac->x + direcoes[direcao].x] <=2){//não é parede...
        int di = abs(direcao - pac->direcao);
        if(di != 2 && di != 0)
            pac->parcial = 0;

        pac->direcao = direcao;
    }
}

// Atualiza a posição do pacman
void pacman_movimenta(Pacman *pac, Cenario *cen){
    if(pac->vivo == 0)
        return;

    // Incrementa a sua posição dentro de uma célula da matriz ou muda de célula
    if(cen->mapa[pac->y + direcoes[pac->direcao].y][pac->x + direcoes[pac->direcao].x] <=2){//não é parede...
        if(pac->direcao < 2){
            pac->parcial += pac->passo;
            if(pac->parcial >= bloco){
                pac->x += direcoes[pac->direcao].x;
                pac->y += direcoes[pac->direcao].y;
                pac->parcial = 0;
            }
        }else{
            pac->parcial -= pac->passo;
            if(pac->parcial <= -bloco){
                pac->x += direcoes[pac->direcao].x;
                pac->y += direcoes[pac->direcao].y;
                pac->parcial = 0;
            }
        }
    }

    // Come uma das pastilhas no mapa
    if(cen->mapa[pac->y][pac->x] == 1){
        pac->pontos += 10;
        cen->nro_pastilhas--;
    }
    if(cen->mapa[pac->y][pac->x] == 2){
        pac->pontos += 50;
        pac->invencivel = 1000;
        cen->nro_pastilhas--;
    }
    //Remove a pastilha comida do mapa
    cen->mapa[pac->y][pac->x] = 0;
}

// Função que desenha o pacman
void pacman_desenha(Pacman *pac){
    float linha, coluna;
    float passo = (pac->parcial/(float)bloco);
    //Verifica a posição
    if(pac->direcao == 0 || pac->direcao == 2){
        linha = pac->y;
        coluna = pac->x + passo;
    }else{
        linha = pac->y + passo;
        coluna = pac->x;
    }

    if(pac->vivo){
        // Escolhe o sprite com base na direção
        int idx = 2*pac->direcao;

        // Escolhe se desenha com boca aberta ou fechada
        if(pac->status < 15)
            desenhaSprite(MAT2X(coluna),MAT2Y(linha), pacmanTex2d[idx]);
        else
            desenhaSprite(MAT2X(coluna),MAT2Y(linha), pacmanTex2d[idx+1]);

        // Alterna entre boca aberta ou fechada
        pac->status = (pac->status+1) % 30;

        if(pac->invencivel > 0)
            pac->invencivel--;
    }else{
        // Mostra animação da morte
        pacman_AnimacaoMorte(coluna,linha,pac);
    }
}

static int pacman_eh_invencivel(Pacman *pac){
    return pac->invencivel > 0;
}

static void pacman_morre(Pacman *pac){
    if(pac->vivo){
        pac->vivo = 0;
        pac->animacao = 0;
    }
}

static void pacman_pontos_fantasma(Pacman *pac){
    pac->pontos += 100;
}

static void pacman_AnimacaoMorte(float coluna,float linha,Pacman* pac){
    pac->animacao++;
    // Verifica qual dos sprites deve ser desenhado para dar o
    // efeito de que o pacman está sumindo quando morre
    if(pac->animacao < 15)
        desenhaSprite(MAT2X(coluna),MAT2Y(linha), pacmanTex2d[8]);
    else
        if(pac->animacao < 30)
            desenhaSprite(MAT2X(coluna),MAT2Y(linha), pacmanTex2d[9]);
        else
            if(pac->animacao < 45)
                desenhaSprite(MAT2X(coluna),MAT2Y(linha), pacmanTex2d[10]);
            else
                desenhaSprite(MAT2X(coluna),MAT2Y(linha), pacmanTex2d[11]);
}

//==============================================================
// Phantom
//==============================================================

static void phantom_move(Phantom *ph, int direcao, Cenario *cen);
static int phantom_DirecaoGrafo(Phantom *ph, Cenario *cen);
static int phantom_DistanciaGrafo(Cenario *cen, int noA, int noB);
static int phantom_SorteiaDirecao(Phantom *ph, Cenario *cen);
static int phantom_VePacman(Phantom *ph, Pacman *pac, Cenario *cen, int direcao);
static void phantom_ProcuraMenorCaminho(Phantom *ph, Cenario *cen);
static int phantom_MovimentaFantasmaVivo(Phantom *ph, Pacman *pac, Cenario *cen);
static int phantom_MovimentaFantasmaMorto(Phantom *ph, Cenario *cen);

// Função que inicializa os dados associados a um fantasma
Phantom* phantom_create(int x, int y){
    Phantom* ph = malloc(sizeof(Phantom));
    if(ph != NULL){
        ph->passo = 3;
        ph->decidiu_cruzamento = 0;
        ph->iniciou_volta = 0;
        ph->indice_atual = 0;
        ph->status = 0;
        ph->direcao = 0;
        ph->parcial = 0;
        ph->xi = x;
        ph->yi = y;
        ph->x = x;
        ph->y = y;
        ph->caminho = NULL;
    }
    return ph;
}

// Função que libera os dados associados a um fantasma
void phantom_destroy(Phantom *ph){
    if(ph->caminho != NULL)
        free(ph->caminho);
    free(ph);
}

// Função que desenha um fantasma
void phantom_desenha(Phantom *ph){
    float linha, coluna;
    float passo = (ph->parcial/(float)bloco);

    //Verifica a posição
    if(ph->direcao == 0 || ph->direcao == 2){
        linha = ph->y;
        coluna = ph->x + passo;
    }else{
        linha = ph->y + passo;
        coluna = ph->x;
    }
    // Escolhe o sprite com base na direção e status (vivo, morto, vulnerável)
    int idx = 3*ph->direcao + ph->status;
    desenhaSprite(MAT2X(coluna),MAT2Y(linha), phantomTex2d[idx]);
}

// Atualiza a posição de um fantasma
void phantom_movimenta(Phantom *ph, Cenario *cen, Pacman *pac){
    int d;
    if(ph->status == 1){
        //Se o fantasma está morto, ele volta pelo camiho do grafo
        d = phantom_MovimentaFantasmaMorto(ph, cen);
    }else{//status é 0 ou 2
        // Fugir ou perseguir o pacman?
        if(pacman_eh_invencivel(pac))
            ph->status = 2;
        else
            ph->status = 0;

        // Chama a função que calcula a nova direção para onde o fantasma
        // deve se mover com base no mapa e na posição do pacman
        d = phantom_MovimentaFantasmaVivo(ph, pac, cen);
        // O que fazer se tocar no pacman?
        if(pac->x == ph->x && pac->y == ph->y){
            if(pacman_eh_invencivel(pac)){
                ph->status = 1; //fantasma morreu...
                pacman_pontos_fantasma(pac);
                ph->iniciou_volta = 0;
            }else{
                if(pacman_vivo(pac))
                    pacman_morre(pac);
            }
        }
    }
    // Movimenta e desenha o fantasma na tela
    phantom_move(ph,d,cen);
}

// Atualiza a posição de um fantasma
static void phantom_move(Phantom *ph, int direcao, Cenario *cen){
    int xt = ph->x;
    int yt = ph->y;

    // Incrementa a sua posição dentro de uma célula da matriz ou muda de célula
    if(cen->mapa[ph->y + direcoes[direcao].y][ph->x + direcoes[direcao].x] <=2){//não é parede...
        if(direcao == ph->direcao){
            if(ph->direcao < 2){
                ph->parcial += ph->passo;
                if(ph->parcial >= bloco){
                    ph->x += direcoes[direcao].x;
                    ph->y += direcoes[direcao].y;
                    ph->parcial = 0;
                }
            }else{
                ph->parcial -= ph->passo;
                if(ph->parcial <= -bloco){
                    ph->x += direcoes[direcao].x;
                    ph->y += direcoes[direcao].y;
                    ph->parcial = 0;
                }
            }
        }else{//mudar de direçao...
            if(abs(direcao - ph->direcao) != 2)
                ph->parcial = 0;

            ph->direcao = direcao;
        }
    }

    if(xt != ph->x || yt != ph->y)
        ph->decidiu_cruzamento = 0;
}

// Função que auxilia a escolha do caminho quando o fantasma morre
static int phantom_DirecaoGrafo(Phantom *ph, Cenario *cen){
    if(cen->grafo[ph->indice_atual].x == cen->grafo[ph->caminho[ph->indice_atual]].x){
        if(cen->grafo[ph->indice_atual].y > cen->grafo[ph->caminho[ph->indice_atual]].y)
            return 3;
        else
            return 1;
    }else{
        if(cen->grafo[ph->indice_atual].x > cen->grafo[ph->caminho[ph->indice_atual]].x)
            return 2;
        else
            return 0;
    }

}

// Função que auxilia a escolha do caminho quando o fantasma morre
static int phantom_DistanciaGrafo(Cenario *cen, int noA, int noB){
    return fabs(cen->grafo[noA].x - cen->grafo[noB].x) + fabs(cen->grafo[noA].y - cen->grafo[noB].y);
}

// Quando o fantasma encontra um cruzamento, ele sorteia uma nova direção
// A tendência e continuar em frente, as vezes mudar de direção e
// quase nunca voltar pelo mesmo caminho
static int phantom_SorteiaDirecao(Phantom *ph, Cenario *cen){
    int i,j,k,maior;
    int peso[4], dir[4];

    for(i=0; i<4; i++)
        peso[i] = rand() % 10 + 1;

    peso[ph->direcao] = 7;
    peso[(ph->direcao + 2) % 4] = 3;

    // Ordena os pesos de cada direção
    for(j=0; j<4; j++){
        maior = 0;
        for(i=0; i<4; i++){
            if(peso[i] > maior){
                maior = peso[i];
                k = i;
            }
        }
        dir[j] = k;
        peso[k] = 0;
    }

    // Escolhe a primeira direção válida
    i = 0;
    while(cen->mapa[ph->y + direcoes[dir[i]].y][ph->x + direcoes[dir[i]].x] > 2)
        i++;

    return dir[i];
}

// Função que agrupa todos os casos de escolha de direção do fantasma
static int phantom_MovimentaFantasmaVivo(Phantom *ph, Pacman *pac, Cenario *cen){
    int d, i;
    if(cenario_EhCruzamento(ph->x, ph->y, cen)){
        if(!ph->decidiu_cruzamento){
            // Cruzamento: viu o pacman, siga naquela direção
            d = -1;
            for(i=0; i<4; i++)
                if(phantom_VePacman(ph, pac, cen, i))
                    d = i;

            // Cruzamento: NÃO viu o pacman, sortear uma direção
            if(d == -1)
                d = phantom_SorteiaDirecao(ph, cen);
            else{
                // Cruzamento: viu o pacman, mas ele está invencível
                // FUGIR!! Sorteia uma nova direção
                if(pacman_eh_invencivel(pac)){
                    i = d;
                    while(i == d)
                        d = phantom_SorteiaDirecao(ph, cen);
                }
            }
            ph->decidiu_cruzamento = 1;
        }else
            d = ph->direcao;
    }else{
        //NÃO é cruzamento: continua no mesmo caminho
        ph->decidiu_cruzamento = 0;
        d = ph->direcao;
        if(pacman_eh_invencivel(pac)){
            //Se viu o pacman e ele é invencível: vai na direção oposta
            if(phantom_VePacman(ph, pac, cen, d))
                d = (d + 2) % 4;
        }

        //inverter direção...
        if(cen->mapa[ph->y + direcoes[d].y][ph->x + direcoes[d].x] >2)
            d = (d + 2) % 4;
    }
    return d;
}

// Função que trata os casos onde o fantasma vê o pacman
static int phantom_VePacman(Phantom *ph, Pacman *pac, Cenario *cen, int direcao){
    int continua = 0;
    if(direcao == 0 || direcao == 2){
        if(pac->y == ph->y)
            continua = 1;
    }else{
        if(pac->x == ph->x)
            continua = 1;
    }

    if(continua){
        int xt = ph->x;
        int yt = ph->y;
        while(cen->mapa[yt + direcoes[direcao].y][xt + direcoes[direcao].x] <= 2){
            yt = yt + direcoes[direcao].y;
            xt = xt + direcoes[direcao].x;

            if(xt == pac->x && yt == pac->y)
                return 1;
        }
    }
    return 0;
}

// Função que auxilia a escolha do caminho quando o fantasma morre
static void phantom_ProcuraMenorCaminho(Phantom *ph, Cenario *cen){
    int i, k, indice_no;
    int continua, d;
    int *dist;

    dist = malloc(cen->NV*sizeof(int));
    if(ph->caminho == NULL)
        ph->caminho = malloc(cen->NV*sizeof(int));

    //inicia calculo menor caminho...
    for(i=0; i<cen->NV; i++){
        dist[i] = 10000;
        ph->caminho[i] = -1;
        if(cen->grafo[i].x == ph->xi && cen->grafo[i].y == ph->yi)
            indice_no = i;

        if(cen->grafo[i].x == ph->x && cen->grafo[i].y == ph->y)
            ph->indice_atual = i;
    }

    dist[indice_no] = 0;

    //calculo menor caminho...
    continua = 1;
    while(continua){
        continua = 0;
        for(i=0; i<cen->NV; i++){
            for(k=0; k<4; k++){
                if(cen->grafo[i].vizinhos[k] >= 0){
                    d = phantom_DistanciaGrafo(cen, i, cen->grafo[i].vizinhos[k]);
                    if(dist[cen->grafo[i].vizinhos[k]] > (dist[i] + d)){
                        dist[cen->grafo[i].vizinhos[k]] = (dist[i] + d);
                        ph->caminho[cen->grafo[i].vizinhos[k]] = i;
                        continua = 1;
                    }
                }
            }
        }
    }
    free(dist);
}

// Função que auxilia a escolha do caminho quando o fantasma morre
static int phantom_MovimentaFantasmaMorto(Phantom *ph, Cenario *cen){
    int d;

    if(!ph->iniciou_volta){
        if(cenario_EhCruzamento(ph->x, ph->y, cen)){
            ph->iniciou_volta = 1;
            phantom_ProcuraMenorCaminho(ph, cen);
            ph->decidiu_cruzamento = 1;
            d = phantom_DirecaoGrafo(ph, cen);
        }else{
            d = ph->direcao;
            if(cen->mapa[ph->y + direcoes[d].y][ph->x + direcoes[d].x] > 2)//é parede...
                d = (d + 2) % 4;
        }
    }else{//faz caminho de volta...
        if(ph->x != ph->xi || ph->y != ph->yi){
            if(cenario_EhCruzamento(ph->x, ph->y, cen)){
                if(ph->decidiu_cruzamento)
                    d = ph->direcao;
                else{//verificar que direção tomar...
                    ph->indice_atual = ph->caminho[ph->indice_atual];
                    d = phantom_DirecaoGrafo(ph, cen);
                    ph->decidiu_cruzamento = 1;
                }
            }else{
                d = ph->direcao;
                ph->decidiu_cruzamento = 0;
            }
        }else{
            ph->status = 0;
            d = ph->direcao;
        }
    }
    return d;
}
