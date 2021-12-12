#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <cstring>
#include <sys/socket.h>

#include "parse.h"

#include <Python.h>

static const char *PORT = "15472";

// http response, sent as confirmation of receipt of JSON packet
static const char *HTTPOK_MSG = "HTTP/1.1 200 OK\nContent-Length: 2\nContent-Type: text/plain; charset=utf-8\r\n\r\nOK";
static const int HTTPOK_SZ = (int)strlen(HTTPOK_MSG);

static const int MAX_BUF = 2048;
static const int BACKLOG = 10;

/*
 * Given a socket, IPv4 or IPv6, get the respective network address structure.
 */
void *get_in_addr(struct sockaddr *sa);

/*
 * Build a scoket, bind it to the first possible interface to the specified PORT,
 * and start accepting connections.
 * In case of any accepted connection, we assume they are HTTP requests.
 * Therefore, it strips the HTTP header and passes the content to a parser function.
 * If that function succeeds, a HTTP/1.1 200 OK message is sent back to the
 * sender.
 */
int getRequests(PyObject *ytmusic);

#endif /* __SOCKET_H__ */