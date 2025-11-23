# projet_graphe_markov_SDD
Projet d'analyse de graphes de Markov — lecture, partition en classes
fortement connexes, diagramme de Hasse et calculs matriciels.

Ce projet fournit :
- un lecteur de graphes pondérés (format simple texte),
- une vérification de validité Markov (somme des probabilités sortantes),
- l'algorithme de Tarjan pour obtenir les composantes fortement connexes
	(classes),
- la construction d'un diagramme de Hasse entre classes (suppression
	d'arcs transitifs),
- des opérations matricielles sur la matrice de transition (puissances,
	période, distribution stationnaire approximée),
- export au format Mermaid pour visualiser le graphe et le diagramme de
	Hasse (`data/graphe_mermaid.mmd`, `data/hasse_mermaid.mmd`).

**Organisation du dépôt**
- **`include/`** : headers (`graph.h`, `hasse.h`, `matrix.h`).
- **`src/`** : code source C (`main.c`, `graph.c`, `hasse.c`, `matrix.c`, `utils.c`).
- **`data/`** : exemples d'entrées et fichiers Mermaid générés.
- **`Makefile`** : règles pour compiler (`markov.exe`).

**Format des fichiers d'entrée (dans `data/`)**
Le fichier contenant le graphe doit être un fichier texte avec :
1) la première ligne : un entier N = nombre de sommets (numérotation 1..N)
2) chaque ligne suivante : trois valeurs séparées par des espaces :
	 `depart arrivee proba`
	 - `depart` et `arrivee` : entiers (1..N)
	 - `proba` : nombre flottant (probabilité de transition)

Exemple (extrait de `data/exemple_meteo.txt`):
```
3
1 2 0.5
1 3 0.5
2 1 1.0
3 3 1.0
```

L'implémentation attend que pour chaque sommet la somme des probabilités
sortantes soit ~1 (tolérance utilisée : ±0.01). Si ce n'est pas le cas,
un avertissement est affiché.

**Compilation**
- Pré-requis : compilateur C (GCC/MinGW) et `make` (ou `mingw32-make`) sur
	Windows. Le Makefile est adapté pour une utilisation sous Windows.

Exemples (PowerShell) :
```
# depuis la racine du projet
cd projet_graphe_markov_SDD
mingw32-make   # ou `make` si disponible
```
La cible par défaut produit l'exécutable `markov.exe`.

Si vous n'avez pas `make`, compilez manuellement :
```
gcc -Iinclude -o markov.exe src/*.c -lm
```

**Exécution**
Lancer l'exécutable puis entrer le nom d'un fichier présent dans `data/`.
```
.\markov.exe
# puis, quand demandé : exemple_meteo.txt
```
Comportement principal :
- lecture du graphe et affichage (liste d'adjacence),
- vérification des propriétés Markoviennes,
- export `data/graphe_mermaid.mmd`,
- calcul des composantes par Tarjan -> partition en classes,
- construction du diagramme de Hasse (liens entre classes),
- export `data/hasse_mermaid.mmd`,
- calculs matriciels : M^3, M^7, convergence de M^n, sous-matrices par
	classe, période, simulation de la limite pour une distribution initiale
	uniforme sur chaque classe.

Les fichiers `.mmd` produits sont prêts à être collés dans un rendu Mermaid
en ligne (par ex. https://mermaid.live/ ou https://www.mermaidchart.com/)
pour obtenir une visualisation graphique.

**Encodage / Windows**
Le programme configure la locale UTF-8 dans `main.c` pour un affichage
correct sous Windows. Assurez-vous que votre console prend en charge UTF-8
si vous rencontrez des caractères invalides.

**Exemples et tests**
- Fichiers d'exemple fournis : `data/exemple_meteo.txt`,
	`data/exemple1.txt`, `data/exemple2.txt`, `data/exemple3.txt`, etc.
- Après exécution avec un exemple, vérifiez `data/graphe_mermaid.mmd` et
	`data/hasse_mermaid.mmd` pour visualiser les résultats.

**Structure des modules (rapide)**
- `graph.*` : lecture, représentation par liste d'adjacence, export
	Mermaid.
- `hasse.*` : suppression des arcs transitifs du diagramme de Hasse.
- `matrix.*` : conversion en matrice de transition, opérations
	matricielles, calcul de périodes et distributions.

**Nettoyage**
```
mingw32-make clean
```

**Remarques**
- Pas de dépendances externes autres que la librairie C standard et `-lm`.
- Le code est commenté et structuré pour les trois parties décrites ci‑dessus.
---
Fin de la documentation.
