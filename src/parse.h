#ifndef __PARSE_H__
#define __PARSE_H__

#include "presence.h"
#include "song_info.hpp"
#include "cache_map.hpp"

#include <Python.h>

/*
    JSON tags:
    - "state": [required] 0 -> none, 1 -> playing, 2 -> paused
    - "song": [required when state != 0] song name
    - "artist": [required when state != 0] artist name
    - "album": (optional) album name
    - "position": (optional) number of millis transcurred since the start of the song 
    - "duration": (optional) number of millis representing the duration of the song
    - "url": (optional) the link to the song in YouTube Music
*/

#include "../include/json.hpp"

using json = nlohmann::json;

static const char* BASE_URL = "https://music.youtube.com/watch?v=";

/*
 * Given the track name (string) look for the specified
 * song in the history of the user authenticated with the headers_auth.json file,
 * and return a PyObject with the song metadata.
 * If the song isn't found, returns a NULL pointer.
 */
PyObject* getMetadataHistory (std::string track_name, PyObject *ytmusic); // TODO: check if description is OK.

/*
 * Parses the JSON received in a request, checks for validity, and builds an
 * `upd_struct` element containing the information.
 * Calls getUrl() if no url provided in the request.
 * Once all possible fields filled, calls updateDiscordPresence() to update
 * the Rich Presence status in Discord.
 * Returns 1 on error, 0 on success.
 */
int parseRequest (char* request, PyObject *ytmusic);


#endif /* __PARSE_H__ */