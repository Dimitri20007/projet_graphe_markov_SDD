# Compilateur
CC = gcc

# Options de compilation : 
# -Wall = affiche les avertissements
# -Wextra = affiche plus de détails
# -Iinclude = indique où sont les fichiers .h
CFLAGS = -Wall -Wextra -Iinclude

# Fichiers sources
SRC = src/main.c src/graph.c src/utils.c

# Fichiers objets (compilés)
OBJ = $(SRC:.c=.o)

# Nom de l'exécutable
EXEC = markov

# Commande par défaut : compilation
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Nettoyer les fichiers compilés
clean:
	rm -f $(OBJ) $(EXEC)