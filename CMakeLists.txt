cmake_minimum_required(VERSION 3.10)
project(IndieStartr C)
#
#set(CMAKE_C_STANDARD 11)

include_directories(src
        /u/ZX/z88dk/include/_DEVELOPMENT/sccz80
        /u/ZX/platotermzx/zxnext_layer2/include
        )

add_executable(PLATOTerm
        src/main

./src/connect.c
./src/connect.h
./src/cursor_sprite.h
./src/font.c
./src/help.c
./src/help.h
./src/io.c
./src/io.h
./src/key.h
./src/keyboard.c
./src/keyboard.h
./src/main.c
./src/plato_key.h
./src/protocol.c
./src/protocol.h
./src/scale.c
./src/screen.c
./src/screen.h
./src/splash.h
./src/splash_cpc.h
./src/splash_einstein.h
./src/splash_memotech.h
./src/splash_microbee.h
./src/splash_msx.h
./src/splash_specnext.h
./src/splash_spectrum.h
./src/splash_svi.h
./src/terminal.c
./src/terminal.h
./src/touch.c
./src/touch.h
)
