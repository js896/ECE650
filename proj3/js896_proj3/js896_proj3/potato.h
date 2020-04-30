#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#define MAX_HOPS 512
#define INFO_SIZE 100

using namespace std;

class Potato {
 public:
  int hops;
  int index;
  int path[MAX_HOPS];
  Potato() {
    hops = 0;
    index = 0;
    memset(path, 0, sizeof(path));
  }
  void showTrace() {
    cout << "Trace of potato:" << endl;
    for (int i = 0; i < index; ++i) {
      if (i > 0) {
        cout << ",";
      }
      cout << path[i];
    }
    cout << endl;
  }
};

class addrInfo {
 public:
  int port;
  char ip[INFO_SIZE];
  addrInfo() {
    port = -1;
    memset(ip, 0, sizeof(ip));
  }
};
