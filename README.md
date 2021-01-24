# Crypto Utilities

## Build
Clone this repository with all of its submodules

- `git clone --recurse-submodules https://github.com/Zilliqa/cryptoutils/`

Run `CMake` by providing a `CMAKE_INSTALL_PREFIX` argument to install it
to a directory of your choice:
  - `mkdir -p build; cd build`
  - `cmake ../ -DCMAKE_INSTALL_PREFIX=../install`
  - `make install`
  
Alternatively, run the provided `build.sh` script. The project will be built in `build/`
subdirectory of the project. 

If you wish to build / install an archive instead of
the default shared library build, provide the additional `CMake` argument 
`-DCRYPTOUTILS_BUILD_ARCHIVE=1`.
