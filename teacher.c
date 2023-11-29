#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void send_to_monitor(int monitor_socket, const char *msg) {
    if (send(monitor_socket, msg, strlen(msg), 0) < 0) {
        perror("Ошибка при отправке сообщения монитору");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Использование: %s <IP-адрес> <порт сервера> <порт монитора> <количество клиентов>\n",
               argv[0]);
        exit(1);
    }
    srand(time(NULL));
    struct sockaddr_in server_addr, monitor_addr;
    int server_socket, monitor_socket;
    int num_clients = atoi(argv[4]);
    char ticket_number[4096];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, num_clients);
    printf("Ожидание подключения монитора...\n");

    monitor_socket = socket(AF_INET, SOCK_STREAM, 0);
    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[1], &monitor_addr.sin_addr);
    bind(monitor_socket, (struct sockaddr *) &monitor_addr, sizeof(monitor_addr));
    listen(monitor_socket, 1);
    int monitor = accept(monitor_socket, NULL, NULL);

    int presenter_sock;
    socklen_t server_addr_len = sizeof(server_addr);
    if ((presenter_sock = accept(server_socket, (struct sockaddr *) &server_addr, &server_addr_len)) < 0) {
        printf("Ошибка в функции accept() для презентатора\n");
        exit(1);
    }
    printf("Ожидание подключения клиентов...\n");
    for (int i = 0; i < num_clients; i++) {
        int client_socket;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr,
                               &client_addr_len);
        if (client_socket < 0) {
            send_to_monitor(presenter_sock,
                            "Ошибка при принятии клиентского сокета\n");
            exit(1);
        }
        memset(ticket_number, 0, sizeof(ticket_number));
        int ticket = (rand() % 100) + 1;
        sprintf(ticket_number, "%d", ticket);
        if (send(client_socket, ticket_number, strlen(ticket_number), 0) < 0) {
            send_to_monitor(presenter_sock,
                            "Ошибка при отправке номера билета клиенту\n");
            exit(1);
        }
        char num[10];
        sprintf(num, "%d", i);
        send_to_monitor(presenter_sock, "Студенту-");
        send_to_monitor(presenter_sock, num);
        send_to_monitor(presenter_sock, " отправлен номер билета: ");
        send_to_monitor(presenter_sock, ticket_number);
        send_to_monitor(presenter_sock, "\n");

        char answer[4096];
        memset(answer, 0, sizeof(answer));
        int recv_size = recv(client_socket, answer, sizeof(answer) - 1, 0);
        if (recv_size <= 0) {
            perror("Ошибка при получении ответа от клиента");
            send_to_monitor(presenter_sock,
                            "Ошибка при получении ответа от клиента\n");
            exit(1);
        }
        send_to_monitor(presenter_sock, "Студент-");
        send_to_monitor(presenter_sock, num);
        send_to_monitor(presenter_sock, " отправил ответ: ");
        send_to_monitor(presenter_sock, answer);
        send_to_monitor(presenter_sock, "\n");
        int grade = (rand() % 10) + 1;
        char grade_str[3];
        snprintf(grade_str, sizeof(grade_str), "%d", grade);

        if (send(client_socket, grade_str, strlen(grade_str), 0) < 0) {
            perror("Ошибка при отправке оценки клиенту");
            send_to_monitor(presenter_sock, "Ошибка при отправке оценки клиенту\n");
            exit(1);
        }
        send_to_monitor(presenter_sock, "Преподаватель отправил оценку: ");
        send_to_monitor(presenter_sock, grade_str);
        send_to_monitor(presenter_sock, " студенту-");
        send_to_monitor(presenter_sock, num);
        send_to_monitor(presenter_sock, "\n");

        close(client_socket);
    }
    send_to_monitor(presenter_sock, "stop");
    close(presenter_sock);
    close(monitor_socket);
    close(server_socket);
    return 0;
}
