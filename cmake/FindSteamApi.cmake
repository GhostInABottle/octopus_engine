# FindSteamApi
# ------------
#
# Locate the Steam SDK library
#
# This module defines:
#
#   STEAM_API_LIBRARIES
#   STEAM_API_FOUND
#   STEAM_API_INCLUDE_DIRS
#

find_path(STEAM_API_INCLUDE_DIR steam_api.h
  HINTS ENV STEAM_API_DIR
  PATH_SUFFIXES include/steam include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  )

find_library(STEAM_API_LIBRARY NAMES steam_api steam_api64 libsteam_api
  HINTS ENV STEAM_API_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  )

set(STEAM_API_INCLUDE_DIRS "${STEAM_API_INCLUDE_DIR}")
set(STEAM_API_LIBRARIES "${STEAM_API_LIBRARY}")

find_package_handle_standard_args(STEAM_API DEFAULT_MSG STEAM_API_LIBRARIES STEAM_API_INCLUDE_DIRS)

mark_as_advanced(STEAM_API_INCLUDE_DIR STEAM_API_LIBRARY STEAM_API_INCLUDE_DIRS STEAM_API_LIBRARIES)
