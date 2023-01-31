# graphtail
[![CMake](https://github.com/demogorgon1/graphtail/actions/workflows/cmake.yml/badge.svg)](https://github.com/demogorgon1/graphtail/actions/workflows/cmake.yml)

You've got a program that outputs a bunch of numbers to a CSV file? You want to visualize those numbers with graphs 
in real-time, while the program is still running and appending new numbers?
If you can say yes to both those questions, _graphtail_ might be the tool you need.

## Installing
### From source
First of all you'll need git and cmake to acquire and build the project. Then you can run:
```
git clone https://github.com/demogorgon1/graphtail.git
cd graphtail
mkdir build
cd build
cmake ..
cmake --build .
```
On Windows, if successful, you'll find ```graphtail.exe``` in ```src\Release```.
On Linux (and similar), if successful, you can run ```make install``` to install ```graphtail```.
