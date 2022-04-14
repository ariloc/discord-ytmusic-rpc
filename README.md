# discord-ytmusic-rpc
A personal project to achieve that my Discord status shows my currently playing song on YouTube Music in my phone.
This project is **not** in any way related or endorsed by Google nor Discord. It's neither related to any of the tools or libraries credited onward.

**NOTE: REMEMBER TO DOWNLOAD THE `discord-rpc-buttons` SUBMODULE. The `build.sh` file uses the contents of the folder for its recompilation. For example, if you're using `git`, use the `--recurse-submodules` parameter of `git clone`.**

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

This is, again, for the purpose of getting the exact URL to the song you're actually playing. Bear in mind this is not foolproof, and sometimes the request may be faster than the history gets updated (and therefore the "Listen Along" button won't be shown), so maybe you would want to have a little bit of delay on your requests from your phone. Also note that rich presence updates are limited to one in 15 seconds [as stated in the documentation](https://discord.com/developers/docs/rich-presence/how-to), so it's also to expect if an update isn't refflected immediately if you throw a bunch of them too quickly.

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
* `img_url`: **\[optional\]** A *VALID* link to an image of the album cover for the currently playing song. If missing, it will be automatically fetched in the same way described for the `url`. Thanks to the user [Bas950](https://github.com/Bas950) for [this commit](https://github.com/PreMiD/Presences/commit/b90ed56099c56220571f6b85b1f046e600e592a5#diff-8ddd6dbf8e05c89eb790399a539a8c6fc14fda6a18135104784447b036cb2b46) to the PreMiD YouTube Music integration in the [Presences repo](https://github.com/PreMiD/Presences), as it brought me info about Discord allowing for links to be used in presences, and how to implement it.

# Set up Xorg and Discord

As I said in the intro, I'll be using a configuration file which specifies a dummy screen, i.e. a screen that doesn't exist, to be able to run Xorg. That configuration file is included in the *Xorg* folder. Of course for this you'll need to install `xorg` itself. In Debian for example, the easiest way would be by running the following command:

```
sudo apt install xorg
```

The same goes for installing Discord, which as far as I know can be a little troublesome depending on your distro. Note that I've tested the program in Debian and Archlinux, but in theory should work in any Linux distro. In the case of Debian, for example, I've installed it using the provided .deb file.

And if you were wondering at this point about making it run in Windows, you'll surely have to see a way of running make, modifying the Makefile to link correctly with Python, amongst other issues. The Xorg step also doesn't apply at all in Windows. As I see no personal use for making it run in Windows, I haven't explored it at all, but you're free to do so if you want.

Going back to the topic, you'll probably need to login into your Discord account for all of this to work. As far as I know, you can't login through the terminal, and therefore you'll need an actual display to access the GUI. The most convenient way I found to do this, is using X11 forwarding through SSH. This way, I can ssh to my server from a PC running a Linux desktop, adding the `-X` argument when connecting. With this, when I ran the `discord` command on the terminal, a new window appeared in the computer where I was ssh'ing from, so I was able to successfully login and close the program. As long as I open Discord often enough, I shouldn't have to login again, at least in my experience.

To then run Discord with the "dummy display" configuration file, you can use this command:

```
xinit discord -- :1 -config /path/to/dummy/config
```

`xinit` will start both Xorg and then Discord (as it needs X to be running), with the config file we specified. I tell it to run in display 1, to avoid any possible conflicts. You can change the display number by replacing the part `:1` with the display number you want. If you don't know what any of this means, I sort of understand it as that each Xorg instance runs in a different display number, so it works like an identifier for each instance. **This explanation may be very well just wrong**, but what I *think* do know, is that you ***don't want** to pick a low number*, as it's more probable that if you're actualy already running Xorg in your system, lower number displays may be already in use. You can just keep changing and trying, until there's no error related to an already running instance in a certain display.

I had issues with this command though, unless I ran it as a superuser. There are ways of avoiding this to make it more secure, but you'll probably stumble with issues related to permissions. The solution that worked for me is to modify to the `Xwrapper.conf` file [suggested in this gist](https://gist.github.com/alepez/6273dc5220c1c5ec5f3f126e739d58bf). Though, in this case, set `needs_root_rights=no` instead of yes, and then install the `xserver-xorg-video-dummy` package. Hopefully there aren't any major security issues with this method.

Anyway, the mentioned solution allowed me to run the command with `cron`, which is a job scheduler, and adding the following line after using the `crontab -e` command while logged in as `user` makes it so it runs on boot:

```
@reboot xinit discord -- :1 -config ./relative/path/to/dummy/config
```

Note that the config path must be a relative path, and not an absolute path, as a consequence of the previous modifications to `Xwrapper.conf`. I wanted to find a solution for this, but I couldn't find anything. So for this, just remember than cron runs tasks in the home directory of the user, and therefore you have to specify a relative path from that folder.

As well as I used `cron` to start Discord with Xorg on boot, I can also start the executable itself (already compiled) with cron. As you might have guessed, it's a matter of appending another line starting with `@reboot` followed by the command to run:

```
@reboot /path/to/compiled/executable /path/to/authentication/headers
```

Here we can use absolute paths if we want, and don't forget to specify the path to the authentication headers file. All `cron` jobs are run in the background, so you don't have to worry about them much.

You can just reboot, wait a bit for Discord and Xorg to start properly, and you should be done with this part. There shouldn't even be a need to login for it to work.

# Screenshots

In order to be able to show presence updates on Discord, I had to create an application from the [Discord Developer Portal](https://discord.com/developers/applications). This way, the program is made such that it works with the assets uploaded for a certain application ID, which is defined in a constant variable in the `presence.h` file. As I am the only one able to modify the mentioned application, you could modify the variables so it works with your own if you want to do so.

Below there are some screenshots of the app and presence working.

![Playing a song](/images/song-playing.png?raw=true "Song playing")
![Song paused](/images/song-paused.png?raw=true "Song paused")
![Logs preview](/images/terminal-logs.png?raw=true "Logs preview")

Note that foreign (non-latin) characters **should also be displayed correctly**, or at least they were in my testing.

# Automation of the requests from the phone

For these kind of tasks, I think the most popular app is Tasker. Though in my case, I've made use of Automagic, which sadly isn't mantained anymore, but that means you can get it for free in its [website](https://automagic4android.com/download_en.html).

However, I also made a working example in Automate by LlamaLab, [which you can get for free in the Play Store](https://play.google.com/store/apps/details?id=com.llamalab.automate&hl=en&gl=US).

I'm **not related** to any of these apps, **nor am I promoting** them. I just use Automagic for other things, so it seemed convenient for me, and I thought it would also be nice making an alternative example in an app which is free, easier to download, and probably has a wider user base.

Both flows for each of the apps are included in the *automation* folder. Remember to change inside the flows where it says *server_ip* with the IP address of the computer running the executable. You can see how to import them in the apps, but you should be able to click them from a file manager and let it suggest you open it automatically with the automation app which will then import it accordingly.

As I spent more time in the Automagic one, it also tries to fix a bug I noticed in YouTube Music, where the album name informed by the app gets stuck on the one from a previous song, when a video is played from the queue. To achieve this, I only send the album name in the JSON request whenever the thumbnail of the playing media has a 1:1 aspect ratio, which is the one typically for the album art of actual songs (not videos). The possibility of a video with a 1:1 aspect ratio playing exists, but the possibility seems very slim to me, and this is the most reliable method I could get to work, in order to differentiate between songs and videos playing. You can modify the flow as you please, whether you prefer or not this fix.

# Final details

For now, I would only suggest using this project inside your local area network. To make this feasable to be used through the internet, I think I would need to add some sort of token field with a unique character string you would use to validate any request, and maybe also support secure connections in some way. It's not that big of a deal if anyone wanted to change your Discord status, but I don't think it would be very fun if someone wanted to show information to your friends as if it were you.

If you have any thoughts about this project, any issue you encounter or any suggestion, let me know! I can't guarantee I would be able to implement it (even less do it quickly), but I will try!
