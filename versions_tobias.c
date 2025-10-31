#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//taille de l'ecran
#define LARGEUR 1000
#define HAUTEUR 1000

//valeurs aleatoires
#define NB_POISSONS 150
#define RAYON_ATTRACTION 200
#define RAYON_ORIENTATION 100
#define RAYON_REPULSION 17
#define V_INITIALE 3.5
#define TAILLE_POISSON 5
#define ALPHA 0.1
#define BRUIT (float)0.02f
#define BLIND M_PI

typedef struct {
    float x, y;
} Vector2;//nom à changer

struct Poisson{
    Vector2 pos;
    Vector2 vitesse;
    struct Poisson* tab_orientation[NB_POISSONS];
    int id;
};
typedef struct Poisson Poisson;

//affichage
SDL_Color COULEURS[] = {
    {255,255,255,255}, // blanc 0
    {0,0,0,255},       // noir 1
    {255,0,0,255},     // rouge 2
    {255,165,0,255},   // orange 3
    {0,255,0,255},     // vert 4
    {255,255,0,255},   // jaune 5
    {0,0,255,255},     // bleu 6
    {0,255,255,255},   // cyan 7
    {128,0,128,255},   // violet 8
    {128,128,128,255}, // gris 9
    {0,255,0,255},     // vert clair 10
    {255,192,203,255}, // rose 11
    {165,42,42,255},   // marron 12
    {75,0,130,255},    // indigo 13
    {240,230,140,255}  // kaki 14
};

//fonctions a refaire 
Vector2 vec_add(Vector2 a, Vector2 b)
    { return (Vector2){a.x+b.x,a.y+b.y}; }

float distance(Vector2 a, Vector2 b){
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

float normer(Vector2 v){
    return sqrtf(v.x*v.x + v.y*v.y);
}

void poisson_init(Poisson* p, float x, float y, float angle, int i){
    p->pos = (Vector2){x,y};
    p->vitesse = (Vector2){cosf(angle)*V_INITIALE, sinf(angle)*V_INITIALE}; 
    for(int i=0; i<NB_POISSONS; i++){
        p->tab_orientation[i] = NULL;
    }
    p->id = i;   
}

float valabs(float x){return sqrtf(x*x);}

bool appartient(Poisson* p, Poisson* tab[NB_POISSONS]){
    for(int i=0; i<NB_POISSONS; i++){
        if(p == tab[i]) { return true; }
    }
    return false;
}


bool blind_zone(Vector2 a, Vector2 b){
    float costheta = (a.x*b.x + a.y*b.y)/(normer(a)*normer(b));
    float theta = acosf(costheta);
    if(theta > (M_PI-(BLIND/2))){return false;}
    return true;
    
}

bool zone_repulsion(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2* d = malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < RAYON_REPULSION && blind_zone(p->vitesse,diff)){
            count++;
            d->x += (p->pos.x - v->pos.x)/distance(p->pos,v->pos);
            d->y += (p->pos.y - v->pos.y)/distance(p->pos,v->pos);  
        }
    }
    if(count != 0){
        float norme = sqrtf((d->x)*(d->x)+(d->y)*(d->y));
        if(norme>0){
            d->x = d->x/norme;
            d->y = d->y/norme;
            p->vitesse.x = ((1-ALPHA)*p->vitesse.x + ALPHA*d->x)*V_INITIALE;
            p->vitesse.y = ((1-ALPHA)*p->vitesse.y + ALPHA*d->y)*V_INITIALE;

            float v = normer(p->vitesse);
            if(v>V_INITIALE){
                p->vitesse.x = ((p->vitesse.x)/v)*V_INITIALE;
                p->vitesse.y = ((p->vitesse.y)/v)*V_INITIALE;
            }
        }
    }
    free(d);
    return (count == 0);
}

//pas l'air de faire grand chose si foutre le bordel
void zone_orientation(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < RAYON_ORIENTATION && blind_zone(p->vitesse,diff)){
            count++;
            d->x += v->vitesse.x/sqrtf((v->vitesse.x)*(v->vitesse.x)+(v->vitesse.y)*(v->vitesse.y));
            d->y += v->vitesse.y/sqrtf((v->vitesse.x)*(v->vitesse.x)+(v->vitesse.y)*(v->vitesse.y));
            p->tab_orientation[j] = v;
        }
        else{
            p->tab_orientation[j] = NULL;
        }
    }
    if(count != 0 ){
        float norme = sqrtf((d->x)*(d->x)+(d->y)*(d->y));
        if(norme>0){
            d->x = d->x/norme;
            d->y= d->y/norme;
            p->vitesse.x = ((1-ALPHA)*p->vitesse.x + ALPHA*d->x)*V_INITIALE;
            p->vitesse.y = ((1-ALPHA)*p->vitesse.y + ALPHA*d->y)*V_INITIALE;

            float v = normer(p->vitesse);
            if(v>V_INITIALE){
                p->vitesse.x = ((p->vitesse.x)/v)*V_INITIALE;
                p->vitesse.y = ((p->vitesse.y)/v)*V_INITIALE;
            }
        }
    }
    free(d);
}


