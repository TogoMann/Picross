#include "fonctions.h"

void creerDossierSiAbsent(const char *chemin) {
    struct stat st = {0};
    if (stat(chemin, &st) == -1) {
        #ifdef _WIN32
            mkdir(chemin);
        #else
            mkdir(chemin, 0755);
        #endif
        printf("Dossier créé: %s\n", chemin);
    }
}

int sauvegarderPartie(Grille *grille_solution, Grille *grille_jeu, Uint32 temps_ecoule) {
    if (!grille_solution || !grille_jeu) return -1;

    FILE *f = fopen(FICHIER_SAUVEGARDE, "wb");
    if (!f) {
        printf("Erreur: Impossible de créer le fichier de sauvegarde\n");
        return -1;
    }

    fwrite(&grille_solution->hauteur, sizeof(int), 1, f);
    fwrite(&grille_solution->largeur, sizeof(int), 1, f);

    fwrite(&temps_ecoule, sizeof(Uint32), 1, f);

    for (int y = 0; y < grille_solution->hauteur; ++y) {
        fwrite(grille_solution->cases[y], sizeof(int), grille_solution->largeur, f);
    }

    for (int y = 0; y < grille_jeu->hauteur; ++y) {
        fwrite(grille_jeu->cases[y], sizeof(int), grille_jeu->largeur, f);
    }

    fclose(f);
    printf("Partie sauvegardée avec succès!\n");
    return 0;
}

