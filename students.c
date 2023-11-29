#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void send_to_monitor(int monitor_socket, const char* msg) {
    if (send(monitor_socket, msg, strlen(msg), 0) < 0) {
        perror("Ошибка при отправке сообщения монитору");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: %s <IP-адрес сервера> <порт сервера>\n", argv[0]);
        exit(1);
    }
    int client_socket;
    struct sockaddr_in server_addr;
    char ticket_number[4096];
    char answer[4096];
    char grade[50];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Ошибка подключения к серверу\n");
        exit(1);
    }

    if (recv(client_socket, ticket_number, sizeof(ticket_number), 0) < 0) {
        printf("Ошибка при получении номера билета от сервера\n");
        exit(1);
    }
    printf("Получен номер билета: %s\n", ticket_number);
    snprintf(answer, sizeof(answer), "Ответ на билет %s", ticket_number);
    send(client_socket, answer, sizeof(answer), 0);
    printf("Отправлен ответ: %s\n", answer);

    if (recv(client_socket, grade, sizeof(grade), 0) < 0) {
        printf("Ошибка при получении оценки от сервера\n");
        exit(1);
    }
    printf("Получена оценка: %s\n", grade);

    close(client_socket);
    return 0;
}
