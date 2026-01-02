#include "simulation.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//taille de l'ecran
#define LARGEUR 1200
#define HAUTEUR 750

//valeurs constantes trouvé par experience sur version SDL
#define NB_POISSONS 150
#define V_INITIALE 3.5
#define TAILLE_POISSON 5
#define ALPHA 0.1
#define BRUIT 0.05
#define BLIND M_PI
#define NB_FRAME 10000
#define REGIME_TRANSITOIRE 3000


//fonctions utilitaires 
Vector2 vec_add(Vector2 a, Vector2 b)
    { return (Vector2){a.x+b.x,a.y+b.y}; }

float distance(Vector2 a, Vector2 b){
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

float normer(Vector2 v){
    return sqrtf(v.x*v.x + v.y*v.y);
}

//intialisation d'un poisson
void poisson_init(Poisson* p, float x, float y, float angle, int i){
    p->pos = (Vector2){x,y};
    p->vitesse = (Vector2){cosf(angle)*V_INITIALE, sinf(angle)*V_INITIALE}; 
}

//zone non prise en compte (l'arriere du poisson) dans les deplacements 
bool blind_zone(Vector2 a, Vector2 b){
    float costheta = (a.x*b.x + a.y*b.y)/(normer(a)*normer(b));
    float theta = acosf(costheta);
    if(theta > (M_PI-(BLIND/2))){return false;}
    return true;
    
}

//fonctions de mouvement cf modele zones concentriques 
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

//deplacement du poisson par rapport à l'ecran (ses dimensions)
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

//la fonction moyenne_position de tobias est le regroupement des fonctions
//moyenne_position_version_centre et stabilite_centre

//calcule la moyenne de tout les poissons pour en calcule le "centre"
Vector2 moyenne_position_version_centre (Poisson poisson[NB_POISSONS]){
    float mx = 0;
    float my = 0;
    for(int i=0; i<NB_POISSONS; i++){
        mx += poisson[i].pos.x;
        my += poisson[i].pos.y;
    }
    mx = mx / NB_POISSONS;
    my = my / NB_POISSONS;
    Vector2 moy = {mx,my};
    return moy; 
}

//calcule la distancte entre 2 centres consecutifs et verfient si le cycle a beaucoup bougé ou pas 
//c'est notre critere de cycle 
bool stabilite_centre (Poisson poisson[NB_POISSONS], Vector2 c, Vector2 c_prec){
    float max = 0; 
    float min = 1e9;
    for (int i = 0; i < NB_POISSONS; i++) {
        float d = distance(poisson[i].pos, c);
        if (d > max) {
            max = d;
        }
        if (d < min) {
            min = d;
        }
    }
    if (max ==0){
        return false; 
    }
    return (distance(c, c_prec) < 1) && ((max-min)/max < 0.3f);
}

//autre critere de cycle : regarde si les poissons sont tous dans un "tube circulaire", donc verifie
// que les poissons ne soient pas regrouppé
bool stabilite_largeur_cycle(Poisson poisson[NB_POISSONS], Vector2 centre, float largeur_min , float largeur_max ){
    for (int i=0; i < NB_POISSONS; i++){
        if ((distance(poisson[i].pos, centre)<largeur_min) || (distance(poisson[i].pos, centre)>largeur_max)){
            return false ; 
        }
    }
    return true ; 
}

//calcule la distance moyenne de chaque poisson au centre c
float rayon_moyen(Poisson poisson[NB_POISSONS], Vector2 c){
    float sum = 0;
    for(int i=0; i<NB_POISSONS; i++){
        sum += distance(poisson[i].pos, c);
    }
    float rayonmoyen = sum / NB_POISSONS;
    return rayonmoyen ; 
}

//fonction qui fait office de main
float simulation_f(float r_r, float r_o, float r_a){
    Poisson poissons[NB_POISSONS];

    for(int i=0;i<NB_POISSONS;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,i);
    }

    Vector2 centre = {0,0};
    Vector2 centre_prec = {0,0}; 

    int nb_centre = 10 ; 
    Vector2 bon_centres[nb_centre]; // tableau qui contiendra nb_centre valeur de bons centres (i.e. centres de cycles)
    int indice = 0; 

    for (int i = 0; i<NB_FRAME ; i++){
       
        for(int i=0;i<NB_POISSONS;i++) { //deplacement des poissons
            poisson_deplacer(&poissons[i]);
            bool zr = zone_repulsion(&poissons[i],poissons, r_r);
            if(zr){
                zone_orientation(&poissons[i],poissons, r_o);
                zone_attration(&poissons[i],poissons, r_a, r_o);
            }
        }
        centre = moyenne_position_version_centre(poissons); //centre=moyennes des positions des poissons
        if (i< REGIME_TRANSITOIRE){
            centre_prec = centre; 
            continue; 
        }
        if (stabilite_centre(poissons, centre, centre_prec)&&stabilite_largeur_cycle(poissons, centre, 80.0f, 500.0f)){
            bon_centres[indice]= centre; 
            indice = indice +1; 
            if (indice == nb_centre) {
                break; 
            }
        }
        else {
            indice = 0 ; 
        }
        centre_prec = centre; 
    }
    if (indice < nb_centre){
        return -1; 
    }
    else { 
        //on a fini de remplir le tableau
        //centre moyen c=(cx,cy) de tous les bons centres enregistrés dans le tableau
        Vector2 c = {0,0}; 
        float sx = 0.0f; 
        float sy = 0.0f;
        for(int k=0;k<nb_centre;k++){
            sx += bon_centres[k].x;
            sy += bon_centres[k].y;
        }
        c.x = sx / nb_centre; 
        c.y = sy / nb_centre;

        return rayon_moyen(poissons, c);
    }
}
            
