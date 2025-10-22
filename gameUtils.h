typedef struct {
    int** grid;
    int rotation;  // 0 pour horaire, 1 pour antihoraire
    int* scores;
} Game;

typedef struct {
    int numPlayer;
    int houseNum;
} Move;