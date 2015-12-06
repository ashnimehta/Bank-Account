#ifndef CLIENT_H
#define CLIENT_H

#include "utils.h"

void* c_input (void* in);
void* s_output(void* out);
int serverconnect(char* server, char* port);
void spawn_threads(int sd);

#endif
