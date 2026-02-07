cmake_minimum_required(VERSION 3.16)
project(fenlason-hack VERSION 0.1.0 LANGUAGES C)

# --- Build type ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Optional override so packaged builds can use a relative hackdir.
set(HACKDIR_OVERRIDE "" CACHE STRING "Override compiled-in HACKDIR (default: build dir)")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Find ncurses (provides termcap compatibility) ---
find_package(Curses REQUIRED)
message(STATUS "Using ncurses: ${CURSES_LIBRARIES}")

# --- Find crypt (optional on musl; required on glibc) ---
find_library(CRYPT_LIB crypt)
if(CRYPT_LIB)
    message(STATUS "Using crypt: ${CRYPT_LIB}")
endif()

# =========================================================================
# Root version — Jay's "export hack" as described in READ_ME
# =========================================================================
#
# Two binaries, per original design:
#   hack  — the game
#   mklev — level generator, exec'd by hack at runtime
#
# READ_ME: "Because of the peculiar restraints on our system, I make mklev
# a separate procedure execd by hack when needed."

set(HACK_ROOT_SOURCES
    src/root/hack.c
    src/root/hack.do.c
    src/root/hack.do1.c
    src/root/hack.lev.c
    src/root/hack.lock.c
    src/root/hack.main.c
    src/root/hack.mon.c
    src/root/hack.pri.c
    src/root/hack.save.c
    src/root/rnd.c
)

add_executable(hack ${HACK_ROOT_SOURCES})

target_include_directories(hack PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/root
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURSES_INCLUDE_DIR}
)

# READ_ME step 6: "modify hack.main.c to use the new directory"
# READ_ME step 7b: compile -UMAGIC to disable gid==42 magic mode check
if(HACKDIR_OVERRIDE)
    set(_HACKDIR_VALUE "${HACKDIR_OVERRIDE}")
else()
    set(_HACKDIR_VALUE "${CMAKE_BINARY_DIR}/hackdir")
endif()

target_compile_definitions(hack PRIVATE
    HACKDIR="${_HACKDIR_VALUE}"
    VTONL  # Modern: force ANSI cursor control to bypass termcap quirks
)

# K&R vintage code: remaining suppressions need proper headers/prototypes to remove
target_compile_options(hack PRIVATE
    -Wall
    -Wno-implicit-int              # K&R 'register x' without type in unconverted files
    -Wno-implicit-function-declaration  # cross-file calls lacking prototypes in hack.h
    -Wno-int-conversion            # alloc() returns char* used as struct pointers
    -Wno-incompatible-pointer-types # g_at() generic pointer casts
    -fno-pie  # Modern: keep stable data addresses between hack and mklev
    $<$<C_COMPILER_ID:Clang>:-Wno-deprecated-non-prototype>
    $<$<C_COMPILER_ID:Clang>:-Wno-knr-promoted-parameter>
    $<$<C_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

target_link_options(hack PRIVATE
    -no-pie  # Modern: keep stable data addresses between hack and mklev
)

target_link_libraries(hack ${CURSES_LIBRARIES})
# Makes CRYPT_LIB linking conditional - only links it if found. This supports building on 
# platforms such as macOS where crypt() is part of the standard C library, so no separate library is needed.
if(CRYPT_LIB)
    target_link_libraries(hack ${CRYPT_LIB}) # crypt: domagic() uses crypt() for wizard password
endif()

# mklev — standalone level generator
add_executable(mklev
    src/root/mklev.c
    src/root/rnd.c
)

target_include_directories(mklev PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/root
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(mklev PRIVATE
    HACKDIR="${_HACKDIR_VALUE}"
    MKLEV_BINARY  # mklev has its own savelev()/mkobj() with different signatures
)
# Suppress diagnostics triggered by original K&R-era code patterns.
# Future work may involve auditing these warnings to better document
# historical assumptions and portability constraints, not to alter behavior.

target_compile_options(mklev PRIVATE
    -Wall
    -Wno-implicit-int
    -Wno-implicit-function-declaration
    -Wno-int-conversion
    -Wno-incompatible-pointer-types
    -fno-pie  # Modern: keep stable data addresses between hack and mklev
    $<$<C_COMPILER_ID:Clang>:-Wno-deprecated-non-prototype>
    $<$<C_COMPILER_ID:Clang>:-Wno-knr-promoted-parameter>
    $<$<C_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

target_link_options(mklev PRIVATE
    -no-pie  # Modern: keep stable data addresses between hack and mklev
)

# =========================================================================
# hackdir setup — automates READ_ME steps 2-5
# =========================================================================
#
# READ_ME step 2: create a hack directory
# READ_ME step 3: make the subdirectory save
# READ_ME step 4: make the directory 700 mode
# READ_ME step 5: create perm (0 length regular file)

set(_HACKDIR_PATH "${CMAKE_BINARY_DIR}/hackdir")
file(MAKE_DIRECTORY "${_HACKDIR_PATH}")
file(MAKE_DIRECTORY "${_HACKDIR_PATH}/save")
file(TOUCH "${_HACKDIR_PATH}/perm")
foreach(name IN ITEMS record news moves)
    set(_src "${CMAKE_CURRENT_SOURCE_DIR}/original/${name}")
    set(_dst "${_HACKDIR_PATH}/${name}")
    file(REMOVE_RECURSE "${_dst}")
    if(EXISTS "${_src}")
        file(COPY "${_src}" DESTINATION "${_HACKDIR_PATH}")
    else()
        file(TOUCH "${_dst}")
    endif()
endforeach()

add_custom_target(setup_hackdir ALL
    COMMENT "Setting up hackdir (READ_ME steps 2-5)"
    VERBATIM
)

# Put mklev where hack can find it — hack.lev.c exec's "./mklev"
add_custom_command(TARGET mklev POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:mklev>
        ${CMAKE_BINARY_DIR}/hackdir/mklev
    COMMENT "Copying mklev to build root and hackdir (hack exec's ./mklev)"
)

add_dependencies(hack setup_hackdir mklev)
