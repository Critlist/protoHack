# Keep the original generator in its own module, just as native hack runs
# a separate process. Each invocation gets fresh globals and its own heap.
set(_wasm "${CMAKE_CURRENT_SOURCE_DIR}/wasm")
foreach(_target hack mklev)
    target_include_directories(${_target} PRIVATE "${_wasm}")
    target_sources(${_target} PRIVATE "${_wasm}/identity.c")
endforeach()
target_sources(hack PRIVATE "${_wasm}/browser.c")
target_sources(mklev PRIVATE "${_wasm}/generator.c")
target_link_options(mklev PRIVATE
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXIT_RUNTIME=1
    -sALLOW_MEMORY_GROWTH=1 -sFORCE_FILESYSTEM=1
    -sEXPORTED_RUNTIME_METHODS=FS)
set_target_properties(mklev PROPERTIES SUFFIX ".js")
set(UTW_TITLE "protoHack — Fenlason Hack (1982)")
set(UTW_COLS 80)
set(UTW_ROWS 24)
set(UTW_PERSIST ON)
set(UTW_PERSIST_DIR "/protohack-save")
set(UTW_PRELOAD_DIR "${CMAKE_BINARY_DIR}/hackdir")
set(UTW_PTY_JS "${_wasm}/emscripten-pty.js")
set(UTW_PRE_JS_EXTRA "${_wasm}/pre-protohack.js")
set(UTW_WEB_FILES "${CMAKE_BINARY_DIR}/mklev.js" "${CMAKE_BINARY_DIR}/mklev.wasm")
include("${_wasm}/utw/wasm/utw.cmake")
utw_enable(hack)
add_dependencies(hack_web mklev)
