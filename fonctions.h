#ifndef FONCTIONS_H
#define FONCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define TAILLE_CASE 40

typedef struct Grille {
    int **cases;
    int hauteur;
    int largeur;
    int taille;
    int nombre_cases_noires;
} Grille;


Grille *chargerGrille(Grille * grille_solution);
Grille *chargerGrillesolution(FILE *fichier);
void Verification(Grille * grille_solution, Grille * grille_jeu);
void libererGrille(Grille *grille);
void dessinerGrille(SDL_Renderer *rendu, Grille *grille);
void gererClic(Grille *grille, int x, int y);
void sauvegarderGrille(const char *nomFichier, Grille *grille);
Grille *chargerGrilleSauvegarde(const char *nomFichier);

#endif
