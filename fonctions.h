#ifndef FONCTIONS_H
#define FONCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define WINDOW_W 800
#define WINDOW_H 600
#define BTN_W 100
#define BTN_H 40
#define TAILLE_CASE 30

#define OFFSET_X 250
#define OFFSET_Y 200

typedef struct Grille {
    int **cases;
    int hauteur;
    int largeur;
    int taille;
    int nombre_cases_noires;
} Grille;

typedef struct Bouton {
    SDL_Texture *texture;
    SDL_Texture *texte;
    SDL_Rect rect;
    SDL_Rect rect_texte;
} Bouton;

Grille *chargerGrille(Grille *grille_solution);
Grille *chargerGrillesolution(FILE *fichier);
int *calculerIndices(int *ligne, int taille, int *nb_indices);
void libererGrille(Grille *grille);
void dessinerGrille(SDL_Renderer *renderer, Grille *grille_jeu, Grille *grille_solution);
void gererClic(Grille *grille, int x, int y);
void Verification(Grille *grille_solution, Grille *grille_jeu);
int read_grid_from_txt(const char *path, int ***out_grid, size_t *out_rows, size_t *out_cols);
void free_grid(int **grid, size_t rows);
void print_grid(const int * const *grid, size_t rows, size_t cols);
int compare_grids(const int * const *g1, const int * const *g2, size_t rows, size_t cols);
int check_grids_from_files(const char *player_path, const char *solution_path);
int grillesIdentiques(Grille *grille_solution, Grille *grille_jeu);

void afficherEcranFin(SDL_Renderer *renderer, const char *message);
SDL_Texture *texture_init(const char *fichier, SDL_Renderer *renderer);
Bouton creer_bouton(const char *fichier, SDL_Renderer *renderer, const char *label, TTF_Font *font, int x, int y);
bool Clique(int souris_x, int souris_y, Bouton b);

void Menu_principal(SDL_Renderer *renderer, SDL_Window *window,Grille *grille_solution, Grille *grille_jeu,int largeurGrillePixels, int hauteurGrillePixels);
void Menu_jouer(SDL_Renderer *renderer, SDL_Window *window,Grille *grille_solution, Grille *grille_jeu, int largeurGrillePixels, int hauteurGrillePixels);
void Menu_parametres(SDL_Renderer *renderer, SDL_Window *window);

#endif