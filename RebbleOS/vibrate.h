#pragma once
/* 
 * This file is part of the RebbleOS distribution.
 *   (https://github.com/pebble-dev)
 * Copyright (c) 2017 Barry Carter <barry.carter@gmail.com>.
 * 
 * RebbleOS is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU Lesser General Public License as   
 * published by the Free Software Foundation, version 3.
 *
 * RebbleOS is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

typedef enum
{
   VIBRATE_CMD_STOP,
   VIBRATE_CMD_PATTERN_1,
   VIBRATE_CMD_PATTERN_2,
   VIBRATE_CMD_MAX,        // not an actual command; used as a maximum index or sentinel value
} VibrateCmd_t;

// #define VIBRATE_CMD_PATTERN_1 1
// #define VIBRATE_CMD_PATTERN_2 2
// #define VIBRATE_CMD_STOP      3

void vibrate_init(void);
