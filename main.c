#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Oyun sabitleri
#define WIDTH 30
#define HEIGHT 20
#define PLAYER 'P'
#define ENEMY 'E'
#define COIN 'O'
#define WALL '#'
#define POWER 'S'
#define EMPTY ' '
#define MAX_ENEMIES 5
#define MAX_HIGHSCORES 5
#define SAVE_FILE "highscores.txt"

// Yön sabitleri
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// Yapılar aynı kalır
typedef struct {
    int x, y;
    int score;
    int lives;
    int level;
    int powerup;
    int powerup_timer;
} Player;

typedef struct {
    int x, y;
    int active;
    int direction;
} Enemy;

typedef struct {
    char name[20];
    int score;
} Highscore;

// Global değişkenler
char map[HEIGHT][WIDTH];
Player player;
Enemy enemies[MAX_ENEMIES];
Highscore highscores[MAX_HIGHSCORES];
int coinX, coinY;
int powerupX, powerupY;
int powerupActive;

// Unix/Linux için klavye giriş fonksiyonları
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

int getch(void) {
    struct termios oldattr, newattr;
    int ch;

    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

    return ch;
}

// Ekranı temizleme fonksiyonu
void clearScreen() {
    printf("\033[H\033[J");  // ANSI escape sequence for clearing screen
}

// Ekranı geciktirme fonksiyonu (milisaniye cinsinden)
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

// Diğer oyun fonksiyonları aynı kalır, sadece clearScreen() ve Sleep() çağrıları değişir
void initializeGame() {
    // Oyuncu başlangıç değerleri
    player.x = WIDTH / 2;
    player.y = HEIGHT / 2;
    player.score = 0;
    player.lives = 3;
    player.level = 1;
    player.powerup = 0;
    player.powerup_timer = 0;

    // Haritayı temizle
    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            if(y == 0 || y == HEIGHT-1 || x == 0 || x == WIDTH-1)
                map[y][x] = WALL;
            else
                map[y][x] = EMPTY;
        }
    }

    // Düşmanları başlat
    for(int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 1;
        enemies[i].x = rand() % (WIDTH-2) + 1;
        enemies[i].y = rand() % (HEIGHT-2) + 1;
        enemies[i].direction = rand() % 4;
    }

    spawnCoin();
    spawnPowerup();
}

// [Önceki fonksiyonlar aynı kalır...]

int main() {
    // Terminal ayarlarını kaydet
    struct termios old_termios, new_termios;
    tcgetattr(0, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ICANON;
    new_termios.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &new_termios);

    srand(time(NULL));
    loadHighscores();
    initializeGame();

    printf("Arcade Oyununa Hoşgeldiniz!\n");
    printf("Kontroller:\n");
    printf("W,A,S,D: Hareket\n");
    printf("Q: Çıkış\n");
    printf("Başlamak için herhangi bir tuşa basın...\n");
    getch();

    while(1) {
        if(kbhit()) {
            char key = getch();
            if(key == 'q') break;
            movePlayer(key);
        }

        moveEnemies();
        checkCollisions();
        updatePowerup();
        drawMap();
        sleep_ms(50); // 50 milisaniye bekle
    }

    // Terminal ayarlarını geri yükle
    tcsetattr(0, TCSANOW, &old_termios);

    return 0;
}