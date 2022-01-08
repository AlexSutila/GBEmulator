# GameBoyEmulator

My okay third honest and most successful attempt at creating a gameboy emulator.
Written entirely in C using win32 for graphical related things.

Passes all blargs cpu instruction tests, as well as memory access timing tests, still some
problems in DMG acid2, right mole is still visible for some reason, but I personally don't
find it to be that big a deal. 

![Screenshots](https://user-images.githubusercontent.com/96510931/147891212-fab95f69-677e-454a-ad16-411728402da1.png)

If one wishes to use this for what ever reason, I've compiled it on other machines by including
all source and header files as existing files in visual studio without any issues. Cloning the repo,
including the header files and source files folders as include directories, and lastly changing the
subsystem to windows in the project properties worked fine for me.

BE SURE to have a savs folder in the same directory as any ROMS.

I am currently working on a debugger to help me weed out a few occasional slip ups I have in some
games. If the project is built in release mode, this debugger will not appear, only in debug builds
as I'm not entirely sure how it will affect performance when its finished. 

![image](https://user-images.githubusercontent.com/96510931/148623056-9484449f-8a83-4933-80dd-1d57604a8148.png)

It can do the following:
  break                         (b)
  step instruction              (n)
  step 100 instructions         (N)
  scroll through memory view    (w/s)

The only cartridge types I emulate as of right now are cartridges without memory bank controllers,
cartridges with MBC1 chips, and cartridges with MBC3 chips without real time clocks. I will still
likely add more when I get around to it.

There is still a lot that is incomplete and everything is still subject to change. I personally
dont feel like I will ever call this project 'finished', as there is always something that
can be tweaked to make the emulation more accurate. I'm not entirely sure how far I will go
with it.
