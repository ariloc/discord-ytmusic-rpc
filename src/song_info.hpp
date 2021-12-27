#ifndef __SONG_INFO_HPP__
#define __SONG_INFO_HPP__

#include <cstdio>
#include <string>

struct songInfo {
    std::string name = "", artist = "", album = "";

    bool operator< (const songInfo &o) const {
        if (name != o.name) return name < o.name;
        if (artist != o.artist) return artist < o.artist;
        return album < o.album;
    }
};

struct upd_struct {
    songInfo track;
    std::string url = "", img_url = "";
    int duration = 0, elapsed = 0, state = 0;

    void print() {
        if (!state) puts("Stopped playing music.");
        else {
            printf("Now %s: %s by %s", (state == 2 ? "paused" : "playing"), track.name.c_str(), track.artist.c_str());
            if (!track.album.empty())
                printf(", in %s", track.album.c_str());
            printf(". ");

            if (duration > 0)
                printf("(%d / %d) seconds elapsed.", elapsed / 1000, duration / 1000);

            putchar('\n');

            if (url.empty()) puts("No URL provided.");
            else printf("URL: %s\n",url.c_str());
        }
    }
};

#endif /* __SONG_INFO_HPP__ */