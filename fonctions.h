#ifndef FONCTIONS_H
#define FONCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define WINDOW_W 800
#define WINDOW_H 600
#define BTN_W 150
#define BTN_H 40
#define TAILLE_CASE_BASE 30
#define DEFAULT_WINDOW_W 1024
#define DEFAULT_WINDOW_H 768
#define OFFSET_X_BASE 250
#define OFFSET_Y_BASE 200
#define ETAT_VIDE 0
#define ETAT_NOIR 1
#define ETAT_MARQUE 2
#define NOMBRE_INDICES 5

#define MODE_NORMAL 0
#define MODE_HEXAGONAL 1

#define DOSSIER_CAPTURES "captures"
#define FICHIER_SAUVEGARDE "sauvegarde.dat"

typedef struct RenderContext {
    int window_w;
    int window_h;
    int offset_x;
    int offset_y;
    int taille_case;
    float scale;
} RenderContext;

typedef struct Grille {
    int **cases;
    int **marques;
    int hauteur;
    int largeur;
    int taille;
    int nombre_cases_noires;
    int **indices_lignes;
    int **indices_colonnes;
    int *nb_indices_lignes;
    int *nb_indices_colonnes;
} Grille;

typedef struct Bouton {
    SDL_Texture *texture;
    SDL_Texture *texte;
    SDL_Rect rect;
    SDL_Rect rect_texte;
} Bouton;

typedef struct Sauvegarde {
    int hauteur;
    int largeur;
    int **cases_jeu;
    int **cases_solution;
    Uint32 temps_ecoule;
    int valide;
} Sauvegarde;

Grille *chargerGrille(Grille *grille_solution);
Grille *chargerGrillesolution(FILE *fichier);
int *calculerIndices(int *ligne, int taille, int *nb_indices);
void libererGrille(Grille *grille);
void calculerEtStockerTousIndices(Grille *grille_solution);
void libererIndices(Grille *grille);

void calculerRenderContext(RenderContext *ctx, int window_w, int window_h, Grille *grille);
void dessinerGrille(SDL_Renderer *renderer, Grille *grille_jeu, Grille *grille_solution, RenderContext *ctx, int mode);
void dessinerChronometre(SDL_Renderer *renderer, Uint32 temps_ecoule, int x, int y, TTF_Font *font);
void dessinerHexagone(SDL_Renderer *renderer, int cx, int cy, int rayon, int rempli, SDL_Color couleur);

void gererClic(Grille *grille, int x, int y, RenderContext *ctx, int mode);
void gererClicDroit(Grille *grille, int x, int y, RenderContext *ctx, int mode);
void Verification(Grille *grille_solution, Grille *grille_jeu);
int read_grid_from_txt(const char *path, int ***out_grid, size_t *out_rows, size_t *out_cols);
void free_grid(int **grid, size_t rows);
void print_grid(const int * const *grid, size_t rows, size_t cols);
int compare_grids(const int * const *g1, const int * const *g2, size_t rows, size_t cols);
int check_grids_from_files(const char *player_path, const char *solution_path);
int grillesIdentiques(Grille *grille_solution, Grille *grille_jeu);

int afficherEcranFin(SDL_Renderer *renderer, SDL_Window *window, const char *message, int victoire, Uint32 temps_total);
int sauvegarderCapture(SDL_Renderer *renderer, SDL_Window *window, const char *nom_fichier);
void creerDossierSiAbsent(const char *chemin);

int sauvegarderPartie(Grille *grille_solution, Grille *grille_jeu, Uint32 temps_ecoule);
int chargerPartie(Grille **grille_solution, Grille **grille_jeu, Uint32 *temps_ecoule);
int partieExiste(void);
void supprimerSauvegarde(void);

SDL_Texture *texture_init(const char *fichier, SDL_Renderer *renderer);
Bouton creer_bouton(const char *fichier, SDL_Renderer *renderer, const char *label, TTF_Font *font, int x, int y);
Bouton creer_bouton_simple(SDL_Renderer *renderer, const char *label, TTF_Font *font, int x, int y, SDL_Color bg_color);
bool Clique(int souris_x, int souris_y, Bouton b);

void Menu_principal(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels);
int Menu_jouer(SDL_Renderer *renderer, SDL_Window *window, Grille *grille_solution, Grille *grille_jeu, int largeurGrillePixels, int hauteurGrillePixels, Uint32 temps_initial, int mode);
void Menu_parametres(SDL_Renderer *renderer, SDL_Window *window);
void Menu_mode(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels);
void Menu_selection_niveau_predefini(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels);

Grille *genererGrilleAleatoire(int taille);
void indice(Grille *grille_solution, Grille *grille_jeu);

#endif
