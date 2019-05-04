#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "hardware.h"

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); perror("ERROR"); fflush(stderr); exit(1); }

// struct weather {
// 	float temperature, humidity;
// };
extern int errno;

int main (int argc, char *argv[]) {
  if (argc < 2) on_error("Usage: %s [port]\n", argv[0]);

  setup();

  int port = atoi(argv[1]);

  int server_fd, client_fd, err;
  struct sockaddr_in server, client;
  // struct weather current_weather;
  struct weather w;
  char buf[BUFFER_SIZE];
  

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) on_error("Could not create socket\n");

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Could not bind socket\n");

  err = listen(server_fd, 128);
  if (err < 0) on_error("Could not listen on socket\n");

  printf("Server is listening on %d\n", port);

  while (1) {
    socklen_t client_len = sizeof(client);
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

    if (client_fd < 0) on_error("Could not establish new connection\n");

    
      //int read = recv(client_fd, buf, BUFFER_SIZE, 0);

      //if (!read) break; // done reading
      //if (read < 0) on_error("Client read failed\n");
    w = getCurrentWeather();
    //printf( "Humidity = %.1f %% Temperature = %.1f *C \n", w.humidity, w.temperature);
	  // current_weather.temperature = 26.2;
	  // current_weather.humidity = 82.2;
      sprintf (buf, "Humidity = %.1f %% Temperature = %.1f *C \n", w.humidity, w.temperature); 
      err = send(client_fd, buf, strlen(buf), 0);
      if (err < 0) on_error("Client write failed with errorcode: %d\n", errno);
      shutdown(client_fd, 2);
  }

  return 0;
}
