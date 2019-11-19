#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <lib/client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


class MyClient
{
public:
    MyClient();
    enum conn_ret_t tryConnect(char server_ip[30]);
    void assertConnection(char server_ip[30]);
    int sendMsg(void * type_buffer, int type_pointer);
    int recvMsg(void *str_buffer);
};

#endif // CLIENT_H
