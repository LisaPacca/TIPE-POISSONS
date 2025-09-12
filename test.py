import pygame
import sys
import math
import random

pygame.init()



LARGEUR, HAUTEUR = 1920, 1080
FENETRE = pygame.display.set_mode((LARGEUR, HAUTEUR),pygame.FULLSCREEN)
pygame.display.set_caption("Poissons avec influence locale et traînée")


NB_POISSONS=30
RAYON_ATTRACTION=100
RAYON_ALIGNEMENT=66
RAYON_REPULSION=33
VITESSE_INITIALE=2.5
TAILLE_POISSON=5
LONGUEUR_TRAINEE=5


COULEURS = {
    "blanc": (255, 255, 255),
    "noir": (0, 0, 0),
    "rouge": (255, 0, 0),
    "orange": (255, 165, 0),
    "vert": (0, 128, 0),
    "bleu": (0, 0, 255),
    "cyan": (0, 255, 255)
}

class Setting:
    def __init__(self,nb_poisson,r1,r2,r3,v_init,taille_poisson=6,longueur_trainee=5):
        self.v_norm=v_init
        self.nb_poisson=nb_poisson
        self.v_init=v_init
        self.r1=r1
        self.r2=r2
        self.r3=r3
        self.taille_poisson=taille_poisson
        self.longueur_trainee=longueur_trainee
        
    
    def set_vitesse(self,v):
        self.v_norm=v




class Poisson:
    def __init__(self, x, y, angle, setting):
        self.pos = pygame.Vector2(x, y)
        self.setting=setting
        self.taille=setting.taille_poisson
        self.vitesse = pygame.Vector2(math.cos(angle), math.sin(angle)) * setting.v_norm
        self.trainee = []  # Liste des anciennes positions

    def trouver_voisins(self, tous):
        voisins1 = []
        voisins2 = []
        voisins3 = []
        for autre in tous:
            if autre is not self:
                distance = self.pos.distance_to(autre.pos)
                if self.setting.r1<distance <= self.setting.r2:
                    voisins2.append(autre)
                elif distance <=self.setting.r1:
                    voisins1.append(autre)
                elif self.setting.r2<distance <=self.setting.r3 :
                    voisins3.append(autre)
                    
        return voisins1,voisins2,voisins3

    def update_direction(self,tous):
        voisins1,voisins2,voisins3 = self.trouver_voisins(tous)
        if voisins1 or voisins2 or voisins3:
            direction_moyenne = pygame.Vector2(0, 0)
            count = 0
            dir_moy = pygame.Vector2(0,0)
            cont = 0
            if voisins1 :
                for v in voisins1:
                    if v.vitesse.length() > 0:
                        self.vitesse, v.vitesse = v.vitesse,self.vitesse
            if voisins2:
                for v in voisins2:
                    direction_moyenne += v.vitesse.normalize()
                    count += 1
                if count > 0:
                    direction_moyenne /= count
                    if direction_moyenne.length() > 0:
                        self.vitesse = direction_moyenne.normalize() * self.setting.v_norm
            if voisins3:
                for v in voisins3:
                    dir_moy += v.pos.normalize()
                    cont +=1
                if cont > 0:
                    dir_moy /= cont
                    if dir_moy.length() >0:
                        self.vitesse = dir_moy.normalize() * self.setting.v_norm
        else:
            if self.vitesse.length() > 0:
                self.vitesse = self.vitesse.normalize() * self.setting.v_norm

    def deplacer(self):
        # Ajouter la position actuelle à la traînée
        self.trainee.append(self.pos.copy())
        if len(self.trainee) > self.setting.longueur_trainee:
            self.trainee.pop(0)

        # Déplacement
        self.pos += self.vitesse

        # Rebond sur les murs
        if self.pos.x - self.taille <= 0 or self.pos.x + self.taille >= LARGEUR:
            self.vitesse.x *= -1
            self.pos.x = max(self.taille, min(LARGEUR - self.taille, self.pos.x))

        if self.pos.y - self.taille <= 0 or self.pos.y + self.taille >= HAUTEUR:
            self.vitesse.y *= -1
            self.pos.y = max(self.taille, min(HAUTEUR - self.taille, self.pos.y))

    def dessiner(self, surface, couleur):
        # Dessiner la traînée
        if len(self.trainee) > 1:
            pygame.draw.lines(surface, couleur, False, [(int(p.x), int(p.y)) for p in self.trainee], 2)

        # Dessiner le corps
        pygame.draw.circle(surface, couleur, (int(self.pos.x), int(self.pos.y)), self.taille)

settings=Setting(nb_poisson=NB_POISSONS,r1=RAYON_REPULSION,r2=RAYON_ALIGNEMENT,r3=RAYON_ATTRACTION,v_init=VITESSE_INITIALE,
                 taille_poisson=TAILLE_POISSON,longueur_trainee=LONGUEUR_TRAINEE)

def main(settings=settings):  
    
    poissons = []
    def setup():
        for _ in range(settings.nb_poisson):
            angle = random.uniform(0, 2 * math.pi)
            x = random.randint(100, LARGEUR - 100)
            y = random.randint(100, HAUTEUR - 100)
            poissons.append(Poisson(x, y, angle,settings))
    
    setup()
    
    
    clock = pygame.time.Clock()
    running = True
    tick_count=60
    up=False
    b=False
    space=False
    theme=False
    pause=False
    while running:
        touches = pygame.key.get_pressed()
        if touches[pygame.K_a]:
            tick_count=60
            settings.v_norm=7.5
        elif touches[pygame.K_z]:
            tick_count=60
            settings.v_norm=30
            
        elif touches[pygame.K_e]:
            settings.v_norm=settings.v_init
            tick_count=60
        elif touches[pygame.K_r]:
            settings.v_norm=settings.v_init
            tick_count=60
            poissons=[]
            setup()
        elif touches[pygame.K_q]:
            pygame.quit()
            sys.exit()
            
        elif touches[pygame.K_UP]:
            up=True 
        elif touches[pygame.K_b]:
            b=True
        elif touches[pygame.K_SPACE]:
            space=True
        if up and not(touches[pygame.K_UP]):
            up=False
            setup()
        if b and not(touches[pygame.K_b]):
            theme=not(theme)
            b=False
        if space and not(touches[pygame.K_SPACE]):
            space=False
            pause=not(pause)
            
            
        
        clock.tick(tick_count)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        if not(pause):
            # Mise à jour des directions
            for poisson in poissons:
                poisson.update_direction(poissons)
        
            # Déplacement
            for poisson in poissons:
                poisson.deplacer()
        
            # Affichage
            if theme :
                FENETRE.fill(COULEURS["blanc"])
            else:
                FENETRE.fill(COULEURS["noir"])
        
        i=0
        for poisson in poissons:
            if i==0 and theme:
                poisson.dessiner(FENETRE, COULEURS["noir"])
            elif i==0 and not(theme):
                poisson.dessiner(FENETRE, COULEURS["blanc"])
            else:
                if poissons[0].pos.distance_to(poisson.pos)<= poisson.setting.r1:
                    poisson.dessiner(FENETRE, COULEURS["orange"])
                elif poissons[0].pos.distance_to(poisson.pos)<= poisson.setting.r2:
                    poisson.dessiner(FENETRE, COULEURS["cyan"])
                elif poissons[0].pos.distance_to(poisson.pos)<= poisson.setting.r3:
                    poisson.dessiner(FENETRE, COULEURS["vert"])
                else:
                    poisson.dessiner(FENETRE, COULEURS["rouge"])
            i+=1
        pygame.display.flip()
    
main()