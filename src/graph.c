#include "graph.h"

/* creerCellule: crée et initialise une cellule représentant une arête */
cellule* creerCellule(int arrivee, float proba) {
    cellule* newCell = (cellule*)malloc(sizeof(cellule));
    if (!newCell) {
        perror("Erreur d'allocation mémoire pour cellule");
        exit(EXIT_FAILURE);
    }
    newCell->arrivee = arrivee;
    newCell->proba = proba;
    newCell->suivant = NULL;
    return newCell;
}

/* creerListe: crée une liste d'arêtes vide */
liste creerListe() {
    liste l;
    l.head = NULL;
    return l;
}

/* ajouterCellule: ajoute une arête (cellule) en tête de liste */
void ajouterCellule(liste* l, int arrivee, float proba) {
    cellule* newCell = creerCellule(arrivee, proba);
    newCell->suivant = l->head;
    l->head = newCell;
}

/* afficherListe: affiche les arêtes d'une liste */
void afficherListe(liste l) {
    cellule* tmp = l.head;
    while (tmp) {
        printf(" -> (%d, %.2f)", tmp->arrivee, tmp->proba);
        tmp = tmp->suivant;
    }
    printf("\n");
}

/* creerListeAdjacence: alloue et initialise une liste d'adjacence */
liste_adjacence creerListeAdjacence(int taille) {
    liste_adjacence la;
    la.taille = taille;
    la.tab = (liste*)malloc(taille * sizeof(liste));
    if (!la.tab) {
        perror("Erreur d'allocation mémoire pour la liste d'adjacence");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < taille; i++) {
        la.tab[i] = creerListe();
    }
    return la;
}

/* afficherListeAdjacence: affiche la liste d'adjacence complète */
void afficherListeAdjacence(liste_adjacence la) {
    for (int i = 0; i < la.taille; i++) {
        printf("Sommet %d :", i + 1);
        afficherListe(la.tab[i]);
    }
}

/* readGraph: lit un graphe pondéré depuis un fichier et retourne la liste d'adjacence */
liste_adjacence readGraph(const char* filename) {
    FILE* file = fopen(filename, "rt");
    if (!file) {
        perror("Impossible d'ouvrir le fichier");
        exit(EXIT_FAILURE);
    }
    int nbvert, depart, arrivee;
    float proba;
    if (fscanf(file, "%d", &nbvert) != 1) {
        perror("Erreur lors de la lecture du nombre de sommets");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    liste_adjacence la = creerListeAdjacence(nbvert);
    while (fscanf(file, "%d %d %f", &depart, &arrivee, &proba) == 3) {
        if (depart >= 1 && depart <= nbvert) {
            ajouterCellule(&la.tab[depart - 1], arrivee, proba);
        } else {
            fprintf(stderr, "Avertissement: sommet de depart %d hors intervalle\n", depart);
        }
    }
    fclose(file);
    return la;
}

/* free_liste_adjacence: libère la mémoire d'une liste d'adjacence */
void free_liste_adjacence(liste_adjacence *la) {
    if (!la || !la->tab) return;
    for (int i = 0; i < la->taille; i++) {
        cellule *cur = la->tab[i].head;
        while (cur) {
            cellule *next = cur->suivant;
            free(cur);
            cur = next;
        }
    }
    free(la->tab);
    la->tab = NULL;
    la->taille = 0;
}
