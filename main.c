#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#define MAX_PASS_LENGTH 32
#define MIN_PASS_LENGTH 4
#define THREAD_COUNT 4
#define MAX_ATTEMPTS 1000000  // Safety limit

// ANSI Colors
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

typedef struct {
    char charset[100];
    int charset_length;
    int min_length;
    int max_length;
    char target_hash[65];  // For SHA-256
    int thread_id;
    int found;
    char *result;
} BruteforceParams;

volatile sig_atomic_t running = 1;
unsigned long long total_attempts = 0;
pthread_mutex_t attempts_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t start_time;

void print_banner() {
    printf(CYAN);
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║      Ethical Bruteforce Tool v1.0        ║\n");
    printf("║      Created by: officialrootman        ║\n");
    printf("║         For Educational Use Only         ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    printf(RESET);
}

void handle_signal(int sig) {
    running = 0;
    printf(YELLOW "\n[*] Stopping bruteforce operation...\n" RESET);
}

void update_progress() {
    pthread_mutex_lock(&attempts_mutex);
    total_attempts++;
    
    if (total_attempts % 1000 == 0) {
        time_t current_time = time(NULL);
        double elapsed = difftime(current_time, start_time);
        double rate = total_attempts / elapsed;
        
        printf(BLUE "\r[*] Attempts: %llu | Rate: %.2f/s" RESET, 
               total_attempts, rate);
        fflush(stdout);
    }
    
    pthread_mutex_unlock(&attempts_mutex);
}

int verify_password(const char *password, const char *target) {
    // This is a demo function - replace with actual hash comparison
    return strcmp(password, target) == 0;
}

void generate_next_password(char *password, int length, const char *charset, 
                          int charset_length, unsigned long long index) {
    for (int i = 0; i < length; i++) {
        password[i] = charset[index % charset_length];
        index /= charset_length;
    }
    password[length] = '\0';
}

void *bruteforce_thread(void *arg) {
    BruteforceParams *params = (BruteforceParams *)arg;
    char current_password[MAX_PASS_LENGTH + 1];
    unsigned long long attempt = 0;
    
    for (int len = params->min_length; len <= params->max_length && running; len++) {
        unsigned long long max_combinations = 1;
        for (int i = 0; i < len; i++) {
            max_combinations *= params->charset_length;
        }
        
        for (unsigned long long i = params->thread_id; 
             i < max_combinations && running && !params->found; 
             i += THREAD_COUNT) {
            
            generate_next_password(current_password, len, params->charset, 
                                 params->charset_length, i);
            
            update_progress();
            
            if (verify_password(current_password, params->target_hash)) {
                params->found = 1;
                params->result = strdup(current_password);
                printf(GREEN "\n[+] Password found: %s\n" RESET, current_password);
                running = 0;
                break;
            }
            
            attempt++;
            if (attempt >= MAX_ATTEMPTS) {
                printf(RED "\n[!] Safety limit reached. Stopping...\n" RESET);
                running = 0;
                break;
            }
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_banner();
        printf(YELLOW "\nUsage: %s <target_password>\n" RESET, argv[0]);
        printf("Note: This is an educational tool. Use responsibly.\n");
        return 1;
    }
    
    print_banner();
    printf("\n[*] Starting ethical bruteforce demonstration...\n");
    printf("[*] This tool is for educational purposes only!\n\n");
    
    // Safety confirmation
    char confirm;
    printf(RED "WARNING: This tool should only be used on test systems.\n");
    printf("Do you confirm this is a test environment? (y/n): " RESET);
    scanf("%c", &confirm);
    
    if (tolower(confirm) != 'y') {
        printf(RED "[!] Operation cancelled by user.\n" RESET);
        return 1;
    }
    
    signal(SIGINT, handle_signal);
    
    // Initialize parameters
    BruteforceParams params[THREAD_COUNT];
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    pthread_t threads[THREAD_COUNT];
    
    start_time = time(NULL);
    
    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        params[i].charset_length = strlen(charset);
        strcpy(params[i].charset, charset);
        params[i].min_length = MIN_PASS_LENGTH;
        params[i].max_length = MAX_PASS_LENGTH;
        strcpy(params[i].target_hash, argv[1]);
        params[i].thread_id = i;
        params[i].found = 0;
        params[i].result = NULL;
        
        pthread_create(&threads[i], NULL, bruteforce_thread, &params[i]);
    }
    
    // Wait for threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("\n[*] Operation completed in %.2f seconds\n", elapsed);
    printf("[*] Total attempts: %llu\n", total_attempts);
    
    // Cleanup
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (params[i].result != NULL) {
            free(params[i].result);
        }
    }
    
    return 0;
}