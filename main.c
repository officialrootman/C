#include <pcap.h>
#include <stdio.h>
#include <arpa/inet.h>

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    printf("Packet captured: Length = %d bytes\n", header->len);
    for (int i = 0; i < header->len; i++) {
        printf("%02x ", packet[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n\n");
}

int main() {
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    // Ağ cihazını seç
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Device not found: %s\n", errbuf);
        return 1;
    }
    printf("Device: %s\n", dev);

    // Ağ cihazını aç
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open device %s: %s\n", dev, errbuf);
        return 2;
    }

    // Paketleri yakala
    printf("Starting packet capture...\n");
    pcap_loop(handle, 0, packet_handler, NULL);

    pcap_close(handle);
    return 0;
}