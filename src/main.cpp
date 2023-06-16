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

static const char* DEFAULT_LANG = "en";

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
    // Initialize auth headers file and language if provided
    char *headers_auth_path = NULL;
    char *lang = NULL;
    if (argc >= 2) {
        std::ifstream f(argv[1]);
        if (f.good()) headers_auth_path = argv[1];
    }
    if (!headers_auth_path)
        fprintf(stderr,"WARNING: No valid path provided for the authentication headers.\n"
                       "Remember to provide it as an argument. For more information, look it up in the repo.\n"
                       "No URLs will be fetched, and they will ONLY be available if provided in the JSON requests.\n\n");
    
    if (argc >= 3) {
        lang = argv[2];
    }
    if (!lang)
        fprintf(stderr,"WARNING: No language set, defaulting to '%s'.\n\n", DEFAULT_LANG);

    // Initialize Discord RPC
    discordInit();

    // Initialize CPython and YTMusic API
    Py_Initialize();
    PyObject *ytmusicapi = PyImport_ImportModule("ytmusicapi");
    PyObject *ytmusic = PyObject_GetAttrString(ytmusicapi, "YTMusic");
    PyObject *ytmusic_params = 
        PyTuple_Pack(5,
            headers_auth_path ? Py_BuildValue("s",headers_auth_path) : Py_None,
            Py_None,
            Py_True,
            Py_None,
            Py_BuildValue("s",lang ? lang : DEFAULT_LANG)
        );
    ytmusic = PyObject_CallObject(ytmusic, ytmusic_params);

    // Setup actions to run at exit
    signalHandlerSetup();
    std::atexit(atExitRun);

    // Print exceptions and exit if thrown
    if (!ytmusic) {
        PyErr_Print();
        exit(1);
    }

    // Start receiving requests
    int stat;
    if ((stat = getRequests(ytmusic))) return stat;

    return 0;
}
