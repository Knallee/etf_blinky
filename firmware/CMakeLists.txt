cmake_minimum_required(VERSION 3.20)

project(
    etf_blinky
    VERSION 2.0
    LANGUAGES C ASM
    HOMEPAGE_URL http://add.me
)


option(BUILD_TESTS "Build test suite for host target" OFF)

if (BUILD_TESTS)
    ### Build tests for host ###
    set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -pedantic -fdiagnostics-color=always -Og -std=gnu99 -fsanitize=thread -DTEST")
    add_executable(test tests/test_scheduler.c src/scheduler.c)
    target_include_directories(test PUBLIC src/)

else()

    ### Build firmware for target ###

    # create compile commands database and make a symbolic link in top dir
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    file(CREATE_LINK
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        SYMBOLIC
    )

    # Not the best practice to use this, but this is just a small project (tm)
    aux_source_directory(src SRC)

    add_executable(${PROJECT_NAME} ${SRC})


    ### Extras ###

    if (NOT WIN32)
        string(ASCII 27 Esc)
        set(ColourReset "${Esc}[m")
        set(Green       "${Esc}[32m")
    endif()

    message(STATUS "${Green}MCU:        ${MCU}${ColourReset}")
    message(STATUS "${Green}F_CPU:      ${F_CPU}${ColourReset}")

    # show utilized space
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_C_OBJDUMP}
        ARGS -Pmem-usage ${PROJECT_NAME}
    )

    # generate ihex
    add_custom_command(OUTPUT hexfile
        COMMAND ${CMAKE_C_OBJCOPY}
        ARGS -O ihex ${PROJECT_NAME} ${PROJECT_NAME}.hex
        DEPENDS ${PROJECT_NAME}
    )
    add_custom_target(hex ALL DEPENDS hexfile)

    # generate bin
    add_custom_command(OUTPUT binfile
        COMMAND ${CMAKE_C_OBJCOPY}
        ARGS -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
        DEPENDS ${PROJECT_NAME}
    )
    add_custom_target(bin ALL DEPENDS binfile)

    add_custom_target(flash
        COMMAND ${AVRDUDE} -c ${AVRDUDE_PRG} -p ${AVRDUDE_TRG} -B 8 -U flash:w:${PROJECT_NAME}.hex
        USES_TERMINAL
    )

    add_custom_target(fuse
        COMMAND ${AVRDUDE} -c ${AVRDUDE_PRG} -p ${AVRDUDE_TRG} -B 8 -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
        USES_TERMINAL
    )

endif()
