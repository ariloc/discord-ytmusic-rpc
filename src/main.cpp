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

int main() {
    discordInit();

    Py_Initialize();
    PyObject *ytmusicapi = PyImport_ImportModule("ytmusicapi");
    PyObject *ytmusic = PyObject_GetAttrString(ytmusicapi, "YTMusic");
    ytmusic = PyObject_CallFunctionObjArgs(ytmusic, Py_BuildValue("s",HEADERS_AUTH_PATH));

    signalHandlerSetup();
    std::atexit(atExitRun);

    int stat;
    if ((stat = getRequests(ytmusic))) return stat;

    return 0;
}