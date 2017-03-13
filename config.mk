# Common source.
CFLAGS_all += -IHardware
CFLAGS_all += -IFreeRTOS/include
CFLAGS_all += -IFreeRTOS/portable/GCC/ARM_CM4F
CFLAGS_all += -IPlatform/CMSIS/Include
CFLAGS_all += -ILibraries/UGUI
CFLAGS_all += -ILibraries/neographics/src/
CFLAGS_all += -ILibraries/neographics/src/draw_command
CFLAGS_all += -ILibraries/neographics/src/path
CFLAGS_all += -ILibraries/neographics/src/primitives
CFLAGS_all += -ILibraries/neographics/src/types
CFLAGS_all += -ILibraries/neographics/src/fonts
CFLAGS_all += -ILibraries/neographics/src/text
CFLAGS_all += -Ilib/minilib/inc
CFLAGS_all += -IWatchfaces
CFLAGS_all += -IApps
CFLAGS_all += -IApps/System
CFLAGS_all += -IConfig
CFLAGS_all += -IRebbleOS
CFLAGS_all += -IRebbleOS/Gui
CFLAGS_all += -IlibRebbleOS
CFLAGS_all += -IlibRebbleOS/ui
CFLAGS_all += -IlibRebbleOS/ui/layer
CFLAGS_all += -IlibRebbleOS/ui/animation
CFLAGS_all += -IlibRebbleOS/input
CFLAGS_all += -IlibRebbleOS/graphics
CFLAGS_all += -IlibRebbleOS/event

# XXX: nostdinc
CFLAGS_all += -O0 -ggdb -Wall -ffunction-sections -fdata-sections -mthumb -mlittle-endian -finline-functions -std=gnu99 -falign-functions=16

LDFLAGS_all += -nostartfiles -nostdlib
LIBS_all += -lgcc

SRCS_all += FreeRTOS/croutine.c
SRCS_all += FreeRTOS/event_groups.c
SRCS_all += FreeRTOS/list.c
SRCS_all += FreeRTOS/queue.c
SRCS_all += FreeRTOS/tasks.c
SRCS_all += FreeRTOS/timers.c
SRCS_all += FreeRTOS/portable/GCC/ARM_CM4F/port.c
SRCS_all += FreeRTOS/portable/MemMang/heap_4.c

SRCS_all += lib/minilib/minilib.c
SRCS_all += lib/minilib/sbrk.c
SRCS_all += lib/minilib/dprint.c
SRCS_all += lib/minilib/rand.c

SRCS_all += Hardware/stdarg.c

SRCS_all += Libraries/UGUI/ugui.c

SRCS_all += Libraries/neographics/src/common.c
SRCS_all += Libraries/neographics/src/context.c
SRCS_all += Libraries/neographics/src/draw_command/draw_command.c
SRCS_all += Libraries/neographics/src/fonts/fonts.c
SRCS_all += Libraries/neographics/src/path/path.c
SRCS_all += Libraries/neographics/src/primitives/circle.c
SRCS_all += Libraries/neographics/src/primitives/line.c
SRCS_all += Libraries/neographics/src/primitives/rect.c
SRCS_all += Libraries/neographics/src/text/text.c

SRCS_all += RebbleOS/ambient.c
SRCS_all += RebbleOS/appmanager.c
SRCS_all += RebbleOS/backlight.c
SRCS_all += RebbleOS/buttons.c
SRCS_all += RebbleOS/display.c
SRCS_all += RebbleOS/debug.c
SRCS_all += RebbleOS/gyro.c
SRCS_all += RebbleOS/main.c
SRCS_all += RebbleOS/power.c
SRCS_all += RebbleOS/rebbleos.c
SRCS_all += RebbleOS/smartstrap.c
SRCS_all += RebbleOS/rebble_time.c
SRCS_all += RebbleOS/rebble_memory.c
SRCS_all += RebbleOS/vibrate.c

SRCS_all += libRebbleOS/librebble.c
SRCS_all += libRebbleOS/math_sin.c
SRCS_all += libRebbleOS/ui/layer/layer.c
SRCS_all += libRebbleOS/ui/layer/scroll_layer.c
SRCS_all += libRebbleOS/ui/layer/text_layer.c
SRCS_all += libRebbleOS/ui/window.c
SRCS_all += libRebbleOS/event/tick_timer_service.c

SRCS_all += Watchfaces/simple.c
SRCS_all += Watchfaces/nivz.c

SRCS_all += Apps/System/systemapp.c
SRCS_all += Apps/System/menu.c

SRCS_all += RebbleOS/Gui/gui.c
SRCS_all += RebbleOS/Gui/neographics.c

include hw/chip/stm32f4xx/config.mk
include hw/chip/stm32f2xx/config.mk
include hw/platform/snowy/config.mk
include hw/platform/tintin/config.mk
