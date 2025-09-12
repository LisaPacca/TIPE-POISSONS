#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define LARGEUR 1920
#define HAUTEUR 1080

#define NB_POISSONS 30
#define RAYON_ATTRACTION 100
#define RAYON_ALIGNEMENT 66
#define RAYON_REPULSION 33
#define VITESSE_INITIALE 2.5
#define TAILLE_POISSON 5
#define LONGUEUR_TRAINEE 5

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    int nb_poisson;
    float r1, r2, r3;
    float v_norm, v_init;
    int taille_poisson;
    int longueur_trainee;
} Setting;

typedef struct {
    Vector2 pos;
    Vector2 vitesse;
    Vector2 trainee[LONGUEUR_TRAINEE];
    int trainee_count;
    Setting* setting;
} Poisson;

SDL_Color COULEURS[] = {
    {255,255,255,255}, // blanc
    {0,0,0,255},       // noir
    {255,0,0,255},     // rouge
    {255,165,0,255},   // orange
    {0,128,0,255},     // vert
    {0,0,255,255},     // bleu
    {0,255,255,255}    // cyan
};

// --- Fonctions utilitaires ---
Vector2 vec_add(Vector2 a, Vector2 b){ return (Vector2){a.x+b.x,a.y+b.y}; }
Vector2 vec_sub(Vector2 a, Vector2 b){ return (Vector2){a.x-b.x,a.y-b.y}; }
Vector2 vec_mul(Vector2 a, float s){ return (Vector2){a.x*s,a.y*s}; }
float vec_len(Vector2 a){ return sqrtf(a.x*a.x + a.y*a.y); }
Vector2 vec_norm(Vector2 a){ float l=vec_len(a); return l>0 ? vec_mul(a,1.0f/l) : (Vector2){0,0}; }
float vec_dist(Vector2 a, Vector2 b){ return vec_len(vec_sub(a,b)); }

// --- Poisson ---
void poisson_init(Poisson* p, float x, float y, float angle, Setting* s){
    p->pos = (Vector2){x,y};
    p->setting = s;
    p->vitesse = (Vector2){cosf(angle)*s->v_norm, sinf(angle)*s->v_norm};
    p->trainee_count=0;
}

void poisson_deplacer(Poisson* p){
    if(p->trainee_count < p->setting->longueur_trainee){
        p->trainee[p->trainee_count++] = p->pos;
    } else {
        for(int i=1;i<p->trainee_count;i++) p->trainee[i-1]=p->trainee[i];
        p->trainee[p->trainee_count-1] = p->pos;
    }

    p->pos = vec_add(p->pos, p->vitesse);

    if(p->pos.x - p->setting->taille_poisson <= 0 || p->pos.x + p->setting->taille_poisson >= LARGEUR){
        p->vitesse.x *= -1;
        if(p->pos.x< p->setting->taille_poisson) p->pos.x = p->setting->taille_poisson;
        if(p->pos.x> LARGEUR - p->setting->taille_poisson) p->pos.x = LARGEUR - p->setting->taille_poisson;
    }
    if(p->pos.y - p->setting->taille_poisson <= 0 || p->pos.y + p->setting->taille_poisson >= HAUTEUR){
        p->vitesse.y *= -1;
        if(p->pos.y< p->setting->taille_poisson) p->pos.y = p->setting->taille_poisson;
        if(p->pos.y> HAUTEUR - p->setting->taille_poisson) p->pos.y = HAUTEUR - p->setting->taille_poisson;
    }
}

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int r){
    for(int w=-r; w<=r; w++){
        for(int h=-r; h<=r; h++){
            if(w*w+h*h <= r*r){
                SDL_RenderDrawPoint(renderer, cx+w, cy+h);
            }
        }
    }
}

void poisson_dessiner(Poisson* p, SDL_Renderer* renderer, SDL_Color c){
    SDL_SetRenderDrawColor(renderer, c.r,c.g,c.b,255);
    for(int i=1;i<p->trainee_count;i++){
        SDL_RenderDrawLine(renderer,(int)p->trainee[i-1].x,(int)p->trainee[i-1].y,
                           (int)p->trainee[i].x,(int)p->trainee[i].y);
    }
    draw_circle(renderer,(int)p->pos.x,(int)p->pos.y,p->setting->taille_poisson);
}

// --- Main ---
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Poissons SDL",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LARGEUR,HAUTEUR,SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Setting settings = {NB_POISSONS, RAYON_REPULSION, RAYON_ALIGNEMENT, RAYON_ATTRACTION,
                        VITESSE_INITIALE,VITESSE_INITIALE,TAILLE_POISSON,LONGUEUR_TRAINEE};

    Poisson poissons[NB_POISSONS];
    for(int i=0;i<settings.nb_poisson;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,&settings);
    }

    bool running=true;
    SDL_Event e;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_q) running=false;
        }

        for(int i=0;i<settings.nb_poisson;i++) poisson_deplacer(&poissons[i]);

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        for(int i=0;i<settings.nb_poisson;i++){
            poisson_dessiner(&poissons[i],renderer,COULEURS[2]); // rouge
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
