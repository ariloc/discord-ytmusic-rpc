#include <stdio.h>
#include <utility>

#include "parse.h"
#include "presence.h"
#include "cache_map.hpp"
#include "song_info.hpp"

#include "../include/json.hpp"

#include <Python.h>

using json = nlohmann::json;

PyObject* getMetadataHistory (std::string track_name, PyObject *ytmusic) {
    PyObject *history = PyObject_CallMethod(ytmusic, "get_history", NULL);
    
    int n = (int)PyObject_Length(history);
    if (n < 1) return NULL;

    std::string vId = "";
    for (int i = 0; i < std::min(n,10); i++) { // get the first 10 elements
        PyObject *item = PyList_GetItem(history,i);

        PyObject *song_name_py = PyDict_GetItemString(item,"title");
        std::string song_name = "";
        if (song_name_py != NULL)
            song_name = PyBytes_AsString(PyUnicode_AsEncodedString(song_name_py, "UTF-8", "strict"));

        if (!song_name.empty() && song_name == track_name) return item;
    }
    return NULL;
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
        static sized_cacheMap<songInfo, std::pair<std::string, std::string> > cacheMap; // we'll save both url and cover url
        cacheMap.max_cache = MAX_CACHE;

        std::pair<std::string, std::string> mapEntry = {"",""};
        try {
            toSend.url = j.at("url");
            toSend.img_url = j.at("img_url");
        } 
        catch (...) { // if url for song or cover not included, let's (try) to get it ourselves!

            // first try finding a match for the song inside the cache
            auto it = cacheMap.find(toSend.track);
            if (it != cacheMap.end()) mapEntry = (*it).first;
            else { // get the fields I want from the metadata
                PyObject* songMetadata = getMetadataHistory(toSend.track.name, ytmusic);

                if (songMetadata != NULL) {
                    std::string videoURL = BASE_URL, imageURL = "";

                    std::string videoId = PyBytes_AsString(PyUnicode_AsEncodedString(PyDict_GetItemString(songMetadata,"videoId"), "UTF-8", "strict"));
                    if (!videoId.empty()) videoURL += videoId;
                    else videoURL = "";

                    PyObject *thumbnail_info = PyList_GetItem(PyDict_GetItemString(songMetadata,"thumbnails"), 0);
                    if (thumbnail_info != NULL) { // if list not empty                        
                        imageURL = PyBytes_AsString(PyUnicode_AsEncodedString(
                                                        PyDict_GetItemString(thumbnail_info, "url"), 
                                                            "UTF-8", "strict"));

                        auto queryStart = imageURL.find('=');
                        if (queryStart != std::string::npos) { // get core url
                            imageURL = imageURL.substr(0, queryStart+1);
                            imageURL += "w1024-h1024-l100-s"; // append my query (1024x1024, quality 100%)
                        }
                    }

                    mapEntry = {videoURL, imageURL};
                }
            }
        }

        try {toSend.url = j.at("url");}
        catch (...) {toSend.url = mapEntry.first;}

        try {toSend.img_url = j.at("img_url");}
        catch (...) {toSend.img_url = mapEntry.second;}

        // most likely if we have the url, we also have the cover url
        // otherwise would be empty, and default image will be shown
        if (!mapEntry.first.empty())
            cacheMap.insert(toSend.track, mapEntry);
    }

    toSend.print(); // show update received
    
    // all good if I got to here
    updateDiscordPresence(toSend); // update presence in Discord

    return 0; // if everything went correctly, return 0
}
