#include "graph.h"

// === Création d'une cellule ===
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

// === Création d'une liste vide ===
liste creerListe() {
    liste l;
    l.head = NULL;
    return l;
}

// === Ajout d'une cellule en début de liste ===
void ajouterCellule(liste* l, int arrivee, float proba) {
    cellule* newCell = creerCellule(arrivee, proba);
    newCell->suivant = l->head;
    l->head = newCell;
}

// === Affichage d'une liste d’arêtes ===
void afficherListe(liste l) {
    cellule* tmp = l.head;
    while (tmp) {
        printf(" -> (%d, %.2f)", tmp->arrivee, tmp->proba);
        tmp = tmp->suivant;
    }
    printf("\n");
}

// === Création d'une liste d’adjacence vide ===
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

// === Affichage de la liste d’adjacence complète ===
void afficherListeAdjacence(liste_adjacence la) {
    for (int i = 0; i < la.taille; i++) {
        printf("Sommet %d :", i + 1);
        afficherListe(la.tab[i]);
    }
}

// === Lecture du graphe depuis un fichier ===
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
        ajouterCellule(&la.tab[depart - 1], arrivee, proba);
    }

    fclose(file);
    return la;
}
