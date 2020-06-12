# NewsTail
Terminal RSS news feed

NewsTail shows news from your favorite RSS feeds rigth in your browser. Where you need to be.

I made this because i would like to create a small project in C++. Unfortunately idiomatic C++ is not that easy, so this is still very much a work in progress.

# Dependencies
GCC and Cmake for building. 

The CMakeLists.txt in /src/ uses Hunter to manage dependencies which are:

OpenSSL - For getting SSL RSS feeds.
Boost - For HTTP client and XML and JSON parsing.

But, Hunter will fetch this automatically. Hunter is nice.

# Build
To build a release version:
./build_once_release.sh

# Configure the feeds
This is done using the config.json file.

# Screenshot

![Screenshot](https://raw.githubusercontent.com/martinskou/newstail/master/screenshot/screenshot.png)

![Screenshot](https://raw.githubusercontent.com/martinskou/newstail/master/screenshot/screenshot2.png)


# Warning
The code is naive and things will break. But I'll make it better.

There are used some generale ASCII terminal codes. NCurses are not used, so these codes will not work in esoteric terminals.

The SSL certificates are not testes.

# Tested and working on these systems
Mac / iTerm2
Mac / Kitty
Mac / Terminal
Manjaro / xfce4-terminal
