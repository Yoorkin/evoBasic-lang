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
CMAKE_COMMAND = /usr/share/clion-2020.1.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /usr/share/clion-2020.1.1/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/yorkin/CLionProjects/evoBasic

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yorkin/CLionProjects/evoBasic/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/evoBasic.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/evoBasic.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/evoBasic.dir/flags.make

CMakeFiles/evoBasic.dir/main.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/evoBasic.dir/main.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/main.cpp.o -c /home/yorkin/CLionProjects/evoBasic/main.cpp

CMakeFiles/evoBasic.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/main.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/main.cpp > CMakeFiles/evoBasic.dir/main.cpp.i

CMakeFiles/evoBasic.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/main.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/main.cpp -o CMakeFiles/evoBasic.dir/main.cpp.s

CMakeFiles/evoBasic.dir/Lexer.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Lexer.cpp.o: ../Lexer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/evoBasic.dir/Lexer.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Lexer.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Lexer.cpp

CMakeFiles/evoBasic.dir/Lexer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Lexer.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Lexer.cpp > CMakeFiles/evoBasic.dir/Lexer.cpp.i

CMakeFiles/evoBasic.dir/Lexer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Lexer.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Lexer.cpp -o CMakeFiles/evoBasic.dir/Lexer.cpp.s

CMakeFiles/evoBasic.dir/AST.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/AST.cpp.o: ../AST.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/evoBasic.dir/AST.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/AST.cpp.o -c /home/yorkin/CLionProjects/evoBasic/AST.cpp

CMakeFiles/evoBasic.dir/AST.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/AST.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/AST.cpp > CMakeFiles/evoBasic.dir/AST.cpp.i

CMakeFiles/evoBasic.dir/AST.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/AST.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/AST.cpp -o CMakeFiles/evoBasic.dir/AST.cpp.s

CMakeFiles/evoBasic.dir/Parser.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Parser.cpp.o: ../Parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/evoBasic.dir/Parser.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Parser.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Parser.cpp

CMakeFiles/evoBasic.dir/Parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Parser.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Parser.cpp > CMakeFiles/evoBasic.dir/Parser.cpp.i

CMakeFiles/evoBasic.dir/Parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Parser.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Parser.cpp -o CMakeFiles/evoBasic.dir/Parser.cpp.s

CMakeFiles/evoBasic.dir/Logger.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Logger.cpp.o: ../Logger.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/evoBasic.dir/Logger.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Logger.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Logger.cpp

CMakeFiles/evoBasic.dir/Logger.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Logger.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Logger.cpp > CMakeFiles/evoBasic.dir/Logger.cpp.i

CMakeFiles/evoBasic.dir/Logger.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Logger.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Logger.cpp -o CMakeFiles/evoBasic.dir/Logger.cpp.s

CMakeFiles/evoBasic.dir/Type.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Type.cpp.o: ../Type.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/evoBasic.dir/Type.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Type.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Type.cpp

CMakeFiles/evoBasic.dir/Type.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Type.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Type.cpp > CMakeFiles/evoBasic.dir/Type.cpp.i

CMakeFiles/evoBasic.dir/Type.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Type.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Type.cpp -o CMakeFiles/evoBasic.dir/Type.cpp.s

CMakeFiles/evoBasic.dir/Semantic.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Semantic.cpp.o: ../Semantic.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/evoBasic.dir/Semantic.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Semantic.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Semantic.cpp

CMakeFiles/evoBasic.dir/Semantic.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Semantic.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Semantic.cpp > CMakeFiles/evoBasic.dir/Semantic.cpp.i

CMakeFiles/evoBasic.dir/Semantic.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Semantic.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Semantic.cpp -o CMakeFiles/evoBasic.dir/Semantic.cpp.s

CMakeFiles/evoBasic.dir/Exception.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Exception.cpp.o: ../Exception.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/evoBasic.dir/Exception.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Exception.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Exception.cpp

