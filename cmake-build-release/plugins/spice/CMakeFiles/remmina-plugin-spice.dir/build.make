# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/163/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/163/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/toolsdevler/git/Remmina

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/toolsdevler/git/Remmina/cmake-build-release

# Include any dependencies generated for this target.
include plugins/spice/CMakeFiles/remmina-plugin-spice.dir/depend.make
# Include the progress variables for this target.
include plugins/spice/CMakeFiles/remmina-plugin-spice.dir/progress.make

# Include the compile flags for this target's objects.
include plugins/spice/CMakeFiles/remmina-plugin-spice.dir/flags.make

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.o: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/flags.make
plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.o: ../plugins/spice/spice_plugin.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/toolsdevler/git/Remmina/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.o"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.o -c /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin.c

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.i"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin.c > CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.i

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.s"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin.c -o CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.s

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.o: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/flags.make
plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.o: ../plugins/spice/spice_plugin_file_transfer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/toolsdevler/git/Remmina/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.o"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.o -c /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin_file_transfer.c

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.i"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin_file_transfer.c > CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.i

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.s"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin_file_transfer.c -o CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.s

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.o: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/flags.make
plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.o: ../plugins/spice/spice_plugin_usb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/toolsdevler/git/Remmina/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.o"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.o -c /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin_usb.c

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.i"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin_usb.c > CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.i

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.s"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/toolsdevler/git/Remmina/plugins/spice/spice_plugin_usb.c -o CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.s

# Object files for target remmina-plugin-spice
remmina__plugin__spice_OBJECTS = \
"CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.o" \
"CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.o" \
"CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.o"

# External object files for target remmina-plugin-spice
remmina__plugin__spice_EXTERNAL_OBJECTS =

plugins/spice/remmina-plugin-spice.so: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin.c.o
plugins/spice/remmina-plugin-spice.so: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_file_transfer.c.o
plugins/spice/remmina-plugin-spice.so: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/spice_plugin_usb.c.o
plugins/spice/remmina-plugin-spice.so: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/build.make
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libgtk-3.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libgdk-3.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libgdk_pixbuf-2.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libpango-1.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libcairo.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libgobject-2.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libglib-2.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libgio-2.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libgmodule-2.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libspice-client-gtk-3.0.so
plugins/spice/remmina-plugin-spice.so: /usr/lib/x86_64-linux-gnu/libspice-client-glib-2.0.so
plugins/spice/remmina-plugin-spice.so: plugins/spice/CMakeFiles/remmina-plugin-spice.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/toolsdevler/git/Remmina/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C shared library remmina-plugin-spice.so"
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/remmina-plugin-spice.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugins/spice/CMakeFiles/remmina-plugin-spice.dir/build: plugins/spice/remmina-plugin-spice.so
.PHONY : plugins/spice/CMakeFiles/remmina-plugin-spice.dir/build

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/clean:
	cd /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice && $(CMAKE_COMMAND) -P CMakeFiles/remmina-plugin-spice.dir/cmake_clean.cmake
.PHONY : plugins/spice/CMakeFiles/remmina-plugin-spice.dir/clean

plugins/spice/CMakeFiles/remmina-plugin-spice.dir/depend:
	cd /home/toolsdevler/git/Remmina/cmake-build-release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/toolsdevler/git/Remmina /home/toolsdevler/git/Remmina/plugins/spice /home/toolsdevler/git/Remmina/cmake-build-release /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice /home/toolsdevler/git/Remmina/cmake-build-release/plugins/spice/CMakeFiles/remmina-plugin-spice.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugins/spice/CMakeFiles/remmina-plugin-spice.dir/depend

