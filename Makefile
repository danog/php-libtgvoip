# Copyright 2016-2017 Daniil Gentili
# (https://daniil.it)
# This file is part of php-libtgvoip.
# php-libtgvoip is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
# The PWRTelegram API is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero General Public License for more details.
# You should have received a copy of the GNU General Public License along with php-libtgvoip.
# If not, see <http://www.gnu.org/licenses/>.

include $(CLEAR_VARS)

DEFINES			=	-DTGVOIP_USE_CALLBACK_AUDIO_IO -DWEBRTC_POSIX -DTGVOIP_USE_DESKTOP_DSP -DWEBRTC_APM_DEBUG_DUMP=0
INCLUDES		=	-Ilibtgvoip -I/usr/include/opus -Ilibtgvoip/webrtc_dsp -pthread
LDINCLUDES		=	
CXXFLAGS		=	${INCLUDES} -O3 -Wall -c -std=c++11 -fpic -finline-functions -ffast-math -fno-strict-aliasing -DUSE_KISS_FFT -DFIXED_POINT -DPHP_LIBTGVOIP -DWEBRTC_POSIX -DTGVOIP_USE_DESKTOP_DSP -DWEBRTC_APM_DEBUG_DUMP=0 -g -DTGVOIP_USE_CXX11_LIB -DTGVOIP_OTHER ${DEFINES} -o
CFLAGS			=	${INCLUDES} -O3 -DUSE_KISS_FFT -fexceptions -fpic ${DEFINES} -g

LFLAGS		=	-shared ${LDINCLUDES} -lphpcpp -lopus -lpthread -lstdc++ -lcrypto -lssl -ldl -pthread -Wl,-z,defs

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    CXXFLAGS += -mfloat-abi=softfp -mfpu=neon
else
    ifeq ($(TARGET_ARCH_ABI),armeabi)
    	CXXFLAGS += -mfloat-abi=softfp -mfpu=neon
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
MKDIR					=	mkdir -p


#
#	All source files are simply all *.cpp files found in the current directory
#
#	A builtin Makefile macro is used to scan the current directory and find 
#	all source files. The object files are all compiled versions of the source
#	file, with the .cpp extension being replaced by .o.
#


SOURCES				=	$(wildcard *.cpp ./libtgvoip/VoIPController.cpp ./libtgvoip/Buffers.cpp ./libtgvoip/CongestionControl.cpp ./libtgvoip/EchoCanceller.cpp ./libtgvoip/JitterBuffer.cpp ./libtgvoip/logging.cpp ./libtgvoip/MediaStreamItf.cpp ./libtgvoip/MessageThread.cpp ./libtgvoip/NetworkSocket.cpp ./libtgvoip/OpusDecoder.cpp ./libtgvoip/OpusEncoder.cpp ./libtgvoip/PacketReassembler.cpp ./libtgvoip/VoIPGroupController.cpp ./libtgvoip/VoIPServerConfig.cpp ./libtgvoip/audio/AudioInput.cpp ./libtgvoip/audio/AudioOutput.cpp ./libtgvoip/audio/AudioIOCallback.cpp ./libtgvoip/audio/AudioIO.cpp ./libtgvoip/audio/Resampler.cpp ./libtgvoip/os/posix/NetworkSocketPosix.cpp ./libtgvoip/webrtc_dsp/webrtc/base/checks.cc ./libtgvoip/webrtc_dsp/webrtc/base/stringutils.cc ./libtgvoip/webrtc_dsp/webrtc/common_audio/audio_util.cc ./libtgvoip/webrtc_dsp/webrtc/common_audio/channel_buffer.cc ./libtgvoip/webrtc_dsp/webrtc/common_audio/fft4g.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/ring_buffer.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/auto_corr_to_refl_coef.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/auto_correlation.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/complex_bit_reverse.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/complex_fft.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/copy_set_operations.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/cross_correlation.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/division_operations.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/dot_product_with_scale.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/downsample_fast.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/energy.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/filter_ar.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/filter_ar_fast_q12.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/filter_ma_fast_q12.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/get_hanning_window.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/get_scaling_square.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/ilbc_specific_functions.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/levinson_durbin.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/lpc_to_refl_coef.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/min_max_operations.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/randomization_functions.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/real_fft.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/refl_coef_to_lpc.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/resample.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/resample_48khz.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/resample_by_2.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/resample_by_2_internal.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/resample_fractional.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/spl_init.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/spl_inl.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/spl_sqrt.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/spl_sqrt_floor.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/splitting_filter_impl.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/sqrt_of_one_minus_x_squared.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/signal_processing/vector_scaling_operations.c ./libtgvoip/webrtc_dsp/webrtc/common_audio/sparse_fir_filter.cc ./libtgvoip/webrtc_dsp/webrtc/common_audio/wav_file.cc ./libtgvoip/webrtc_dsp/webrtc/common_audio/wav_header.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aec/aec_core.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aec/aec_core_sse2.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aec/aec_resampler.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aec/echo_cancellation.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aecm/aecm_core.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aecm/aecm_core_c.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/aecm/echo_control_mobile.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/agc/legacy/analog_agc.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/agc/legacy/digital_agc.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/logging/apm_data_dumper.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/ns/noise_suppression.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/ns/noise_suppression_x.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/ns/ns_core.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/ns/nsx_core.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/ns/nsx_core_c.c ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/splitting_filter.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/three_band_filter_bank.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/utility/block_mean_calculator.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/utility/delay_estimator.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/utility/delay_estimator_wrapper.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/utility/ooura_fft.cc ./libtgvoip/webrtc_dsp/webrtc/modules/audio_processing/utility/ooura_fft_sse2.cc ./libtgvoip/webrtc_dsp/webrtc/system_wrappers/source/cpu_features.cc)
OBJECTS_CPP			=	$(SOURCES:%.cpp=%.o)
OBJECTS_CC			=	$(OBJECTS_CPP:%.cc=%.o)
OBJECTS				=	$(OBJECTS_CC:%.c=%.o)

#
#	From here the build instructions start
#

all:					${OBJECTS} ${EXTENSION}

${EXTENSION}:				${OBJECTS}
					${CXX} -o $@ ${OBJECTS} ${LFLAGS}

${OBJECTS}:


install:
					${CP} ${EXTENSION} ${EXTENSION_DIR}
					${CP} ${INI} ${INI_DIR}


clean:
					${RM} ${EXTENSION} ${OBJECTS}


