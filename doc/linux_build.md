# Building on Linux

The following are the steps needed to prepare the dependencies and build the engine using Ubuntu.

* Install the following packages or build them if a newer version is needed:
  * build-essential
  * libglfw3-dev (3.4)
  * libboost-dev
    * libboost-filesystem-dev (if GCC version is less tha 8)
  * libfreetype6-dev
  * libharfbuzz-dev
  * liblua5.2-dev
  * libgl-dev
  * libglew-dev
  * libphysfs-dev
  * fmod
    * Download from fmod.com
    * Extract the files
    * Copy files in core/inc to /usr/local/include/FMOD
    * Copy files in core/lib/x86_64 to /usr/local/lib
* Go to the engine's base folder and run `cmake .`
* If it's successful, simply run `make` to produce the executable `octopus_engine`
