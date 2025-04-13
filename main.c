#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <json-c/json.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS 20
#define MAX_COMMAND_HISTORY 100
#define CONFIG_FILE "honeypot_config.json"
#define LOG_FILE "honeypot.log"
#define ATTACKER_DB "attacker_profiles.json"

// Veri yapıları
typedef struct {
    char ip[INET_ADDRSTRLEN];
    time_t first_seen;
    time_t last_seen;
    int connection_count;
    char commands[MAX_COMMAND_HISTORY][BUFFER_SIZE];
    int command_count;
} AttackerProfile;

typedef struct {
    const char *service_name;
    const char *version;
    const char *banner;
} ServiceEmulator;

// Sahte servis yapılandırmaları
static const ServiceEmulator service_emulators[] = {
    {"SSH", "OpenSSH_8.2p1 Ubuntu-4ubuntu0.5", "SSH-2.0-OpenSSH_8.2p1 Ubuntu-4ubuntu0.5"},
    {"FTP", "vsftpd 3.0.3", "220 (vsFTPd 3.0.3)"},
    {"SMTP", "Postfix 3.4.13", "220 mail.example.com ESMTP Postfix"},
    {"HTTP", "Apache/2.4.41", "Apache/2.4.41 (Ubuntu) Server"},
    {"MySQL", "8.0.28-0ubuntu0.20.04.3", "5.7.34-0ubuntu0.18.04.1"}
};

// Sahte dosya sistemi yapısı
typedef struct {
    char path[256];
    char content[1024];
    char permissions[12];
    char owner[32];
    char group[32];
    size_t size;
    time_t modified;
} VirtualFile;

// Global değişkenler
static int server_fd = -1;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t profile_mutex = PTHREAD_MUTEX_INITIALIZER;
static AttackerProfile *attacker_profiles = NULL;
static int profile_count = 0;

// Yapılandırma yapısı
typedef struct {
    int port;
    int max_connections;
    int verbose_logging;
    char *allowed_ips[100];
    int allowed_ip_count;
    char *banned_ips[100];
    int banned_ip_count;
    int emulate_delays;
    int max_session_duration;
} HoneypotConfig;

static HoneypotConfig config;

// Sanal dosya sistemi
static VirtualFile virtual_fs[] = {
    {"/etc/passwd", "root:x:0:0:root:/root:/bin/bash\n...", "rw-r--r--", "root", "root", 0, 0},
    {"/etc/shadow", "root:$6$xyz...hash:18000:0:99999:7:::\n...", "rw-r-----", "root", "shadow", 0, 0},
    {"/var/log/auth.log", "Apr 13 11:41:44 server sshd[1234]: Failed password for root...", "rw-r-----", "syslog", "adm", 0, 0},
    {"/home/user/.bash_history", "ls -la\ncd /root\nsudo su\n...", "rw-------", "user", "user", 0, 0},
    {"/root/.ssh/id_rsa", "-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA...", "rw-------", "root", "root", 0, 0}
};

// Sahte sistem komutları ve çıktıları
typedef struct {
    const char *command;
    const char *output;
} CommandOutput;

static const CommandOutput command_responses[] = {
    {"uname -a", "Linux honeypot 5.4.0-42-generic #46-Ubuntu SMP Fri Jul 10 00:24:02 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux"},
    {"whoami", "root"},
    {"id", "uid=0(root) gid=0(root) groups=0(root)"},
    {"ps aux", "USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND\n"
               "root         1  0.0  0.0 168940  9488 ?        Ss   Apr12   0:02 /sbin/init\n"
               "root       623  0.0  0.0  94772  6960 ?        S    Apr12   0:00 /usr/sbin/sshd"},
    {"netstat -tunlp", "Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name\n"
                      "tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      623/sshd\n"
                      "tcp        0      0 127.0.0.1:3306          0.0.0.0:*               LISTEN      845/mysqld"},
};

// İleri bildirimler
void initialize_config(void);
void load_attacker_profiles(void);
void save_attacker_profiles(void);
void update_attacker_profile(const char *ip, const char *command);
void generate_deceptive_response(const char *command, char *response, size_t response_size);
void log_activity(const char *ip_address, const char *activity, int level);
void *connection_handler(void *socket_desc);
void cleanup_resources(void);

// Sinyal yakalayıcı
void handle_signal(int signal) {
    syslog(LOG_NOTICE, "Received signal %d. Shutting down honeypot...", signal);
    printf("\nShutting down honeypot...\n");
    cleanup_resources();
    exit(EXIT_SUCCESS);
}

// Yapılandırma yükleyici
void initialize_config(void) {
    json_object *config_obj = json_object_from_file(CONFIG_FILE);
    if (config_obj == NULL) {
        // Varsayılan yapılandırma
        config.port = PORT;
        config.max_connections = MAX_CONNECTIONS;
        config.verbose_logging = 1;
        config.emulate_delays = 1;
        config.max_session_duration = 300; // 5 dakika
        return;
    }

    // JSON'dan yapılandırma yükleme
    json_object_object_foreach(config_obj, key, val) {
        if (strcmp(key, "port") == 0)
            config.port = json_object_get_int(val);
        // Diğer yapılandırma parametreleri...
    }
    json_object_put(config_obj);
}

