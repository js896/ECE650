//this is the ringmaster
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>

#include "potato.h"

using namespace std;

class Ringmaster {
 public:
  int status;
  int socket_fd;
  int accept_fd;
  const char * hostname = NULL;
  struct addrinfo host_info, *host_info_list;
  int hops;
  int numPlayers;
  char ** arg;
  vector<int> socketfdForPlayers;
  vector<string> ipAddr;
  vector<int> playerListenPorts;

  Ringmaster(char ** argv) : arg(argv) {
    numPlayers = atoi(arg[2]);
    hops = atoi(arg[3]);
    socketfdForPlayers.resize(numPlayers);
    ipAddr.resize(numPlayers);
    playerListenPorts.resize(numPlayers);
    cout << "Potato Ringmaster\n";
    cout << "Players  = " << numPlayers << endl;
    cout << "Hops = " << hops << endl;
  }
  virtual ~Ringmaster() {
    for (int i = 0; i < numPlayers; ++i) {
      close(socketfdForPlayers[i]);
    }
    close(socket_fd);
  }

  int acceptConnect(string & ip) {
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    accept_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (accept_fd == -1) {
      cerr << "Error: cannot accept connection on socket \n" << endl;
      exit(EXIT_FAILURE);
    }
    struct sockaddr_in * temp = (struct sockaddr_in *)&socket_addr;
    ip = inet_ntoa(temp->sin_addr);
    return accept_fd;
  }

  void buildServer(char * port) {
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;
    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
      cerr << "Error: cannot get address info for host" << endl;
      exit(EXIT_FAILURE);
    }

    socket_fd =
        socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
      cerr << "Error: cannot create socket" << endl;
      exit(EXIT_FAILURE);
    }
    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot bind socket" << std::endl;
      exit(EXIT_FAILURE);
    }
    status = listen(socket_fd, 100);
    if (status == -1) {
      std::cerr << "Error: cannot listen on socket" << std::endl;
      exit(EXIT_FAILURE);
    }

    freeaddrinfo(host_info_list);
  }

  void connectToPlayers() {
    for (int i = 0; i < numPlayers; ++i) {
      socketfdForPlayers[i] = acceptConnect(ipAddr[i]);
      send(socketfdForPlayers[i], &i, sizeof(i), 0);
      send(socketfdForPlayers[i], &numPlayers, sizeof(numPlayers), 0);
      recv(socketfdForPlayers[i], &playerListenPorts[i], sizeof(playerListenPorts[i]), MSG_WAITALL);
      cout << "Player " << i << " is ready to play\n";
    }
  }

  void connectPlayers() {
    for (int i = 0; i < numPlayers; ++i) {
      addrInfo addr_info;
      int neighbor = (i + 1) % numPlayers;
      addr_info.port = playerListenPorts[neighbor];
      strcpy(addr_info.ip, ipAddr[neighbor].c_str());
      send(socketfdForPlayers[i], &addr_info, sizeof(addr_info), 0);
    }
  }
  void init() {
    Potato potato;
    potato.hops = hops;
    //when the game needs to end

    if (hops == 0) {
      for (int i = 0; i < numPlayers; ++i) {
        if (send(socketfdForPlayers[i], &potato, sizeof(potato), 0) != sizeof(potato)) {
          cerr << "Sent a bad potato" << endl;
        }
      }
      return;
    }

    //send the potato to a random player
    int random = rand() % numPlayers;
    cout << "Ready to start the game, sending potato to player " << random << endl;
    if (send(socketfdForPlayers[random], &potato, sizeof(potato), 0) != sizeof(potato)) {
      cerr << "Sent a bad potato" << endl;
    }
    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < numPlayers; ++i) {
      FD_SET(socketfdForPlayers[i], &rfds);
    }
    select(accept_fd + 1, &rfds, NULL, NULL, NULL);
    for (int i = 0; i < numPlayers; ++i) {
      if (FD_ISSET(socketfdForPlayers[i], &rfds)) {
        recv(socketfdForPlayers[i], &potato, sizeof(potato), MSG_WAITALL);
        break;
      }
    }
    potato.showTrace();
  }
};

int main(int argc, char ** argv) {
  if (argc < 4) {
    cout << "Usage: ./ringmaster <port_num><num_players><num_hops>\n";
  }
  Ringmaster * rm = new Ringmaster(argv);
  rm->buildServer(rm->arg[1]);
  rm->connectToPlayers();
  rm->connectPlayers();
  rm->init();
  delete rm;
  return EXIT_SUCCESS;
}
