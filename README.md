# ZeusOps Debug Utilities
A quick-and-dirty utility for tracking issues with a system while playing ARMA.

![A Screenshot of one of the Plots](/screenshots/FPS.jpg?raw=true "Tracking FPS during an Operation")

## License
These utilities are governed by the GNU GPL v2.0 license, but includes works from different people, licensed under different terms. See [`LICENSE`](LICENSE) for more information.

## Requirements
 - [CMake](https://cmake.org/)
 - [Qt 5](https://www.qt.io/)
 - [Visual Studio 2019]

## Supported Platforms
Only Windows 10, at least version 1901.

## Binaries
Currently none.

## How to use
1. Use `zeusDebug.exe` for creating log files. Click `Start Measurement`, start and play ARMA3 and stop the measurement once you are done.
2. Once you have a log file from yourself or a buddy, open it in the plotting utility `zeusDebugPlot.exe` by selecting the containing folder at the top. It should be loaded and you can view the graphs in the tabs.

## How to build

1. Clone the repository `https://github.com/blizzard4591/zeusUtilities.git` to a folder of your choice, we will call this folder `D:\zeusUtilities`.
2. Fix the path to your Qt5 installation in `CMakeLists.txt` in the variable `CMAKE_PREFIX_PATH`.
3. Execute CMake on the sources, if possible, do an out-of-tree build. 
	E.g., in CMake, the source directory would be `D:\zeusUtilities` and `Where to build the binaries` would be `D:\zeusUtilities\build`. Then click `Configure` and `Generate`.
4. Open the create project file `ZeusDebug.sln` in Visual Studio. Build the solution in either `Debug` or `Release` configuration, depending on your needs (if unsure, choose Release).


## FAQ
### Is this basically like UIforETW and ProcessHacker, just without all the nice features, images and graphs?
Yes.

### Wow, this is a cobbled together mess of stuff, couldn't this be cleaned up a lot?
Yes.

### Hey, this only works for ARMA3, can I adapt this for X?
Yes.

### Yes?
Yes.

