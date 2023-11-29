#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Использование: %s <IP-адрес> <порт сервера> <порт монитора>\n",
           argv[0]);
    exit(1);
  }

  int server_socket, monitor_socket;
  struct sockaddr_in server_addr, monitor_addr;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));
  inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

  monitor_socket = socket(AF_INET, SOCK_STREAM, 0);
  monitor_addr.sin_family = AF_INET;
  monitor_addr.sin_port = htons(atoi(argv[3]));
  inet_pton(AF_INET, argv[1], &monitor_addr.sin_addr);

  if (connect(server_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) < 0) {
    printf("Ошибка подключения к серверу\n");
    exit(1);
  }

  if (connect(monitor_socket, (struct sockaddr *)&monitor_addr,
              sizeof(monitor_addr)) < 0) {
    printf("Ошибка подключения к монитору\n");
    exit(1);
  }
  while (1) {

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    int recv_size = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
    if (recv_size < 0) {
      perror("Ошибка при получении данных от сервера");
      exit(1);
    }
    if (strcmp(buffer, "stop") == 0) {
      break;
    }
    printf("%s", buffer);
  }

  close(monitor_socket);

  return 0;
}