int chargerPartie(Grille **grille_solution, Grille **grille_jeu, Uint32 *temps_ecoule) {
    FILE *f = fopen(FICHIER_SAUVEGARDE, "rb");
    if (!f) {
        printf("Erreur: Aucune sauvegarde trouvée\n");
        return -1;
    }

    int hauteur, largeur;
    Uint32 temps;

    if (fread(&hauteur, sizeof(int), 1, f) != 1 ||
        fread(&largeur, sizeof(int), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (fread(&temps, sizeof(Uint32), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (*grille_solution) libererGrille(*grille_solution);
    if (*grille_jeu) libererGrille(*grille_jeu);

    *grille_solution = malloc(sizeof(Grille));
    if (!*grille_solution) {
        fclose(f);
        return -1;
    }
    (*grille_solution)->hauteur = hauteur;
    (*grille_solution)->largeur = largeur;
    (*grille_solution)->taille = hauteur * largeur;
    (*grille_solution)->nombre_cases_noires = 0;
    (*grille_solution)->indices_lignes = NULL;
    (*grille_solution)->indices_colonnes = NULL;
    (*grille_solution)->nb_indices_lignes = NULL;
    (*grille_solution)->nb_indices_colonnes = NULL;

    (*grille_solution)->cases = malloc(hauteur * sizeof(int*));
    for (int y = 0; y < hauteur; ++y) {
        (*grille_solution)->cases[y] = malloc(largeur * sizeof(int));
        if (fread((*grille_solution)->cases[y], sizeof(int), largeur, f) != (size_t)largeur) {
            fclose(f);
            libererGrille(*grille_solution);
            *grille_solution = NULL;
            return -1;
        }
        for (int x = 0; x < largeur; ++x) {
            if ((*grille_solution)->cases[y][x] == ETAT_NOIR) {
                (*grille_solution)->nombre_cases_noires++;
            }
        }
    }

    *grille_jeu = malloc(sizeof(Grille));
    if (!*grille_jeu) {
        fclose(f);
        libererGrille(*grille_solution);
        *grille_solution = NULL;
        return -1;
    }
    (*grille_jeu)->hauteur = hauteur;
    (*grille_jeu)->largeur = largeur;
    (*grille_jeu)->taille = hauteur * largeur;
    (*grille_jeu)->nombre_cases_noires = 0;
    (*grille_jeu)->indices_lignes = NULL;
    (*grille_jeu)->indices_colonnes = NULL;
    (*grille_jeu)->nb_indices_lignes = NULL;
    (*grille_jeu)->nb_indices_colonnes = NULL;

    (*grille_jeu)->cases = malloc(hauteur * sizeof(int*));
    for (int y = 0; y < hauteur; ++y) {
        (*grille_jeu)->cases[y] = malloc(largeur * sizeof(int));
        if (fread((*grille_jeu)->cases[y], sizeof(int), largeur, f) != (size_t)largeur) {
            fclose(f);
            libererGrille(*grille_solution);
            libererGrille(*grille_jeu);
            *grille_solution = NULL;
            *grille_jeu = NULL;
            return -1;
        }
    }

    fclose(f);

    calculerEtStockerTousIndices(*grille_solution);

    *temps_ecoule = temps;
    printf("Partie chargée avec succès! Temps: %u ms\n", temps);
    return 0;
}

int partieExiste(void) {
    FILE *f = fopen(FICHIER_SAUVEGARDE, "rb");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

void supprimerSauvegarde(void) {
    remove(FICHIER_SAUVEGARDE);
    printf("Sauvegarde supprimée.\n");
}

void calculerRenderContext(RenderContext *ctx, int window_w, int window_h, Grille *grille) {
    if (!ctx || !grille) return;

    ctx->window_w = window_w;
    ctx->window_h = window_h;

    int espace_grille_w = window_w - OFFSET_X_BASE - 50;
    int espace_grille_h = window_h - OFFSET_Y_BASE - 50;

    int taille_par_largeur = espace_grille_w / grille->largeur;
    int taille_par_hauteur = espace_grille_h / grille->hauteur;

    ctx->taille_case = (taille_par_largeur < taille_par_hauteur) ? taille_par_largeur : taille_par_hauteur;

    if (ctx->taille_case < 15) ctx->taille_case = 15;
    if (ctx->taille_case > 50) ctx->taille_case = 50;

    ctx->scale = (float)ctx->taille_case / TAILLE_CASE_BASE;

    ctx->offset_x = (int)(OFFSET_X_BASE * ctx->scale);
    ctx->offset_y = (int)(OFFSET_Y_BASE * ctx->scale);

    if (ctx->offset_x < 150) ctx->offset_x = 150;
    if (ctx->offset_y < 120) ctx->offset_y = 120;
}

void dessinerChronometre(SDL_Renderer *renderer, Uint32 temps_ecoule, int x, int y, TTF_Font *font) {
    if (!renderer || !font) return;

    Uint32 secondes_totales = temps_ecoule / 1000;
    Uint32 minutes = secondes_totales / 60;
    Uint32 secondes = secondes_totales % 60;

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Temps: %02u:%02u", minutes, secondes);

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Surface *surf = TTF_RenderText_Solid(font, buffer, blanc);
    if (surf) {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect rect = {x, y, surf->w, surf->h};
        SDL_FreeSurface(surf);
        SDL_RenderCopy(renderer, tex, NULL, &rect);
        SDL_DestroyTexture(tex);
    }
}

Bouton creer_bouton_simple(SDL_Renderer *renderer, const char *label, TTF_Font *font, int x, int y, SDL_Color bg_color) {
    Bouton b;
    b.texture = NULL;

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Surface *surf_texte = TTF_RenderText_Solid(font, label, blanc);
    b.texte = SDL_CreateTextureFromSurface(renderer, surf_texte);

    int text_w = surf_texte->w;
    int text_h = surf_texte->h;
    SDL_FreeSurface(surf_texte);

    b.rect.x = x;
    b.rect.y = y;
    b.rect.w = text_w + 40;
    b.rect.h = text_h + 20;

    b.rect_texte.w = text_w;
    b.rect_texte.h = text_h;
    b.rect_texte.x = b.rect.x + (b.rect.w - text_w) / 2;
    b.rect_texte.y = b.rect.y + (b.rect.h - text_h) / 2;

    SDL_Surface *bg_surf = SDL_CreateRGBSurface(0, b.rect.w, b.rect.h, 32, 0, 0, 0, 0);
    SDL_FillRect(bg_surf, NULL, SDL_MapRGB(bg_surf->format, bg_color.r, bg_color.g, bg_color.b));
    b.texture = SDL_CreateTextureFromSurface(renderer, bg_surf);
    SDL_FreeSurface(bg_surf);

    return b;
}

int sauvegarderCapture(SDL_Renderer *renderer, SDL_Window *window, const char *nom_fichier) {
    if (!renderer || !window || !nom_fichier) return -1;

    creerDossierSiAbsent(DOSSIER_CAPTURES);

    char chemin_complet[512];
    snprintf(chemin_complet, sizeof(chemin_complet), "%s/%s", DOSSIER_CAPTURES, nom_fichier);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    if (!surface) {
        printf("Erreur création surface: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderReadPixels(renderer, NULL, surface->format->format,
                             surface->pixels, surface->pitch) != 0) {
        printf("Erreur lecture pixels: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    if (IMG_SavePNG(surface, chemin_complet) != 0) {
        printf("Erreur sauvegarde PNG: %s\n", IMG_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    SDL_FreeSurface(surface);
    printf("Capture sauvegardée: %s\n", chemin_complet);
    return 0;
}

static int **allouerCases(int hauteur, int largeur) {
    int **cases = malloc(hauteur * sizeof(int *));
    if (!cases){
        return NULL;
    }

    for (int i = 0; i < hauteur; ++i)
    {
        cases[i] = calloc(largeur, sizeof(int));

        if (!cases[i])
        {
            printf("Erreur d'allocation des cases\n");
            for (int j = 0; j < i; ++j)
            {
                free(cases[j]);
            }
            free(cases);
            return NULL;
        }
    }
    return cases;
}

Grille *chargerGrillesolution(FILE *fichier) {

    if (!fichier)
    {
        printf("Erreur fichier\n");
        return NULL;
    }

    char ligne[1024];
    int **cases = NULL;
    int hauteur = 0;
    int largeur = 0;
    int capacite = 0;
    int nb_noires = 0;

    while (fgets(ligne, sizeof(ligne), fichier))
    {
        size_t longueur = strlen(ligne);

        while (longueur > 0 && (ligne[longueur - 1] == '\n' || ligne[longueur - 1] == '\r'))
        {
            ligne[--longueur] = '\0';
        }
        int count = 0;

        for (size_t i = 0; i < longueur; ++i)
        {
            if (ligne[i] == '0' || ligne[i] == '1')
            {
                count++;
            }
        }
        if (count == 0) continue;
        if (largeur == 0) largeur = count;
        else if (largeur != count) {
            for (int r = 0; r < hauteur; ++r) free(cases[r]);
            free(cases);
            return NULL;
        }
        if (hauteur >= capacite) {
            int nouvelle_capacite = (capacite == 0) ? 4 : capacite * 2;
            int **nouv = realloc(cases, nouvelle_capacite * sizeof(int *));
            if (!nouv) {
                for (int r = 0; r < hauteur; ++r) free(cases[r]);
                free(cases);
                return NULL;
            }
            cases = nouv;
            capacite = nouvelle_capacite;
        }
        int *row = malloc(largeur * sizeof(int));
        if (!row) {
            for (int r = 0; r < hauteur; ++r) free(cases[r]);
            free(cases);
            return NULL;
        }
        int c = 0;
        for (size_t i = 0; i < longueur && c < largeur; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                row[c] = (ligne[i] == '1') ? 1 : 0;
                if (row[c] == 1) nb_noires++;
                c++;
            }
        }
        cases[hauteur++] = row;
    }
    if (hauteur == 0 || largeur == 0) {
        free(cases);
        return NULL;
    }
    Grille *g = malloc(sizeof(Grille));
    if (!g) {
        for (int r = 0; r < hauteur; ++r) free(cases[r]);
        free(cases);
        return NULL;
    }
    g->cases = cases;
    g->hauteur = hauteur;
    g->largeur = largeur;
    g->taille = hauteur * largeur;
    g->nombre_cases_noires = nb_noires;

    g->indices_lignes = NULL;
    g->indices_colonnes = NULL;
    g->nb_indices_lignes = NULL;
    g->nb_indices_colonnes = NULL;

    return g;
}

Grille *chargerGrille(Grille *grille_solution) {

    if (!grille_solution)
    {
        printf("Erreur chargement grille \n");
        return NULL;
    }
    Grille *g = malloc(sizeof(Grille));

    if (!g)
    {
        printf("Erreur allocation grille \n");
        return NULL;
    }
    g->hauteur = grille_solution->hauteur;
    g->largeur = grille_solution->largeur;
    g->taille  = g->hauteur * g->largeur;
    g->nombre_cases_noires = 0;
    g->cases = allouerCases(g->hauteur, g->largeur);
    if (!g->cases) {
        free(g);
        return NULL;
    }
    g->indices_lignes = NULL;
    g->indices_colonnes = NULL;
    g->nb_indices_lignes = NULL;
    g->nb_indices_colonnes = NULL;

    return g;
}

int read_grid_from_txt(const char *path, int ***out_grid, size_t *out_rows, size_t *out_cols) {
    if (!path || !out_grid || !out_rows || !out_cols) {
        return -1;
    }

    FILE *fichier = fopen(path, "r");
    if (!fichier) {
        return -1;
    }

    char ligne[1024];
    int **cases = NULL;
    size_t hauteur = 0;
    size_t largeur = 0;
    size_t capacite = 0;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        size_t longueur = strlen(ligne);

        while (longueur > 0 && (ligne[longueur - 1] == '\n' || ligne[longueur - 1] == '\r')) {
            ligne[--longueur] = '\0';
        }

        size_t count = 0;
        for (size_t i = 0; i < longueur; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                count++;
            }
        }

        if (count == 0) continue;

        if (largeur == 0) {
            largeur = count;
        } else if (largeur != count) {
            for (size_t r = 0; r < hauteur; ++r) free(cases[r]);
            free(cases);
            fclose(fichier);
            return -1;
        }

        if (hauteur >= capacite) {
            size_t nouvelle_capacite = (capacite == 0) ? 4 : capacite * 2;
            int **nouv = realloc(cases, nouvelle_capacite * sizeof(int *));
            if (!nouv) {
                for (size_t r = 0; r < hauteur; ++r) free(cases[r]);
                free(cases);
                fclose(fichier);
                return -1;
            }
            cases = nouv;
            capacite = nouvelle_capacite;
        }

        int *row = malloc(largeur * sizeof(int));
        if (!row) {
            for (size_t r = 0; r < hauteur; ++r) free(cases[r]);
            free(cases);
            fclose(fichier);
            return -1;
        }

        size_t c = 0;
        for (size_t i = 0; i < longueur && c < largeur; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                row[c] = (ligne[i] == '1') ? 1 : 0;
                c++;
            }
        }
        cases[hauteur++] = row;
    }

    fclose(fichier);

    if (hauteur == 0 || largeur == 0) {
        free(cases);
        return -1;
    }

    *out_grid = cases;
    *out_rows = hauteur;
    *out_cols = largeur;

    return 0;
}

int *calculerIndices(int *ligne, int taille, int *nb_indices)
{
    int *indices = malloc(taille * sizeof(int));
    int count = 0;
    int n = 0;
    for (int i = 0; i < taille; ++i) {
        if (ligne[i] == 1)
        {
            n++;
        }
        else if (n > 0)
        {
             indices[count++] = n;
             n = 0;
        }
    }

    if (n > 0)
    {
        indices[count++] = n;
    }

    *nb_indices = count;
    return indices;
}

void libererIndices(Grille *grille)
{
    if (!grille){
        printf("Erreur chargement grille pour libérer indices\n");
        return;
    }

    if (grille->indices_lignes)
    {
        for (int i = 0; i < grille->hauteur; ++i)
        {
            free(grille->indices_lignes[i]);
        }
        free(grille->indices_lignes);
    }

    if (grille->indices_colonnes)
    {
        for (int i = 0; i < grille->largeur; ++i)
        {
            free(grille->indices_colonnes[i]);
        }
        free(grille->indices_colonnes);
    }

    free(grille->nb_indices_lignes);
    free(grille->nb_indices_colonnes);

    grille->indices_lignes = NULL;
    grille->indices_colonnes = NULL;
    grille->nb_indices_lignes = NULL;
    grille->nb_indices_colonnes = NULL;
}

void calculerEtStockerTousIndices(Grille *grille_solution){

    if (!grille_solution)
    {
        printf("Erreur de chargement grille pour stockage indice\n");
        return;
    }

    int H = grille_solution->hauteur;
    int L = grille_solution->largeur;

    grille_solution->indices_lignes = malloc(H * sizeof(int*));
    grille_solution->indices_colonnes = malloc(L * sizeof(int*));
    grille_solution->nb_indices_lignes = malloc(H * sizeof(int));
    grille_solution->nb_indices_colonnes = malloc(L * sizeof(int));

    if (!grille_solution->indices_lignes || !grille_solution->indices_colonnes || !grille_solution->nb_indices_lignes || !grille_solution->nb_indices_colonnes) {
        libererIndices(grille_solution);
        return;
    }

    for (int y = 0; y < H; ++y)
    {
        int nb;
        int *indices = calculerIndices(grille_solution->cases[y], L, &nb);
        grille_solution->indices_lignes[y] = indices;
        grille_solution->nb_indices_lignes[y] = nb;
    }

    for (int x = 0; x < L; ++x)
    {
        int col[H];

        for (int y = 0; y < H; ++y)
        {
            col[y] = grille_solution->cases[y][x];
        }

        int nb;
        int *indices = calculerIndices(col, H, &nb);
        grille_solution->indices_colonnes[x] = indices;
        grille_solution->nb_indices_colonnes[x] = nb;
    }
}

void libererGrille(Grille *grille)
{
    if (!grille)
    {
        printf("Erreur chargement grille pour libérer grille\n");
        return;
    }

    libererIndices(grille);

    if (grille->cases)
    {
        for (int i = 0; i < grille->hauteur; ++i)
        {
            free(grille->cases[i]);
        }
        free(grille->cases);
    }
    free(grille);
}

void dessinerHexagone(SDL_Renderer *renderer, int cx, int cy, int rayon, int rempli, SDL_Color couleur) {
    Sint16 vx[6], vy[6];
    for (int i = 0; i < 6; i++) {
        double angle_deg = 30 + 60 * i;
        double angle_rad = angle_deg * (M_PI / 180.0);
        vx[i] = (Sint16)(cx + rayon * cos(angle_rad));
        vy[i] = (Sint16)(cy + rayon * sin(angle_rad));
    }
    if (rempli) {
        filledPolygonRGBA(renderer, vx, vy, 6, couleur.r, couleur.g, couleur.b, couleur.a);
    }
    polygonRGBA(renderer, vx, vy, 6, 0, 0, 0, 255);
}

void dessinerGrille(SDL_Renderer *renderer, Grille *grille_jeu, Grille *grille_solution, RenderContext *ctx, int mode) {

    if (!grille_jeu || !grille_solution || !renderer || !ctx)
    {
        printf("Erreur dessin grille\n");
        return;
    }

    int OFFSET_X = ctx->offset_x;
    int OFFSET_Y = ctx->offset_y;
    int TAILLE_CASE = ctx->taille_case;

    int font_size = (int)(18 * ctx->scale);
    if (font_size < 10) font_size = 10;
    if (font_size > 24) font_size = 24;
    TTF_Font *font = TTF_OpenFont("fonts/sans.ttf", font_size);
    if (!font) return;
    SDL_Color couleurTexte = {0, 0, 0, 255};

    if (mode == MODE_NORMAL) {
        SDL_Rect fondGauche = {0, OFFSET_Y, OFFSET_X, grille_solution->hauteur * TAILLE_CASE};
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_RenderFillRect(renderer, &fondGauche);

        SDL_Rect fondHaut = {OFFSET_X, 0, grille_solution->largeur * TAILLE_CASE, OFFSET_Y};
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_RenderFillRect(renderer, &fondHaut);

        SDL_Rect coinHautGauche = {0, 0, OFFSET_X, OFFSET_Y};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &coinHautGauche);

        for (int y = 0; y < grille_jeu->hauteur; ++y)
        {
            for (int x = 0; x < grille_jeu->largeur; ++x)
            {
                SDL_Rect rect = {x * TAILLE_CASE + OFFSET_X + 1, y * TAILLE_CASE + OFFSET_Y + 1, TAILLE_CASE - 2, TAILLE_CASE - 2};
                if (grille_jeu->cases[y][x] == ETAT_NOIR) SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &rect);
                if (grille_jeu->cases[y][x] == ETAT_MARQUE)
                {
                    int x1 = x * TAILLE_CASE + OFFSET_X;
                    int y1 = y * TAILLE_CASE + OFFSET_Y;
                    int x2 = x1 + TAILLE_CASE;
                    int y2 = y1 + TAILLE_CASE;
                    int PADDING = TAILLE_CASE / 6;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawLine(renderer, x1 + PADDING, y1 + PADDING, x2 - PADDING, y2 - PADDING);
                    SDL_RenderDrawLine(renderer, x2 - PADDING, y1 + PADDING, x1 + PADDING, y2 - PADDING);
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int y = 0; y <= grille_solution->hauteur; ++y)
        {
            SDL_RenderDrawLine(renderer, 0, OFFSET_Y + y * TAILLE_CASE, OFFSET_X + grille_solution->largeur * TAILLE_CASE, OFFSET_Y + y * TAILLE_CASE);
        }
        for (int x = 0; x <= grille_solution->largeur; ++x)
        {
            SDL_RenderDrawLine(renderer, OFFSET_X + x * TAILLE_CASE, 0, OFFSET_X + x * TAILLE_CASE, OFFSET_Y + grille_solution->hauteur * TAILLE_CASE);
        }
        SDL_RenderDrawLine(renderer, 0, OFFSET_Y, OFFSET_X + grille_solution->largeur * TAILLE_CASE, OFFSET_Y);
        SDL_RenderDrawLine(renderer, OFFSET_X, 0, OFFSET_X, OFFSET_Y + grille_solution->hauteur * TAILLE_CASE);

        for (int y = 0; y < grille_solution->hauteur; ++y)
        {
            int nb;
            int *indices;
            int indices_alloues = 0;
            if (grille_solution->indices_lignes && grille_solution->nb_indices_lignes) {
                indices = grille_solution->indices_lignes[y];
                nb = grille_solution->nb_indices_lignes[y];
            } else {
                indices = calculerIndices(grille_solution->cases[y], grille_solution->largeur, &nb);
                indices_alloues = 1;
            }
            int espace = (int)(25 * ctx->scale);
            if (espace < 15) espace = 15;
            for (int i = 0; i < nb; ++i)
            {
                char buf[12];
                snprintf(buf, sizeof(buf), "%d", indices[i]);
                SDL_Surface *surf = TTF_RenderText_Solid(font, buf, couleurTexte);
                if(surf) {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                    int tw = surf->w;
                    int th = surf->h;
                    SDL_FreeSurface(surf);
                    int pos_x = OFFSET_X - ((nb - i) * espace) + (espace - tw) / 2;
                    int pos_y = y * TAILLE_CASE + OFFSET_Y + (TAILLE_CASE - th) / 2;
                    SDL_Rect r = { pos_x, pos_y, tw, th };
                    SDL_RenderCopy(renderer, tex, NULL, &r);
                    SDL_DestroyTexture(tex);
                }
            }
            if (indices_alloues) free(indices);
        }
        for (int x = 0; x < grille_solution->largeur; ++x)
        {
            int nb;
            int *indices;
            int indices_alloues = 0;
            if (grille_solution->indices_colonnes && grille_solution->nb_indices_colonnes) {
                indices = grille_solution->indices_colonnes[x];
                nb = grille_solution->nb_indices_colonnes[x];
            } else {
                int col[grille_solution->hauteur];
                for (int y = 0; y < grille_solution->hauteur; ++y) col[y] = grille_solution->cases[y][x];
                indices = calculerIndices(col, grille_solution->hauteur, &nb);
                indices_alloues = 1;
            }
            int espace = (int)(20 * ctx->scale);
            if (espace < 12) espace = 12;
            for (int i = 0; i < nb; ++i)
            {
                char b[12];
                snprintf(b, sizeof(b), "%d", indices[i]);
                SDL_Surface *surf = TTF_RenderText_Solid(font, b, couleurTexte);
                if(surf)
                {
                    SDL_Texture *tx = SDL_CreateTextureFromSurface(renderer, surf);
                    int tw = surf->w;
                    int th = surf->h;
                    SDL_FreeSurface(surf);
                    int pos_x = x * TAILLE_CASE + OFFSET_X + (TAILLE_CASE - tw) / 2;
                    int pos_y = OFFSET_Y - ((nb - i) * espace) + (espace - th) / 2;
                    SDL_Rect r = { pos_x, pos_y, tw, th };
                    SDL_RenderCopy(renderer, tx, NULL, &r);
                    SDL_DestroyTexture(tx);
                }
            }
            if (indices_alloues) free(indices);
        }
    }
    else {
        float radius = TAILLE_CASE / 2.0f;
        float hexWidth = sqrt(3) * radius;
        float vertDist = 1.5f * radius;

        for (int y = 0; y < grille_jeu->hauteur; ++y) {
            for (int x = 0; x < grille_jeu->largeur; ++x) {
                float xPos = OFFSET_X + x * hexWidth;
                if (y % 2 != 0) xPos += hexWidth / 2.0f;
                float yPos = OFFSET_Y + y * vertDist;

                SDL_Color couleur;
                if (grille_jeu->cases[y][x] == ETAT_NOIR) { couleur.r=0; couleur.g=0; couleur.b=0; couleur.a=255; }
                else if (grille_jeu->cases[y][x] == ETAT_MARQUE) { couleur.r=150; couleur.g=150; couleur.b=150; couleur.a=255; }
                else { couleur.r=255; couleur.g=255; couleur.b=255; couleur.a=255; }

                dessinerHexagone(renderer, (int)xPos, (int)yPos, (int)radius, 1, couleur);
            }
        }

        for (int y = 0; y < grille_solution->hauteur; ++y) {
            int nb;
            int *indices;
            int indices_alloues = 0;
            if (grille_solution->indices_lignes && grille_solution->nb_indices_lignes) {
                indices = grille_solution->indices_lignes[y];
                nb = grille_solution->nb_indices_lignes[y];
            } else {
                indices = calculerIndices(grille_solution->cases[y], grille_solution->largeur, &nb);
                indices_alloues = 1;
            }
            float startX = OFFSET_X;
            if (y % 2 != 0) startX += hexWidth / 2.0f;
            float yPos = OFFSET_Y + y * vertDist;

            for (int i = 0; i < nb; ++i) {
                char buf[12];
                snprintf(buf, sizeof(buf), "%d", indices[i]);
                float xInd = startX - (nb - i) * (hexWidth * 0.8f) - 15;

                SDL_Surface *surf = TTF_RenderText_Solid(font, buf, couleurTexte);
                if(surf) {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_Rect r = {(int)xInd, (int)(yPos-10), surf->w, surf->h};
                    SDL_RenderCopy(renderer, tex, NULL, &r);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
            if (indices_alloues) free(indices);
        }

        for (int x = 0; x < grille_solution->largeur; ++x) {
            int nb;
            int *indices;
            int indices_alloues = 0;
            if (grille_solution->indices_colonnes && grille_solution->nb_indices_colonnes) {
                indices = grille_solution->indices_colonnes[x];
                nb = grille_solution->nb_indices_colonnes[x];
            } else {
                int col[grille_solution->hauteur];
                for(int y=0; y<grille_solution->hauteur; ++y) col[y] = grille_solution->cases[y][x];
                indices = calculerIndices(col, grille_solution->hauteur, &nb);
                indices_alloues = 1;
            }
            float xPos = OFFSET_X + x * hexWidth;
            float yPos = OFFSET_Y - 25;

            for (int i = 0; i < nb; ++i) {
                char buf[12];
                snprintf(buf, sizeof(buf), "%d", indices[i]);
                float yInd = yPos - (nb - i) * 20;

                SDL_Surface *surf = TTF_RenderText_Solid(font, buf, couleurTexte);
                if(surf) {
                    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_Rect r = {(int)(xPos-5), (int)yInd, surf->w, surf->h};
                    SDL_RenderCopy(renderer, tex, NULL, &r);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
            if (indices_alloues) free(indices);
        }
    }

    TTF_CloseFont(font);
}

void gererClicDroit(Grille *grille, int x, int y, RenderContext *ctx, int mode)
{
    if (!grille || !ctx) return;

    if (mode == MODE_NORMAL) {
        int col = (x - ctx->offset_x) / ctx->taille_case;
        int lig = (y - ctx->offset_y) / ctx->taille_case;

        if (lig < 0 || lig >= grille->hauteur || col < 0 || col >= grille->largeur) return;

        if (grille->cases[lig][col] == ETAT_VIDE) grille->cases[lig][col] = ETAT_MARQUE;
        else if (grille->cases[lig][col] == ETAT_MARQUE) grille->cases[lig][col] = ETAT_VIDE;
    }
    else {
        float radius = ctx->taille_case / 2.0f;
        float hexWidth = sqrt(3) * radius;
        float vertDist = 1.5f * radius;
        float seuilSq = (radius * 0.9f) * (radius * 0.9f);

        for (int lig = 0; lig < grille->hauteur; ++lig) {
            for (int col = 0; col < grille->largeur; ++col) {
                float cx = ctx->offset_x + col * hexWidth;
                if (lig % 2 != 0) cx += hexWidth / 2.0f;
                float cy = ctx->offset_y + lig * vertDist;

                float dx = x - cx;
                float dy = y - cy;
                if (dx*dx + dy*dy < seuilSq) {
                    if (grille->cases[lig][col] == ETAT_VIDE) grille->cases[lig][col] = ETAT_MARQUE;
                    else if (grille->cases[lig][col] == ETAT_MARQUE) grille->cases[lig][col] = ETAT_VIDE;
                    return;
                }
            }
        }
    }
}

void gererClic(Grille *grille, int x, int y, RenderContext *ctx, int mode)
{
    if (!grille || !ctx) return;

    if (mode == MODE_NORMAL) {
        int col = (x - ctx->offset_x) / ctx->taille_case;
        int lig = (y - ctx->offset_y) / ctx->taille_case;

        if (lig < 0 || lig >= grille->hauteur || col < 0 || col >= grille->largeur) return;

        if (grille->cases[lig][col] == ETAT_MARQUE) grille->cases[lig][col] = ETAT_VIDE;
        grille->cases[lig][col] = (grille->cases[lig][col] == ETAT_NOIR) ? ETAT_VIDE : ETAT_NOIR;
    }
    else {
        float radius = ctx->taille_case / 2.0f;
        float hexWidth = sqrt(3) * radius;
        float vertDist = 1.5f * radius;
        float seuilSq = (radius * 0.9f) * (radius * 0.9f);

        for (int lig = 0; lig < grille->hauteur; ++lig) {
            for (int col = 0; col < grille->largeur; ++col) {
                float cx = ctx->offset_x + col * hexWidth;
                if (lig % 2 != 0) cx += hexWidth / 2.0f;
                float cy = ctx->offset_y + lig * vertDist;

                float dx = x - cx;
                float dy = y - cy;
                if (dx*dx + dy*dy < seuilSq) {
                    if (grille->cases[lig][col] == ETAT_MARQUE) grille->cases[lig][col] = ETAT_VIDE;
                    grille->cases[lig][col] = (grille->cases[lig][col] == ETAT_NOIR) ? ETAT_VIDE : ETAT_NOIR;
                    return;
                }
            }
        }
    }
}

void Verification(Grille *grille_solution, Grille *grille_jeu) {
    if (!grille_solution || !grille_jeu) {
        printf("Erreur chargement grille jeu ou solution pour vérifications\n");
        return;
    }
    if (grille_solution->hauteur != grille_jeu->hauteur || grille_solution->largeur != grille_jeu->largeur) {
        printf("Flop ! (dimensions différentes)\n");
        return;
    }
    int ok = 1;
    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            int jeu_case = grille_jeu->cases[y][x];
            int sol_case = grille_solution->cases[y][x];
            int jeu_effectif = (jeu_case == ETAT_MARQUE) ? ETAT_VIDE : jeu_case;
            if (sol_case == ETAT_NOIR) {
                if (jeu_effectif != ETAT_NOIR) { ok = 0; break; }
            }
            else {
                if (jeu_effectif == ETAT_NOIR) { ok = 0; break; }
            }
        }
        if (!ok) break;
    }
    if (ok) printf("Tout bon !\n"); else printf("Flop !\n");
}

void free_grid(int **grid, size_t rows) {
    if (!grid) return;
    for (size_t r = 0; r < rows; ++r) free(grid[r]);
    free(grid);
}

void print_grid(const int * const *grid, size_t rows, size_t cols) {
    if (!grid) return;
    printf("Grille (%zu x %zu):\n", rows, cols);
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) printf("%d", grid[r][c]);
        printf("\n");
    }
}

int compare_grids(const int * const *g1, const int * const *g2, size_t rows, size_t cols) {
    if (!g1 || !g2) return 0;
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            if (g1[r][c] != g2[r][c]) return 0;
        }
    }
    return 1;
}

int check_grids_from_files(const char *player_path, const char *solution_path) {
    int **g1 = NULL, **g2 = NULL;
    size_t r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    int rc1 = read_grid_from_txt(player_path, &g1, &r1, &c1);
    int rc2 = read_grid_from_txt(solution_path, &g2, &r2, &c2);
    if (rc1 != 0 || rc2 != 0) {
        if (g1) free_grid(g1, r1);
        if (g2) free_grid(g2, r2);
        return -1;
    }
    if (r1 != r2 || c1 != c2) {
        free_grid(g1, r1);
        free_grid(g2, r2);
        return 0;
    }
    int same = compare_grids((const int * const *)g1, (const int * const *)g2, r1, c1);
    free_grid(g1, r1);
    free_grid(g2, r2);
    return same ? 1 : 0;
}

int afficherEcranFin(SDL_Renderer *renderer, SDL_Window *window, const char *message, int victoire, Uint32 temps_total) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);

    TTF_Font *font = TTF_OpenFont("fonts/font.otf", 36);
    TTF_Font *font_small = TTF_OpenFont("fonts/sans.ttf", 24);

    if (!font || !font_small) {
        if (font) TTF_CloseFont(font);
        if (font_small) TTF_CloseFont(font_small);
        return 0;
    }

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color vert = {50, 200, 50, 255};
    SDL_Color rouge = {200, 50, 50, 255};

    Bouton btn_oui, btn_non;
    int has_buttons = 0;

    if (victoire) {
        btn_oui = creer_bouton_simple(renderer, "OUI", font_small, w/2 - 120, h/2 + 100, vert);
        btn_non = creer_bouton_simple(renderer, "NON", font_small, w/2 + 20, h/2 + 100, rouge);
        has_buttons = 1;
    }

    int enCours = 1;
    int sauvegarde_acceptee = 0;
    SDL_Event e;

    while (enCours) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                enCours = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;

                if (has_buttons) {
                    if (Clique(mx, my, btn_oui)) {
                        sauvegarde_acceptee = 1;
                        enCours = 0;
                    }
                    else if (Clique(mx, my, btn_non)) {
                        sauvegarde_acceptee = 0;
                        enCours = 0;
                    }
                } else {
                    enCours = 0;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                if (!victoire) {
                    enCours = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect fond = {w/4, h/4, w/2, h/2};
        SDL_RenderFillRect(renderer, &fond);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &fond);

        SDL_Surface *surf_msg = TTF_RenderText_Solid(font, message, blanc);
        if (surf_msg) {
            SDL_Texture *tex_msg = SDL_CreateTextureFromSurface(renderer, surf_msg);
            SDL_Rect rect_msg = {w/2 - surf_msg->w/2, h/2 - 80, surf_msg->w, surf_msg->h};
            SDL_FreeSurface(surf_msg);
            SDL_RenderCopy(renderer, tex_msg, NULL, &rect_msg);
            SDL_DestroyTexture(tex_msg);
        }

        Uint32 secondes = temps_total / 1000;
        Uint32 minutes = secondes / 60;
        secondes = secondes % 60;
        char temps_str[64];
        snprintf(temps_str, sizeof(temps_str), "Temps: %02u:%02u", minutes, secondes);

        SDL_Surface *surf_temps = TTF_RenderText_Solid(font_small, temps_str, blanc);
        if (surf_temps) {
            SDL_Texture *tex_temps = SDL_CreateTextureFromSurface(renderer, surf_temps);
            SDL_Rect rect_temps = {w/2 - surf_temps->w/2, h/2 - 20, surf_temps->w, surf_temps->h};
            SDL_FreeSurface(surf_temps);
            SDL_RenderCopy(renderer, tex_temps, NULL, &rect_temps);
            SDL_DestroyTexture(tex_temps);
        }

        if (victoire) {
            const char *question = "Sauvegarder le design ?";
            SDL_Surface *surf_q = TTF_RenderText_Solid(font_small, question, blanc);
            if (surf_q) {
                SDL_Texture *tex_q = SDL_CreateTextureFromSurface(renderer, surf_q);
                SDL_Rect rect_q = {w/2 - surf_q->w/2, h/2 + 40, surf_q->w, surf_q->h};
                SDL_FreeSurface(surf_q);
                SDL_RenderCopy(renderer, tex_q, NULL, &rect_q);
                SDL_DestroyTexture(tex_q);
            }

            SDL_RenderCopy(renderer, btn_oui.texture, NULL, &btn_oui.rect);
            SDL_RenderCopy(renderer, btn_oui.texte, NULL, &btn_oui.rect_texte);

            SDL_RenderCopy(renderer, btn_non.texture, NULL, &btn_non.rect);
            SDL_RenderCopy(renderer, btn_non.texte, NULL, &btn_non.rect_texte);
        } else {
            const char *info = "Cliquez pour continuer...";
            SDL_Surface *surf_info = TTF_RenderText_Solid(font_small, info, blanc);
            if (surf_info) {
                SDL_Texture *tex_info = SDL_CreateTextureFromSurface(renderer, surf_info);
                SDL_Rect rect_info = {w/2 - surf_info->w/2, h/2 + 60, surf_info->w, surf_info->h};
                SDL_FreeSurface(surf_info);
                SDL_RenderCopy(renderer, tex_info, NULL, &rect_info);
                SDL_DestroyTexture(tex_info);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (has_buttons) {
        SDL_DestroyTexture(btn_oui.texture);
        SDL_DestroyTexture(btn_oui.texte);
        SDL_DestroyTexture(btn_non.texture);
        SDL_DestroyTexture(btn_non.texte);
    }
    TTF_CloseFont(font);
    TTF_CloseFont(font_small);

    return sauvegarde_acceptee;
}

int grillesIdentiques(Grille *grille_solution, Grille *grille_jeu) {
    if (!grille_solution || !grille_jeu) return 0;

    if (grille_solution->hauteur != grille_jeu->hauteur ||
        grille_solution->largeur != grille_jeu->largeur) {
        return 0;
    }

    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            int sol_case = grille_solution->cases[y][x];
            int jeu_case = grille_jeu->cases[y][x];

            int jeu_effectif = (jeu_case == ETAT_MARQUE) ? ETAT_VIDE : jeu_case;

            if (sol_case != jeu_effectif) {
                return 0;
            }
        }
    }
    return 1;
}

SDL_Texture * texture_init(const char * fichier, SDL_Renderer * renderer)
{
    SDL_Texture *texture = IMG_LoadTexture(renderer, fichier);
    if (!texture){
    printf("Impossible de charger la texture %s : %s\n", fichier, IMG_GetError());
    }
    return texture;
}

Bouton creer_bouton(const char *fichier, SDL_Renderer *renderer, const char *label, TTF_Font *font, int x, int y)
{
    Bouton b;

    b.texture = texture_init(fichier, renderer);
    b.rect.x = x;
    b.rect.y = y;
    b.rect.w = BTN_W;
    b.rect.h = BTN_H;

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Surface *surf_texte = TTF_RenderText_Solid(font, label, blanc);
    b.texte = SDL_CreateTextureFromSurface(renderer, surf_texte);
    SDL_FreeSurface(surf_texte);

    int text_w, text_h;
    SDL_QueryTexture(b.texte, NULL, NULL, &text_w, &text_h);

    if (text_w > b.rect.w) b.rect.w = text_w + 20;
    if (text_h > b.rect.h) b.rect.h = text_h + 10;
    b.rect_texte.w = text_w;
    b.rect_texte.h = text_h;
    b.rect_texte.x = b.rect.x + (b.rect.w - text_w)/2;
    b.rect_texte.y = b.rect.y + (b.rect.h - text_h)/2;

    return b;
}

bool Clique(int souris_x, int souris_y, Bouton b)
{
    if (souris_x >= b.rect.x && souris_y >= b.rect.y && souris_x <= b.rect.x + b.rect.w && souris_y <= b.rect.y + b.rect.h)
    {
        return true;
    }
    return false;
}

void Menu_principal(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels)
{
    (void)window;

    Mix_Music *musique = Mix_LoadMUS("music/musique.wav");
    if (musique) Mix_PlayMusic(musique, -1);

    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) return;

    TTF_Font *font = TTF_OpenFont("fonts/font.otf", 36);
    TTF_Font *font_small = TTF_OpenFont("fonts/sans.ttf", 24);
    if (!font || !font_small) {
        if (font) TTF_CloseFont(font);
        if (font_small) TTF_CloseFont(font_small);
        return;
    }

    SDL_Color blanc = {255, 255, 255, 255};

    SDL_Surface *surf_titre = TTF_RenderText_Solid(font, "Picross Game", blanc);
    SDL_Texture *texture_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
    SDL_FreeSurface(surf_titre);
    int titre_w, titre_h;
    SDL_QueryTexture(texture_titre, NULL, NULL, &titre_w, &titre_h);
    SDL_Rect rect_titre = { (DEFAULT_WINDOW_W - titre_w) / 2, 50, titre_w, titre_h };

    Bouton btn_jouer   = creer_bouton("textures/btn_jouer.jpg", renderer, "Jouer", font, 100, 200);

    Bouton btn_reprendre;
    int sauvegarde_presente = partieExiste();
    if (sauvegarde_presente) {
        btn_reprendre = creer_bouton("textures/btn_jouer.jpg", renderer, "Reprendre la partie", font, 100, 270);
    } else {
        btn_reprendre = creer_bouton("textures/btn_jouer.jpg", renderer, "Reprendre la partie", font, 100, 270);
    }

    Bouton btn_param   = creer_bouton("textures/param.jpg", renderer, "Options", font, 100, 340);
    Bouton btn_quitter = creer_bouton("textures/quit.jpg", renderer, "Quitter", font, 100, 410);

    SDL_Event event;
    int running = 1;

    while (running) {
        int nouvelle_sauvegarde = partieExiste();
        if (nouvelle_sauvegarde != sauvegarde_presente) {
            sauvegarde_presente = nouvelle_sauvegarde;
            SDL_DestroyTexture(btn_reprendre.texture);
            SDL_DestroyTexture(btn_reprendre.texte);
            if (sauvegarde_presente) {
                btn_reprendre = creer_bouton("textures/btn_jouer.jpg", renderer, "Reprendre la partie", font, 100, 270);
            } else {
                btn_reprendre = creer_bouton("textures/btn_jouer.jpg", renderer, "Reprendre la partie", font, 100, 270);
            }
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                if (Clique(mx, my, btn_jouer)) {
                    Menu_mode(renderer, window, grille_solution_ptr, grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels);
                }
                else if (Clique(mx, my, btn_reprendre) && sauvegarde_presente) {
                    Uint32 temps_sauvegarde = 0;
                    if (chargerPartie(grille_solution_ptr, grille_jeu_ptr, &temps_sauvegarde) == 0) {
                        printf("Partie chargée avec succès!\n");
                        Menu_jouer(renderer, window, *grille_solution_ptr, *grille_jeu_ptr,
                                   largeurGrillePixels, hauteurGrillePixels, temps_sauvegarde, MODE_NORMAL);
                    } else {
                        printf("Erreur lors du chargement de la partie.\n");
                    }
                }
                else if (Clique(mx, my, btn_param))
                    Menu_parametres(renderer, window);
                else if (Clique(mx, my, btn_quitter))
                    running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        SDL_RenderCopy(renderer, texture_titre, NULL, &rect_titre);

        SDL_RenderCopy(renderer, btn_jouer.texture, NULL, &btn_jouer.rect);
        SDL_RenderCopy(renderer, btn_jouer.texte, NULL, &btn_jouer.rect_texte);

        SDL_RenderCopy(renderer, btn_reprendre.texture, NULL, &btn_reprendre.rect);
        SDL_RenderCopy(renderer, btn_reprendre.texte, NULL, &btn_reprendre.rect_texte);

        SDL_RenderCopy(renderer, btn_param.texture, NULL, &btn_param.rect);
        SDL_RenderCopy(renderer, btn_param.texte, NULL, &btn_param.rect_texte);

        SDL_RenderCopy(renderer, btn_quitter.texture, NULL, &btn_quitter.rect);
        SDL_RenderCopy(renderer, btn_quitter.texte, NULL, &btn_quitter.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture_titre);
    SDL_DestroyTexture(texture_fond);

    SDL_DestroyTexture(btn_jouer.texture);
    SDL_DestroyTexture(btn_jouer.texte);
    SDL_DestroyTexture(btn_reprendre.texture);
    SDL_DestroyTexture(btn_reprendre.texte);
    SDL_DestroyTexture(btn_param.texture);
    SDL_DestroyTexture(btn_param.texte);
    SDL_DestroyTexture(btn_quitter.texture);
    SDL_DestroyTexture(btn_quitter.texte);

    TTF_CloseFont(font);
    TTF_CloseFont(font_small);
    if (musique) Mix_FreeMusic(musique);
}


int Menu_jouer(SDL_Renderer *renderer, SDL_Window *window, Grille *grille_solution, Grille *grille_jeu, int largeurGrillePixels, int hauteurGrillePixels, Uint32 temps_initial, int mode)
{
    (void)largeurGrillePixels;
    (void)hauteurGrillePixels;

    if (!grille_solution || !grille_jeu) {
        printf("Erreur: Tentative de jouer sans grille chargée.\n");
        return 0;
    }

    TTF_Font *font = TTF_OpenFont("fonts/sans.ttf", 24);
    TTF_Font *font_small = TTF_OpenFont("fonts/sans.ttf", 18);
    if (!font || !font_small) {
        printf("Erreur: Impossible de charger la police (Menu_jouer).\n");
        if (font) TTF_CloseFont(font);
        if (font_small) TTF_CloseFont(font_small);
        return 0;
    }

    RenderContext ctx;
    int win_w, win_h;
    SDL_GetWindowSize(window, &win_w, &win_h);
    calculerRenderContext(&ctx, win_w, win_h, grille_solution);

    Uint32 temps_debut = SDL_GetTicks() - temps_initial;

    Bouton boutonFinis = creer_bouton("textures/btn_jouer.jpg", renderer, "Verifier", font, win_w - 170, 10);
    Bouton boutonIndice = creer_bouton("textures/btn_jouer.jpg", renderer, "Indice", font, win_w - 170 - BTN_W - 10, 10);
    Bouton boutonSauvegarder = creer_bouton("textures/btn_jouer.jpg", renderer, "Sauver & Quitter", font, win_w - 225, 60);

    SDL_Event e;
    int enCours = 1;
    int resultat = 0;

    while(enCours) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                enCours = 0;
                resultat = 0;
            }
            else if(e.type == SDL_WINDOWEVENT) {
                if(e.window.event == SDL_WINDOWEVENT_RESIZED ||
                   e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    SDL_GetWindowSize(window, &win_w, &win_h);
                    calculerRenderContext(&ctx, win_w, win_h, grille_solution);

                    boutonFinis.rect.x = win_w - 170;
                    boutonFinis.rect_texte.x = boutonFinis.rect.x + (boutonFinis.rect.w - boutonFinis.rect_texte.w) / 2;

                    boutonIndice.rect.x = win_w - 170 - BTN_W - 10;
                    boutonIndice.rect_texte.x = boutonIndice.rect.x + (boutonIndice.rect.w - boutonIndice.rect_texte.w) / 2;

                    boutonSauvegarder.rect.x = win_w - 225;
                    boutonSauvegarder.rect_texte.x = boutonSauvegarder.rect.x + (boutonSauvegarder.rect.w - boutonSauvegarder.rect_texte.w) / 2;
                }
            }
            else if(e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;

                if (e.button.button == SDL_BUTTON_LEFT) {
                    if(Clique(mx, my, boutonFinis)) {
                        Uint32 temps_total = SDL_GetTicks() - temps_debut;

                        int ok = grillesIdentiques(grille_solution, grille_jeu);
                        if(ok) {
                            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                            SDL_RenderClear(renderer);
                            dessinerGrille(renderer, grille_jeu, grille_solution, &ctx, mode);

                            SDL_RenderCopy(renderer, boutonFinis.texture, NULL, &boutonFinis.rect);
                            SDL_RenderCopy(renderer, boutonFinis.texte, NULL, &boutonFinis.rect_texte);
                            SDL_RenderCopy(renderer, boutonIndice.texture, NULL, &boutonIndice.rect);
                            SDL_RenderCopy(renderer, boutonIndice.texte, NULL, &boutonIndice.rect_texte);
                            dessinerChronometre(renderer, temps_total, win_w - 170, 60, font);

                            SDL_RenderPresent(renderer);

                            char nom_capture[256];
                            Uint32 timestamp = SDL_GetTicks();
                            snprintf(nom_capture, sizeof(nom_capture), "picross_design_%u.png", timestamp);

                            sauvegarderCapture(renderer, window, nom_capture);

                            int garder_capture = afficherEcranFin(renderer, window, "Tout bon !", 1, temps_total);

                            if (!garder_capture) {
                                char chemin_complet[512];
                                snprintf(chemin_complet, sizeof(chemin_complet), "%s/%s", DOSSIER_CAPTURES, nom_capture);
                                remove(chemin_complet);
                                printf("Capture supprimée: %s\n", chemin_complet);
                            }

                            supprimerSauvegarde();

                            enCours = 0;
                            resultat = 2;
                        } else {
                            afficherEcranFin(renderer, window, "Flop !", 0, temps_total);
                        }
                    }
                    else if (Clique(mx, my, boutonIndice)) {
                        indice(grille_solution, grille_jeu);
                    }
                    else if (Clique(mx, my, boutonSauvegarder)) {
                        Uint32 temps_ecoule = SDL_GetTicks() - temps_debut;
                        if (sauvegarderPartie(grille_solution, grille_jeu, temps_ecoule) == 0) {
                            printf("Partie sauvegardée! Temps: %u ms\n", temps_ecoule);
                        }
                        enCours = 0;
                        resultat = 1;
                    }
                    else {
                        gererClic(grille_jeu, mx, my, &ctx, mode);
                    }
                }
                else if (e.button.button == SDL_BUTTON_RIGHT) {
                    gererClicDroit(grille_jeu, mx, my, &ctx, mode);
                }
            }
        }

        Uint32 temps_ecoule = SDL_GetTicks() - temps_debut;

        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        dessinerGrille(renderer, grille_jeu, grille_solution, &ctx, mode);

        SDL_RenderCopy(renderer, boutonFinis.texture, NULL, &boutonFinis.rect);
        SDL_RenderCopy(renderer, boutonFinis.texte, NULL, &boutonFinis.rect_texte);

        SDL_RenderCopy(renderer, boutonIndice.texture, NULL, &boutonIndice.rect);
        SDL_RenderCopy(renderer, boutonIndice.texte, NULL, &boutonIndice.rect_texte);

        SDL_RenderCopy(renderer, boutonSauvegarder.texture, NULL, &boutonSauvegarder.rect);
        SDL_RenderCopy(renderer, boutonSauvegarder.texte, NULL, &boutonSauvegarder.rect_texte);

        dessinerChronometre(renderer, temps_ecoule, win_w - 170, 100, font);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(boutonFinis.texture);
    SDL_DestroyTexture(boutonFinis.texte);
    SDL_DestroyTexture(boutonIndice.texture);
    SDL_DestroyTexture(boutonIndice.texte);
    SDL_DestroyTexture(boutonSauvegarder.texture);
    SDL_DestroyTexture(boutonSauvegarder.texte);
    TTF_CloseFont(font);
    TTF_CloseFont(font_small);

    return resultat;
}

