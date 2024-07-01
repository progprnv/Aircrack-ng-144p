#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#define MAX_PACKET_SIZE 8192

// Structure to hold information about Wi-Fi networks
struct wifi_network {
    char ssid[32];
    char bssid[18];
    int channel;
    int signal_strength;
};

// Function to scan for Wi-Fi networks and print basic information
void scan_wifi_networks() {
    int sock;
    struct iwreq wreq;
    char buffer[MAX_PACKET_SIZE];
    struct wifi_network networks[10]; // Assuming we can find up to 10 networks

    // Open a socket for wireless operations
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return;
    }

    memset(&wreq, 0, sizeof(struct iwreq));
    wreq.u.data.pointer = buffer;
    wreq.u.data.length = MAX_PACKET_SIZE;
    strcpy(wreq.ifr_name, "wlan0"); // Replace with your Wi-Fi interface name

    // Perform scan operation
    if (ioctl(sock, SIOCSIWSCAN, &wreq) < 0) {
        perror("Scan request failed");
        close(sock);
        return;
    }

    // Read scan results
    if (ioctl(sock, SIOCGIWSCAN, &wreq) < 0) {
        perror("Reading scan results failed");
        close(sock);
        return;
    }

    close(sock);

    // Parse and print scan results
    struct iw_event *event = (struct iw_event *) buffer;
    int num_networks = 0;

    while ((char *) event < buffer + wreq.u.data.length) {
        if (event->cmd == SIOCGIWAP) {
            struct wifi_network *net = &networks[num_networks];

            // Copy BSSID
            sprintf(net->bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
                    event->u.ap_addr.sa_data[0], event->u.ap_addr.sa_data[1],
                    event->u.ap_addr.sa_data[2], event->u.ap_addr.sa_data[3],
                    event->u.ap_addr.sa_data[4], event->u.ap_addr.sa_data[5]);

            // Copy SSID
            strncpy(net->ssid, (char *) event + IW_EV_POINT_LEN, event->u.essid.pointer);
            net->ssid[event->u.essid.pointer] = '\0';

            // Other information such as channel and signal strength can be obtained similarly
            // For simplicity, we assume channel and signal strength are set manually
            net->channel = 1; // Example value, actual channel information needs more parsing
            net->signal_strength = 70; // Example value, actual signal strength needs more parsing

            num_networks++;
        }

        // Move to the next event in buffer
        event = (struct iw_event *) ((char *) event + event->len);
    }

    // Print scan results
    printf("Wi-Fi Networks Found:\n");
    for (int i = 0; i < num_networks; i++) {
        printf("SSID: %s, BSSID: %s, Channel: %d, Signal Strength: %d%%\n",
               networks[i].ssid, networks[i].bssid, networks[i].channel, networks[i].signal_strength);
    }
}

int main() {
    scan_wifi_networks();
    return 0;
}
