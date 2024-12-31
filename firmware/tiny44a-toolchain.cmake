set(CMAKE_C_COMPILER "avr-gcc")
set(CMAKE_C_OBJCOPY "avr-objcopy")
set(CMAKE_C_OBJDUMP "avr-objdump")

set(MCU attiny44a)
set(F_CPU 8000000)

set(AVRDUDE "avrdude")
set(AVRDUDE_TRG "t44a")
set(AVRDUDE_PRG "atmelice_isp")

# compiler flags
set(C_MCU "-mmcu=${MCU}")
set(C_STD "-std=gnu99")
set(C_WARN "-Wall -Wextra -Wshadow -Wdouble-promotion -fno-common")
set(C_OPT "-Os -ffunction-sections -fdata-sections -Wl,--gc-sections -fpack-struct -fshort-enums -mcall-prologues -flto")
set(C_DEBUG "-Og -ggdb -gdwarf-3")
set(C_DIAG "-fdiagnostics-color=always -Wl,-Map=output.map -fcallgraph-info=su -fstack-usage -Wstack-usage=20")

set(CMAKE_C_FLAGS_RELEASE "${C_MCU} ${C_STD} ${C_WARN} ${C_OPT} ${C_DIAG}")
set(CMAKE_C_FLAGS_DEBUG "${C_MCU} ${C_STD} ${C_WARN} ${C_DEBUG} ${C_DIAG}")

add_compile_definitions(F_CPU=${F_CPU})

