# Copyright (C) 2014 Google Inc.
#
# This file is part of ycmd.
#
# ycmd is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ycmd is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ycmd.  If not, see <http://www.gnu.org/licenses/>.

import os
import ycm_core

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
flags = [
    '-mcpu=cortex-m4',    
    '-mthumb',
    '-DUSE_HAL_DRIVER',
    '-DSTM32F407xx',
    '-Wall',
    '-fdata-sections',
    '-ffunction-sections',
    '-g',
    '-gdwarf-2',
    '-Werror',
    '-target=arm-arm-none-eabi-gcc',
    '-ICore/Inc',
    '-IDrivers/STM32F4xx_HAL_Driver/Inc',
    '-IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy',
    '-IDrivers/CMSIS/Device/ST/STM32F4xx/Include',
    '-IDrivers/CMSIS/Include',
    '-IUSB_DEVICE/App',
    '-IUSB_DEVICE/Target',
    '-IMiddlewares/Third_Party/FatFs/src',
    '-IMiddlewares/ST/STM32_USB_Host_Library/Core/Inc',
    '-IMiddlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc',
    '-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc',
    '-IMiddlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc',
    '-Imyware',
# THIS IS IMPORTANT! Without a "-std=<something>" flag, clang won't know which
# language to use when compiling headers. So it will guess. Badly. So C++
# headers will be compiled as C headers. You don't want that so ALWAYS specify
# a "-std=<something>".
# For a C project, you would set this to something like 'c99' instead of
# 'c++11'.
	'-std=c99',
# ...and the same thing goes for the magic -x option which specifies the
# language that the files to be compiled are written in. This is mostly
# relevant for c++ headers.
# For a C project, you would set this to 'c' instead of 'c++'.
	'-x',
        'c',
        '-isystem',
        '/usr/lib/arm-none-eabi/include',
	]

SOURCE_EXTENSIONS = [
    '.cpp',
    '.cxx',
    '.cc',
    '.c',
]
def FlagsForFile( filename, **kwargs ):
	    return {
		        'flags': flags,
		        'do_cache': True
		    }

