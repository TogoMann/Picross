#include "fonctions.h"

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return -1;
    }

    printf("SDL initialisé avec succès !\n");
    FILE *fichier = fopen("test.txt", "r");

    if (!fichier) {
        printf("Impossible d’ouvrir le fichier test.txt\n");
        SDL_Quit();
        return -1;
    }

    Grille *grille_solution = chargerGrillesolution(fichier);
    if (!grille_solution) {
        printf("Échec du chargement de la grille solution.\n");
        SDL_Quit();
        return -1;
    }

    Grille *grille_jeu = chargerGrilleSauvegarde("save.txt");
    if (!grille_jeu) {
        grille_jeu = chargerGrille(grille_solution);
        if (!grille_jeu) {
            printf("Échec du chargement de la grille de jeu.\n");
            libererGrille(grille_solution);
            SDL_Quit();
            return -1;
        }
    }

    int largeurFenetre = grille_solution->largeur * TAILLE_CASE;
    int hauteurFenetre = grille_solution->hauteur * TAILLE_CASE;
    SDL_Window *fenetre = SDL_CreateWindow("Affichage de la grille",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,largeurFenetre, hauteurFenetre, 0);

    if (!fenetre) {
        printf("Impossible de créer la fenêtre.\n");
        libererGrille(grille_jeu);
        libererGrille(grille_solution);
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu) {
        printf("Impossible de créer le renderer.\n");
        SDL_DestroyWindow(fenetre);
        libererGrille(grille_jeu);
        libererGrille(grille_solution);
        SDL_Quit();
        return -1;
    }

    SDL_Event evenement;
    int enCours = 1;

    while (enCours) {
        while (SDL_PollEvent(&evenement)) {
            if (evenement.type == SDL_QUIT) {
                enCours = 0;
            } else if (evenement.type == SDL_MOUSEBUTTONDOWN) {
                gererClic(grille_jeu, evenement.button.x, evenement.button.y);
                Verification(grille_solution, grille_jeu);
            } else if (evenement.type == SDL_KEYDOWN) {
                if (evenement.key.keysym.sym == SDLK_s) {
                    sauvegarderGrille("save.txt", grille_jeu);
                } else if (evenement.key.keysym.sym == SDLK_r) {
                    Grille *tmp = chargerGrilleSauvegarde("save.txt");
                    if (tmp) {
                        libererGrille(grille_jeu);
                        grille_jeu = tmp;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(rendu, 255, 255, 255, 255);
        SDL_RenderClear(rendu);
        dessinerGrille(rendu, grille_jeu);
        SDL_RenderPresent(rendu);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    libererGrille(grille_jeu);
    libererGrille(grille_solution);
    SDL_Quit();

    return 0;
}
