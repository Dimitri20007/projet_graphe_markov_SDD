#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---------------------------
   Structures de la Partie 1
   ---------------------------*/
// cellule = arête
typedef struct cellule {
    int arrivee;          // numéro du sommet d'arrivée (1-based)
    float proba;
    struct cellule* suivant;
} cellule;

// liste = toutes les arêtes sortantes d’un sommet
typedef struct {
    cellule* head;
} liste;

// liste d’adjacence = tableau de listes
typedef struct {
    int taille;   // nombre de sommets
    liste* tab;   // tab[0] correspond au sommet 1
} liste_adjacence;

/* Fonctions Partie 1 */
cellule* creerCellule(int arrivee, float proba);
liste creerListe();
void ajouterCellule(liste* l, int arrivee, float proba);
void afficherListe(liste l);
liste_adjacence creerListeAdjacence(int taille);
void afficherListeAdjacence(liste_adjacence la);
liste_adjacence readGraph(const char* filename);
void verifierMarkov(liste_adjacence la);
char* getId(int num);
void exporterMermaid(liste_adjacence la, const char* filename);

/* ---------------------------
   Structures et fonctions Partie 2 (Tarjan / Hasse)
   ---------------------------*/
typedef struct {
    int id;         // numéro du sommet (1..n)
    int index;      // index (numérotation temporelle) initialisé à -1
    int lowlink;    // lowlink
    int onstack;    // 0 ou 1
} t_tarjan_vertex;

typedef struct {
    char name[16];     // "C1", "C2", ...
    int *members;      // tableau dynamique des sommets (1-based)
    int size;
    int capacity;
} t_classe;

typedef struct {
    t_classe *classes; // tableau dynamique de classes
    int nb;
    int cap;
} t_partition;

typedef struct {
    int from; // indice de classe (1-based pour affichage)
    int to;
} t_link;

typedef struct {
    t_link *links;
    int size;
    int cap;
} t_link_array;

typedef struct {
    int *data;
    int top;
    int cap;
} int_stack;

/* Fonctions Tarjan & utilitaires */
t_tarjan_vertex* init_tarjan_vertices(int n);
t_partition tarjan(liste_adjacence la);
void free_partition(t_partition *p);
void init_partition(t_partition *p);
void add_class(t_partition *p, t_classe c);
void init_classe(t_classe *c, const char *name);
void add_member_to_classe(t_classe *c, int vertex);
void init_stack(int_stack *s);
void push_stack(int_stack *s, int v);
int pop_stack(int_stack *s);
int stack_empty(int_stack *s);
void free_stack(int_stack *s);
t_link_array create_links_from_partition(liste_adjacence la, t_partition *p, int *vertex_to_class);
void init_link_array(t_link_array *la);
void add_link_if_not_exists(t_link_array *la, int from, int to);
int link_exists(t_link_array *la, int from, int to);
void free_link_array(t_link_array *la);
int* build_vertex_to_class_map(t_partition *p, int n);
void exporterMermaidHasse(t_partition *p, t_link_array *links, const char *filename);
int compute_caracteristics(t_partition *p, t_link_array *links, int class_number);
void compute_and_print_characteristics(t_partition *p, t_link_array *links);
void free_liste_adjacence(liste_adjacence *la);

#endif