// Ana fonksiyon
int main(void) {
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Sistem log başlatma
    openlog("honeypot", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_NOTICE, "Honeypot starting up...");

    // Yapılandırma yükleme
    initialize_config();

    // Sinyal işleyicileri
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Soket oluşturma
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syslog(LOG_ERR, "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Soket seçenekleri
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        syslog(LOG_ERR, "setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.port);

    // Bağlama
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "Bind failed");
        exit(EXIT_FAILURE);
    }

    // Dinleme
    if (listen(server_fd, config.max_connections) < 0) {
        syslog(LOG_ERR, "Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Enhanced Honeypot v2.0 running on port %d...\n", config.port);
    syslog(LOG_NOTICE, "Honeypot listening on port %d", config.port);

    // Ana döngü
    while (1) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            if (errno == EINTR) continue;
            syslog(LOG_ERR, "Accept failed: %s", strerror(errno));
            continue;
        }

        // Yeni bağlantı için thread oluşturma
        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        
        if (pthread_create(&thread_id, NULL, connection_handler, (void*)new_sock) < 0) {
            syslog(LOG_ERR, "Could not create thread");
            free(new_sock);
            close(new_socket);
            continue;
        }

        // Thread'i ayır
        pthread_detach(thread_id);
    }

    cleanup_resources();
    return 0;
}

// Bağlantı işleyici
void *connection_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char client_ip[INET_ADDRSTRLEN];
    time_t session_start = time(NULL);

    free(socket_desc);

    // İstemci IP'sini al
    getpeername(sock, (struct sockaddr*)&addr, &addr_len);
    inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    // Bağlantı logla
    log_activity(client_ip, "New connection established", LOG_NOTICE);

    // Oturum zaman aşımı ayarla
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (1) {
        // Maksimum oturum süresini kontrol et
        if (time(NULL) - session_start > config.max_session_duration) {
            log_activity(client_ip, "Session timeout reached", LOG_NOTICE);
            break;
        }

        // Veri oku
        ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                log_activity(client_ip, "Connection closed by client", LOG_NOTICE);
            } else {
                if (errno != EWOULDBLOCK) {
                    log_activity(client_ip, "Error reading from client", LOG_ERR);
                }
            }
            break;
        }

        buffer[bytes_read] = '\0';
        log_activity(client_ip, buffer, LOG_INFO);

        // Saldırgan profilini güncelle
        update_attacker_profile(client_ip, buffer);

        // Yanıt oluştur
        generate_deceptive_response(buffer, response, BUFFER_SIZE);

        // Gerçekçi gecikme simülasyonu
        if (config.emulate_delays) {
            usleep(rand() % 100000); // 0-100ms arası rastgele gecikme
        }

        // Yanıt gönder
        if (send(sock, response, strlen(response), 0) < 0) {
            log_activity(client_ip, "Failed to send response", LOG_ERR);
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, BUFFER_SIZE);
    }

    close(sock);
    return NULL;
}

// Kaynakları temizle
void cleanup_resources(void) {
    if (server_fd != -1) {
        close(server_fd);
    }
    
    save_attacker_profiles();
    
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&profile_mutex);
    
    closelog();
}

// Saldırgan profili güncelleme
void update_attacker_profile(const char *ip, const char *command) {
    pthread_mutex_lock(&profile_mutex);
    
    AttackerProfile *profile = NULL;
    for (int i = 0; i < profile_count; i++) {
        if (strcmp(attacker_profiles[i].ip, ip) == 0) {
            profile = &attacker_profiles[i];
            break;
        }
    }

    if (profile == NULL) {
        // Yeni profil oluştur
        attacker_profiles = realloc(attacker_profiles, (profile_count + 1) * sizeof(AttackerProfile));
        profile = &attacker_profiles[profile_count++];
        strcpy(profile->ip, ip);
        profile->first_seen = time(NULL);
        profile->connection_count = 0;
        profile->command_count = 0;
    }

    profile->last_seen = time(NULL);
    profile->connection_count++;

    if (profile->command_count < MAX_COMMAND_HISTORY) {
        strncpy(profile->commands[profile->command_count++], command, BUFFER_SIZE - 1);
    }

    pthread_mutex_unlock(&profile_mutex);
}

// Log aktivitesi
void log_activity(const char *ip_address, const char *activity, int level) {
    pthread_mutex_lock(&log_mutex);
    
    time_t now;
    time(&now);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';

    // Sistem log
    syslog(level, "IP: %s - %s", ip_address, activity);

    // Dosya log
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        fprintf(log_file, "[%s] IP: %s - %s\n", timestamp, ip_address, activity);
        fclose(log_file);
    }

    pthread_mutex_unlock(&log_mutex);
}

// Yanıt oluşturucu
void generate_deceptive_response(const char *command, char *response, size_t response_size) {
    // Komut analizi
    for (size_t i = 0; i < sizeof(command_responses) / sizeof(command_responses[0]); i++) {
        if (strstr(command, command_responses[i].command) != NULL) {
            strncpy(response, command_responses[i].output, response_size - 1);
            return;
        }
    }

    // Varsayılan yanıt
    strncpy(response, "Command not found\n", response_size - 1);
}