void zone_attration(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < RAYON_ATTRACTION && dist > RAYON_ORIENTATION && blind_zone(p->vitesse, diff)){
            count++;
            d->x += (v->pos.x - p->pos.x)/distance(v->pos,p->pos)*V_INITIALE;
            d->y += (v->pos.y - p->pos.y)/distance(v->pos,p->pos)*V_INITIALE;
        }
    }
    if(count != 0){
       float norme = sqrtf((d->x)*(d->x)+(d->y)*(d->y));
        if(norme>0){
            d->x = d->x/norme;
            d->y= d->y/norme;
            p->vitesse.x = ((1-ALPHA)*p->vitesse.x + ALPHA*d->x);
            p->vitesse.y = ((1-ALPHA)*p->vitesse.y + ALPHA*d->y);

            float v = normer(p->vitesse);
            if(v>V_INITIALE){
                p->vitesse.x = ((p->vitesse.x)/v)*V_INITIALE;
                p->vitesse.y = ((p->vitesse.y)/v)*V_INITIALE;
            }
        }
    }
    free(d);
}

void poisson_deplacer(Poisson* p){
    float r = (((float)rand() / RAND_MAX)*2*BRUIT) - BRUIT;
    p->vitesse.x = cosf(r)*p->vitesse.x - sinf(r)*p->vitesse.y;
    p->vitesse.y = sinf(r)*p->vitesse.x + cosf(r)*p->vitesse.y;
    p->pos = vec_add(p->pos, p->vitesse);

    if(p->pos.x - TAILLE_POISSON <= 0 || p->pos.x + TAILLE_POISSON >= LARGEUR){
        if(p->pos.x< TAILLE_POISSON){ 
            p->pos.x = TAILLE_POISSON;
            p->vitesse.x *= -1;
        }        
        if(p->pos.x> LARGEUR - TAILLE_POISSON){
            p->pos.x = LARGEUR - TAILLE_POISSON;
            p->vitesse.x *= -1;
        }
    }
    if(p->pos.y - TAILLE_POISSON<= 0 || p->pos.y + TAILLE_POISSON >= HAUTEUR){
        if(p->pos.y< TAILLE_POISSON){
            p->pos.y = TAILLE_POISSON;
            p->vitesse.y *= -1;
        }
        if(p->pos.y> HAUTEUR - TAILLE_POISSON){
            p->pos.y = HAUTEUR - TAILLE_POISSON;
            p->vitesse.y *= -1;
        }
    }
}
void draw_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r; // Décision initiale pour le tracé

    // Dessiner les points des 8 octants
    while (y >= x) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);

        x++;

        // Mise à jour de la décision
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void draw_circle_full(SDL_Renderer* renderer, int cx, int cy, int r) {
    for(int w = -r; w <= r; w++) {
        for(int h = -r; h <= r; h++) {
            if(w*w + h*h <= r*r) {
                SDL_RenderDrawPoint(renderer, cx + w, cy + h);
            }
        }
    }
}

//dessine un poisson 
void poisson_dessiner(Poisson* p, SDL_Renderer* renderer, SDL_Color c1, SDL_Color c2, SDL_Color c3, SDL_Color c4,Poisson voisin[NB_POISSONS]){
    SDL_SetRenderDrawColor(renderer, c1.r,c1.g,c1.b,255);
    draw_circle(renderer,(int)p->pos.x,(int)p->pos.y,TAILLE_POISSON);
    if(p == &voisin[0]){SDL_SetRenderDrawColor(renderer, c3.r,c3.g,c3.b,255);
    }
    else if (appartient(p, voisin[0].tab_orientation)) {
    SDL_SetRenderDrawColor(renderer, c4.r,c4.g,c4.b,255);
    }
    else {SDL_SetRenderDrawColor(renderer, c2.r,c2.g,c2.b,255);}
    draw_circle(renderer,(int)(p->pos.x + p->vitesse.x * 2),(int)(p->pos.y + p->vitesse.y * 2),TAILLE_POISSON);
    float n = sqrtf(p->vitesse.x * p->vitesse.x + p->vitesse.y * p->vitesse.y);
    if (n > 0.001f) {
        float dx = (p->vitesse.x / n) * TAILLE_POISSON * 2.0f;
        float dy = (p->vitesse.y / n) * TAILLE_POISSON * 2.0f;
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderDrawLine(renderer,(int)p->pos.x, (int)p->pos.y,(int)(p->pos.x - dx), (int)(p->pos.y - dy)
        );
    }
}

void moyenne_position(){}

//
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("MES POISSONS D'AMOUR",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LARGEUR,HAUTEUR,SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Poisson poissons[NB_POISSONS];

    for(int i=0;i<NB_POISSONS;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,i);
    }

    bool running=true;
    SDL_Event e;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_q) running=false;
        }

        for(int i=0;i<NB_POISSONS;i++) {
            poisson_deplacer(&poissons[i]);
            bool zr = zone_repulsion(&poissons[i],poissons);
            if(zr){
                zone_orientation(&poissons[i],poissons);
                zone_attration(&poissons[i],poissons);
            }
        }

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        for(int i=NB_POISSONS-1 ;i>=0;i--){
            poisson_dessiner(&poissons[i],renderer,COULEURS[7] /*cyan*/, COULEURS[0]/*blanc*/,  COULEURS[4] /*vert*/, COULEURS[2] /*rouge*/,poissons); 
        }
        

        SDL_RenderPresent(renderer);
        SDL_Delay(15);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
