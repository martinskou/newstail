# NewsTail
Terminal RSS news feed

NewsTail shows news from your favorite RSS feeds rigth in your browser. Where you need to be.

I made this because i would like to create a small project in C++. Unfortunately idiomatic C++ is not that easy, so this is still very much a work in progress.

# Dependencies
Cmake for building. 

The CMakeLists.txt in /src/ uses Hunter to manage dependencies which are:

OpenSSL - For getting SSL RSS feeds.
Boost - For HTTP client and XML and JSON parsing.

# Build
To build a release version:
./build_once_release.sh

# Configure the feeds
This is done using the config.json file.

# Screenshot

![Screenshot](https://raw.githubusercontent.com/martinskou/newstail/blob/master/sreenshot/screenshot.png)

