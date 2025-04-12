#include <stdio.h>
#include <stdlib.h>

// Global 3x3 tahta dizisi. Her hücre başlangıçta numaralandırılmış durumda.
char board[3][3];

// Tahtayı ilk durumuna getiren fonksiyon: 1'den 9'a kadar sayılarla doldurur.
void initializeBoard() {
    char pos = '1';
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = pos++;
        }
    }
}

// Tahtayı ekrana güzel bir şekilde yazdıran fonksiyon.
void displayBoard() {
    printf("\n");
    for (int i = 0; i < 3; i++) {
        printf(" %c | %c | %c \n", board[i][0], board[i][1], board[i][2]);
        if (i != 2)
            printf("---|---|---\n");
    }
    printf("\n");
}

// Galip kontrolü yapan fonksiyon.
// Eğer bir oyuncu kazanmışsa, o oyuncunun işaretini ('X' veya 'O') geri döndürür.
// Hiçbir kazanan yoksa ' ' (boş karakter) döndürür.
char checkWinner() {
    // Satır kontrolü
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
    }
    // Sütun kontrolü
    for (int j = 0; j < 3; j++) {
        if (board[0][j] == board[1][j] && board[1][j] == board[2][j])
            return board[0][j];
    }
    // Çapraz kontrolü
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];

    // Kazanan yoksa boş karakter döndürürüz.
    return ' ';
}

// Tahtada hala hamle yapılabilecek boş hücre var mı kontrol eder.
int isBoardFull() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] != 'X' && board[i][j] != 'O')
                return 0; // Boş hücre bulundu.
        }
    }
    return 1; // Tahta tamamen dolu.
}

int main() {
    int choice;
    char winner = ' ';
    char currentPlayer = 'X';

    // Oyun başlangıcında tahtayı hazırlıyoruz.
    initializeBoard();
    
    // Oyun döngüsü: ya bir oyuncu kazanır ya da tahta dolarsa oyun sona erer.
    while (1) {
        displayBoard();
        printf("Oyuncu %c, hamle yapmak için (1-9) bir hücre numarası girin: ", currentPlayer);
        
        if (scanf("%d", &choice) != 1) {
            printf("Lütfen geçerli bir tamsayı giriniz!\n");
            // Geçersiz girdiyi temizlemek için buffer'ı boşaltıyoruz
            while (getchar() != '\n');
            continue;
        }
        
        // Girilen değerin 1 ile 9 arasında olup olmadığını kontrol ediyoruz.
        if (choice < 1 || choice > 9) {
            printf("Geçersiz hücre seçimi! 1 ile 9 arasında bir sayı girmelisiniz.\n");
            continue;
        }
        
        // Kullanıcının girdiği sayı, tahtadaki karşılık gelen satır ve sütuna çevrilir.
        int row = (choice - 1) / 3;
        int col = (choice - 1) % 3;
        
        // Eğer seçilen hücrede zaten 'X' veya 'O' varsa, hamle geçersizdir.
        if (board[row][col] == 'X' || board[row][col] == 'O') {
            printf("Bu hücre dolu! Lütfen boş bir hücre seçin.\n");
            continue;
        }
        
        // Geçerli hamle yapılıyor.
        board[row][col] = currentPlayer;
        
        // Hamleden sonra kazanan kontrol edilir.
        winner = checkWinner();
        if (winner == 'X' || winner == 'O') {
            displayBoard();
            printf("Tebrikler! Oyuncu %c kazandı!\n", winner);
            break;
        }
        
        // Tahta dolu ise oyun berabere biter.
        if (isBoardFull()) {
            displayBoard();
            printf("Oyun berabere bitti!\n");
            break;
        }
        
        // Oyuncular sırayla hamle yapar. 'X' ise 'O', 'O' ise 'X'e geçiş yapılır.
        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    }
    
    return 0;
}
