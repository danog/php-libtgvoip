# Copyright 2016-2017 Daniil Gentili
# (https://daniil.it)
# This file is part of php-libtgvoip.
# php-libtgvoip is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
# The PWRTelegram API is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero General Public License for more details.
# You should have received a copy of the GNU General Public License along with php-libtgvoip.
# If not, see <http://www.gnu.org/licenses/>.

include $(CLEAR_VARS)


COMPILER_FLAGS		=	-Wall -O3 -I/usr/include/opus -I/usr/include/openssl -Ilibtgvoip/webrtc_dsp -c -std=c++11 -fpic -DANDROID -finline-functions -ffast-math -Os -fno-strict-aliasing -DUSE_KISS_FFT -DFIXED_POINT -DTGVOIP_USE_CUSTOM_CRYPTO -DPHP_LIBTGVOIP -o
#COMPILER_FLAGS      =   -Wall -c -O2 -std=c++11 -fpic -I/usr/include/opus -o
LINKER_FLAGS		=	-shared
LINKER_DEPENDENCIES	=	-lphpcpp -I/usr/include/opus

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    COMPILER_FLAGS += -mfloat-abi=softfp -mfpu=neon
else
    ifeq ($(TARGET_ARCH_ABI),armeabi)
    	COMPILER_FLAGS += -mfloat-abi=softfp -mfpu=neon
    endif
endif



include $(BUILD_STATIC_LIBRARY)
#
#	Name of your extension
#
#	This is the name of your extension. Based on this extension name, the
#	name of the library file (name.so) and the name of the config file (name.ini)
#	are automatically generated
#

NAME				=	libtgvoip


#
#	Php.ini directories
#
#	In the past, PHP used a single php.ini configuration file. Today, most
#	PHP installations use a conf.d directory that holds a set of config files,
#	one for each extension. Use this variable to specify this directory.
#

INI_DIR				=	$(shell php --ini | sed '/Scan for additional .ini files in: /!d;s/Scan for additional .ini files in: //')


#
#	The extension dirs
#
#	This is normally a directory like /usr/lib/php5/20121221 (based on the 
#	PHP version that you use. We make use of the command line 'php-config' 
#	instruction to find out what the extension directory is, you can override
#	this with a different fixed directory
#

EXTENSION_DIR		=	$(shell php-config --extension-dir)


#
#	The name of the extension and the name of the .ini file
#
#	These two variables are based on the name of the extension. We simply add
#	a certain extension to them (.so or .ini)
#

EXTENSION 			=	${NAME}.so
INI 				=	${NAME}.ini


#
#	Compiler
#
#	By default, the GNU C++ compiler is used. If you want to use a different
#	compiler, you can change that here. You can change this for both the 
#	compiler (the program that turns the c++ files into object files) and for
#	the linker (the program that links all object files into the single .so
#	library file. By default, g++ (the GNU C++ compiler) is used for both.
#

COMPILER			=	g++
LINKER				=	g++


#
#	Compiler and linker flags
#
#	This variable holds the flags that are passed to the compiler. By default, 
# 	we include the -O2 flag. This flag tells the compiler to optimize the code, 
#	but it makes debugging more difficult. So if you're debugging your application, 
#	you probably want to remove this -O2 flag. At the same time, you can then 
#	add the -g flag to instruct the compiler to include debug information in
#	the library (but this will make the final libphpcpp.so file much bigger, so
#	you want to leave that flag out on production servers).
#
#	If your extension depends on other libraries (and it does at least depend on
#	one: the PHP-CPP library), you should update the LINKER_DEPENDENCIES variable
#	with a list of all flags that should be passed to the linker.
#



#
#	Command to remove files, copy files and create directories.
#
#	I've never encountered a *nix environment in which these commands do not work. 
#	So you can probably leave this as it is
#

RM					=	rm -f
CP					=	cp -f
MKDIR				=	mkdir -p


#
#	All source files are simply all *.cpp files found in the current directory
#
#	A builtin Makefile macro is used to scan the current directory and find 
#	all source files. The object files are all compiled versions of the source
#	file, with the .cpp extension being replaced by .o.
#

SOURCES				=	$(wildcard *.cpp)
OBJECTS				=	$(SOURCES:%.cpp=%.o)


#
#	From here the build instructions start
#

all:					${OBJECTS} ${EXTENSION}

${EXTENSION}:				${OBJECTS}
					${LINKER} ${LINKER_FLAGS} -o $@ ${OBJECTS} ${LINKER_DEPENDENCIES}

${OBJECTS}:
					${COMPILER} ${COMPILER_FLAGS} $@ ${@:%.o=%.cpp}

install:
					${CP} ${EXTENSION} ${EXTENSION_DIR}
					${CP} ${INI} ${INI_DIR}


clean:
					${RM} ${EXTENSION} ${OBJECTS}


