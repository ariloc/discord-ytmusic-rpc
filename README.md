# discord-ytmusic-rpc
A personal project to achieve that my Discord status shows my currently playing song on YouTube Music in my phone.
This project is **not** in any way related or endorsed by Google nor Discord. It's neither related to any of the tools or libraries credited onward.

# Introduction / What's this about

In order to acheive this, after lots of googling, I've got to the conclusion that the best I can do is to run a Discord instance in a computer (ideally running 24/7, like some sort of home server), and run some code which will listen for HTTP requests with the required information in JSON format. It's not the best, but it should work. Of course, those requests will be made by my phone, where I'll be running some suitable automation app that will watch for any Media Session changes from the YouTube Music app, so it can automatically send the information through my LAN (initially, at least).

For this, I'll make use of my slightly modified version of the [discord/discord-rpc](https://github.com/discord/discord-rpc) library, [ariloc/discord-rpc-buttons](https://github.com/ariloc/discord-rpc-buttons). If it isn't clear, this would help me to update my rich presence status, allowing me to display the currently playing song, artist and album, and even the time remaining for the song to finish. 
I'd also like to have a button which links to the song actually playing (inspired by [PreMiD's YouTube Music integration](https://premid.app/store/presences/YouTube%20Music), which again, it **isn't** related in any way to this project), but from my testing it doesn't seem that easy to get that link without automating UI actions in my phone (i.e. pressing the share button in the app, or enabling "Stats for nerds" to get the YouTube video ID), which I personally think would make the process unreliable or at least inconvenient. 

Fortunately, I found about [sigma67](https://github.com/sigma67/)'s unofficial [YouTube Music API](https://github.com/sigma67/ytmusicapi), which has the `get_history()` function, allowing me to get my most recently played songs. I can then look for the name of the song I'm supposedly playing, find it in the list provided by the API, and get the video ID from it. The only problem I have with the API is that it's written in Python, while the rest of the code is written in C++, but I managed to get it working embedding a Python interpreter inside the code with `Python.h`. Also to avoid too many unnecessary requests to the API, I implemented a bit of a "cache" that would remember the last songs played and their respective url. Probably made it too complicated, but it works in my testing at least. The amount of entries preserved in this cache is defined by the `MAX_CACHE` value, if you want to change it.

Also to manage requests with JSON, I'm using the "JSON for Modern C++" library": Lohmann, N. JSON for Modern C++ (Version 3.10.4) [Computer software]. [https://github.com/nlohmann](https://github.com/nlohmann). You can check out the repo [here](https://github.com/nlohmann/json).

In order to listen for requests in a certain port with C++, I made use of sockets with help of [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) book. 

And finally, as in my case the computer where the script will be listening to requests is running Linux with no GUI, I have to be running an instance of Xorg with a "dummy screen" configuration so Discord can run properly (refer to [https://askubuntu.com/questions/453109/add-fake-display-when-no-monitor-is-plugged-in](https://askubuntu.com/questions/453109/add-fake-display-when-no-monitor-is-plugged-in)).

Anyway, hope you don't dislike my terrible code, and that all that should be credited, were done so properly!

# Compiling

The `build.sh` file included should do all the work for you. It's nothing really fancy, but it should work. 

First it checks (or at least it should) if you have installed the right dependencies. You'll need `g++`, `cmake` and `python3`. As far as I know, it should work with any version of `python3`, but I've tested it working with 

Also, about python, you'll need the unofficial YouTube Music API module by [sigma67](https://github.com/sigma67/). If you skipped the introduction, [here's the repo again](https://github.com/sigma67/ytmusicapi) and you can follow [here](https://ytmusicapi.readthedocs.io/en/latest/setup.html) instructions in how to install it.

After all of that, the script tries to erase all build files included in the discord-rpc-buttons library, the static library in the /lib folder and includes in the /include folder, to then recompile it all, just for good measure.

Once that's done, it runs `make` which compiles the actual source files and links them with the corresponding libraries, leaving you with a `discord-ytmusic-rpc` executable in the main folder.

# Making it work

The only step remaining to make it work fully is to get the `headers_auth.json` file. The purpose of this file is to be able to perform authenticated requests in your Google account, and fetch your song history. How to get it [it's better explained in the unofficial YouTube Music API documentation](https://ytmusicapi.readthedocs.io/en/latest/setup.html). For reference, I got mine by running a python interactive shell, importing the library, and using the `setup(filepath="")` command as described in the aforementioned link. After that, you can just copy the file whereever you want, and remember to specify the path as an argument to the executable. As an example, if you're using Linux and the file is in the same folder, you can run the program the following way:

```
./discord-ytmusic-rpc "./headers_auth.json"
```

This is, again, for the purpose of getting the exact URL to the song you're actually playing. Bear in mind this is not foolproof, and sometimes the request may be faster than the history gets updated (and therefore the "Listen Along" button won't be shown), so maybe you would want to have a little bit of delay on your requests from your phone.

About the requests, the program by default listens in port `15472` (no reason, just an arbitrary number). If you want to change it, it's defined in the `socket.h` file. I know it's not the best to have constants in header files... but it works and I spent already much more time into this that I should've.

The program expects HTTP requests with a JSON formatted content. It really just ignores the HTTP headers, so any errors thrown (if any) are from the validation of the provided JSON. The fields you can use are the following:

* `state`: **\[REQUIRED\]** A single integer that represents the playback state.
  * `0`: Stopped playing (clear Discord Rich Presence). Any other fields will be ignored in this case.
  * `1`: Playing
  * `2`: Paused
* `song`: **\[REQUIRED when `status = 0` or `status = 1`\]** The song title.
* `artist`: **\[REQUIRED when `status = 0` or `status = 1`\]** The artist name.
* `album`: **\[optional\]** The album name.
* `duration`: **\[optional\]** An integer representing the duration of the song, expressed in millis.
* `position`: **\[optional\]** An integer representing the amount of time elapsed from the start of the song, expressed in millis. Both `duration` and `position` need to be provided to show any time-related information.
* `url`: **\[optional\]** A *VALID* link to the song. If the provided link is invalid, rich presence may not be shown. Otherwise if missing, it will be automatically fetched from the song history if the authentication headers are properly setup.

# Screenshots

WIP

# Set up Xorg and Discord

Work in Progress, dedicated folder *should* be added soon.

# Phone automation of the requests

WIP
