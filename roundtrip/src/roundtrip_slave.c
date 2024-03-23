#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Definition des Structs
typedef enum {
    REQUEST,
    RESPONSE,
    ACK
} MessageType;

typedef struct {
    long timestamp;
    int message_id;
    MessageType type;
} Message;

int main() {
    // Erstellen des UDP-Sockets
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Fehler beim Erstellen des Sockets");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Serveradresse konfigurieren
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12345); // Beispielport

    // Binden des Sockets an die Serveradresse
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Fehler beim Binden des Sockets");
        exit(EXIT_FAILURE);
    }

    printf("Warte auf Nachrichten...\n");

    while (1) {
        Message received_msg;
        socklen_t len = sizeof(client_addr);

        // Empfangen der Nachricht
        if (recvfrom(sockfd, &received_msg, sizeof(received_msg), 0, (struct sockaddr *)&client_addr, &len) < 0) {
            perror("Fehler beim Empfangen der Nachricht");
            exit(EXIT_FAILURE);
        }

        // Ändern des Enum-Werts auf ACK
        received_msg.type = ACK;

        // Zurücksenden der modifizierten Nachricht
        if (sendto(sockfd, &received_msg, sizeof(received_msg), 0, (const struct sockaddr *)&client_addr, len) < 0) {
            perror("Fehler beim Zurücksenden der Nachricht");
            exit(EXIT_FAILURE);
        }

        printf("Nachricht empfangen und zurückgesendet.\n");
    }

    // Socket schließen
    close(sockfd);

    return 0;
}
