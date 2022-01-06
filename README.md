# GameBoyEmulator

My second (okay third) honest and most successful attempt at creating a gameboy emulator.
Written entirely in C using win32 for graphical related things.

Passes all blargs cpu instruction tests, as well as memory access timing tests, still some
problems in DMG acid2, right mole is still visible for some reason, but I personally don't
find it to be that big a deal. 

It still fails to run a few games, I have a collection of screenshots below to show some of
the games that it does run that I took while playing them myself. Although, as I type this
I haven't got very far in any of these games, they do at least play

![Screenshots](https://user-images.githubusercontent.com/96510931/147891212-fab95f69-677e-454a-ad16-411728402da1.png)

If one wishes to use this for what ever reason, I've compiled it on other machines by including
all source and header files as existing files in visual studio without any issues. Cloning the repo,
including the header files and source files folders as include directories, and lastly changing the
subsystem to windows in the project properties worked fine for me.

BE SURE to have a savs folder in the same directory as any ROMS.

I am currently working on a debugger to help me weed out a few occasional slip ups I have in some
games. If the project is built in release mode, this debugger will not appear, only in debug builds
as I'm not entirely sure how it will affect performance when its finished. 

![image](https://user-images.githubusercontent.com/96510931/148293549-3423ca80-2165-4ebd-a8cf-341407fe84fe.png)

As of now, you can only break and pause the emulation (b), step one instruction (n), and scroll 
through the memory view (w and s to move 0x100 bytes at a time). I plan on adding a disassembler
and maybe a call stack view because debugging this has been rough.

The only cartridge types I emulate as of right now are cartridges without memory bank controllers
and cartridges with MBC1 chips. I plan on adding more in the future if I get around to it I
suppose.

There is still a lot that is incomplete and everything is still subject to change. I personally
dont feel like I will ever call this project 'finished', as there is always something that
can be tweaked to make the emulation more accurate. I'm not entirely sure how far I will go
with it.
