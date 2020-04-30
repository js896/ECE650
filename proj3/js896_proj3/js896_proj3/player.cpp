//this is the player
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "potato.h"

using namespace std;

class Player {
 public:
  int status;
  int socket_fd;
  int accept_fd;
  const char * hostname = NULL;
  struct addrinfo host_info, *host_info_list;
  int playerID;
  int numPlayers;
  int fdMaster;
  int fdNeighbor;
  char ** arg;

  Player(char ** argv) : arg(argv) {}
  ~Player() {
    close(accept_fd);
    close(fdMaster);
    close(fdNeighbor);
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

  int buildServer() {
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;
    status = getaddrinfo(hostname, "", &host_info, &host_info_list);
    if (status != 0) {
      cerr << "Error: cannot get address info for host" << endl;
      exit(EXIT_FAILURE);
    }

    //cast sockaddr into sockaddr_in
    struct sockaddr_in * addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
    addr_in->sin_port = 0;

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

    //get os assignde port number
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(socket_fd, (struct sockaddr *)&sin, &len) == -1) {
      cerr << "getsockname error";
    }
    return ntohs(sin.sin_port);
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

  void connectServer(char * node, char * port, int & socket_fd) {
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(node, port, &host_info, &host_info_list)) != 0) {
      cerr << "Error: canont get address info for host" << endl;
      exit(EXIT_FAILURE);
    }
    if ((socket_fd = socket(host_info_list->ai_family,
                            host_info_list->ai_socktype,
                            host_info_list->ai_protocol)) == -1) {
      cerr << "Error: can't create socket" << endl;
      exit(EXIT_FAILURE);
    }
    if ((status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen)) == -1) {
      cerr << "Error: can't connect to socket" << endl;
      exit(EXIT_FAILURE);
    }
    freeaddrinfo(host_info_list);
  }

  void connectMaster() {
    //receive playerID
    recv(fdMaster, &playerID, sizeof(playerID), 0);
    //receive numPlayers
    recv(fdMaster, &numPlayers, sizeof(numPlayers), 0);

    int listenPort = buildServer();
    send(fdMaster, &listenPort, sizeof(listenPort), 0);
    cout << "Connect as player " << playerID << " out of " << numPlayers << " total players"
         << endl;
  }

  void connectNeighbor() {
    addrInfo addrinfo;
    recv(fdMaster, &addrinfo, sizeof(addrinfo), MSG_WAITALL);

    char portID[9];
    sprintf(portID, "%d", addrinfo.port);
    connectServer(addrinfo.ip, portID, fdNeighbor);
    string temp;
    acceptConnect(temp);
  }

  void passingPotato() {
    srand((unsigned int)time(NULL) + playerID);
    Potato potato;
    fd_set rfds;
    int fd[] = {accept_fd, fdNeighbor, fdMaster};
    int largest = accept_fd > fdNeighbor ? accept_fd : fdNeighbor;
    largest = largest > fdMaster ? largest : fdMaster;
    int nfds = 1 + largest;
    int size;
    while (true) {
      FD_ZERO(&rfds);
      for (int i = 0; i < 3; ++i) {
        FD_SET(fd[i], &rfds);
      }
      select(nfds, &rfds, NULL, NULL, NULL);
      for (int i = 0; i < 3; ++i) {
        if (FD_ISSET(fd[i], &rfds)) {
          size = recv(fd[i], &potato, sizeof(potato), 0);

          break;
        }
      }
      if (size == 0) {
        return;
      }
      else {
        --potato.hops;
        potato.path[potato.index++] = playerID;
        if (potato.hops == 0) {
          cout << "I'm it" << endl;
          send(fdMaster, &potato, sizeof(potato), 0);
          return;
        }
        int random = rand() % 2;
        cout << "Sending potato to "
             << (random == 0 ? ((playerID - 1 + numPlayers) % numPlayers)
                             : (playerID + 1) % numPlayers)
             << endl;
        send(fd[random], &potato, sizeof(potato), 0);
      }
    }
  }
};

int main(int argc, char ** argv) {
  if (argc < 3) {
    cout << "Usage: ./player <machine_name><port_num>";
  }
  Player * player = new Player(argv);
  player->connectServer(player->arg[1], player->arg[2], player->fdMaster);
  player->connectMaster();
  player->connectNeighbor();
  player->passingPotato();
  delete player;
  return EXIT_SUCCESS;
}
