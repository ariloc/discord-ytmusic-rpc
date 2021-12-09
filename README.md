# discord-ytmusic-rpc
A personal project to achieve that my Discord status shows my currently playing song on YouTube Music in my phone.
This project is **not** in any way related or endorsed by Google nor Discord.

In order to acheive this, after lots of googling, I've got to the conclusion that the best I can do is to run a Discord instance in a computer (ideally running 24/7, like some sort of home server), and run some code which will listen for HTTP requests with the required information in JSON format. It's not the best, but it should work. Of course, those requests will be made by my phone, where I'll be running some suitable automation app that will watch for any Media Session changes from the YouTube Music app, so it can automatically send the information through my LAN (initially, at least).

For this, I'll make use of my slightly modified version of the [discord/discord-rpc](https://github.com/discord/discord-rpc) library, [ariloc/discord-rpc-buttons](https://github.com/ariloc/discord-rpc-buttons). If it isn't clear, this would help me to update my rich presence status, allowing me to display the currently playing song, artist and album, and even the time remaining for the song to finish. 
I'd also like to have a button which links to the song actually playing (inspired by [PreMiD's YouTube Music integration](https://premid.app/store/presences/YouTube%20Music), which again, it **isn't** related in any way to this project), but from my testing it doesn't seem that easy to get that link without automating UI actions (i.e. pressing the share button in the app, or enabling "Stats for nerds" to get the YouTube video ID), which I personally think would make the process unreliable or at least inconvenient. 

Fortunately, I found about ([sigma67](https://github.com/sigma67/ytmusicapi))'s unofficial [YouTube Music API](https://github.com/sigma67/ytmusicapi), which has the `get_history()` function, allowing me to get my most recently played songs. I can then look for the name of the song I'm supposedly playing, find it in the list provided by the API, and get the video ID from it. The only problem I have with the API is that it's written in Python, while the rest of the code is written in C++, but linking them up didn't seem to much of a trouble.

Also to manage requests with JSON, I'm using the "JSON for Modern C++" library": Lohmann, N. JSON for Modern C++ (Version 3.10.4) [Computer software]. [https://github.com/nlohmann](https://github.com/nlohmann). You can check out the repo [here](https://github.com/nlohmann/json).

In order to listen for requests in a certain port with C++, I made use of sockets with help of [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) book. 

And finally, as in my case the computer where the script will be listening to requests is running Linux with no GUI, I have to be running an instance of Xorg with a "dummy screen" configuration so Discord can run properly (refer to [https://askubuntu.com/questions/453109/add-fake-display-when-no-monitor-is-plugged-in](https://askubuntu.com/questions/453109/add-fake-display-when-no-monitor-is-plugged-in)).

Anyway, hope you don't dislike my terrible code, and that all that should be credited, were done so properly!
