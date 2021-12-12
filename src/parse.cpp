#include <stdio.h>

#include "parse.h"
#include "presence.h"
#include "cache_map.hpp"
#include "song_info.hpp"

#include "../include/json.hpp"

#include <Python.h>

using json = nlohmann::json;

void getUrl (upd_struct &u, PyObject *ytmusic) {
    PyObject *history = PyObject_CallMethod(ytmusic, "get_history", NULL);
    
    int n = (int)PyObject_Length(history);
    if (n < 1) return;

    std::string vId = "";
    for (int i = 0; i < std::min(n,10) && vId.empty(); i++) { // get the first 10 elements
        PyObject *item = PyList_GetItem(history,i);

        PyObject *song_name_py = PyDict_GetItemString(item,"title");

        PyObject *videoId_py = PyDict_GetItemString(item, "videoId");

        std::string song_name = PyBytes_AsString(PyUnicode_AsEncodedString(song_name_py, "UTF-8", "strict"));
        std::string videoId = PyBytes_AsString(PyUnicode_AsEncodedString(videoId_py, "UTF-8", "strict"));

        if (song_name == u.track.name) swap(vId, videoId);
    }

    if (!vId.empty())
        u.url = BASE_URL + vId;
}

int parseRequest (char* request, PyObject *ytmusic) {
    json j;
    try {
        j = json::parse(request); // parse as JSON with library
    }
    catch (...) {fprintf(stderr,"Error parsing JSON.\n"); return 1;}

    upd_struct toSend;

    try {toSend.state = j.at("state");}
    catch (...) {
        fprintf(stderr,"state not provided correctly in JSON.\n");
        return 1;
    }

    if (toSend.state) { // when state isn't none (!= 0)
        try {toSend.duration = j.at("duration"), toSend.elapsed = j.at("position");}
        catch (...) {}
        
        try {toSend.track.name = j.at("song"), toSend.track.artist = j.at("artist");}
        catch (...) {
            fprintf(stderr,"Error getting song and/or artist from JSON.\n");
            return 1;
        }

        try {toSend.track.album = j.at("album");}
        catch (...) {}

        /* static allocations are kept intact between different calls of the function.
            keep in memory the most recent urls, to reduce requests to the API. */
        static sized_cacheMap<songInfo, std::string> cacheMap;
        cacheMap.max_cache = MAX_CACHE;

        try {toSend.url = j.at("url");} 
        catch (...) { // if url not included, let's (try) to get it ourselves!

            // first try finding it inside the cache
            auto it = cacheMap.find(toSend.track);
            if (it != cacheMap.end()) toSend.url = (*it).first;
            else getUrl(toSend, ytmusic);

            if (!toSend.url.empty()) 
                cacheMap.insert(toSend.track, toSend.url);
        } 
    }

    toSend.print(); // show update received
    
    // all good if I got to here
    updateDiscordPresence(toSend); // update presence in Discord

    return 0; // if everything went correctly, return 0
}