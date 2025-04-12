#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 65536
#define LOG_FILE "ids.log"

// Function to log suspicious activity
void log_suspicious_activity(const char *message, const char *ip) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log != NULL) {
        fprintf(log, "Suspicious activity detected from IP: %s - %s\n", ip, message);
        fclose(log);
    } else {
        perror("Failed to open log file");
    }
}

// Function to analyze packet data
void analyze_packet(const char *data, int size, const char *ip) {
    // Check for suspicious patterns (example: "malicious" keyword)
    if (strstr(data, "malicious") != NULL) {
        log_suspicious_activity("Keyword 'malicious' found in packet data", ip);
        printf("Alert: Suspicious activity detected from IP %s\n", ip);
    }
}

int main() {
    int raw_socket;
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);
    char buffer[BUFFER_SIZE];

    // Create a raw socket
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (raw_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("IDS is running and monitoring network traffic...\n");

    while (1) {
        // Receive packets
        int data_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&source_addr, &addr_len);
        if (data_size < 0) {
            perror("Failed to receive packets");
            continue;
        }

        // Convert source IP to human-readable format
        char source_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(source_addr.sin_addr), source_ip, INET_ADDRSTRLEN);

        // Analyze the packet data
        analyze_packet(buffer, data_size, source_ip);
    }

    // Close the socket (not reached in this infinite loop)
    close(raw_socket);
    return 0;
}