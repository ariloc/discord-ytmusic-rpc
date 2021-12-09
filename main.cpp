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

#include "json/json.hpp"

#include "discord-rpc/include/discord_rpc.h"

const char *PORT = "15472";

// http response, sent as confirmation of receipt of JSON packet
const char *HTTPOK_MSG = "HTTP/1.1 200 OK\nContent-Length: 2\nContent-Type: text/plain; charset=utf-8\r\n\r\nOK";
const int HTTPOK_SZ = strlen(HTTPOK_MSG);

static const char* APPLICATION_ID = "811008884293500999";
const char *MAIN_IMG = "main_img";
const char *PLAY_ICON = "play_icon";
const char *PAUSE_ICON = "pause_icon";

const char *PLAYING_TXT = "Playing";
const char *PAUSED_TXT = "Paused";
const char *LISTEN_ALONG_TXT = "Listen Along";

const int MAX_BUF = 2048;
const int BACKLOG = 10;

using json = nlohmann::json;

struct upd_struct {
    std::string song = "", artist = "", album = "", url = "";
    int duration = 0, elapsed = 0, state = 0;

    void print() {
        if (!state) puts("Stopped playing music.");
        else {
            printf("Now %s: %s by %s", (state == 2 ? "paused" : "playing"), song.c_str(), artist.c_str());
            if (!album.empty())
                printf(", in %s", album.c_str());
            printf(". ");

            if (duration > 0)
                printf("(%d / %d) seconds elapsed.", elapsed / 1000, duration / 1000);

            putchar('\n');

            if (url.empty()) puts("No URL provided.");
            else printf("URL: %s\n",url.c_str());
        }
    }
};

static void discordInit() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);
}

static void updateDiscordPresence (upd_struct &u) {
    if (!u.state) Discord_ClearPresence();
    else {
        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));

        std::string artist_album = (u.artist) + (!u.album.empty() ? " - " + u.album : "");
        discordPresence.state = artist_album.c_str(); // combine artist and album in one string
        discordPresence.details = u.song.c_str();

        if (u.state == 1 && u.duration > 0) { // only include timestamps if it's playing and if times provided
            discordPresence.startTimestamp = time(0);
            discordPresence.endTimestamp = time(0) + (u.duration - u.elapsed) / 1000; // add seconds to end of song
        }

        discordPresence.largeImageKey = MAIN_IMG;
        discordPresence.smallImageKey = (u.state == 2 ? PAUSE_ICON : PLAY_ICON); // icon if paused or playing
        discordPresence.smallImageText = (u.state == 2 ? PAUSED_TXT : PLAYING_TXT); // icon text if paused or playing
        discordPresence.instance = (int)(u.state == 1); // if playing, you can say that the event is ongoing

        if (!u.url.empty()) { // Show button only when URL is available
            discordPresence.button1_label = LISTEN_ALONG_TXT;
            discordPresence.button1_url = u.url.c_str();
        }

        Discord_UpdatePresence(&discordPresence);
    }
}

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

void processInfo (int fd) {
    char buf[MAX_BUF];
    memset(buf,0,sizeof(buf)); // initialize it, just in case

    if (recv(fd, buf, MAX_BUF, 0) <= 0) // receive the data
        return; // -1 on error, 0 if connection closed, both cases return

    // search HTTP header
    char *aux = strstr(buf, "\r\n\r\n");
    if (!aux) {fprintf(stderr,"Error while removing HTTP header.\n"); return;}

    aux += 4; // if found, remove it by moving pointer to content

    json j = json::parse(aux); // parse as JSON with library

    upd_struct toSend;
    toSend.song = toSend.artist = toSend.album = toSend.url = "";
    toSend.elapsed = toSend.duration = toSend.state = 0;

    if (!j.contains("state")) {
        fprintf(stderr,"state not provided in JSON.\n");
        return;
    }
    toSend.state = j.at("state");

    if (j.contains("position") && j.contains("duration"))
        toSend.duration = j.at("duration"), toSend.elapsed = j.at("position");
    
    if (toSend.state) {
        if (!j.contains("song") || !j.contains("artist")) {
            fprintf(stderr,"Either song or artist aren't provided in JSON, while state isn't 0.\n");
            return;
        }
        toSend.song = j.at("song"), toSend.artist = j.at("artist");

        if (j.contains("album"))
            toSend.album = j.at("album");
    }

    if (j.contains("url")) // only fill url container if provided with HTTP request
        toSend.url = j.at("url");
    else { // if not included, let's get it ourselves!

    }

    toSend.print(); // show update received
    
    // all good if I got to here
    updateDiscordPresence(toSend); // update presence in Discord

    // send acknowledge of receipt
    if (send(fd, HTTPOK_MSG, HTTPOK_SZ, 0) == -1)
        perror("send");
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int waitIncoming() {
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

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

        // get address so it can be printed
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

        processInfo(new_fd);

        close(new_fd);
	}
}

int main() {
    discordInit();

    int stat;
    if (stat = waitIncoming()) {
        Discord_Shutdown();
        return stat;
    }

    Discord_Shutdown();

    return 0;
}