# neo-coat
Don't have a girlfriend ? No worries, we have your back with neo-coat, a wifi display controller. With neo-coat, you can do various tasks such as displaying pictures, drawing and playing snake. This repository is made for storing the firmware and instructions of this product. Feel free to mess with it, just remember to credit me in your version :)
# What is different ? 
There have been many other firmwares on controlling leds on the internet. Most can exhibit individual features like drawing, animations or playing games. However, neither of them have all of these features combined and made to such perfection like neo-coat does. Here is an overview of what are different:
- **No computer connection.** Your Esp is the head of everything from the game to it's UI.
- **An all-in-one solution.** Everything is here, no need for special firmware on different things.
- **Ease of modification.** This firmware is made for hobbyist to mess with. It's functions are made into objects with careful comments on each code segment. Even a kid can mess with it thanks to the insane level of abstractions.
- **Great UI.** None the less, the UI is amazing following the new, trendy Neomorphism style. 
- **Speed.** Fastled and Async Web Server, this is a rare combination and is unique to this firmware. 
# Ingridients
To make a neo-coat, you'll need:
- A Neopixel matrix or any leds supported by FastLed. You can alter the software to support any Leds if you wish.
- An Esp8266. Other boards would require further coding.
- Power supply that can power your matrix and your esp8266. For Neopixels and esp8266, you can use a 5v charger ( any smartphone charger ) and calculate the Amps by the following formula: Numbers of Leds * 60mA = I;
You'll need Platformio for the compilation and upload of this program.

# Notes 
- If you wish to see the first version of neo-coat, switch to original branch. This branch is the original version and it is optimal for anyone who wants to make a 6x6 matrix as well. Choose this branch if you don't need to alter the code either for your matri's dimensions or just simple interest.
- Switch to compatibility branch for ease of reading and editing. This branch is made for people who want to understand the code and change it. It is optimized by splitting code from the original version to different libraries, thus reducing the complexity of main.cpp. More details in Compatibility sector.
# Instructions
Coming soon

# Compatibilty 
This sector is for compatibility branch. Here are some instructions on making changes to this branch:
- If you want to change the dimensions of the matrix, change the macros of HEIGHT and WIDTH at the beginning of the main.cpp. Remember that you also need to change these in index.js in the data folder.
- If you want to change how pixel position is calculated or how to draw them, change the following function in drawable library in libs folder.
- If you want to change animations logic or change what type of file to animate, take a look at the animation library in the libs folder.
# Credit 
This section is to credit and pay respect for those who contributed largely to the project:
- **Acrobotic** - Animations API
- **Emanuel Knöpfel** - Snake Library
- **me-no-dev** - ESPAsyncWebServer
- The authors of the **FastLed library** and those who contributed to it.
- The authors of the **standard libraries** used in the project.
# Copyright
Copyright (C) 2014  Pham Tuan Binh

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Created by Pham Tuan Binh, 11/10/2020.
