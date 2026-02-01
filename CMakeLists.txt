cmake_minimum_required(VERSION 3.16)
project(fenlason-hack VERSION 0.1.0 LANGUAGES C)

# --- Build type ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Find ncurses (provides termcap compatibility) ---
find_package(Curses REQUIRED)
message(STATUS "Using ncurses: ${CURSES_LIBRARIES}")

# =========================================================================
# Root version — Jay's "export hack" as described in READ_ME
# =========================================================================
#
# Two binaries, per original design:
#   hack-root  — the game
#   mklev-root — level generator, exec'd by hack at runtime
#
# READ_ME: "Because of the peculiar restraints on our system, I make mklev
# a separate procedure execd by hack when needed."

set(HACK_ROOT_SOURCES
    src/root/hack.c
    src/root/hack.do.c
    src/root/hack.do1.c
    src/root/hack.lev.c
    src/root/hack.main.c
    src/root/hack.mon.c
    src/root/hack.pri.c
    src/root/rnd.c
)

add_executable(hack-root ${HACK_ROOT_SOURCES})

target_include_directories(hack-root PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/root
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURSES_INCLUDE_DIR}
)

# READ_ME step 6: "modify hack.main.c to use the new directory"
# READ_ME step 7b: compile -UMAGIC to disable gid==42 magic mode check
target_compile_definitions(hack-root PRIVATE
    HACKDIR="${CMAKE_BINARY_DIR}/hackdir"
)

# K&R vintage code: suppress warnings, don't break the build
target_compile_options(hack-root PRIVATE
    -Wall
    -Wno-deprecated-non-prototype
    -Wno-parentheses
    -Wno-unused-variable
    -Wno-implicit-int
    -Wno-implicit-function-declaration
    -Wno-return-type
    -Wno-int-conversion
    -Wno-incompatible-pointer-types
    -Wno-misleading-indentation
    -Wno-error=return-mismatch
    $<$<C_COMPILER_ID:Clang>:-Wno-knr-promoted-parameter>
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

target_link_libraries(hack-root ${CURSES_LIBRARIES} crypt) # crypt: domagic() uses crypt() for wizard password

# mklev — standalone level generator
add_executable(mklev-root
    src/root/mklev.c
    src/root/rnd.c
)

target_include_directories(mklev-root PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/root
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(mklev-root PRIVATE
    HACKDIR="${CMAKE_BINARY_DIR}/hackdir"
    MKLEV_BINARY  # mklev has its own savelev()/mkobj() with different signatures
)

target_compile_options(mklev-root PRIVATE
    -Wall
    -Wno-deprecated-non-prototype
    -Wno-implicit-int
    -Wno-implicit-function-declaration
    -Wno-return-type
    -Wno-int-conversion
    -Wno-incompatible-pointer-types
    -Wno-error=return-mismatch
    $<$<C_COMPILER_ID:Clang>:-Wno-knr-promoted-parameter>
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

# =========================================================================
# hackdir setup — automates READ_ME steps 2-5
# =========================================================================
#
# READ_ME step 2: create a hack directory
# READ_ME step 3: make the subdirectory save
# READ_ME step 4: make the directory 700 mode
# READ_ME step 5: create perm (0 length regular file)

add_custom_target(setup_hackdir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/hackdir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/hackdir/save
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/hackdir/perm
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/hackdir/record

    # Copy game data files from original
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/original/news
        ${CMAKE_BINARY_DIR}/hackdir/news
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/original/moves
        ${CMAKE_BINARY_DIR}/hackdir/moves

    COMMENT "Setting up hackdir (READ_ME steps 2-5)"
    VERBATIM
)

# Put mklev where hack can find it — hack.lev.c exec's "./mklev"
add_custom_command(TARGET mklev-root POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:mklev-root>
        ${CMAKE_BINARY_DIR}/mklev
    COMMENT "Copying mklev to build root (hack exec's ./mklev)"
)

add_dependencies(hack-root setup_hackdir mklev-root)
