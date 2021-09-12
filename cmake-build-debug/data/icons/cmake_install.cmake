# Install script for directory: /home/toolsdevler/git/Remmina/data/icons

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/icons/hicolor/scalable/actions/remmina-camera-photo-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-connect-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-disconnect-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-document-save-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-document-send-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-duplicate-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-dynres-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-fit-window-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-fullscreen-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-multi-monitor-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-go-bottom-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-keyboard-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-pan-down-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-pan-up-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-pin-down-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-pin-up-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-preferences-system-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-scale-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-switch-page-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/remmina-system-run-symbolic.svg;/usr/local/share/icons/hicolor/scalable/actions/view-list.svg")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/icons/hicolor/scalable/actions" TYPE FILE FILES
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-camera-photo-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-connect-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-disconnect-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-document-save-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-document-send-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-duplicate-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-dynres-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-fit-window-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-fullscreen-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-multi-monitor-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-go-bottom-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-keyboard-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-pan-down-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-pan-up-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-pin-down-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-pin-up-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-preferences-system-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-scale-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-switch-page-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/remmina-system-run-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/actions/view-list.svg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/icons/hicolor/scalable/emblems/remmina-sftp-symbolic.svg;/usr/local/share/icons/hicolor/scalable/emblems/remmina-ssh-symbolic.svg")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/icons/hicolor/scalable/emblems" TYPE FILE FILES
    "/home/toolsdevler/git/Remmina/data/icons/scalable/emblems/remmina-sftp-symbolic.svg"
    "/home/toolsdevler/git/Remmina/data/icons/scalable/emblems/remmina-ssh-symbolic.svg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  
        set(DESTDIR_VALUE "$ENV{DESTDIR}")
        if (NOT DESTDIR_VALUE)
            execute_process(COMMAND /usr/bin/gtk-update-icon-cache -qfit /usr/local/share/icons/hicolor)
        endif()
        
endif()

