import os

# toolchains options
ARCH='arm'
CPU='cortex-m33'
CROSS_TOOL='gcc'

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'

RTT_ROOT = os.path.abspath('../../../../')
SDK_ROOT = os.path.abspath('../')

# Ensure build directory exists
BUILD_DIR = os.path.join(SDK_ROOT, 'application', 'build')
BUILD_OBJS_DIR = os.path.join(SDK_ROOT, 'application', 'build', 'objects')

os.makedirs(BUILD_OBJS_DIR, exist_ok=True)

TARGET_NAME = os.path.join(BUILD_DIR, 'encortec-application')

# toolchains
PREFIX = 'arm-none-eabi-'
CC = PREFIX + 'gcc'
AS = PREFIX + 'gcc'
AR = PREFIX + 'ar'
CXX = PREFIX + 'g++'
LINK = PREFIX + 'gcc'
TARGET_EXT = 'elf'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'

DEVICE = ' -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections'
CFLAGS = DEVICE + ' -MMD -std=gnu11 -Dgcc'
AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
LFLAGS = DEVICE + f' -Wl,--gc-sections,-Map={TARGET_NAME}.map,-cref,-u,main -T board/linker_scripts/link.lds'

CPATH = ''
LPATH = ''

if BUILD == 'debug':
    CFLAGS += ' -O0 -gdwarf-2 -g'
    AFLAGS += ' -gdwarf-2'
else:
    CFLAGS += ' -O2'

CXXFLAGS = CFLAGS 

POST_ACTION = OBJCPY + f' -O binary {TARGET_NAME}.elf {TARGET_NAME}.bin\n' + SIZE + f' {TARGET_NAME}.elf \n'
