#ifndef CLIENT_H
#define CLIENT_H

#include "client.h"

void* c_input (void* in);
void* s_output(void* out);
int serverconnect(char* server, char* port);
int main(int argc, char** argv);
void spawn_threads(int sd);

#endif