CMakeFiles/evoBasic.dir/Exception.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Exception.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Exception.cpp > CMakeFiles/evoBasic.dir/Exception.cpp.i

CMakeFiles/evoBasic.dir/Exception.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Exception.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Exception.cpp -o CMakeFiles/evoBasic.dir/Exception.cpp.s

CMakeFiles/evoBasic.dir/cmdDistribute.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/cmdDistribute.cpp.o: ../cmdDistribute.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/evoBasic.dir/cmdDistribute.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/cmdDistribute.cpp.o -c /home/yorkin/CLionProjects/evoBasic/cmdDistribute.cpp

CMakeFiles/evoBasic.dir/cmdDistribute.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/cmdDistribute.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/cmdDistribute.cpp > CMakeFiles/evoBasic.dir/cmdDistribute.cpp.i

CMakeFiles/evoBasic.dir/cmdDistribute.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/cmdDistribute.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/cmdDistribute.cpp -o CMakeFiles/evoBasic.dir/cmdDistribute.cpp.s

CMakeFiles/evoBasic.dir/Link.cpp.o: CMakeFiles/evoBasic.dir/flags.make
CMakeFiles/evoBasic.dir/Link.cpp.o: ../Link.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/evoBasic.dir/Link.cpp.o"
	/usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/evoBasic.dir/Link.cpp.o -c /home/yorkin/CLionProjects/evoBasic/Link.cpp

CMakeFiles/evoBasic.dir/Link.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/evoBasic.dir/Link.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yorkin/CLionProjects/evoBasic/Link.cpp > CMakeFiles/evoBasic.dir/Link.cpp.i

CMakeFiles/evoBasic.dir/Link.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/evoBasic.dir/Link.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yorkin/CLionProjects/evoBasic/Link.cpp -o CMakeFiles/evoBasic.dir/Link.cpp.s

# Object files for target evoBasic
evoBasic_OBJECTS = \
"CMakeFiles/evoBasic.dir/main.cpp.o" \
"CMakeFiles/evoBasic.dir/Lexer.cpp.o" \
"CMakeFiles/evoBasic.dir/AST.cpp.o" \
"CMakeFiles/evoBasic.dir/Parser.cpp.o" \
"CMakeFiles/evoBasic.dir/Logger.cpp.o" \
"CMakeFiles/evoBasic.dir/Type.cpp.o" \
"CMakeFiles/evoBasic.dir/Semantic.cpp.o" \
"CMakeFiles/evoBasic.dir/Exception.cpp.o" \
"CMakeFiles/evoBasic.dir/cmdDistribute.cpp.o" \
"CMakeFiles/evoBasic.dir/Link.cpp.o"

# External object files for target evoBasic
evoBasic_EXTERNAL_OBJECTS =

evoBasic: CMakeFiles/evoBasic.dir/main.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Lexer.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/AST.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Parser.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Logger.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Type.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Semantic.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Exception.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/cmdDistribute.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/Link.cpp.o
evoBasic: CMakeFiles/evoBasic.dir/build.make
evoBasic: CMakeFiles/evoBasic.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Linking CXX executable evoBasic"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/evoBasic.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/evoBasic.dir/build: evoBasic

.PHONY : CMakeFiles/evoBasic.dir/build

CMakeFiles/evoBasic.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/evoBasic.dir/cmake_clean.cmake
.PHONY : CMakeFiles/evoBasic.dir/clean

CMakeFiles/evoBasic.dir/depend:
	cd /home/yorkin/CLionProjects/evoBasic/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yorkin/CLionProjects/evoBasic /home/yorkin/CLionProjects/evoBasic /home/yorkin/CLionProjects/evoBasic/cmake-build-debug /home/yorkin/CLionProjects/evoBasic/cmake-build-debug /home/yorkin/CLionProjects/evoBasic/cmake-build-debug/CMakeFiles/evoBasic.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/evoBasic.dir/depend