void Menu_parametres(SDL_Renderer *renderer, SDL_Window *window)
{
    if (!renderer || !window) return;

    TTF_Font *font = TTF_OpenFont("fonts/font.otf", 24);
    if (!font) return;

    SDL_Color blanc = {255, 255, 255, 255};

    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) return;
    Bouton btn_retour = creer_bouton("textures/btn_jouer.jpg", renderer, "Retour", font, 50, 50);

    const int BAR_W = 200;
    const int BAR_H = 10;
    const int BAR_X = 270;
    const int BAR_Y = 160;
    const int CURSOR_SIZE = 15;

    int volume = Mix_VolumeMusic(-1);
    char txt_volume[32];

    SDL_Texture *tex_volume = NULL;
    SDL_Rect rect_volume = {50, 150, 200, 30};

    bool dragging_volume = false;
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;

                if (mx >= BAR_X && mx <= BAR_X + BAR_W && my >= BAR_Y - CURSOR_SIZE && my <= BAR_Y + BAR_H + CURSOR_SIZE) {
                    dragging_volume = true;
                }

                if (Clique(mx, my, btn_retour)) {
                    running = 0;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                dragging_volume = false;
            }

            else if (event.type == SDL_MOUSEMOTION)
            {
                if (dragging_volume) {
                    int mx = event.motion.x;
                    int new_cursor_x = mx;
                    if (new_cursor_x < BAR_X) new_cursor_x = BAR_X;
                    if (new_cursor_x > BAR_X + BAR_W) new_cursor_x = BAR_X + BAR_W;

                    float ratio = (float)(new_cursor_x - BAR_X) / BAR_W;
                    volume = (int)(ratio * MIX_MAX_VOLUME);

                    Mix_VolumeMusic(volume);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture_fond, NULL, NULL);

        if(tex_volume) SDL_DestroyTexture(tex_volume);
        snprintf(txt_volume, sizeof(txt_volume), "Volume : (%d)", volume);
        SDL_Surface *surf_vol = TTF_RenderText_Solid(font, txt_volume, blanc);
        tex_volume = SDL_CreateTextureFromSurface(renderer, surf_vol);
        SDL_FreeSurface(surf_vol);
        SDL_RenderCopy(renderer, tex_volume, NULL, &rect_volume);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect bar_rect = {BAR_X, BAR_Y, BAR_W, BAR_H};
        SDL_RenderFillRect(renderer, &bar_rect);

        int cursor_x = BAR_X + (int)((float)volume / MIX_MAX_VOLUME * BAR_W);

        SDL_Rect cursor_rect = {cursor_x - 5, BAR_Y - 5, 10, BAR_H + 10};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &cursor_rect);

        SDL_RenderCopy(renderer, btn_retour.texture, NULL, &btn_retour.rect);
        SDL_RenderCopy(renderer, btn_retour.texte, NULL, &btn_retour.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyTexture(texture_fond);
    SDL_DestroyTexture(btn_retour.texture);
    SDL_DestroyTexture(btn_retour.texte);
    if(tex_volume) SDL_DestroyTexture(tex_volume);
    TTF_CloseFont(font);
}

