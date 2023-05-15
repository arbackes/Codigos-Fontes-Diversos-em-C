
typedef struct mandelbrot_set MANDELBROT;

MANDELBROT* inicializa();
void finaliza(MANDELBROT* M);
void calcula(MANDELBROT* M);
void desenha(MANDELBROT* M);

void muda_cor(MANDELBROT* M);
void muda_coordenada(MANDELBROT* M);
