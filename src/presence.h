#ifndef __PRESENCE_H__
#define __PRESENCE_H__

#include "song_info.hpp"

static const char *APPLICATION_ID = "811008884293500999";
static const char *MAIN_IMG = "main_img";
static const char *PLAY_ICON = "play_icon";
static const char *PAUSE_ICON = "pause_icon";

static const char *PLAYING_TXT = "Playing";
static const char *PAUSED_TXT = "Paused";
static const char *LISTEN_ALONG_TXT = "Listen Along"; 

/*
 * Initialize Discord Rich Presence with defined APPLICATION_ID
 */
void discordInit();

/*
 * Clear Discord Rich Presence.
 */
void clearPresence();

/*
 * Update Discord Rich Presence with specified update information included in `u`.
 */
void updateDiscordPresence (upd_struct &u);

#endif /* __PRESENCE_H__ */
