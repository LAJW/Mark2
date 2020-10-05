# Dead Project
This is a dead project full of regrets. Avert your eyes. Dropped because of game design problems I couldn't solve - ship management and cRPG gameplay can't be performed by a human in real time. Go watch the [FTL postmortem](https://www.youtube.com/watch?v=P4Um97AUqp4) - they were here too and salvaged it.

Note: CMake + Cotire is seemingly impossible to set up correctly on Windows, so don't rely on it for quick iteration in games. This is what put the final nail in the coffin here.

# Introduction
A 2D cRPG set in a post-post-apocalyptic world, with a twist - you're
controlling a ship/hovercraft built from 2D blocks, each of which has
permadeath.

# Getting Started
We're supporting two builds: Windows, and Mac. Linux might be coming. 

# Build and Test - Visual Studio
1. Install Visual Studio 2017 C++ (with native CMake support) (19 needs tweaks)
2. Run the following command in the project root
   ```
   git submodule update --init --recursive
   ```
   You'll need to run it every now and then, when we upgrade submodules to
   latest releases.
3. Load CMake project in VS - "File -> Open -> CMake..."
4. Select `x64-Release` as the configuration and `main.exe` or `unit_test.exe`
   as the executable.
5. Hit `F5`

Notes:
- Whenever you add or remove a file, pull new changes or switch the branch,
  you'll have to run "CMake -> Cache (x64) -> Generate"
- The executables are located in here:
  ```
  %userprofile%\CMakeBuilds\hash\build\x64-Release\bin
  ```
- While regular CMake-soultion generation will work (it's used by CI), the
  native support is preferred for development.

# Build and Test - Non-VS
1. Install CMake and a compiler of your choice. You'll also need OpenGL dev
   packages.
2. Run a following commands in project root to build and run the game
   executable using 8-thread parallelization:
   ```
   git submodule update --init --recursive
   cmake -H. -Bbuild
   cmake --build . --target main -- -j8
   cd main
   ../build/bin/main
   ```
3. Run this to build and run unit tests:
   ```
   cmake --build . --target unit_test -- -j8
   build/bin/unit_test
   ```
