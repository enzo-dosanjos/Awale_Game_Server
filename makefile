# Variables de compilation
CXX := gcc
CXXFLAGS = -ansi -pedantic -Wall -std=c18
CXXFLAGS_DEBUG := -ansi -pedantic -Wall -std=c18 -g -DMAP
ServTARGET := Awale
ClientTARGET := Client
TARGET := $(ServTARGET) $(ClientTARGET)
BUILD_DIR := build

SERVSOURCES = gameLogic.c gameUtils.c ihm.c gameServer.c mainServer.c commands.c
CLIENTSOURCES = client.c
SERVOBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SERVSOURCES))
CLIENTOBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CLIENTSOURCES))

all: $(TARGET)
server: $(ServTARGET)
client: $(ClientTARGET)

debug: CXXFLAGS := $(CXXFLAGS_DEBUG)
debug: clean $(TARGET)

# Création de l'exécutable
$(ServTARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(ServTARGET) $(SERVOBJECTS)

$(ClientTARGET): $(patsubst %.c, $(BUILD_DIR)/%.o, $(CLIENTSOURCES))
	$(CXX) $(CXXFLAGS) -o $(ClientTARGET) $(CLIENTOBJECTS)

# Compilation de chaque fichier source en objet
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Création du répertoire build/
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# Nettoyage du projet
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

cleano:
	rm -rf $(BUILD_DIR)