Grille *genererGrilleAleatoire(int taille) {

    Grille *g = malloc(sizeof(Grille));
    if (!g) return NULL;

    g->hauteur = taille;
    g->largeur = taille;
    g->taille = taille * taille;
    g->nombre_cases_noires = 0;
    g->cases = allouerCases(taille, taille);
    if (!g->cases) {
        free(g);
        return NULL;
    }

    for (int y = 0; y < taille; ++y) {
        for (int x = 0; x < taille; ++x) {
            if ((rand() % 2) == 0)
            {
                g->cases[y][x] = ETAT_NOIR;
                g->nombre_cases_noires++;
            }
            else
            {
                g->cases[y][x] = ETAT_VIDE;
            }
        }
    }
    g->indices_lignes = NULL;
    g->indices_colonnes = NULL;
    g->nb_indices_lignes = NULL;
    g->nb_indices_colonnes = NULL;

    return g;
}

void Menu_mode(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels)
{

    TTF_Font *font_titre = TTF_OpenFont("fonts/font.otf", 36);
    TTF_Font *font_btn = TTF_OpenFont("fonts/font.otf", 24);

    if (!font_titre || !font_btn) {
        if (font_titre) TTF_CloseFont(font_titre);
        if (font_btn) TTF_CloseFont(font_btn);
        return;
    }

    SDL_Color blanc = {255, 255, 255, 255};

    SDL_Surface *surf_titre = TTF_RenderText_Solid(font_titre, "Choix du mode de jeu", blanc);
    if (!surf_titre) {
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }

    SDL_Texture *texture_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
    SDL_FreeSurface(surf_titre);

    int titre_w, titre_h;
    SDL_QueryTexture(texture_titre, NULL, NULL, &titre_w, &titre_h);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect rect_titre = { (w - titre_w) / 2, 50, titre_w, titre_h };

    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) {
        SDL_DestroyTexture(texture_titre);
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }

    Bouton btn_predefini = creer_bouton("textures/btn_jouer.jpg", renderer, "Predefini", font_btn, 100, 200);
    Bouton btn_hexagones = creer_bouton("textures/btn_jouer.jpg", renderer, "Hexagones", font_btn, 100, 270);
    Bouton btn_aleatoire = creer_bouton("textures/btn_jouer.jpg", renderer, "Aleatoire", font_btn, 100, 340);
    Bouton btn_retour   = creer_bouton("textures/quit.jpg", renderer, "Retour", font_btn, 100, 410);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;

                if (Clique(mx, my, btn_predefini)) {
                    Menu_selection_niveau_predefini(renderer, window, grille_solution_ptr, grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels);
                    running = 0;
                }
                else if (Clique(mx, my, btn_hexagones)) {
                    if (*grille_jeu_ptr) libererGrille(*grille_jeu_ptr);
                    if (*grille_solution_ptr) libererGrille(*grille_solution_ptr);
                    *grille_solution_ptr = genererGrilleAleatoire(10);

                    if (!*grille_solution_ptr) {
                        printf("Erreur: Échec de la génération aléatoire.\n");
                        running = 0;
                        continue;
                    }
                    *grille_jeu_ptr = chargerGrille(*grille_solution_ptr);
                    calculerEtStockerTousIndices(*grille_solution_ptr);
                    Menu_jouer(renderer, window, *grille_solution_ptr, *grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels, 0, MODE_HEXAGONAL);
                }
                else if (Clique(mx, my, btn_aleatoire)) {
                    if (*grille_jeu_ptr) libererGrille(*grille_jeu_ptr);
                    if (*grille_solution_ptr) libererGrille(*grille_solution_ptr);
                    *grille_solution_ptr = genererGrilleAleatoire(10);

                    if (!*grille_solution_ptr) {
                        printf("Erreur: Échec de la génération aléatoire.\n");
                        running = 0;
                        continue;
                    }
                    *grille_jeu_ptr = chargerGrille(*grille_solution_ptr);
                    calculerEtStockerTousIndices(*grille_solution_ptr);
                    Menu_jouer(renderer, window, *grille_solution_ptr, *grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels, 0, MODE_NORMAL);

                }
                else if (Clique(mx, my, btn_retour)) {
                    running = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        if (texture_fond) {
             SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        }

        SDL_RenderCopy(renderer, texture_titre, NULL, &rect_titre);

        SDL_RenderCopy(renderer, btn_predefini.texture, NULL, &btn_predefini.rect);
        SDL_RenderCopy(renderer, btn_predefini.texte, NULL, &btn_predefini.rect_texte);

        SDL_RenderCopy(renderer, btn_hexagones.texture, NULL, &btn_hexagones.rect);
        SDL_RenderCopy(renderer, btn_hexagones.texte, NULL, &btn_hexagones.rect_texte);

        SDL_RenderCopy(renderer, btn_aleatoire.texture, NULL, &btn_aleatoire.rect);
        SDL_RenderCopy(renderer, btn_aleatoire.texte, NULL, &btn_aleatoire.rect_texte);

        SDL_RenderCopy(renderer, btn_retour.texture, NULL, &btn_retour.rect);
        SDL_RenderCopy(renderer, btn_retour.texte, NULL, &btn_retour.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture_titre);
    SDL_DestroyTexture(texture_fond);
    SDL_DestroyTexture(btn_predefini.texture);
    SDL_DestroyTexture(btn_predefini.texte);
    SDL_DestroyTexture(btn_hexagones.texture);
    SDL_DestroyTexture(btn_hexagones.texte);
    SDL_DestroyTexture(btn_aleatoire.texture);
    SDL_DestroyTexture(btn_aleatoire.texte);
    SDL_DestroyTexture(btn_retour.texture);
    SDL_DestroyTexture(btn_retour.texte);
    TTF_CloseFont(font_titre);
    TTF_CloseFont(font_btn);
}


