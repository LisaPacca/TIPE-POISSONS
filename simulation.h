#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifndef SIMULATION_H
#define SIMULATION_H 

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

float distance(Vector2 a, Vector2 b);
void poisson_init(Poisson* p, float x, float y, float angle, int i);
void poisson_deplacer(Poisson* p);
float rayon_moyen(Poisson poisson[NB_POISSONS], float mx, float my);
Vector2 moyenne_position(Poisson poisson[NB_POISSONS], int nb_frame, Vector2 moy_prec);

bool blind_zone(Vector2 a, Vector2 b);
bool zone_repulsion(Poisson* p, Poisson voisin[NB_POISSONS], float r_r);
void zone_orientation(Poisson* p, Poisson voisin[NB_POISSONS], float r_o);
void zone_attration(Poisson* p, Poisson voisin[NB_POISSONS], float r_a, float r_o);
float simulation_f (float r_r, float r_o, float r_a) ; 


#endif 

