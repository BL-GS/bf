# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf

# Include any dependencies generated for this target.
include CMakeFiles/bf.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/bf.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/bf.dir/flags.make

CMakeFiles/bf.dir/src/bf.c.o: CMakeFiles/bf.dir/flags.make
CMakeFiles/bf.dir/src/bf.c.o: src/bf.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/bf.dir/src/bf.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/bf.dir/src/bf.c.o   -c /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/src/bf.c

CMakeFiles/bf.dir/src/bf.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/bf.dir/src/bf.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/src/bf.c > CMakeFiles/bf.dir/src/bf.c.i

CMakeFiles/bf.dir/src/bf.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/bf.dir/src/bf.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/src/bf.c -o CMakeFiles/bf.dir/src/bf.c.s

CMakeFiles/bf.dir/src/bf_utils.c.o: CMakeFiles/bf.dir/flags.make
CMakeFiles/bf.dir/src/bf_utils.c.o: src/bf_utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/bf.dir/src/bf_utils.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/bf.dir/src/bf_utils.c.o   -c /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/src/bf_utils.c

CMakeFiles/bf.dir/src/bf_utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/bf.dir/src/bf_utils.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/src/bf_utils.c > CMakeFiles/bf.dir/src/bf_utils.c.i

CMakeFiles/bf.dir/src/bf_utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/bf.dir/src/bf_utils.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/src/bf_utils.c -o CMakeFiles/bf.dir/src/bf_utils.c.s

# Object files for target bf
bf_OBJECTS = \
"CMakeFiles/bf.dir/src/bf.c.o" \
"CMakeFiles/bf.dir/src/bf_utils.c.o"

# External object files for target bf
bf_EXTERNAL_OBJECTS =

bf: CMakeFiles/bf.dir/src/bf.c.o
bf: CMakeFiles/bf.dir/src/bf_utils.c.o
bf: CMakeFiles/bf.dir/build.make
bf: /usr/lib/x86_64-linux-gnu/libfuse.so
bf: /home/blgs/lib/libddriver.a
bf: CMakeFiles/bf.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable bf"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bf.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/bf.dir/build: bf

.PHONY : CMakeFiles/bf.dir/build

CMakeFiles/bf.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/bf.dir/cmake_clean.cmake
.PHONY : CMakeFiles/bf.dir/clean

CMakeFiles/bf.dir/depend:
	cd /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf /home/blgs/RiscV/WorkSpace/Code/user-land-filesystem/fs/bf/CMakeFiles/bf.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/bf.dir/depend