void Menu_selection_niveau_predefini(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels)
{

    TTF_Font *font_titre = TTF_OpenFont("fonts/font.otf", 36);
    TTF_Font *font_btn = TTF_OpenFont("fonts/sans.ttf", 24);

    if (!font_titre || !font_btn) {
        if (font_titre) TTF_CloseFont(font_titre);
        if (font_btn) TTF_CloseFont(font_btn);
        return;
    }
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Surface *surf_titre = TTF_RenderText_Solid(font_titre, "Selectionnez le Niveau", blanc);
    if (!surf_titre) {
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }

    SDL_Texture *texture_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
    SDL_FreeSurface(surf_titre);

    int titre_w, titre_h;
    SDL_QueryTexture(texture_titre, NULL, NULL, &titre_w, &titre_h);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect rect_titre = { (w - titre_w) / 2, 50, titre_w, titre_h };


    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) {
        SDL_DestroyTexture(texture_titre);
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }

    Bouton btn_niv1 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 1", font_btn, 100, 200);
    Bouton btn_niv2 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 2", font_btn, 100, 270);
    Bouton btn_niv3 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 3", font_btn, 100, 340);
    Bouton btn_niv4 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 4", font_btn, 100, 410);
    Bouton btn_retour   = creer_bouton("textures/quit.jpg", renderer, "Retour", font_btn, 100, 480);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;

                char chemin_fichier[100];
                int niveau_selectionne = 0;

                if (Clique(mx, my, btn_niv1)) niveau_selectionne = 1;
                else if (Clique(mx, my, btn_niv2)) niveau_selectionne = 2;
                else if (Clique(mx, my, btn_niv3)) niveau_selectionne = 3;
                else if (Clique(mx, my, btn_niv4)) niveau_selectionne = 4;
                else if (Clique(mx, my, btn_retour)) {
                    running = 0;
                    continue;
                }

                if (niveau_selectionne > 0) {
                    snprintf(chemin_fichier, sizeof(chemin_fichier), "predefinis/predefini_niv%d.txt", niveau_selectionne);

                    printf("Tentative de chargement du niveau : %s\n", chemin_fichier);

                    FILE *f = fopen(chemin_fichier, "r");

                    if (f) {
                        if (*grille_jeu_ptr) libererGrille(*grille_jeu_ptr);
                        if (*grille_solution_ptr) libererGrille(*grille_solution_ptr);
                        *grille_solution_ptr = chargerGrillesolution(f);
                        fclose(f);

                        if (!*grille_solution_ptr) {
                            printf("Erreur: Échec du chargement de la nouvelle grille solution.\n");
                            continue;
                        }
                        *grille_jeu_ptr = chargerGrille(*grille_solution_ptr);

                        if (!*grille_jeu_ptr){
                            printf("Erreur: Échec de la création de la grille de jeu.\n");
                            libererGrille(*grille_solution_ptr);
                            *grille_solution_ptr = NULL;
                            continue;
                        }
                        calculerEtStockerTousIndices(*grille_solution_ptr);
                        Menu_jouer(renderer, window, *grille_solution_ptr, *grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels, 0, MODE_NORMAL);

                        running = 0;

                    } else {
                       printf("Erreur: Impossible d'ouvrir le fichier %s\n", chemin_fichier);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        if (texture_fond) {
             SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        }

        SDL_RenderCopy(renderer, texture_titre, NULL, &rect_titre);

        SDL_RenderCopy(renderer, btn_niv1.texture, NULL, &btn_niv1.rect);
        SDL_RenderCopy(renderer, btn_niv1.texte, NULL, &btn_niv1.rect_texte);

        SDL_RenderCopy(renderer, btn_niv2.texture, NULL, &btn_niv2.rect);
        SDL_RenderCopy(renderer, btn_niv2.texte, NULL, &btn_niv2.rect_texte);

        SDL_RenderCopy(renderer, btn_niv3.texture, NULL, &btn_niv3.rect);
        SDL_RenderCopy(renderer, btn_niv3.texte, NULL, &btn_niv3.rect_texte);

         SDL_RenderCopy(renderer, btn_niv4.texture, NULL, &btn_niv4.rect);
        SDL_RenderCopy(renderer, btn_niv4.texte, NULL, &btn_niv4.rect_texte);

        SDL_RenderCopy(renderer, btn_retour.texture, NULL, &btn_retour.rect);
        SDL_RenderCopy(renderer, btn_retour.texte, NULL, &btn_retour.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture_titre);
    SDL_DestroyTexture(texture_fond);
    SDL_DestroyTexture(btn_niv1.texture);
    SDL_DestroyTexture(btn_niv1.texte);
    SDL_DestroyTexture(btn_niv2.texture);
    SDL_DestroyTexture(btn_niv2.texte);
    SDL_DestroyTexture(btn_niv3.texture);
    SDL_DestroyTexture(btn_niv3.texte);
    SDL_DestroyTexture(btn_niv4.texture);
    SDL_DestroyTexture(btn_niv4.texte);
    SDL_DestroyTexture(btn_retour.texture);
    SDL_DestroyTexture(btn_retour.texte);
    TTF_CloseFont(font_titre);
    TTF_CloseFont(font_btn);
}

void indice(Grille *grille_solution, Grille *grille_jeu)
{
    int indices_donnes = 0;
    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            if (grille_solution->cases[y][x] == ETAT_NOIR && grille_jeu->cases[y][x] != ETAT_NOIR)
            {

                grille_jeu->cases[y][x] = ETAT_NOIR;
                indices_donnes++;
                if (indices_donnes >= NOMBRE_INDICES) {
                    return;
                }
            }
        }
    }
}
