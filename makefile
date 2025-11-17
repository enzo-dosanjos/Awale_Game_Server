# Variables de compilation
CXX := gcc
CXXFLAGS = -ansi -pedantic -Wall -std=c18
CXXFLAGS_DEBUG := -ansi -pedantic -Wall -std=c18 -g -DMAP
ServTARGET := Awale
ClientTARGET := Client
TARGET := $(ServTARGET) $(ClientTARGET)
BUILD_DIR := build

SERVSOURCES = src/game/gameLogic.c src/game/gameUtils.c src/game/ihm.c \
			  src/server/dataManagers/clientManager.c src/server/dataManagers/gameSessionManager.c \
			  src/server/commandProcessor.c \
			  src/server/commands/chatService.c src/server/commands/accountService.c src/server/commands/challengeService.c \
			  src/server/commands/gameService.c src/server/commands/generalService.c src/server/commands/profileService.c \
			  src/server/mainServer.c \
              src/server/networking/gameServer.c src/server/networking/serverUtils.c
CLIENTSOURCES = src/client/client.c
SERVOBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SERVSOURCES))
CLIENTOBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(CLIENTSOURCES))

all: clean $(TARGET)
server: $(ServTARGET)
client: $(ClientTARGET)

debug: CXXFLAGS := $(CXXFLAGS_DEBUG)
debug: clean $(TARGET)

# Création de l'exécutable
$(ServTARGET): $(SERVOBJECTS)
	$(CXX) $(CXXFLAGS) -o $(ServTARGET) $(SERVOBJECTS)

$(ClientTARGET): $(CLIENTOBJECTS)
	$(CXX) $(CXXFLAGS) -o $(ClientTARGET) $(CLIENTOBJECTS)

# Compilation de chaque fichier source en objet
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
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