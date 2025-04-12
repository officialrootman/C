#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <openssl/sha.h> // For hash algorithms

#define MAX_PASS_LENGTH 32
#define MIN_PASS_LENGTH 4
#define DEFAULT_THREAD_COUNT 4
#define MAX_ATTEMPTS 1000000  // Safety limit

// ANSI Colors for UI
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

// Bruteforce Parameters
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

// Function Prototypes
void print_banner();
void handle_signal(int sig);
void update_progress();
void compute_sha256(const char *str, char *output);
int verify_password(const char *password, const char *target_hash);
void generate_next_password(char *password, int length, const char *charset, int charset_length, unsigned long long index);
void *bruteforce_thread(void *arg);
void parse_arguments(int argc, char *argv[], char *charset, int *min_length, int *max_length, int *thread_count, char *target_hash);
void display_summary(double elapsed_time);

// Entry Point
int main(int argc, char *argv[]) {
    char charset[100];
    int min_length, max_length, thread_count;
    char target_hash[65];

    // Parse User Input
    parse_arguments(argc, argv, charset, &min_length, &max_length, &thread_count, target_hash);

    // Print Banner and Setup Signal Handling
    print_banner();
    signal(SIGINT, handle_signal);

    // Initialize Threads
    pthread_t threads[thread_count];
    BruteforceParams params[thread_count];
    start_time = time(NULL);

    for (int i = 0; i < thread_count; i++) {
        params[i].charset_length = strlen(charset);
        strcpy(params[i].charset, charset);
        params[i].min_length = min_length;
        params[i].max_length = max_length;
        strcpy(params[i].target_hash, target_hash);
        params[i].thread_id = i;
        params[i].found = 0;
        params[i].result = NULL;

        if (pthread_create(&threads[i], NULL, bruteforce_thread, &params[i])) {
            fprintf(stderr, RED "[!] Error creating thread %d\n" RESET, i);
            exit(EXIT_FAILURE);
        }
    }

    // Join Threads
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);

    // Display Final Summary
    display_summary(elapsed_time);

    return 0;
}

// Print Tool Banner
void print_banner() {
    printf(CYAN);
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║      Ethical Bruteforce Tool v3.0        ║\n");
    printf("║      Created by: officialrootman         ║\n");
    printf("║         For Educational Use Only         ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    printf(RESET);
}

// Signal Handler for Graceful Exit
void handle_signal(int sig) {
    running = 0;
    printf(YELLOW "\n[*] Stopping bruteforce operation...\n" RESET);
}

// Progress Update
void update_progress() {
    pthread_mutex_lock(&attempts_mutex);
    total_attempts++;

    if (total_attempts % 1000 == 0) {
        time_t current_time = time(NULL);
        double elapsed = difftime(current_time, start_time);
        double rate = total_attempts / elapsed;
        printf(BLUE "\r[*] Attempts: %llu | Rate: %.2f/s" RESET, total_attempts, rate);
        fflush(stdout);
    }

    pthread_mutex_unlock(&attempts_mutex);
}

// Compute SHA-256 Hash
void compute_sha256(const char *str, char *output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)str, strlen(str), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
}

// Verify Password Against Target Hash
int verify_password(const char *password, const char *target_hash) {
    char hash_output[65];
    compute_sha256(password, hash_output);
    return strcmp(hash_output, target_hash) == 0;
}

// Generate Next Password
void generate_next_password(char *password, int length, const char *charset, int charset_length, unsigned long long index) {
    for (int i = 0; i < length; i++) {
        password[i] = charset[index % charset_length];
        index /= charset_length;
    }
    password[length] = '\0';
}

// Thread Function for Bruteforce
void *bruteforce_thread(void *arg) {
    BruteforceParams *params = (BruteforceParams *)arg;
    char current_password[MAX_PASS_LENGTH + 1];
    unsigned long long attempt = 0;

    for (int len = params->min_length; len <= params->max_length && running; len++) {
        unsigned long long max_combinations = 1;
        for (int i = 0; i < len; i++) {
            max_combinations *= params->charset_length;
        }

        for (unsigned long long i = params->thread_id; i < max_combinations && running && !params->found; i += DEFAULT_THREAD_COUNT) {
            generate_next_password(current_password, len, params->charset, params->charset_length, i);
            update_progress();

            if (verify_password(current_password, params->target_hash)) {
                params->found = 1;
                params->result = strdup(current_password);
                printf(GREEN "\n[+] Password found: %s\n" RESET, current_password);
                running = 0;
                break;
            }
        }
    }

    return NULL;
}

// Parse Command-Line Arguments
void parse_arguments(int argc, char *argv[], char *charset, int *min_length, int *max_length, int *thread_count, char *target_hash) {
    if (argc < 5) {
        fprintf(stderr, RED "Usage: %s <target_hash> <charset> <min_length> <max_length> [thread_count]\n" RESET, argv[0]);
        fprintf(stderr, "Example: %s <hash> abcdefghijklmnopqrstuvwxyz 4 8\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(target_hash, argv[1]);
    strcpy(charset, argv[2]);
    *min_length = atoi(argv[3]);
    *max_length = atoi(argv[4]);
    *thread_count = (argc == 6) ? atoi(argv[5]) : DEFAULT_THREAD_COUNT;

    if (*min_length < MIN_PASS_LENGTH || *max_length > MAX_PASS_LENGTH) {
        fprintf(stderr, RED "[!] Password length must be between %d and %d\n" RESET, MIN_PASS_LENGTH, MAX_PASS_LENGTH);
        exit(EXIT_FAILURE);
    }
}

// Display Summary
void display_summary(double elapsed_time) {
    printf("\n[*] Operation completed in %.2f seconds\n", elapsed_time);
    printf("[*] Total attempts: %llu\n", total_attempts);
}