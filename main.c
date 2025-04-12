#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>

#define MIN_PORT 1
#define MAX_PORT 65535
#define TIMEOUT_SECONDS 1
#define MAX_INPUT_LENGTH 256

// ANSI color codes
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void print_banner() {
    printf(ANSI_COLOR_CYAN);
    printf("╔════════════════════════════════════════════╗\n");
    printf("║        Advanced Port Scanner v2.0          ║\n");
    printf("║        Created by: officialrootman        ║\n");
    printf("║     IP & Domain Scanner - Ethical Only!    ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf(ANSI_COLOR_RESET);
}

// Function to resolve domain name to IP
char* resolve_domain(const char* domain) {
    struct hostent *host_info;
    struct in_addr **addr_list;
    
    if ((host_info = gethostbyname(domain)) == NULL) {
        printf(ANSI_COLOR_RED "Failed to resolve domain: %s\n" ANSI_COLOR_RESET, domain);
        return NULL;
    }
    
    addr_list = (struct in_addr **)host_info->h_addr_list;
    if (addr_list[0] != NULL) {
        return inet_ntoa(*addr_list[0]);
    }
    
    return NULL;
}

void scan_port(const char* target, int port) {
    struct sockaddr_in addr;
    int sock;
    struct timeval timeout;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return;
    }
    
    // Set timeout
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(target);
    
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        // Get service name if available
        struct servent *service = getservbyport(htons(port), "tcp");
        if (service != NULL) {
            printf(ANSI_COLOR_GREEN "[+] Port %d is open - Service: %s" ANSI_COLOR_RESET "\n", 
                   port, service->s_name);
        } else {
            printf(ANSI_COLOR_GREEN "[+] Port %d is open" ANSI_COLOR_RESET "\n", port);
        }
    }
    
    close(sock);
}

void perform_scan(const char* target, int start_port, int end_port) {
    printf(ANSI_COLOR_YELLOW "\nStarting scan on target: %s\n" ANSI_COLOR_RESET, target);
    printf("Port range: %d-%d\n\n", start_port, end_port);
    
    time_t scan_start = time(NULL);
    
    for (int port = start_port; port <= end_port; port++) {
        printf("\rScanning port %d... ", port);
        fflush(stdout);
        scan_port(target, port);
    }
    
    time_t scan_end = time(NULL);
    double scan_time = difftime(scan_end, scan_start);
    
    printf(ANSI_COLOR_YELLOW "\nScan completed in %.2f seconds\n" ANSI_COLOR_RESET, scan_time);
}

int main() {
    char target[MAX_INPUT_LENGTH];
    int choice, start_port, end_port;
    
    print_banner();
    
    printf(ANSI_COLOR_YELLOW "\nSelect scan type:\n" ANSI_COLOR_RESET);
    printf("1. IP Address Scan\n");
    printf("2. Domain Name Scan\n");
    printf("Choice (1 or 2): ");
    scanf("%d", &choice);
    getchar(); // Clear newline
    
    if (choice != 1 && choice != 2) {
        printf(ANSI_COLOR_RED "Invalid choice. Exiting...\n" ANSI_COLOR_RESET);
        return 1;
    }
    
    // Get target
    if (choice == 1) {
        printf("Enter IP address to scan: ");
    } else {
        printf("Enter domain name to scan (e.g., example.com): ");
    }
    fgets(target, MAX_INPUT_LENGTH, stdin);
    target[strcspn(target, "\n")] = 0; // Remove newline
    
    // Get port range
    printf("Enter start port (1-65535): ");
    scanf("%d", &start_port);
    printf("Enter end port (1-65535): ");
    scanf("%d", &end_port);
    
    // Validate port range
    if (start_port < MIN_PORT || end_port > MAX_PORT || start_port > end_port) {
        printf(ANSI_COLOR_RED "Error: Invalid port range. Use ports between 1-65535\n" ANSI_COLOR_RESET);
        return 1;
    }
    
    char* scan_target = target;
    if (choice == 2) {
        // Resolve domain name
        printf(ANSI_COLOR_BLUE "\nResolving domain name...\n" ANSI_COLOR_RESET);
        scan_target = resolve_domain(target);
        if (scan_target == NULL) {
            printf(ANSI_COLOR_RED "Failed to resolve domain. Exiting...\n" ANSI_COLOR_RESET);
            return 1;
        }
        printf(ANSI_COLOR_GREEN "Domain resolved to IP: %s\n" ANSI_COLOR_RESET, scan_target);
    }
    
    perform_scan(scan_target, start_port, end_port);
    
    return 0;
}