#include "simulation.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//taille de l'ecran
#define LARGEUR 1200
#define HAUTEUR 750

//valeurs aleatoires
#define NB_POISSONS 150
#define V_INITIALE 3.5
#define TAILLE_POISSON 5
#define ALPHA 0.1
#define BRUIT 0.05
#define BLIND M_PI

struct {
    float x, y;
} ;
typedef struct Vector2 Vector2;//nom à changer


struct Poisson{
    Vector2 pos; //position de poisson dans l'écran 
    Vector2 vitesse; // vitesse 
};
typedef struct Poisson Poisson;


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
}

float valabs(float x){return sqrtf(x*x);}


bool blind_zone(Vector2 a, Vector2 b){
    float costheta = (a.x*b.x + a.y*b.y)/(normer(a)*normer(b));
    float theta = acosf(costheta);
    if(theta > (M_PI-(BLIND/2))){return false;}
    return true;
    
}

bool zone_repulsion(Poisson* p, Poisson voisin[NB_POISSONS], float r_r){
    Vector2* d = malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < r_r && blind_zone(p->vitesse,diff)){
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
void zone_orientation(Poisson* p, Poisson voisin[NB_POISSONS], float r_o){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < r_o && blind_zone(p->vitesse,diff)){
            count++;
            d->x += v->vitesse.x/sqrtf((v->vitesse.x)*(v->vitesse.x)+(v->vitesse.y)*(v->vitesse.y));
            d->y += v->vitesse.y/sqrtf((v->vitesse.x)*(v->vitesse.x)+(v->vitesse.y)*(v->vitesse.y));
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


void zone_attration(Poisson* p, Poisson voisin[NB_POISSONS], float r_a, float r_o){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < r_a && dist > r_o && blind_zone(p->vitesse, diff)){
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



float rayon_moyen(Poisson poisson[NB_POISSONS], float mx, float my){

    float sum = 0;
    for(int i=0; i<NB_POISSONS; i++){
        sum += sqrtf((poisson[i].pos.x - mx)*(poisson[i].pos.x - mx) + (poisson[i].pos.y - my)*(poisson[i].pos.y - my));
    }
    float rayonmoyen = sum / NB_POISSONS;
    return rayonmoyen ; 
}


Vector2 moyenne_position(Poisson poisson[NB_POISSONS], int nb_frame, Vector2 moy_prec){
    float mx = 0;
    float my = 0;
    for(int i=0; i<NB_POISSONS; i++){
        mx += poisson[i].pos.x;
        my += poisson[i].pos.y;
    }
    mx = mx / NB_POISSONS;
    my = my / NB_POISSONS;
    bool dist_moy = distance((Vector2){mx, my}, moy_prec) < 30;
    
    float max = 0;
    float min = 0;
    for(int i=0; i<NB_POISSONS; i++){
        float dist = sqrtf((poisson[i].pos.x - mx)*(poisson[i].pos.x - mx) + (poisson[i].pos.y - my)*(poisson[i].pos.y - my));
        if(i == 0){
            max = dist;
            min = dist;
        }
        else {
            if(dist > max){
                max = dist;
            }
            if(dist < min){
                min = dist;
            }
        }
    };
    bool cycle = ((max-min)<600) && dist_moy;
    if(cycle){
        rayon_moyen(poisson, mx, my);
    }
    
    return (Vector2){mx, my};
}

/*void cycle_infecte(Poisson poisson[NB_POISSONS], int nb_frame){
    bool all_infecte = true;
    for(int i=0; i<NB_POISSONS; i++){
        if(!poisson[i].infecte){
            if(nb_frame % 200 == 0){
            printf("Poisson %d n'est pas infecté.\n", poisson[i].id);
            }

            all_infecte = false;
        }
    }
    if(all_infecte){
        printf("Tous les poissons sont infectés !\n");
    }
}
*/


float simulation_f(float r_r, float r_o, float r_a){
    Poisson poissons[NB_POISSONS];

    for(int i=0;i<NB_POISSONS;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,i);
    }

    int nb_frame = 2000;
    Vector2 moy = {0,0};

    for (int i = 0; i<nb_frame ; i++){
        for(int i=0;i<NB_POISSONS;i++) {
            poisson_deplacer(&poissons[i]);
            Vector2 moy = moyenne_position(poissons, nb_frame, moy);
            bool zr = zone_repulsion(&poissons[i],poissons, r_r);
            if(zr){
                zone_orientation(&poissons[i],poissons, r_o);
                zone_attration(&poissons[i],poissons, r_a, r_o);
            }
           /* if(poissons[i].id != 0){
                (&poissons[i])->infecte = false;
            }
            cycle_infecte(poissons, nb_frame); */
        }
    }
    return rayon_moyen(poissons, moy.x, moy.y);
}
