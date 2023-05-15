
//=========================================================
typedef struct TPacman Pacman;
typedef struct TPhantom Phantom;
typedef struct TCenario Cenario;

//=========================================================
void carregaTexturas();

//=========================================================
Cenario* cenario_carrega(char *arquivo);
void cenario_destroy(Cenario* cen);
void cenario_desenha(Cenario* cen);
void desenhaTela(int tipo);

//=========================================================
Pacman* pacman_create(int x, int y);
void pacman_destroy(Pacman *pac);
int pacman_vivo(Pacman *pac);
void pacman_desenha(Pacman *pac);
void pacman_AlteraDirecao(Pacman *pac, int direcao, Cenario *cen);
void pacman_movimenta(Pacman *pac, Cenario *cen);

////=========================================================
Phantom* phantom_create(int x, int y);
void phantom_destroy(Phantom *ph);
void phantom_movimenta(Phantom *ph, Cenario *cen, Pacman *pac);
void phantom_desenha(Phantom *ph);
