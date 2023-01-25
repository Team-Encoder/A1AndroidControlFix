#
# Copyright 2009 Cedric Priscal
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License. 
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
APP_ABI 			:= armeabi armeabi-v7a
TARGET_PLATFORM := android-8
LOCAL_MODULE    := serial_port
LOCAL_SRC_FILES := SerialPort.c
LOCAL_LDLIBS    := -llog
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
APP_ABI 			:= armeabi armeabi-v7a
TARGET_PLATFORM 	:= android-25
LOCAL_MODULE 		:=controlfix
LOCAL_SRC_FILES 	:= controlfix.cpp uinput_gamepad.cpp
LOCAL_LDLIBS    += -llog
LOCAL_CPPFLAGS  += -std=c++11

#all: $(LOCAL_MODULE)
#	 $(shell (mv libs/$(TARGET_ARCH_ABI)/controlfix < libs/$(TARGET_ARCH_ABI)/libcontrolfix.so))

include $(BUILD_EXECUTABLE)
all: $(LOCAL_MODULE)
#$(shell cmd /c fix_lib.bat)
#$(shell cmd /c move libs\$(TARGET_ARCH_ABI)\controlfix libs\$(TARGET_ARCH_ABI)\libcontrolfix.so)
#$(shell cmd /c move build\intermediates\ndkBuild\release\obj\local\$(TARGET_ARCH_ABI)\controlfix build\intermediates\ndkBuild\release\obj\local\$(TARGET_ARCH_ABI)\libcontrolfix.so)
#$(shell cmd /c move build\intermediates\stripped_native_libs\release\out\lib\$(TARGET_ARCH_ABI)\controlfix build\intermediates\stripped_native_libs\release\out\lib\$(TARGET_ARCH_ABI)\libcontrolfix.so)
#$(shell cmd /c move build\intermediates\merged_native_libs\release\out\lib\$(TARGET_ARCH_ABI)\controlfix build\intermediates\merged_native_libs\release\out\lib\$(TARGET_ARCH_ABI)\libcontrolfix.so)


#$(shell cmd /c cd > testtest.txt)