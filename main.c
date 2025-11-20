#include "fonctions.h"

typedef struct {
    SDL_Rect rect;
    const char *label;
} BoutonFinis;

static int point_in_rect(int x, int y, const SDL_Rect *r) {
    return (x >= r->x && x < r->x + r->w &&
            y >= r->y && y < r->y + r->h);
}

static void dessinerBoutonFinis(SDL_Renderer *rendu, const BoutonFinis *btn) {
    SDL_SetRenderDrawColor(rendu, 60, 60, 60, 255);
    SDL_RenderFillRect(rendu, &btn->rect);

    SDL_SetRenderDrawColor(rendu, 255, 255, 255, 255);
    SDL_RenderDrawRect(rendu, &btn->rect);

    int text_x = btn->rect.x + 10;
    int text_y = btn->rect.y + btn->rect.h / 4;
    stringRGBA(rendu, text_x, text_y, btn->label,
               255, 255, 255, 255);
}

static void afficherEcranFin(SDL_Renderer *rendu, const char *message) {
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(rendu, &w, &h);

    int enCours = 1;
    SDL_Event e;

    while (enCours) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                enCours = 0;
            } else if (e.type == SDL_KEYDOWN ||
                       e.type == SDL_MOUSEBUTTONDOWN) {
                enCours = 0;
            }
        }

        SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
        SDL_RenderClear(rendu);

        int x = w / 2 - 80;
        int y = h / 2;
        stringRGBA(rendu, x, y, message,
                   255, 255, 255, 255);

        SDL_RenderPresent(rendu);
        SDL_Delay(16);
    }
}

static int grillesIdentiques(Grille *grille_solution, Grille *grille_jeu) {
    if (!grille_solution || !grille_jeu) return 0;

    if (grille_solution->hauteur != grille_jeu->hauteur ||
        grille_solution->largeur != grille_jeu->largeur) {
        return 0;
    }

    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            if (grille_solution->cases[y][x] != grille_jeu->cases[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

int main(void) {

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
    fclose(fichier);

    if (!grille_solution) {
        printf("Échec du chargement de la grille solution.\n");
        SDL_Quit();
        return -1;
    }

    Grille *grille_jeu = chargerGrille(grille_solution);
    if (!grille_jeu) {
        printf("Échec du chargement de la grille de jeu.\n");
        libererGrille(grille_solution);
        SDL_Quit();
        return -1;
    }

    int largeurGrillePixels  = grille_solution->largeur * TAILLE_CASE;
    int hauteurGrillePixels  = grille_solution->hauteur * TAILLE_CASE;
    int largeurFenetre       = largeurGrillePixels + BTN_W + 20;
    int hauteurFenetre       = hauteurGrillePixels;

    SDL_Window *fenetre = SDL_CreateWindow(
        "Picross",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        largeurFenetre, hauteurFenetre,
        0
    );

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

    BoutonFinis boutonFinis;
    boutonFinis.rect.w = BTN_W;
    boutonFinis.rect.h = BTN_H;
    boutonFinis.rect.x = largeurGrillePixels + 10;
    boutonFinis.rect.y = 10;
    boutonFinis.label  = "finis";

    SDL_Event evenement;
    int enCours = 1;

    while (enCours) {
        while (SDL_PollEvent(&evenement)) {
            if (evenement.type == SDL_QUIT) {
                enCours = 0;
            } else if (evenement.type == SDL_MOUSEBUTTONDOWN &&
                       evenement.button.button == SDL_BUTTON_LEFT) {

                int mx = evenement.button.x;
                int my = evenement.button.y;

                if (point_in_rect(mx, my, &boutonFinis.rect)) {
                    int ok = grillesIdentiques(grille_solution, grille_jeu);
                    if (ok) {
                        afficherEcranFin(rendu, "Tout bon !");
                    } else {
                        afficherEcranFin(rendu, "Flop !");
                    }
                } else {
                    if (mx < largeurGrillePixels && my < hauteurGrillePixels) {
                        gererClic(grille_jeu, mx, my);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(rendu, 200, 200, 200, 255);
        SDL_RenderClear(rendu);

        dessinerGrille(rendu, grille_jeu);

        dessinerBoutonFinis(rendu, &boutonFinis);

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