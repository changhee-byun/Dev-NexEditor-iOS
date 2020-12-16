#/*
# * Copyright (C) 2010 The Android Open Source Project
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# */

_NDK_VERSION_KITKAT_ := android-19
_NDK_VERSION_JELLY_BEAN_MR2_ := android-18
_NDK_VERSION_JELLY_BEAN_MR1_ := android-17
_NDK_VERSION_JELLYBEAN_ := android-16
_NDK_VERSION_ICE_CREAM_SANDWICH_MR1_ := android-15
_NDK_VERSION_ICE_CREAM_SANDWICH_ := android-14
_NDK_VERSION_HONEYCOMB_MR2_ := android-13
_NDK_VERSION_HONEYCOMB_MR1_ := android-12
_NDK_VERSION_GINGERBREAD_ := android-9
_NDK_VERSION_FROYO_ := android-8

ifeq ($(_NDK_VERSION_KITKAT_), $(findstring $(_NDK_VERSION_KITKAT_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := KITKAT
endif

ifeq ($(_NDK_VERSION_JELLY_BEAN_MR2_), $(findstring $(_NDK_VERSION_JELLY_BEAN_MR2_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := JELLYBEAN_PLUS_422
endif

ifeq ($(_NDK_VERSION_JELLY_BEAN_MR1_), $(findstring $(_NDK_VERSION_JELLY_BEAN_MR1_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := JELLYBEAN_PLUS
endif

ifeq ($(_NDK_VERSION_JELLYBEAN_), $(findstring $(_NDK_VERSION_JELLYBEAN_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := JELLYBEAN
endif

ifeq ($(_NDK_VERSION_ICE_CREAM_SANDWICH_MR1_), $(findstring $(_NDK_VERSION_ICE_CREAM_SANDWICH_MR1_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := ICE_CREAM_SANDWICH
endif

ifeq ($(_NDK_VERSION_ICE_CREAM_SANDWICH_), $(findstring $(_NDK_VERSION_ICE_CREAM_SANDWICH_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := ICE_CREAM_SANDWICH
endif

ifeq ($(_NDK_VERSION_HONEYCOMB_MR2_), $(findstring $(_NDK_VERSION_HONEYCOMB_MR2_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := HONEYCOMB
endif

ifeq ($(_NDK_VERSION_HONEYCOMB_MR1_), $(findstring $(_NDK_VERSION_HONEYCOMB_MR1_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := HONEYCOMB
endif


ifeq ($(_NDK_VERSION_GINGERBREAD_), $(findstring $(_NDK_VERSION_GINGERBREAD_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := GINGERBREAD
endif

ifeq ($(_NDK_VERSION_FROYO_), $(findstring $(_NDK_VERSION_FROYO_), $(TARGET_PLATFORM)))
_NEX_ANDROID_PLATFORM_CODENAME_ := FROYO
endif


$(info [nex_android_version.mk 70] TARGET VERSION: $(_NEX_ANDROID_PLATFORM_CODENAME_))