#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>

// ===== Structures =====

// cellule = arête
typedef struct cellule {
    int arrivee;
    float proba;
    struct cellule* suivant;
} cellule;

// liste = toutes les arêtes sortantes d’un sommet
typedef struct {
    cellule* head;
} liste;

// liste d’adjacence = tableau de listes
typedef struct {
    int taille;
    liste* tab;
} liste_adjacence;

// ===== Fonctions =====

// création
cellule* creerCellule(int arrivee, float proba);
liste creerListe();
void ajouterCellule(liste* l, int arrivee, float proba);
void afficherListe(liste l);

// liste d’adjacence
liste_adjacence creerListeAdjacence(int taille);
void afficherListeAdjacence(liste_adjacence la);
liste_adjacence readGraph(const char* filename);

// vérification
void verifierMarkov(liste_adjacence la);

// export vers Mermaid
char* getId(int num);
void exporterMermaid(liste_adjacence la, const char* filename);

#endif
