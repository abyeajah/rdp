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
CMAKE_COMMAND = /snap/clion/162/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/162/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/toolsdevler/git/Remmina

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/toolsdevler/git/Remmina/cmake-build-debug

# Include any dependencies generated for this target.
include plugins/st/CMakeFiles/remmina-plugin-st.dir/depend.make
# Include the progress variables for this target.
include plugins/st/CMakeFiles/remmina-plugin-st.dir/progress.make

# Include the compile flags for this target's objects.
include plugins/st/CMakeFiles/remmina-plugin-st.dir/flags.make

plugins/st/CMakeFiles/remmina-plugin-st.dir/st_plugin.c.o: plugins/st/CMakeFiles/remmina-plugin-st.dir/flags.make
plugins/st/CMakeFiles/remmina-plugin-st.dir/st_plugin.c.o: ../plugins/st/st_plugin.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/toolsdevler/git/Remmina/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object plugins/st/CMakeFiles/remmina-plugin-st.dir/st_plugin.c.o"
	cd /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/remmina-plugin-st.dir/st_plugin.c.o -c /home/toolsdevler/git/Remmina/plugins/st/st_plugin.c

plugins/st/CMakeFiles/remmina-plugin-st.dir/st_plugin.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/remmina-plugin-st.dir/st_plugin.c.i"
	cd /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/toolsdevler/git/Remmina/plugins/st/st_plugin.c > CMakeFiles/remmina-plugin-st.dir/st_plugin.c.i

plugins/st/CMakeFiles/remmina-plugin-st.dir/st_plugin.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/remmina-plugin-st.dir/st_plugin.c.s"
	cd /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st && /usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/toolsdevler/git/Remmina/plugins/st/st_plugin.c -o CMakeFiles/remmina-plugin-st.dir/st_plugin.c.s

# Object files for target remmina-plugin-st
remmina__plugin__st_OBJECTS = \
"CMakeFiles/remmina-plugin-st.dir/st_plugin.c.o"

# External object files for target remmina-plugin-st
remmina__plugin__st_EXTERNAL_OBJECTS =

plugins/st/remmina-plugin-st.so: plugins/st/CMakeFiles/remmina-plugin-st.dir/st_plugin.c.o
plugins/st/remmina-plugin-st.so: plugins/st/CMakeFiles/remmina-plugin-st.dir/build.make
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libgtk-3.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libgdk-3.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libgdk_pixbuf-2.0.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libpango-1.0.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libcairo.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libgobject-2.0.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libglib-2.0.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libgio-2.0.so
plugins/st/remmina-plugin-st.so: /usr/lib/x86_64-linux-gnu/libgmodule-2.0.so
plugins/st/remmina-plugin-st.so: plugins/st/CMakeFiles/remmina-plugin-st.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/toolsdevler/git/Remmina/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared module remmina-plugin-st.so"
	cd /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/remmina-plugin-st.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugins/st/CMakeFiles/remmina-plugin-st.dir/build: plugins/st/remmina-plugin-st.so
.PHONY : plugins/st/CMakeFiles/remmina-plugin-st.dir/build

plugins/st/CMakeFiles/remmina-plugin-st.dir/clean:
	cd /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st && $(CMAKE_COMMAND) -P CMakeFiles/remmina-plugin-st.dir/cmake_clean.cmake
.PHONY : plugins/st/CMakeFiles/remmina-plugin-st.dir/clean

plugins/st/CMakeFiles/remmina-plugin-st.dir/depend:
	cd /home/toolsdevler/git/Remmina/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/toolsdevler/git/Remmina /home/toolsdevler/git/Remmina/plugins/st /home/toolsdevler/git/Remmina/cmake-build-debug /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st /home/toolsdevler/git/Remmina/cmake-build-debug/plugins/st/CMakeFiles/remmina-plugin-st.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugins/st/CMakeFiles/remmina-plugin-st.dir/depend

