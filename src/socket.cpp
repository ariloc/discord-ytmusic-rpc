#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "socket.h"
#include "parse.h"

#include <Python.h>

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int getRequests(PyObject *ytmusic) {
    static char buf[MAX_BUF];
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // can be IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // setup a stream
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure, free memory

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (true) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

        // get address so it can be printed
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

        memset(buf,0,sizeof(buf)); // clean buffer, just in case

        if (recv(new_fd, buf, MAX_BUF, 0) > 0) { // receive the data
            // return value is -1 on error, 0 if connection closed
            // neither of those cases is useful

            // look for HTTP header
            char *aux = strstr(buf, "\r\n\r\n");
            if (aux) {
                aux += 4; // if found, remove it by moving pointer to content
                
                if (!parseRequest(aux, ytmusic)) { // once data is assured, check and parse JSON
                    // send acknowledge of receipt if everything was OK
                    if (send(new_fd, HTTPOK_MSG, HTTPOK_SZ, 0) == -1)
                        perror("send"); // inform error if any, while sending acknowledge
                }
            }
            else fprintf(stderr,"Error while removing HTTP header.\n");
        }
        else fprintf(stderr,"Error while receiving data.\n");
        

        close(new_fd);
	}
}