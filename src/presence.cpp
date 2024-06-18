#include <string>
#include <cstring>
#include <ctime>

#include "presence.h"
#include "song_info.hpp"
#include "idle_timer.h"

#include "../include/discord_rpc.h"

static IdleTimer idleTimer = IdleTimer([](){
    puts("Idle timer timeout reached. Clearing presence.");
    clearPresence();
});

void discordInit() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);
}

void clearPresence() { Discord_ClearPresence(); }

void updateDiscordPresence (upd_struct &u) {
    if (!u.state) {
        clearPresence();
        idleTimer.cancel();
    }
    else {
        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));

        std::string artist_album = (u.track.artist) + (!u.track.album.empty() ? " - " + u.track.album : "");
        discordPresence.state = artist_album.c_str(); // combine artist and album in one string
        discordPresence.details = u.track.name.c_str();

        if (u.state == 1 && u.duration > 0) { // only include timestamps if it's playing and if times provided
            discordPresence.startTimestamp = time(0);
            discordPresence.endTimestamp = time(0) + (u.duration - u.elapsed) / 1000; // add seconds to end of song
        }

        discordPresence.largeImageKey = (!u.img_url.empty() ? u.img_url.c_str() : MAIN_IMG); // display album cover image if provided
        discordPresence.smallImageKey = (u.state == 2 ? PAUSE_ICON : PLAY_ICON); // icon if paused or playing
        discordPresence.smallImageText = (u.state == 2 ? PAUSED_TXT : PLAYING_TXT); // icon text if paused or playing
        discordPresence.instance = (int)(u.state == 1); // if playing, you can say that the event is ongoing

        if (!u.url.empty()) { // Show button only when URL is available
            discordPresence.button1_label = LISTEN_ALONG_TXT;
            discordPresence.button1_url = u.url.c_str();
        }

        Discord_UpdatePresence(&discordPresence);
        idleTimer.start();
    }
}
