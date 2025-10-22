typedef struct {
    int** grid;
    int rotation;  // 0 for clockwise, 1 for counter-clockwise
    int* scores;
} Game;

typedef struct {
    int numPlayer;
    int houseNum;
} Move;


void freeGame(Game game);