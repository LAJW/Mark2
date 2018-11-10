# Introduction
A 2D action RPG set in a post-post-apocalyptic world, with a twist - you're
controlling a ship/hovercraft built from 2D blocks, each of which has
permadeath.

# Getting Started
We're supporting two builds: Windows, and Mac. Linux might be coming. 

# Build and Test - Visual Studio
1. Install Visual Studio 2017 C++ (with native CMake support)
2. Run a following command in the project root
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

# Contribute

1. Create a branch using a following naming convention:
   ```
   <user-name>/branch-name
   ```

2. Make some changes. Each commit should start with one of the following
   prefixes. Each change should be small enough to fit within the bounds of the
   description. It's there for easier rollback:

   | Prefix    | Description |
   | --------- | ----------- |
   | FEAT:     | A new feature, functionality or a new unused function |
   | FIX:      | Change that fixes a bug in code |
   | TEST:     | Commit containing changes only to unit tests |
   | MAINT:    | Maintenance change - affecting build scripts or configuration scripts |
   | BREAK:    | Change that brakes backwards compatibility in some way (moving assets, changing save file structure, architectural-level change stopping other PRs from getting easily merged, etc.) |
   | REFACTOR: | Change that does not add or remove functionality |
   | ASSET:    | Changes to game assets (images, sounds) |

   A linter is provided with this repository. You can validate commit messages
   starting from origin/develop by running the following in PowerShell: 

   ```
   & ../scripts/git-lint.ps1
   ```

   You can also install hooks so that they check if your commit message is
   incorrectly formatted when committing:

   ```
   & ../scripts/install-hooks.ps1
   ```

   The validator will run any time you run `git commit`. Don't worry - this
   only installs hooks for the local repository, and globally not for the
   enitre machine.

3. After you're done committing, push your branch to the repository
4. Create a PR
5. Answer review comments, and get at least one approval. Remember not to
   squash existing commits until you're about to merge.
6. Satisfy the needs of CI (unless we've run out of time)
7. Run this to reformat your code:
   ```
   git filter-branch --tree-filter 'git-clang-format-3.8 origin/develop' -f -- origin/develop..HEAD
   ```
8. Merge
9. Delete the branch