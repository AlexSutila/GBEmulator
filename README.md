# GameBoyEmulator

My third honest and most successful attempt at creating a gameboy emulator.
Written entirely in C using win32 for graphical related things. In its current
state it passes all blargs cpu instruction and memory access timing tests and
dmg acid 2 to prove basic graphical accuracy. 

There are still lots of inaccuracies and areas that I'm not happy with and I 
plan on rewriting a lot of it. Keep in mind this is still very much a work in 
progress. I also have yet to implement any sound support. 

![passing tests](https://user-images.githubusercontent.com/96510931/152865596-5f94be09-f963-4f6f-bc8c-cf6852a817db.png)

## Gameplay

I've tested quite a few games, including but not limited to the games shown in the screenshot
below. All of these games run fine, but there exists a handfull that still don't. 

![Screenshots](https://user-images.githubusercontent.com/96510931/147891212-fab95f69-677e-454a-ad16-411728402da1.png)

## Compiling

If one wishes to use this for what ever reason, you can simply clone this repo and
open the sln file in visual studio 2022. I've tested this from one machine to another
so it should work. 

Be sure to have a savs folder in the same directory as any ROMS
