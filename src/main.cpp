#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
#include <iostream>
#include <fstream>

#include "socket.h"

#include "../include/json.hpp"
#include "../include/discord_rpc.h"

#include <Python.h>

using json = nlohmann::json;

void signalHandlerSetup() { // exit when signal received
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = [](int){exit(1);};
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

void atExitRun() {
    Py_Finalize();
    Discord_Shutdown();
}

int main(int argc, char *argv[]) {
    char *headers_auth_path = NULL;
    if (argc == 2) {
        std::ifstream f(argv[1]);
        if (f.good()) headers_auth_path = argv[1];
    }
    if (!headers_auth_path)
        fprintf(stderr,"WARNING: No valid path provided for the authentication headers.\n"
                       "Remember to provide it as an argument. For more information, look it up in the repo.\n"
                       "No URLs will be fetched, and they will ONLY be available if provided in the JSON requests.\n\n");

    discordInit();

    Py_Initialize();
    PyObject *ytmusicapi = PyImport_ImportModule("ytmusicapi");
    PyObject *ytmusic = PyObject_GetAttrString(ytmusicapi, "YTMusic");
    ytmusic = PyObject_CallObject(ytmusic, 
                                 (headers_auth_path ? PyTuple_Pack(1,Py_BuildValue("s",headers_auth_path)) : NULL));

    signalHandlerSetup();
    std::atexit(atExitRun);

    int stat;
    if ((stat = getRequests(ytmusic))) return stat;

    return 0;
}