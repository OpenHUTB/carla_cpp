// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 
// 禁用虚幻4相关的宏
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_INCLUDED_DISABLE_UE4_MACROS_HEADER
#define LIBCARLA_INCLUDED_DISABLE_UE4_MACROS_HEADER

// Unreal/CarlaUE4/Plugins/Carla/Source/Carla/Carla.h
#include "Carla.h"

#ifndef BOOST_ERROR_CODE_HEADER_ONLY
#  define BOOST_ERROR_CODE_HEADER_ONLY
#endif // BOOST_ERROR_CODE_HEADER_ONLY

#ifndef BOOST_NO_EXCEPTIONS
#  error LibCarla should be compiled with -DBOOST_NO_EXCEPTIONS inside UE4.
#endif // BOOST_NO_EXCEPTIONS
#ifndef ASIO_NO_EXCEPTIONS
#  error LibCarla should be compiled with -DASIO_NO_EXCEPTIONS inside UE4.
#endif // ASIO_NO_EXCEPTIONS
#ifndef LIBCARLA_NO_EXCEPTIONS
#  error LibCarla should be compiled with -DLIBCARLA_NO_EXCEPTIONS inside UE4.
#endif // LIBCARLA_NO_EXCEPTIONS

#endif // LIBCARLA_INCLUDED_DISABLE_UE4_MACROS_HEADER

#define LIBCARLA_INCLUDED_FROM_UE4

// NOTE(Andrei): 禁用未定义​​宏生成的警告
// __GNUC__, __GNUC_MINOR__
// MSGPACK_ARCH_AMD64
// DBG, BETA, OFFICIAL_BUILD
// NTDDI_WIN7SP1
// _APISET_RTLSUPPORT_VER
// _APISET_INTERLOCKED_VER
// _APISET_SECURITYBASE_VER
// _WIN32_WINNT_WINTHRESHOLD
// NOTE(Andrei): Macros to detect which compiler is
// http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
// _MSC_VER 是MSVC编译器的内置宏，定义了编译器的版本。
// 在程序中加入_MSC_VER宏可以根据编译器版本让编译器有选择性地编译一段程序。
// _MSC_VER Microsoft 的 C 编译器的版本（MS是Microsoft的简写。MSC 就是 Microsoft 的 C 编译器。VER是Version 的简写）
// MSVC++ 14.2  _MSC_VER == 1920 (Visual Studio 2019 Version 16.0)
#if defined(_MSC_VER)
#  pragma warning(push)  // 将当前的警告状态推入警告状态堆栈中，保存了当前的警告设置，以便可以在稍后的代码中进行还原。
#  pragma warning(disable: 4668 4191 4647)  // 只在vs中禁用警告：确定这些警告是安全的或者是你有意为之的，以避免编译输出中出现大量的无关警告信息。
// 在编译 #pragma warning(pop) 之前的代码时候，4668 4191 4647三类警告将不会出现，具体警告请查看：https://learn.microsoft.com/zh-cn/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4668?view=msvc-160
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wmissing-braces"
#  pragma clang diagnostic ignored "-Wunusable-partial-specialization"
#  pragma clang diagnostic ignored "-Wundef"
#  pragma clang diagnostic ignored "-Wall"
#  pragma clang diagnostic ignored "-Wshadow"
#endif

// #pragma push_macro 将指定的宏压入栈中，相当于暂时存储，以备以后使用；
#pragma push_macro("GET_AI_CONFIG_VAR")
#undef GET_AI_CONFIG_VAR

#pragma push_macro("BT_VLOG")
#undef BT_VLOG

#pragma push_macro("BT_SEARCHLOG")
#undef BT_SEARCHLOG

#pragma push_macro("EQSHEADERLOG")
#undef EQSHEADERLOG

#pragma push_macro("MEM_STAT_UPDATE_WRAPPER")
#undef MEM_STAT_UPDATE_WRAPPER

#pragma push_macro("GET_STRUCT_NAME_CHECKED")
#undef GET_STRUCT_NAME_CHECKED

#pragma push_macro("PRINT_TABLE_ROW")
#undef PRINT_TABLE_ROW

#pragma push_macro("SIGHT_LOG_SEGMENT")
#undef SIGHT_LOG_SEGMENT

#pragma push_macro("SIGHT_LOG_LOCATION")
#undef SIGHT_LOG_LOCATION

#pragma push_macro("ANALYTICS_FLUSH_TRACKING_BEGIN")
#undef ANALYTICS_FLUSH_TRACKING_BEGIN

#pragma push_macro("ANALYTICS_FLUSH_TRACKING_END")
#undef ANALYTICS_FLUSH_TRACKING_END

#pragma push_macro("OCULUS_DEVICE_LOOP")
#undef OCULUS_DEVICE_LOOP

#pragma push_macro("OPENSLES_RETURN_ON_FAIL")
#undef OPENSLES_RETURN_ON_FAIL

#pragma push_macro("OPENSLES_CHECK_ON_FAIL")
#undef OPENSLES_CHECK_ON_FAIL

#pragma push_macro("OPENSLES_LOG_ON_FAIL")
#undef OPENSLES_LOG_ON_FAIL

#pragma push_macro("CASE_ENUM_TO_TEXT")
#undef CASE_ENUM_TO_TEXT

#pragma push_macro("TRACE_BLENDSPACE_PLAYER")
#undef TRACE_BLENDSPACE_PLAYER

#pragma push_macro("SEQUENCER_INSTANCE_PLAYER_TYPE")
#undef SEQUENCER_INSTANCE_PLAYER_TYPE

#pragma push_macro("IMAGE_BRUSH")
#undef IMAGE_BRUSH

#pragma push_macro("BOX_BRUSH")
#undef BOX_BRUSH

#pragma push_macro("BORDER_BRUSH")
#undef BORDER_BRUSH

#pragma push_macro("DEFAULT_FONT")
#undef DEFAULT_FONT

#pragma push_macro("INTERNAL_DECORATOR")
#undef INTERNAL_DECORATOR

#pragma push_macro("LLM_SCOPE_METAL")
#undef LLM_SCOPE_METAL

#pragma push_macro("LLM_PLATFORM_SCOPE_METAL")
#undef LLM_PLATFORM_SCOPE_METAL

#pragma push_macro("METAL_DEBUG_OPTION")
#undef METAL_DEBUG_OPTION

#pragma push_macro("METAL_DEBUG_ONLY")
#undef METAL_DEBUG_ONLY

#pragma push_macro("METAL_DEBUG_LAYER")
#undef METAL_DEBUG_LAYER

#pragma push_macro("METAL_GPUPROFILE")
#undef METAL_GPUPROFILE

#pragma push_macro("UNREAL_TO_METAL_BUFFER_INDEX")
#undef UNREAL_TO_METAL_BUFFER_INDEX

#pragma push_macro("METAL_TO_UNREAL_BUFFER_INDEX")
#undef METAL_TO_UNREAL_BUFFER_INDEX

#pragma push_macro("METAL_FATAL_ERROR")
#undef METAL_FATAL_ERROR

#pragma push_macro("METAL_FATAL_ASSERT")
#undef METAL_FATAL_ASSERT

#pragma push_macro("METAL_IGNORED")
#undef METAL_IGNORED

#pragma push_macro("NOT_SUPPORTED")
#undef NOT_SUPPORTED

#pragma push_macro("METAL_INC_DWORD_STAT_BY")
#undef METAL_INC_DWORD_STAT_BY

#pragma push_macro("CHECK_JNI_RESULT")
#undef CHECK_JNI_RESULT

#pragma push_macro("SET_PRESSED")
#undef SET_PRESSED

#pragma push_macro("ADD_WINDOWS_MESSAGE_STRING")
#undef ADD_WINDOWS_MESSAGE_STRING

#pragma push_macro("ADD_IMN_STRING")
#undef ADD_IMN_STRING

#pragma push_macro("ADD_IMR_STRING")
#undef ADD_IMR_STRING

#pragma push_macro("IsTouchEvent")
#undef IsTouchEvent

#pragma push_macro("ADDTOMAP")
#undef ADDTOMAP

#pragma push_macro("UE_PACKAGEREADER_CORRUPTPACKAGE_WARNING")
#undef UE_PACKAGEREADER_CORRUPTPACKAGE_WARNING

#pragma push_macro("MUTEX_INITIALIZE")
#undef MUTEX_INITIALIZE

#pragma push_macro("MUTEX_DESTROY")
#undef MUTEX_DESTROY

#pragma push_macro("MUTEX_LOCK")
#undef MUTEX_LOCK

#pragma push_macro("MUTEX_UNLOCK")
#undef MUTEX_UNLOCK

#pragma push_macro("SAFE_RELEASE")
#undef SAFE_RELEASE

#pragma push_macro("AUDIO_MIXER_DEBUG_LOG")
#undef AUDIO_MIXER_DEBUG_LOG

#pragma push_macro("AUDIO_PLATFORM_ERROR")
#undef AUDIO_PLATFORM_ERROR

#pragma push_macro("AUDIO_MIXER_CHECK")
#undef AUDIO_MIXER_CHECK

#pragma push_macro("AUDIO_MIXER_CHECK_GAME_THREAD")
#undef AUDIO_MIXER_CHECK_GAME_THREAD

#pragma push_macro("AUDIO_MIXER_CHECK_AUDIO_PLAT_THREAD")
#undef AUDIO_MIXER_CHECK_AUDIO_PLAT_THREAD

#pragma push_macro("DEFINE_AR_COMPONENT_DEBUG_MODE")
#undef DEFINE_AR_COMPONENT_DEBUG_MODE

#pragma push_macro("DEFINE_AR_COMPONENT_VIRTUALS")
#undef DEFINE_AR_COMPONENT_VIRTUALS

#pragma push_macro("DEFINE_AR_SI_DELEGATE_FUNCS")
#undef DEFINE_AR_SI_DELEGATE_FUNCS

#pragma push_macro("DEFINE_AR_BPLIB_DELEGATE_FUNCS")
#undef DEFINE_AR_BPLIB_DELEGATE_FUNCS

#pragma push_macro("DECLARE_AR_SI_DELEGATE_FUNCS")
#undef DECLARE_AR_SI_DELEGATE_FUNCS

#pragma push_macro("DEFINE_AR_DELEGATE_BASE")
#undef DEFINE_AR_DELEGATE_BASE

#pragma push_macro("DEFINE_AR_DELEGATE_ONE_PARAM")
#undef DEFINE_AR_DELEGATE_ONE_PARAM

#pragma push_macro("CHECK_HR")
#undef CHECK_HR

#pragma push_macro("CHECK_HR_DEFAULT")
#undef CHECK_HR_DEFAULT

#pragma push_macro("CHECK_HR_COM")
#undef CHECK_HR_COM

#pragma push_macro("CHECK_HR_VOID")
#undef CHECK_HR_VOID

#pragma push_macro("CHECK_AMF_RET")
#undef CHECK_AMF_RET

#pragma push_macro("CHECK_AMF_NORET")
#undef CHECK_AMF_NORET

#pragma push_macro("CHECK_NV_RES")
#undef CHECK_NV_RES

#pragma push_macro("NV_RESULT")
#undef NV_RESULT

#pragma push_macro("B")
#undef B

#pragma push_macro("AMF_DECLARE_IID")
#undef AMF_DECLARE_IID

#pragma push_macro("AMF_MACRO_STRING2")
#undef AMF_MACRO_STRING2

#pragma push_macro("AMF_MACRO_STRING")
#undef AMF_MACRO_STRING

#pragma push_macro("AMF_TODO")
#undef AMF_TODO

#pragma push_macro("AMF_ALIGN")
#undef AMF_ALIGN

#pragma push_macro("amf_countof")
#undef amf_countof

#pragma push_macro("AMF_MIN")
#undef AMF_MIN

#pragma push_macro("AMF_MAX")
#undef AMF_MAX

#pragma push_macro("AMF_ASSIGN_PROPERTY_DATA")
#undef AMF_ASSIGN_PROPERTY_DATA

#pragma push_macro("AMF_QUERY_INTERFACE")
#undef AMF_QUERY_INTERFACE

#pragma push_macro("AMF_ASSIGN_PROPERTY_INTERFACE")
#undef AMF_ASSIGN_PROPERTY_INTERFACE

#pragma push_macro("AMF_GET_PROPERTY_INTERFACE")
#undef AMF_GET_PROPERTY_INTERFACE

#pragma push_macro("AMF_ASSIGN_PROPERTY_TYPE")
#undef AMF_ASSIGN_PROPERTY_TYPE

#pragma push_macro("AMF_ASSIGN_PROPERTY_INT64")
#undef AMF_ASSIGN_PROPERTY_INT64

#pragma push_macro("AMF_ASSIGN_PROPERTY_DOUBLE")
#undef AMF_ASSIGN_PROPERTY_DOUBLE

#pragma push_macro("AMF_ASSIGN_PROPERTY_BOOL")
#undef AMF_ASSIGN_PROPERTY_BOOL

#pragma push_macro("AMF_ASSIGN_PROPERTY_RECT")
#undef AMF_ASSIGN_PROPERTY_RECT

#pragma push_macro("AMF_ASSIGN_PROPERTY_SIZE")
#undef AMF_ASSIGN_PROPERTY_SIZE

#pragma push_macro("AMF_ASSIGN_PROPERTY_POINT")
#undef AMF_ASSIGN_PROPERTY_POINT

#pragma push_macro("AMF_ASSIGN_PROPERTY_RATE")
#undef AMF_ASSIGN_PROPERTY_RATE

#pragma push_macro("AMF_ASSIGN_PROPERTY_RATIO")
#undef AMF_ASSIGN_PROPERTY_RATIO

#pragma push_macro("AMF_ASSIGN_PROPERTY_COLOR")
#undef AMF_ASSIGN_PROPERTY_COLOR

#pragma push_macro("AMFVariantEmpty")
#undef AMFVariantEmpty

#pragma push_macro("AMFVariantBool")
#undef AMFVariantBool

#pragma push_macro("AMFVariantInt64")
#undef AMFVariantInt64

#pragma push_macro("AMFVariantDouble")
#undef AMFVariantDouble

#pragma push_macro("AMFVariantRect")
#undef AMFVariantRect

#pragma push_macro("AMFVariantSize")
#undef AMFVariantSize

#pragma push_macro("AMFVariantPoint")
#undef AMFVariantPoint

#pragma push_macro("AMFVariantRate")
#undef AMFVariantRate

#pragma push_macro("AMFVariantRatio")
#undef AMFVariantRatio

#pragma push_macro("AMFVariantColor")
#undef AMFVariantColor

#pragma push_macro("AMFVariantString")
#undef AMFVariantString

#pragma push_macro("AMFVariantWString")
#undef AMFVariantWString

#pragma push_macro("AMFVariantInterface")
#undef AMFVariantInterface

#pragma push_macro("AMF_VARIANT_RETURN_IF_INVALID_POINTER")
#undef AMF_VARIANT_RETURN_IF_INVALID_POINTER

#pragma push_macro("AMFConvertTool")
#undef AMFConvertTool

#pragma push_macro("AMF_MAKE_FULL_VERSION")
#undef AMF_MAKE_FULL_VERSION

#pragma push_macro("AMF_GET_MAJOR_VERSION")
#undef AMF_GET_MAJOR_VERSION

#pragma push_macro("AMF_GET_MINOR_VERSION")
#undef AMF_GET_MINOR_VERSION

#pragma push_macro("AMF_GET_SUBMINOR_VERSION")
#undef AMF_GET_SUBMINOR_VERSION

#pragma push_macro("AMF_GET_BUILD_VERSION")
#undef AMF_GET_BUILD_VERSION

#pragma push_macro("NVENCAPI_STRUCT_VERSION")
#undef NVENCAPI_STRUCT_VERSION

#pragma push_macro("__lseek")
#undef __lseek

#pragma push_macro("__pread")
#undef __pread

#pragma push_macro("__pwrite")
#undef __pwrite

#pragma push_macro("__ftruncate")
#undef __ftruncate

#pragma push_macro("VK_MAKE_VERSION")
#undef VK_MAKE_VERSION

#pragma push_macro("VK_VERSION_MAJOR")
#undef VK_VERSION_MAJOR

#pragma push_macro("VK_VERSION_MINOR")
#undef VK_VERSION_MINOR

#pragma push_macro("VK_VERSION_PATCH")
#undef VK_VERSION_PATCH

#pragma push_macro("VK_DEFINE_HANDLE")
#undef VK_DEFINE_HANDLE

#pragma push_macro("SET_DWORD_STAT_BY_FNAME")
#undef SET_DWORD_STAT_BY_FNAME

#pragma push_macro("SET_FLOAT_STAT_BY_FNAME")
#undef SET_FLOAT_STAT_BY_FNAME

#pragma push_macro("CSV_STAT_PTR")
#undef CSV_STAT_PTR

#pragma push_macro("CSV_CUSTOM_STAT_DEFINED_BY_PTR")
#undef CSV_CUSTOM_STAT_DEFINED_BY_PTR

#pragma push_macro("HANDLE_CASE")
#undef HANDLE_CASE

#pragma push_macro("KDBG_CODE")
#undef KDBG_CODE

#pragma push_macro("APPSDBG_CODE")
#undef APPSDBG_CODE

#pragma push_macro("TASKGRAPH_SCOPE_CYCLE_COUNTER")
#undef TASKGRAPH_SCOPE_CYCLE_COUNTER

#pragma push_macro("likely")
#undef likely

#pragma push_macro("unlikely")
#undef unlikely

#pragma push_macro("ALLOC")
#undef ALLOC

#pragma push_macro("ALLOC_AND_ZERO")
#undef ALLOC_AND_ZERO

#pragma push_macro("FREEMEM")
#undef FREEMEM

#pragma push_macro("MEM_INIT")
#undef MEM_INIT

#pragma push_macro("LZ4_STATIC_ASSERT")
#undef LZ4_STATIC_ASSERT

#pragma push_macro("MIN")
#undef MIN

#pragma push_macro("HASH_FUNCTION")
#undef HASH_FUNCTION

#pragma push_macro("DELTANEXTMAXD")
#undef DELTANEXTMAXD

#pragma push_macro("DELTANEXTU16")
#undef DELTANEXTU16

#pragma push_macro("UPDATABLE")
#undef UPDATABLE

#pragma push_macro("SUBSTRINGTEST")
#undef SUBSTRINGTEST

#pragma push_macro("FP_TEXT_PASTE")
#undef FP_TEXT_PASTE

// #pragma push_macro("WTEXT")
// #undef WTEXT

#pragma push_macro("CASE")
#undef CASE

#pragma push_macro("LLM_TAG_NAME_ARRAY")
#undef LLM_TAG_NAME_ARRAY

#pragma push_macro("LLM_TAG_STAT_ARRAY")
#undef LLM_TAG_STAT_ARRAY

#pragma push_macro("LLM_TAG_STATGROUP_ARRAY")
#undef LLM_TAG_STATGROUP_ARRAY

#pragma push_macro("bswap_32")
#undef bswap_32

#pragma push_macro("bswap_64")
#undef bswap_64

#pragma push_macro("uint32_in_expected_order")
#undef uint32_in_expected_order

#pragma push_macro("uint64_in_expected_order")
#undef uint64_in_expected_order

#pragma push_macro("LIKELY")
#undef LIKELY

#pragma push_macro("PERMUTE3")
#undef PERMUTE3

#pragma push_macro("DEF_GETPLURALFORM_CAST")
#undef DEF_GETPLURALFORM_CAST

#pragma push_macro("DEF_ASNUMBER_CAST")
#undef DEF_ASNUMBER_CAST

#pragma push_macro("DEF_ASNUMBER")
#undef DEF_ASNUMBER

#pragma push_macro("DEF_ASCURRENCY_CAST")
#undef DEF_ASCURRENCY_CAST

#pragma push_macro("DEF_ASCURRENCY")
#undef DEF_ASCURRENCY

#pragma push_macro("DEF_ASPERCENT_CAST")
#undef DEF_ASPERCENT_CAST

#pragma push_macro("DEF_ASPERCENT")
#undef DEF_ASPERCENT

#pragma push_macro("CONDITIONAL_CREATE_TEXT_HISTORY")
#undef CONDITIONAL_CREATE_TEXT_HISTORY

#pragma push_macro("ENUM_CASE_FROM_STRING")
#undef ENUM_CASE_FROM_STRING

#pragma push_macro("ENUM_CASE_TO_STRING")
#undef ENUM_CASE_TO_STRING

#pragma push_macro("WRITE_CUSTOM_OPTION")
#undef WRITE_CUSTOM_OPTION

#pragma push_macro("READ_BOOL_OPTION")
#undef READ_BOOL_OPTION

#pragma push_macro("READ_CUSTOM_OPTION")
#undef READ_CUSTOM_OPTION

#pragma push_macro("TEXT_STRINGIFICATION_FUNC_MODIFY_BUFFER_AND_VALIDATE")
#undef TEXT_STRINGIFICATION_FUNC_MODIFY_BUFFER_AND_VALIDATE

#pragma push_macro("TEXT_STRINGIFICATION_PEEK_MARKER")
#undef TEXT_STRINGIFICATION_PEEK_MARKER

#pragma push_macro("TEXT_STRINGIFICATION_PEEK_INSENSITIVE_MARKER")
#undef TEXT_STRINGIFICATION_PEEK_INSENSITIVE_MARKER

#pragma push_macro("TEXT_STRINGIFICATION_SKIP_MARKER")
#undef TEXT_STRINGIFICATION_SKIP_MARKER

#pragma push_macro("TEXT_STRINGIFICATION_SKIP_INSENSITIVE_MARKER")
#undef TEXT_STRINGIFICATION_SKIP_INSENSITIVE_MARKER

#pragma push_macro("TEXT_STRINGIFICATION_SKIP_MARKER_LEN")
#undef TEXT_STRINGIFICATION_SKIP_MARKER_LEN

#pragma push_macro("TEXT_STRINGIFICATION_SKIP_WHITESPACE")
#undef TEXT_STRINGIFICATION_SKIP_WHITESPACE

#pragma push_macro("TEXT_STRINGIFICATION_SKIP_WHITESPACE_TO_CHAR")
#undef TEXT_STRINGIFICATION_SKIP_WHITESPACE_TO_CHAR

#pragma push_macro("TEXT_STRINGIFICATION_SKIP_WHITESPACE_AND_CHAR")
#undef TEXT_STRINGIFICATION_SKIP_WHITESPACE_AND_CHAR

#pragma push_macro("TEXT_STRINGIFICATION_READ_NUMBER")
#undef TEXT_STRINGIFICATION_READ_NUMBER

#pragma push_macro("TEXT_STRINGIFICATION_READ_ALNUM")
#undef TEXT_STRINGIFICATION_READ_ALNUM

#pragma push_macro("TEXT_STRINGIFICATION_READ_QUOTED_STRING")
#undef TEXT_STRINGIFICATION_READ_QUOTED_STRING

#pragma push_macro("TEXT_STRINGIFICATION_READ_SCOPED_ENUM")
#undef TEXT_STRINGIFICATION_READ_SCOPED_ENUM

#pragma push_macro("PLATFORM_MAC_MAKE_FOURCC")
#undef PLATFORM_MAC_MAKE_FOURCC

#pragma push_macro("MEMPRO_STATIC_ASSERT")
#undef MEMPRO_STATIC_ASSERT

#pragma push_macro("ENDIAN_TEST")
#undef ENDIAN_TEST

#pragma push_macro("_T")
#undef _T

#pragma push_macro("KEYLENGTH")
#undef KEYLENGTH

#pragma push_macro("RKLENGTH")
#undef RKLENGTH

#pragma push_macro("NROUNDS")
#undef NROUNDS

#pragma push_macro("GETU32")
#undef GETU32

#pragma push_macro("PUTU32")
#undef PUTU32

#pragma push_macro("DEFINE_LOG_CATEGORY_HELPER")
#undef DEFINE_LOG_CATEGORY_HELPER

#pragma push_macro("ADD_64b_2_64b")
#undef ADD_64b_2_64b

#pragma push_macro("ADD_16b_2_64b")
#undef ADD_16b_2_64b

#pragma push_macro("MD5_F")
#undef MD5_F

#pragma push_macro("MD5_G")
#undef MD5_G

#pragma push_macro("MD5_H")
#undef MD5_H

#pragma push_macro("MD5_I")
#undef MD5_I

#pragma push_macro("ROTLEFT")
#undef ROTLEFT

#pragma push_macro("MD5_FF")
#undef MD5_FF

#pragma push_macro("MD5_GG")
#undef MD5_GG

#pragma push_macro("MD5_HH")
#undef MD5_HH

#pragma push_macro("MD5_II")
#undef MD5_II

#pragma push_macro("ROL32")
#undef ROL32

#pragma push_macro("SHABLK0")
#undef SHABLK0

#pragma push_macro("SHABLK")
#undef SHABLK

#pragma push_macro("_R0")
#undef _R0

#pragma push_macro("_R1")
#undef _R1

#pragma push_macro("_R2")
#undef _R2

#pragma push_macro("_R3")
#undef _R3

#pragma push_macro("_R4")
#undef _R4

#pragma push_macro("GROWABLE_LOGF")
#undef GROWABLE_LOGF

#pragma push_macro("ABTEST_LOG")
#undef ABTEST_LOG

#pragma push_macro("CPUPROFILERTRACE_OUTPUTBEGINEVENT_PROLOGUE")
#undef CPUPROFILERTRACE_OUTPUTBEGINEVENT_PROLOGUE

#pragma push_macro("CPUPROFILERTRACE_OUTPUTBEGINEVENT_EPILOGUE")
#undef CPUPROFILERTRACE_OUTPUTBEGINEVENT_EPILOGUE

#pragma push_macro("STATS_HIERARCHICAL_TIMER_FUNC")
#undef STATS_HIERARCHICAL_TIMER_FUNC

#pragma push_macro("BENCHMARK")
#undef BENCHMARK

#pragma push_macro("TEST")
#undef TEST

#pragma push_macro("TEST_EX")
#undef TEST_EX

#pragma push_macro("TEST_QUAT_ROTATE")
#undef TEST_QUAT_ROTATE

#pragma push_macro("INTERP_WITH_RANGE")
#undef INTERP_WITH_RANGE

#pragma push_macro("TestUnixEquivalent")
#undef TestUnixEquivalent

#pragma push_macro("TestYear")
#undef TestYear

#pragma push_macro("TestMonth")
#undef TestMonth

#pragma push_macro("TestMonthOfYear")
#undef TestMonthOfYear

#pragma push_macro("TestDay")
#undef TestDay

#pragma push_macro("TestHour")
#undef TestHour

#pragma push_macro("TestMinute")
#undef TestMinute

#pragma push_macro("TestSecond")
#undef TestSecond

#pragma push_macro("TestMillisecond")
#undef TestMillisecond

#pragma push_macro("UE_LOG_UNIX_FILE")
#undef UE_LOG_UNIX_FILE

#pragma push_macro("REGISTER_NAME")
#undef REGISTER_NAME

#pragma push_macro("DECLARE_LOG_CATEGORY_EXTERN_HELPER")
#undef DECLARE_LOG_CATEGORY_EXTERN_HELPER

#pragma push_macro("SCOPED_BOOT_TIMING")
#undef SCOPED_BOOT_TIMING

#pragma push_macro("FOREACH_ENUM_EPIXELFORMAT")
#undef FOREACH_ENUM_EPIXELFORMAT

#pragma push_macro("PLATFORM_CODE_SECTION")
#undef PLATFORM_CODE_SECTION

#pragma push_macro("GCC_PACK")
#undef GCC_PACK

#pragma push_macro("GCC_ALIGN")
#undef GCC_ALIGN

#pragma push_macro("PLATFORM_BREAK")
#undef PLATFORM_BREAK

#pragma push_macro("UE_DEBUG_BREAK_IMPL")
#undef UE_DEBUG_BREAK_IMPL

#pragma push_macro("_aligned_malloc")
#undef _aligned_malloc

#pragma push_macro("_aligned_realloc")
#undef _aligned_realloc

#pragma push_macro("_aligned_free")
#undef _aligned_free

// #pragma push_macro("TEXT")
// #undef TEXT

#pragma push_macro("LLM_SCOPE_APPLE")
#undef LLM_SCOPE_APPLE

#pragma push_macro("LLM_PLATFORM_SCOPE_APPLE")
#undef LLM_PLATFORM_SCOPE_APPLE

#pragma push_macro("APPLE_PLATFORM_OBJECT_ALLOC_OVERRIDES")
#undef APPLE_PLATFORM_OBJECT_ALLOC_OVERRIDES

#pragma push_macro("checkThreadGraph")
#undef checkThreadGraph

#pragma push_macro("CA_SUPPRESS")
#undef CA_SUPPRESS

#pragma push_macro("CA_ASSUME")
#undef CA_ASSUME

#pragma push_macro("CA_CONSTANT_IF")
#undef CA_CONSTANT_IF

#pragma push_macro("TSAN_BEFORE")
#undef TSAN_BEFORE

#pragma push_macro("TSAN_AFTER")
#undef TSAN_AFTER

#pragma push_macro("TSAN_ATOMIC")
#undef TSAN_ATOMIC

#pragma push_macro("DEPRECATED")
#undef DEPRECATED

#pragma push_macro("EMIT_CUSTOM_WARNING_AT_LINE")
#undef EMIT_CUSTOM_WARNING_AT_LINE

#pragma push_macro("LZ4_QUOTE")
#undef LZ4_QUOTE

#pragma push_macro("LZ4_EXPAND_AND_QUOTE")
#undef LZ4_EXPAND_AND_QUOTE

#pragma push_macro("LZ4_COMPRESSBOUND")
#undef LZ4_COMPRESSBOUND

#pragma push_macro("LZ4_DECODER_RING_BUFFER_SIZE")
#undef LZ4_DECODER_RING_BUFFER_SIZE

#pragma push_macro("LZ4_DECOMPRESS_INPLACE_MARGIN")
#undef LZ4_DECOMPRESS_INPLACE_MARGIN

#pragma push_macro("LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE")
#undef LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE

#pragma push_macro("LZ4_COMPRESS_INPLACE_BUFFER_SIZE")
#undef LZ4_COMPRESS_INPLACE_BUFFER_SIZE

#pragma push_macro("checkLockFreePointerList")
#undef checkLockFreePointerList

#pragma push_macro("ExchangeB")
#undef ExchangeB

// #pragma push_macro("TCHAR_TO_ANSI")
// #undef TCHAR_TO_ANSI

// #pragma push_macro("ANSI_TO_TCHAR")
// #undef ANSI_TO_TCHAR

// #pragma push_macro("TCHAR_TO_UTF8")
// #undef TCHAR_TO_UTF8

// #pragma push_macro("UTF8_TO_TCHAR")
// #undef UTF8_TO_TCHAR

// #pragma push_macro("TCHAR_TO_UTF16")
// #undef TCHAR_TO_UTF16

// #pragma push_macro("UTF16_TO_TCHAR")
// #undef UTF16_TO_TCHAR

// #pragma push_macro("TCHAR_TO_UTF32")
// #undef TCHAR_TO_UTF32

// #pragma push_macro("UTF32_TO_TCHAR")
// #undef UTF32_TO_TCHAR

// #pragma push_macro("TCHAR_TO_WCHAR")
// #undef TCHAR_TO_WCHAR

// #pragma push_macro("WCHAR_TO_TCHAR")
// #undef WCHAR_TO_TCHAR

#pragma push_macro("FUNC_CONCAT")
#undef FUNC_CONCAT

#pragma push_macro("FUNC_DECLARE_DELEGATE")
#undef FUNC_DECLARE_DELEGATE

#pragma push_macro("FUNC_DECLARE_MULTICAST_DELEGATE")
#undef FUNC_DECLARE_MULTICAST_DELEGATE

#pragma push_macro("FUNC_DECLARE_EVENT")
#undef FUNC_DECLARE_EVENT

#pragma push_macro("DECLARE_DERIVED_EVENT")
#undef DECLARE_DERIVED_EVENT

#pragma push_macro("FUNC_DECLARE_DYNAMIC_DELEGATE")
#undef FUNC_DECLARE_DYNAMIC_DELEGATE

#pragma push_macro("FUNC_DECLARE_DYNAMIC_DELEGATE_RETVAL")
#undef FUNC_DECLARE_DYNAMIC_DELEGATE_RETVAL

#pragma push_macro("FUNC_DECLARE_DYNAMIC_MULTICAST_DELEGATE")
#undef FUNC_DECLARE_DYNAMIC_MULTICAST_DELEGATE

#pragma push_macro("STATIC_FUNCTION_FNAME")
#undef STATIC_FUNCTION_FNAME

#pragma push_macro("BindDynamic")
#undef BindDynamic

#pragma push_macro("AddDynamic")
#undef AddDynamic

#pragma push_macro("AddUniqueDynamic")
#undef AddUniqueDynamic

#pragma push_macro("RemoveDynamic")
#undef RemoveDynamic

#pragma push_macro("IsAlreadyBound")
#undef IsAlreadyBound

#pragma push_macro("DECLARE_DELEGATE")
#undef DECLARE_DELEGATE

#pragma push_macro("DECLARE_MULTICAST_DELEGATE")
#undef DECLARE_MULTICAST_DELEGATE

#pragma push_macro("DECLARE_EVENT")
#undef DECLARE_EVENT

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE")
#undef DECLARE_DYNAMIC_DELEGATE

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE

#pragma push_macro("DECLARE_DELEGATE_RetVal")
#undef DECLARE_DELEGATE_RetVal

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal

#pragma push_macro("DECLARE_DELEGATE_OneParam")
#undef DECLARE_DELEGATE_OneParam

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_OneParam")
#undef DECLARE_MULTICAST_DELEGATE_OneParam

#pragma push_macro("DECLARE_EVENT_OneParam")
#undef DECLARE_EVENT_OneParam

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_OneParam")
#undef DECLARE_DYNAMIC_DELEGATE_OneParam

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam

#pragma push_macro("DECLARE_DELEGATE_RetVal_OneParam")
#undef DECLARE_DELEGATE_RetVal_OneParam

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam

#pragma push_macro("DECLARE_DELEGATE_TwoParams")
#undef DECLARE_DELEGATE_TwoParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_TwoParams")
#undef DECLARE_MULTICAST_DELEGATE_TwoParams

#pragma push_macro("DECLARE_EVENT_TwoParams")
#undef DECLARE_EVENT_TwoParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_TwoParams")
#undef DECLARE_DYNAMIC_DELEGATE_TwoParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_TwoParams")
#undef DECLARE_DELEGATE_RetVal_TwoParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams

#pragma push_macro("DECLARE_DELEGATE_ThreeParams")
#undef DECLARE_DELEGATE_ThreeParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_ThreeParams")
#undef DECLARE_MULTICAST_DELEGATE_ThreeParams

#pragma push_macro("DECLARE_EVENT_ThreeParams")
#undef DECLARE_EVENT_ThreeParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_ThreeParams")
#undef DECLARE_DYNAMIC_DELEGATE_ThreeParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_ThreeParams")
#undef DECLARE_DELEGATE_RetVal_ThreeParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams

#pragma push_macro("DECLARE_DELEGATE_FourParams")
#undef DECLARE_DELEGATE_FourParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_FourParams")
#undef DECLARE_MULTICAST_DELEGATE_FourParams

#pragma push_macro("DECLARE_EVENT_FourParams")
#undef DECLARE_EVENT_FourParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_FourParams")
#undef DECLARE_DYNAMIC_DELEGATE_FourParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_FourParams")
#undef DECLARE_DELEGATE_RetVal_FourParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams

#pragma push_macro("DECLARE_DELEGATE_FiveParams")
#undef DECLARE_DELEGATE_FiveParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_FiveParams")
#undef DECLARE_MULTICAST_DELEGATE_FiveParams

#pragma push_macro("DECLARE_EVENT_FiveParams")
#undef DECLARE_EVENT_FiveParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_FiveParams")
#undef DECLARE_DYNAMIC_DELEGATE_FiveParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_FiveParams")
#undef DECLARE_DELEGATE_RetVal_FiveParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_FiveParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_FiveParams

#pragma push_macro("DECLARE_DELEGATE_SixParams")
#undef DECLARE_DELEGATE_SixParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_SixParams")
#undef DECLARE_MULTICAST_DELEGATE_SixParams

#pragma push_macro("DECLARE_EVENT_SixParams")
#undef DECLARE_EVENT_SixParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_SixParams")
#undef DECLARE_DYNAMIC_DELEGATE_SixParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_SixParams")
#undef DECLARE_DELEGATE_RetVal_SixParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_SixParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_SixParams

#pragma push_macro("DECLARE_DELEGATE_SevenParams")
#undef DECLARE_DELEGATE_SevenParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_SevenParams")
#undef DECLARE_MULTICAST_DELEGATE_SevenParams

#pragma push_macro("DECLARE_EVENT_SevenParams")
#undef DECLARE_EVENT_SevenParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_SevenParams")
#undef DECLARE_DYNAMIC_DELEGATE_SevenParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_SevenParams")
#undef DECLARE_DELEGATE_RetVal_SevenParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_SevenParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_SevenParams

#pragma push_macro("DECLARE_DELEGATE_EightParams")
#undef DECLARE_DELEGATE_EightParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_EightParams")
#undef DECLARE_MULTICAST_DELEGATE_EightParams

#pragma push_macro("DECLARE_EVENT_EightParams")
#undef DECLARE_EVENT_EightParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_EightParams")
#undef DECLARE_DYNAMIC_DELEGATE_EightParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_EightParams")
#undef DECLARE_DELEGATE_RetVal_EightParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_EightParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_EightParams

#pragma push_macro("DECLARE_DELEGATE_NineParams")
#undef DECLARE_DELEGATE_NineParams

#pragma push_macro("DECLARE_MULTICAST_DELEGATE_NineParams")
#undef DECLARE_MULTICAST_DELEGATE_NineParams

#pragma push_macro("DECLARE_EVENT_NineParams")
#undef DECLARE_EVENT_NineParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_NineParams")
#undef DECLARE_DYNAMIC_DELEGATE_NineParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_DELEGATE_NineParams")
#undef DECLARE_DYNAMIC_MULTICAST_DELEGATE_NineParams

#pragma push_macro("DECLARE_DELEGATE_RetVal_NineParams")
#undef DECLARE_DELEGATE_RetVal_NineParams

#pragma push_macro("DECLARE_DYNAMIC_DELEGATE_RetVal_NineParams")
#undef DECLARE_DYNAMIC_DELEGATE_RetVal_NineParams

#pragma push_macro("CHECK_CONCURRENT_ACCESS")
#undef CHECK_CONCURRENT_ACCESS

#pragma push_macro("FRAMEPRO_FRAME_START")
#undef FRAMEPRO_FRAME_START

#pragma push_macro("FRAMEPRO_SHUTDOWN")
#undef FRAMEPRO_SHUTDOWN

#pragma push_macro("FRAMEPRO_SET_PORT")
#undef FRAMEPRO_SET_PORT

#pragma push_macro("FRAMEPRO_SET_SESSION_INFO")
#undef FRAMEPRO_SET_SESSION_INFO

#pragma push_macro("FRAMEPRO_SET_ALLOCATOR")
#undef FRAMEPRO_SET_ALLOCATOR

#pragma push_macro("FRAMEPRO_SET_THREAD_NAME")
#undef FRAMEPRO_SET_THREAD_NAME

#pragma push_macro("FRAMEPRO_THREAD_ORDER")
#undef FRAMEPRO_THREAD_ORDER

#pragma push_macro("FRAMEPRO_REGISTER_STRING")
#undef FRAMEPRO_REGISTER_STRING

#pragma push_macro("FRAMEPRO_START_RECORDING")
#undef FRAMEPRO_START_RECORDING

#pragma push_macro("FRAMEPRO_STOP_RECORDING")
#undef FRAMEPRO_STOP_RECORDING

#pragma push_macro("FRAMEPRO_REGISTER_CONNECTION_CHANGED_CALLBACK")
#undef FRAMEPRO_REGISTER_CONNECTION_CHANGED_CALLBACK

#pragma push_macro("FRAMEPRO_UNREGISTER_CONNECTION_CHANGED_CALLBACK")
#undef FRAMEPRO_UNREGISTER_CONNECTION_CHANGED_CALLBACK

#pragma push_macro("FRAMEPRO_SET_THREAD_PRIORITY")
#undef FRAMEPRO_SET_THREAD_PRIORITY

#pragma push_macro("FRAMEPRO_SET_THREAD_AFFINITY")
#undef FRAMEPRO_SET_THREAD_AFFINITY

#pragma push_macro("FRAMEPRO_BLOCK_SOCKETS")
#undef FRAMEPRO_BLOCK_SOCKETS

#pragma push_macro("FRAMEPRO_UNBLOCK_SOCKETS")
#undef FRAMEPRO_UNBLOCK_SOCKETS

#pragma push_macro("FRAMEPRO_CLEANUP_THREAD")
#undef FRAMEPRO_CLEANUP_THREAD

#pragma push_macro("FRAMEPRO_THREAD_SCOPE")
#undef FRAMEPRO_THREAD_SCOPE

#pragma push_macro("FRAMEPRO_LOG")
#undef FRAMEPRO_LOG

#pragma push_macro("FRAMEPRO_COLOUR")
#undef FRAMEPRO_COLOUR

#pragma push_macro("FRAMEPRO_SET_CONDITIONAL_SCOPE_MIN_TIME")
#undef FRAMEPRO_SET_CONDITIONAL_SCOPE_MIN_TIME

#pragma push_macro("FRAMEPRO_SCOPE")
#undef FRAMEPRO_SCOPE

#pragma push_macro("FRAMEPRO_NAMED_SCOPE")
#undef FRAMEPRO_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_NAMED_SCOPE_W")
#undef FRAMEPRO_NAMED_SCOPE_W

#pragma push_macro("FRAMEPRO_ID_SCOPE")
#undef FRAMEPRO_ID_SCOPE

#pragma push_macro("FRAMEPRO_DYNAMIC_SCOPE")
#undef FRAMEPRO_DYNAMIC_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_SCOPE")
#undef FRAMEPRO_CONDITIONAL_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_ID_SCOPE")
#undef FRAMEPRO_CONDITIONAL_ID_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_NAMED_SCOPE")
#undef FRAMEPRO_CONDITIONAL_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_NAMED_SCOPE_W")
#undef FRAMEPRO_CONDITIONAL_NAMED_SCOPE_W

#pragma push_macro("FRAMEPRO_CONDITIONAL_BOOL_SCOPE")
#undef FRAMEPRO_CONDITIONAL_BOOL_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_BOOL_ID_SCOPE")
#undef FRAMEPRO_CONDITIONAL_BOOL_ID_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_BOOL_NAMED_SCOPE")
#undef FRAMEPRO_CONDITIONAL_BOOL_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_BOOL_NAMED_SCOPE_W")
#undef FRAMEPRO_CONDITIONAL_BOOL_NAMED_SCOPE_W

#pragma push_macro("FRAMEPRO_START_NAMED_SCOPE")
#undef FRAMEPRO_START_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_STOP_NAMED_SCOPE")
#undef FRAMEPRO_STOP_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_START_SCOPE")
#undef FRAMEPRO_CONDITIONAL_START_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_STOP_NAMED_SCOPE")
#undef FRAMEPRO_CONDITIONAL_STOP_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_STOP_DYNAMIC_SCOPE")
#undef FRAMEPRO_CONDITIONAL_STOP_DYNAMIC_SCOPE

#pragma push_macro("FRAMEPRO_CONDITIONAL_PARENT_SCOPE")
#undef FRAMEPRO_CONDITIONAL_PARENT_SCOPE

#pragma push_macro("FRAMEPRO_SET_SCOPE_COLOUR")
#undef FRAMEPRO_SET_SCOPE_COLOUR

#pragma push_macro("FRAMEPRO_IDLE_SCOPE")
#undef FRAMEPRO_IDLE_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_NAMED_SCOPE")
#undef FRAMEPRO_IDLE_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_NAMED_SCOPE_W")
#undef FRAMEPRO_IDLE_NAMED_SCOPE_W

#pragma push_macro("FRAMEPRO_IDLE_ID_SCOPE")
#undef FRAMEPRO_IDLE_ID_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_DYNAMIC_SCOPE")
#undef FRAMEPRO_IDLE_DYNAMIC_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_SCOPE")
#undef FRAMEPRO_IDLE_CONDITIONAL_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_ID_SCOPE")
#undef FRAMEPRO_IDLE_CONDITIONAL_ID_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_NAMED_SCOPE")
#undef FRAMEPRO_IDLE_CONDITIONAL_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_NAMED_SCOPE_W")
#undef FRAMEPRO_IDLE_CONDITIONAL_NAMED_SCOPE_W

#pragma push_macro("FRAMEPRO_IDLE_START_NAMED_SCOPE")
#undef FRAMEPRO_IDLE_START_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_STOP_NAMED_SCOPE")
#undef FRAMEPRO_IDLE_STOP_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_START_SCOPE")
#undef FRAMEPRO_IDLE_CONDITIONAL_START_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_STOP_NAMED_SCOPE")
#undef FRAMEPRO_IDLE_CONDITIONAL_STOP_NAMED_SCOPE

#pragma push_macro("FRAMEPRO_IDLE_CONDITIONAL_STOP_DYNAMIC_SCOPE")
#undef FRAMEPRO_IDLE_CONDITIONAL_STOP_DYNAMIC_SCOPE

#pragma push_macro("FRAMEPRO_CUSTOM_STAT")
#undef FRAMEPRO_CUSTOM_STAT

#pragma push_macro("FRAMEPRO_DYNAMIC_CUSTOM_STAT")
#undef FRAMEPRO_DYNAMIC_CUSTOM_STAT

#pragma push_macro("FRAMEPRO_SCOPE_CUSTOM_STAT")
#undef FRAMEPRO_SCOPE_CUSTOM_STAT

#pragma push_macro("FRAMEPRO_SET_CUSTOM_STAT_GRAPH")
#undef FRAMEPRO_SET_CUSTOM_STAT_GRAPH

#pragma push_macro("FRAMEPRO_SET_CUSTOM_STAT_UNIT")
#undef FRAMEPRO_SET_CUSTOM_STAT_UNIT

#pragma push_macro("FRAMEPRO_SET_CUSTOM_STAT_COLOUR")
#undef FRAMEPRO_SET_CUSTOM_STAT_COLOUR

#pragma push_macro("FRAMEPRO_HIRES_SCOPE")
#undef FRAMEPRO_HIRES_SCOPE

#pragma push_macro("FRAMEPRO_DECL_GLOBAL_HIRES_TIMER")
#undef FRAMEPRO_DECL_GLOBAL_HIRES_TIMER

#pragma push_macro("FRAMEPRO_GLOBAL_HIRES_SCOPE")
#undef FRAMEPRO_GLOBAL_HIRES_SCOPE

#pragma push_macro("FRAMEPRO_EVENT")
#undef FRAMEPRO_EVENT

#pragma push_macro("FRAMEPRO_WAIT_EVENT_SCOPE")
#undef FRAMEPRO_WAIT_EVENT_SCOPE

#pragma push_macro("FRAMEPRO_TRIGGER_WAIT_EVENT")
#undef FRAMEPRO_TRIGGER_WAIT_EVENT

#pragma push_macro("FRAMEPRO_STRINGIZE")
#undef FRAMEPRO_STRINGIZE

#pragma push_macro("FRAMEPRO_STRINGIZE2")
#undef FRAMEPRO_STRINGIZE2

#pragma push_macro("FRAMEPRO_JOIN")
#undef FRAMEPRO_JOIN

#pragma push_macro("FRAMEPRO_JOIN2")
#undef FRAMEPRO_JOIN2

#pragma push_macro("FRAMEPRO_UNIQUE")
#undef FRAMEPRO_UNIQUE

#pragma push_macro("FRAMEPRO_WIDESTR")
#undef FRAMEPRO_WIDESTR

#pragma push_macro("FRAMEPRO_WIDESTR2")
#undef FRAMEPRO_WIDESTR2

#pragma push_macro("FRAMEPRO_ASSERT")
#undef FRAMEPRO_ASSERT

#pragma push_macro("FRAMEPRO_UNREFERENCED")
#undef FRAMEPRO_UNREFERENCED

#pragma push_macro("FRAMEPRO_GET_CLOCK_COUNT")
#undef FRAMEPRO_GET_CLOCK_COUNT

#pragma push_macro("MULTI_STATEMENT")
#undef MULTI_STATEMENT

#pragma push_macro("FRAMEPRO_ALIGN_STRUCT")
#undef FRAMEPRO_ALIGN_STRUCT

#pragma push_macro("EMIT_CUSTOM_WARNING")
#undef EMIT_CUSTOM_WARNING

#pragma push_macro("DEPRECATED_MACRO")
#undef DEPRECATED_MACRO

#pragma push_macro("PLATFORM_MEMORY_SIZE_BUCKET_LIST")
#undef PLATFORM_MEMORY_SIZE_BUCKET_LIST

#pragma push_macro("PLATFORM_MEMORY_SIZE_BUCKET_ENUM")
#undef PLATFORM_MEMORY_SIZE_BUCKET_ENUM

#pragma push_macro("PLATFORM_MEMORY_SIZE_BUCKET_LEXTOSTRING")
#undef PLATFORM_MEMORY_SIZE_BUCKET_LEXTOSTRING

#pragma push_macro("FMemory_Alloca")
#undef FMemory_Alloca

#pragma push_macro("UE_DEBUG_BREAK")
#undef UE_DEBUG_BREAK

#pragma push_macro("cvarCheckCode")
#undef cvarCheckCode

#pragma push_macro("FILE_LOG")
#undef FILE_LOG

#pragma push_macro("DECLARE_LLM_MEMORY_STAT")
#undef DECLARE_LLM_MEMORY_STAT

#pragma push_macro("DECLARE_LLM_MEMORY_STAT_EXTERN")
#undef DECLARE_LLM_MEMORY_STAT_EXTERN

#pragma push_macro("LLMCheckMessage")
#undef LLMCheckMessage

#pragma push_macro("LLMCheckfMessage")
#undef LLMCheckfMessage

#pragma push_macro("LLMEnsureMessage")
#undef LLMEnsureMessage

#pragma push_macro("LLMCheck")
#undef LLMCheck

#pragma push_macro("LLMCheckf")
#undef LLMCheckf

#pragma push_macro("LLMEnsure")
#undef LLMEnsure

#pragma push_macro("LLM_ENUM_GENERIC_TAGS")
#undef LLM_ENUM_GENERIC_TAGS

#pragma push_macro("LLM_ENUM")
#undef LLM_ENUM

#pragma push_macro("LLM")
#undef LLM

#pragma push_macro("LLM_IF_ENABLED")
#undef LLM_IF_ENABLED

#pragma push_macro("LLM_SCOPE")
#undef LLM_SCOPE

#pragma push_macro("LLM_PLATFORM_SCOPE")
#undef LLM_PLATFORM_SCOPE

#pragma push_macro("LLM_SCOPED_PAUSE_TRACKING")
#undef LLM_SCOPED_PAUSE_TRACKING

#pragma push_macro("LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER")
#undef LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER

#pragma push_macro("LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT")
#undef LLM_SCOPED_PAUSE_TRACKING_WITH_ENUM_AND_AMOUNT

#pragma push_macro("LLM_REALLOC_SCOPE")
#undef LLM_REALLOC_SCOPE

#pragma push_macro("LLM_REALLOC_PLATFORM_SCOPE")
#undef LLM_REALLOC_PLATFORM_SCOPE

#pragma push_macro("LLM_SCOPED_TAG_WITH_STAT")
#undef LLM_SCOPED_TAG_WITH_STAT

#pragma push_macro("LLM_SCOPED_TAG_WITH_STAT_IN_SET")
#undef LLM_SCOPED_TAG_WITH_STAT_IN_SET

#pragma push_macro("LLM_SCOPED_TAG_WITH_STAT_NAME")
#undef LLM_SCOPED_TAG_WITH_STAT_NAME

#pragma push_macro("LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET")
#undef LLM_SCOPED_TAG_WITH_STAT_NAME_IN_SET

#pragma push_macro("LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG")
#undef LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG

#pragma push_macro("LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET")
#undef LLM_SCOPED_SINGLE_PLATFORM_STAT_TAG_IN_SET

#pragma push_macro("LLM_SCOPED_SINGLE_STAT_TAG")
#undef LLM_SCOPED_SINGLE_STAT_TAG

#pragma push_macro("LLM_SCOPED_SINGLE_STAT_TAG_IN_SET")
#undef LLM_SCOPED_SINGLE_STAT_TAG_IN_SET

#pragma push_macro("LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT")
#undef LLM_SCOPED_PAUSE_TRACKING_WITH_STAT_AND_AMOUNT

#pragma push_macro("LLM_SCOPED_TAG_WITH_OBJECT_IN_SET")
#undef LLM_SCOPED_TAG_WITH_OBJECT_IN_SET

#pragma push_macro("LLM_PUSH_STATS_FOR_ASSET_TAGS")
#undef LLM_PUSH_STATS_FOR_ASSET_TAGS

#pragma push_macro("LLM_DUMP_TAG")
#undef LLM_DUMP_TAG

#pragma push_macro("LLM_DUMP_PLATFORM_TAG")
#undef LLM_DUMP_PLATFORM_TAG

#pragma push_macro("LLM_SCOPED_SINGLE_RHI_STAT_TAG")
#undef LLM_SCOPED_SINGLE_RHI_STAT_TAG

#pragma push_macro("LLM_SCOPED_SINGLE_RHI_STAT_TAG_IN_SET")
#undef LLM_SCOPED_SINGLE_RHI_STAT_TAG_IN_SET

#pragma push_macro("MEM_TIME")
#undef MEM_TIME

#pragma push_macro("MBA_STAT")
#undef MBA_STAT

#pragma push_macro("MBG_STAT")
#undef MBG_STAT

#pragma push_macro("MALLOCLEAK_WHITELIST_SCOPE")
#undef MALLOCLEAK_WHITELIST_SCOPE

#pragma push_macro("MALLOCLEAK_SCOPED_CONTEXT")
#undef MALLOCLEAK_SCOPED_CONTEXT

#pragma push_macro("FUNCTION_CHECK_RETURN")
#undef FUNCTION_CHECK_RETURN

#pragma push_macro("UE_ASSUME")
#undef UE_ASSUME

#pragma push_macro("ASSUME")
#undef ASSUME

#pragma push_macro("UNLIKELY")
#undef UNLIKELY

#pragma push_macro("DECLARE_UINT64")
#undef DECLARE_UINT64

#pragma push_macro("MS_ALIGN")
#undef MS_ALIGN

#pragma push_macro("MSVC_PRAGMA")
#undef MSVC_PRAGMA

#pragma push_macro("FLUSH_CACHE_LINE")
#undef FLUSH_CACHE_LINE

#pragma push_macro("MSC_FORMAT_DIAGNOSTIC_HELPER_2")
#undef MSC_FORMAT_DIAGNOSTIC_HELPER_2

#pragma push_macro("MSC_FORMAT_DIAGNOSTIC_HELPER")
#undef MSC_FORMAT_DIAGNOSTIC_HELPER

#pragma push_macro("COMPILE_WARNING")
#undef COMPILE_WARNING

#pragma push_macro("COMPILE_ERROR")
#undef COMPILE_ERROR

#pragma push_macro("GCC_DIAGNOSTIC_HELPER")
#undef GCC_DIAGNOSTIC_HELPER

#pragma push_macro("checkAtCompileTime")
#undef checkAtCompileTime

#pragma push_macro("DEPRECATED_FORGAME")
#undef DEPRECATED_FORGAME

#pragma push_macro("INT32_MAIN_INT32_ARGC_TCHAR_ARGV")
#undef INT32_MAIN_INT32_ARGC_TCHAR_ARGV

#pragma push_macro("TEXT_PASTE")
#undef TEXT_PASTE

#pragma push_macro("NAMED_EVENT_STR")
#undef NAMED_EVENT_STR

#pragma push_macro("SCOPED_NAMED_EVENT")
#undef SCOPED_NAMED_EVENT

#pragma push_macro("SCOPED_NAMED_EVENT_FSTRING")
#undef SCOPED_NAMED_EVENT_FSTRING

#pragma push_macro("SCOPED_NAMED_EVENT_TCHAR")
#undef SCOPED_NAMED_EVENT_TCHAR

#pragma push_macro("SCOPED_NAMED_EVENT_TEXT")
#undef SCOPED_NAMED_EVENT_TEXT

#pragma push_macro("SCOPED_NAMED_EVENT_F")
#undef SCOPED_NAMED_EVENT_F

#pragma push_macro("SCOPED_PROFILER_COLOR")
#undef SCOPED_PROFILER_COLOR

#pragma push_macro("PREPROCESSOR_TO_STRING")
#undef PREPROCESSOR_TO_STRING

#pragma push_macro("PREPROCESSOR_TO_STRING_INNER")
#undef PREPROCESSOR_TO_STRING_INNER

#pragma push_macro("PREPROCESSOR_JOIN")
#undef PREPROCESSOR_JOIN

#pragma push_macro("PREPROCESSOR_JOIN_INNER")
#undef PREPROCESSOR_JOIN_INNER

#pragma push_macro("PREPROCESSOR_JOIN_FIRST")
#undef PREPROCESSOR_JOIN_FIRST

#pragma push_macro("PREPROCESSOR_JOIN_FIRST_INNER")
#undef PREPROCESSOR_JOIN_FIRST_INNER

#pragma push_macro("PREPROCESSOR_IF")
#undef PREPROCESSOR_IF

#pragma push_macro("PREPROCESSOR_IF_INNER_1")
#undef PREPROCESSOR_IF_INNER_1

#pragma push_macro("PREPROCESSOR_IF_INNER_0")
#undef PREPROCESSOR_IF_INNER_0

#pragma push_macro("PREPROCESSOR_COMMA_SEPARATED")
#undef PREPROCESSOR_COMMA_SEPARATED

#pragma push_macro("DEFINE_VARIABLE")
#undef DEFINE_VARIABLE

#pragma push_macro("PREPROCESSOR_REMOVE_OPTIONAL_PARENS")
#undef PREPROCESSOR_REMOVE_OPTIONAL_PARENS

#pragma push_macro("PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL")
#undef PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL

#pragma push_macro("COMPILED_PLATFORM_HEADER")
#undef COMPILED_PLATFORM_HEADER

#pragma push_macro("COMPILED_PLATFORM_HEADER_WITH_PREFIX")
#undef COMPILED_PLATFORM_HEADER_WITH_PREFIX

#pragma push_macro("FAST_DECIMAL_FORMAT_SIGNED_IMPL")
#undef FAST_DECIMAL_FORMAT_SIGNED_IMPL

#pragma push_macro("FAST_DECIMAL_FORMAT_UNSIGNED_IMPL")
#undef FAST_DECIMAL_FORMAT_UNSIGNED_IMPL

#pragma push_macro("FAST_DECIMAL_FORMAT_FRACTIONAL_IMPL")
#undef FAST_DECIMAL_FORMAT_FRACTIONAL_IMPL

#pragma push_macro("FAST_DECIMAL_PARSE_INTEGER_IMPL")
#undef FAST_DECIMAL_PARSE_INTEGER_IMPL

#pragma push_macro("FAST_DECIMAL_PARSE_FRACTIONAL_IMPL")
#undef FAST_DECIMAL_PARSE_FRACTIONAL_IMPL

#pragma push_macro("LOCTEXT")
#undef LOCTEXT

#pragma push_macro("NSLOCTEXT")
#undef NSLOCTEXT

#pragma push_macro("INVTEXT")
#undef INVTEXT

#pragma push_macro("LOCGEN_NUMBER")
#undef LOCGEN_NUMBER

#pragma push_macro("LOCGEN_NUMBER_GROUPED")
#undef LOCGEN_NUMBER_GROUPED

#pragma push_macro("LOCGEN_NUMBER_UNGROUPED")
#undef LOCGEN_NUMBER_UNGROUPED

#pragma push_macro("LOCGEN_NUMBER_CUSTOM")
#undef LOCGEN_NUMBER_CUSTOM

#pragma push_macro("LOCGEN_PERCENT")
#undef LOCGEN_PERCENT

#pragma push_macro("LOCGEN_PERCENT_GROUPED")
#undef LOCGEN_PERCENT_GROUPED

#pragma push_macro("LOCGEN_PERCENT_UNGROUPED")
#undef LOCGEN_PERCENT_UNGROUPED

#pragma push_macro("LOCGEN_PERCENT_CUSTOM")
#undef LOCGEN_PERCENT_CUSTOM

#pragma push_macro("LOCGEN_CURRENCY")
#undef LOCGEN_CURRENCY

#pragma push_macro("LOCGEN_DATE_UTC")
#undef LOCGEN_DATE_UTC

#pragma push_macro("LOCGEN_DATE_LOCAL")
#undef LOCGEN_DATE_LOCAL

#pragma push_macro("LOCGEN_TIME_UTC")
#undef LOCGEN_TIME_UTC

#pragma push_macro("LOCGEN_TIME_LOCAL")
#undef LOCGEN_TIME_LOCAL

#pragma push_macro("LOCGEN_DATETIME_UTC")
#undef LOCGEN_DATETIME_UTC

#pragma push_macro("LOCGEN_DATETIME_LOCAL")
#undef LOCGEN_DATETIME_LOCAL

#pragma push_macro("LOCGEN_TOUPPER")
#undef LOCGEN_TOUPPER

#pragma push_macro("LOCGEN_TOLOWER")
#undef LOCGEN_TOLOWER

#pragma push_macro("LOCGEN_FORMAT_ORDERED")
#undef LOCGEN_FORMAT_ORDERED

#pragma push_macro("LOCGEN_FORMAT_NAMED")
#undef LOCGEN_FORMAT_NAMED

#pragma push_macro("LOCTABLE_NEW")
#undef LOCTABLE_NEW

#pragma push_macro("LOCTABLE_FROMFILE_ENGINE")
#undef LOCTABLE_FROMFILE_ENGINE

#pragma push_macro("LOCTABLE_FROMFILE_GAME")
#undef LOCTABLE_FROMFILE_GAME

#pragma push_macro("LOCTABLE_SETSTRING")
#undef LOCTABLE_SETSTRING

#pragma push_macro("LOCTABLE_SETMETA")
#undef LOCTABLE_SETMETA

#pragma push_macro("LOCTABLE")
#undef LOCTABLE

#pragma push_macro("_aligned_msize")
#undef _aligned_msize

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL")
#undef UE_LOG_EXPAND_IS_FATAL

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_Fatal")
#undef UE_LOG_EXPAND_IS_FATAL_Fatal

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_Error")
#undef UE_LOG_EXPAND_IS_FATAL_Error

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_Warning")
#undef UE_LOG_EXPAND_IS_FATAL_Warning

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_Display")
#undef UE_LOG_EXPAND_IS_FATAL_Display

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_Log")
#undef UE_LOG_EXPAND_IS_FATAL_Log

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_Verbose")
#undef UE_LOG_EXPAND_IS_FATAL_Verbose

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_VeryVerbose")
#undef UE_LOG_EXPAND_IS_FATAL_VeryVerbose

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_All")
#undef UE_LOG_EXPAND_IS_FATAL_All

#pragma push_macro("UE_LOG_EXPAND_IS_FATAL_SetColor")
#undef UE_LOG_EXPAND_IS_FATAL_SetColor

#pragma push_macro("UE_LOG_SOURCE_FILE")
#undef UE_LOG_SOURCE_FILE

#pragma push_macro("UE_LOG")
#undef UE_LOG

#pragma push_macro("UE_LOG_CLINKAGE")
#undef UE_LOG_CLINKAGE

#pragma push_macro("UE_CLOG")
#undef UE_CLOG

#pragma push_macro("UE_LOG_ACTIVE")
#undef UE_LOG_ACTIVE

#pragma push_macro("UE_LOG_ANY_ACTIVE")
#undef UE_LOG_ANY_ACTIVE

#pragma push_macro("UE_SUPPRESS")
#undef UE_SUPPRESS

#pragma push_macro("UE_GET_LOG_VERBOSITY")
#undef UE_GET_LOG_VERBOSITY

#pragma push_macro("UE_SET_LOG_VERBOSITY")
#undef UE_SET_LOG_VERBOSITY

#pragma push_macro("DECLARE_LOG_CATEGORY_EXTERN")
#undef DECLARE_LOG_CATEGORY_EXTERN

#pragma push_macro("DEFINE_LOG_CATEGORY")
#undef DEFINE_LOG_CATEGORY

#pragma push_macro("DEFINE_LOG_CATEGORY_STATIC")
#undef DEFINE_LOG_CATEGORY_STATIC

#pragma push_macro("DECLARE_LOG_CATEGORY_CLASS")
#undef DECLARE_LOG_CATEGORY_CLASS

#pragma push_macro("DEFINE_LOG_CATEGORY_CLASS")
#undef DEFINE_LOG_CATEGORY_CLASS

#pragma push_macro("UE_SECURITY_LOG")
#undef UE_SECURITY_LOG

#pragma push_macro("NOTIFY_CLIENT_OF_SECURITY_EVENT_IF_NOT_SHIPPING")
#undef NOTIFY_CLIENT_OF_SECURITY_EVENT_IF_NOT_SHIPPING

#pragma push_macro("CLOSE_CONNECTION_DUE_TO_SECURITY_VIOLATION_INNER")
#undef CLOSE_CONNECTION_DUE_TO_SECURITY_VIOLATION_INNER

#pragma push_macro("CLOSE_CONNECTION_DUE_TO_SECURITY_VIOLATION")
#undef CLOSE_CONNECTION_DUE_TO_SECURITY_VIOLATION

#pragma push_macro("logOrEnsureNanError")
#undef logOrEnsureNanError

#pragma push_macro("LOG_SCOPE_VERBOSITY_OVERRIDE")
#undef LOG_SCOPE_VERBOSITY_OVERRIDE

#pragma push_macro("TRACE_LOG_CATEGORY")
#undef TRACE_LOG_CATEGORY

#pragma push_macro("TRACE_LOG_MESSAGE")
#undef TRACE_LOG_MESSAGE

#pragma push_macro("CREATE_FUNCTION_SHIM")
#undef CREATE_FUNCTION_SHIM

#pragma push_macro("CREATE_GLOBAL_SHIM")
#undef CREATE_GLOBAL_SHIM

#pragma push_macro("CREATE_DEPRECATED_SHIM")
#undef CREATE_DEPRECATED_SHIM

#pragma push_macro("CREATE_DEPRECATED_MSG_SHIM")
#undef CREATE_DEPRECATED_MSG_SHIM

#pragma push_macro("LUMIN_MLSDK_API_DEPRECATED_MSG")
#undef LUMIN_MLSDK_API_DEPRECATED_MSG

#pragma push_macro("AOS_TO_SOA2_ISPC")
#undef AOS_TO_SOA2_ISPC

#pragma push_macro("AOS_TO_SOA3_ISPC")
#undef AOS_TO_SOA3_ISPC

#pragma push_macro("AOS_TO_SOA4_ISPC")
#undef AOS_TO_SOA4_ISPC

#pragma push_macro("AOS_TO_SOA6_ISPC")
#undef AOS_TO_SOA6_ISPC

#pragma push_macro("DEFINE_EXPRESSION_OPERATOR_NODE")
#undef DEFINE_EXPRESSION_OPERATOR_NODE

#pragma push_macro("FOREACH_OCTREE_CHILD_NODE")
#undef FOREACH_OCTREE_CHILD_NODE

#pragma push_macro("DEFINE_INTERPCURVE_WRAPPER_STRUCT")
#undef DEFINE_INTERPCURVE_WRAPPER_STRUCT

#pragma push_macro("DEFINE_INTERVAL_WRAPPER_STRUCT")
#undef DEFINE_INTERVAL_WRAPPER_STRUCT

#pragma push_macro("DEFINE_RANGE_WRAPPER_STRUCT")
#undef DEFINE_RANGE_WRAPPER_STRUCT

#pragma push_macro("DEFINE_RANGEBOUND_WRAPPER_STRUCT")
#undef DEFINE_RANGEBOUND_WRAPPER_STRUCT

#pragma push_macro("NonZeroAnimWeight")
#undef NonZeroAnimWeight

#pragma push_macro("NonOneAnimWeight")
#undef NonOneAnimWeight

#pragma push_macro("ScalarReciprocal")
#undef ScalarReciprocal

#pragma push_macro("SOA_TO_AOS2_ISPC")
#undef SOA_TO_AOS2_ISPC

#pragma push_macro("SOA_TO_AOS3_ISPC")
#undef SOA_TO_AOS3_ISPC

#pragma push_macro("SOA_TO_AOS4_ISPC")
#undef SOA_TO_AOS4_ISPC

#pragma push_macro("SOA_TO_AOS6_ISPC")
#undef SOA_TO_AOS6_ISPC

#pragma push_macro("_PS_CONST")
#undef _PS_CONST

#pragma push_macro("_PI32_CONST")
#undef _PI32_CONST

#pragma push_macro("_PS_CONST_TYPE")
#undef _PS_CONST_TYPE

#pragma push_macro("COPY_XMM_TO_MM")
#undef COPY_XMM_TO_MM

#pragma push_macro("COPY_MM_TO_XMM")
#undef COPY_MM_TO_XMM

#pragma push_macro("DECLARE_VECTOR_REGISTER")
#undef DECLARE_VECTOR_REGISTER

#pragma push_macro("VectorZero")
#undef VectorZero

#pragma push_macro("VectorOne")
#undef VectorOne

#pragma push_macro("VectorLoad")
#undef VectorLoad

#pragma push_macro("VectorLoadFloat3")
#undef VectorLoadFloat3

#pragma push_macro("VectorLoadFloat3_W0")
#undef VectorLoadFloat3_W0

#pragma push_macro("VectorLoadFloat3_W1")
#undef VectorLoadFloat3_W1

#pragma push_macro("VectorLoadAligned")
#undef VectorLoadAligned

#pragma push_macro("VectorLoadFloat1")
#undef VectorLoadFloat1

#pragma push_macro("VectorLoadFloat2")
#undef VectorLoadFloat2

#pragma push_macro("VectorSetFloat3")
#undef VectorSetFloat3

#pragma push_macro("VectorSetFloat1")
#undef VectorSetFloat1

#pragma push_macro("VectorSet")
#undef VectorSet

#pragma push_macro("VectorStoreAligned")
#undef VectorStoreAligned

#pragma push_macro("VectorStoreAlignedStreamed")
#undef VectorStoreAlignedStreamed

#pragma push_macro("VectorStore")
#undef VectorStore

#pragma push_macro("VectorStoreFloat3")
#undef VectorStoreFloat3

#pragma push_macro("VectorStoreFloat1")
#undef VectorStoreFloat1

#pragma push_macro("VectorReplicate")
#undef VectorReplicate

#pragma push_macro("VectorAbs")
#undef VectorAbs

#pragma push_macro("VectorNegate")
#undef VectorNegate

#pragma push_macro("VectorAdd")
#undef VectorAdd

#pragma push_macro("VectorSubtract")
#undef VectorSubtract

#pragma push_macro("VectorMultiply")
#undef VectorMultiply

#pragma push_macro("VectorDivide")
#undef VectorDivide

#pragma push_macro("VectorMultiplyAdd")
#undef VectorMultiplyAdd

#pragma push_macro("VectorDot3")
#undef VectorDot3

#pragma push_macro("VectorDot4")
#undef VectorDot4

#pragma push_macro("VectorCompareEQ")
#undef VectorCompareEQ

#pragma push_macro("VectorCompareNE")
#undef VectorCompareNE

#pragma push_macro("VectorCompareGT")
#undef VectorCompareGT

#pragma push_macro("VectorCompareGE")
#undef VectorCompareGE

#pragma push_macro("VectorCompareLT")
#undef VectorCompareLT

#pragma push_macro("VectorCompareLE")
#undef VectorCompareLE

#pragma push_macro("VectorSelect")
#undef VectorSelect

#pragma push_macro("VectorBitwiseOr")
#undef VectorBitwiseOr

#pragma push_macro("VectorBitwiseAnd")
#undef VectorBitwiseAnd

#pragma push_macro("VectorBitwiseXor")
#undef VectorBitwiseXor

#pragma push_macro("VectorMaskBits")
#undef VectorMaskBits

#pragma push_macro("VectorCross")
#undef VectorCross

#pragma push_macro("VectorPow")
#undef VectorPow

#pragma push_macro("VectorReciprocalSqrt")
#undef VectorReciprocalSqrt

#pragma push_macro("VectorReciprocal")
#undef VectorReciprocal

#pragma push_macro("VectorReciprocalLen")
#undef VectorReciprocalLen

#pragma push_macro("VectorReciprocalSqrtAccurate")
#undef VectorReciprocalSqrtAccurate

#pragma push_macro("VectorReciprocalAccurate")
#undef VectorReciprocalAccurate

#pragma push_macro("VectorNormalize")
#undef VectorNormalize

#pragma push_macro("VectorSet_W0")
#undef VectorSet_W0

#pragma push_macro("VectorSet_W1")
#undef VectorSet_W1

#pragma push_macro("VectorMin")
#undef VectorMin

#pragma push_macro("VectorMax")
#undef VectorMax

#pragma push_macro("VectorSwizzle")
#undef VectorSwizzle

#pragma push_macro("VectorShuffle")
#undef VectorShuffle

#pragma push_macro("VectorMask_LT")
#undef VectorMask_LT

#pragma push_macro("VectorMask_LE")
#undef VectorMask_LE

#pragma push_macro("VectorMask_GT")
#undef VectorMask_GT

#pragma push_macro("VectorMask_GE")
#undef VectorMask_GE

#pragma push_macro("VectorMask_EQ")
#undef VectorMask_EQ

#pragma push_macro("VectorMask_NE")
#undef VectorMask_NE

#pragma push_macro("VectorLoadByte4")
#undef VectorLoadByte4

#pragma push_macro("VectorLoadSignedByte4")
#undef VectorLoadSignedByte4

#pragma push_macro("VectorStoreByte4")
#undef VectorStoreByte4

#pragma push_macro("VectorStoreSignedByte4")
#undef VectorStoreSignedByte4

#pragma push_macro("VectorLoadURGB10A2N")
#undef VectorLoadURGB10A2N

#pragma push_macro("VectorStoreURGB10A2N")
#undef VectorStoreURGB10A2N

#pragma push_macro("VectorLoadURGBA16N")
#undef VectorLoadURGBA16N

#pragma push_macro("VectorLoadSRGBA16N")
#undef VectorLoadSRGBA16N

#pragma push_macro("VectorStoreURGBA16N")
#undef VectorStoreURGBA16N

#pragma push_macro("VectorResetFloatRegisters")
#undef VectorResetFloatRegisters

#pragma push_macro("VectorGetControlRegister")
#undef VectorGetControlRegister

#pragma push_macro("VectorIntAnd")
#undef VectorIntAnd

#pragma push_macro("VectorIntOr")
#undef VectorIntOr

#pragma push_macro("VectorIntXor")
#undef VectorIntXor

#pragma push_macro("VectorIntAndNot")
#undef VectorIntAndNot

#pragma push_macro("VectorIntNot")
#undef VectorIntNot

#pragma push_macro("VectorIntCompareEQ")
#undef VectorIntCompareEQ

#pragma push_macro("VectorIntCompareNEQ")
#undef VectorIntCompareNEQ

#pragma push_macro("VectorIntCompareGT")
#undef VectorIntCompareGT

#pragma push_macro("VectorIntCompareLT")
#undef VectorIntCompareLT

#pragma push_macro("VectorIntCompareGE")
#undef VectorIntCompareGE

#pragma push_macro("VectorIntCompareLE")
#undef VectorIntCompareLE

#pragma push_macro("VectorIntAdd")
#undef VectorIntAdd

#pragma push_macro("VectorIntSubtract")
#undef VectorIntSubtract

#pragma push_macro("VectorIntNegate")
#undef VectorIntNegate

#pragma push_macro("VectorIntSign")
#undef VectorIntSign

#pragma push_macro("VectorIntToFloat")
#undef VectorIntToFloat

#pragma push_macro("VectorFloatToInt")
#undef VectorFloatToInt

#pragma push_macro("VectorIntStore")
#undef VectorIntStore

#pragma push_macro("VectorIntLoad")
#undef VectorIntLoad

#pragma push_macro("VectorIntStoreAligned")
#undef VectorIntStoreAligned

#pragma push_macro("VectorIntLoadAligned")
#undef VectorIntLoadAligned

#pragma push_macro("VectorIntLoad1")
#undef VectorIntLoad1

#pragma push_macro("VectorLoadByte4Reverse")
#undef VectorLoadByte4Reverse

#pragma push_macro("VectorPermute")
#undef VectorPermute

#pragma push_macro("VectorSetComponent")
#undef VectorSetComponent

#pragma push_macro("VectorIntMultiply")
#undef VectorIntMultiply

#pragma push_macro("VectorIntMin")
#undef VectorIntMin

#pragma push_macro("VectorIntMax")
#undef VectorIntMax

#pragma push_macro("VectorIntAbs")
#undef VectorIntAbs

#pragma push_macro("SHUFFLEMASK")
#undef SHUFFLEMASK

#pragma push_macro("VectorMergeVecXYZ_VecW")
#undef VectorMergeVecXYZ_VecW

#pragma push_macro("VectorAnyGreaterThan")
#undef VectorAnyGreaterThan

#pragma push_macro("INTEL_ORDER_VECTOR")
#undef INTEL_ORDER_VECTOR

#pragma push_macro("MEMPRO_TRACK_ALLOC")
#undef MEMPRO_TRACK_ALLOC

#pragma push_macro("MEMPRO_TRACK_FREE")
#undef MEMPRO_TRACK_FREE

#pragma push_macro("MEMPRO_ASSERT")
#undef MEMPRO_ASSERT

#pragma push_macro("MEMPRO_DISABLE_WARNING")
#undef MEMPRO_DISABLE_WARNING

#pragma push_macro("MEMPRO_ALIGN_SUFFIX")
#undef MEMPRO_ALIGN_SUFFIX

#pragma push_macro("_DebugBreakAndPromptForRemote")
#undef _DebugBreakAndPromptForRemote

#pragma push_macro("checkCode")
#undef checkCode

#pragma push_macro("verify")
#undef verify

#pragma push_macro("check")
#undef check

#pragma push_macro("UE_CHECK_IMPL")
#undef UE_CHECK_IMPL

#pragma push_macro("verifyf")
#undef verifyf

#pragma push_macro("checkf")
#undef checkf

#pragma push_macro("UE_CHECK_F_IMPL")
#undef UE_CHECK_F_IMPL

#pragma push_macro("checkNoEntry")
#undef checkNoEntry

#pragma push_macro("checkNoReentry")
#undef checkNoReentry

#pragma push_macro("checkNoRecursion")
#undef checkNoRecursion

#pragma push_macro("unimplemented")
#undef unimplemented

#pragma push_macro("checkSlow")
#undef checkSlow

#pragma push_macro("checkfSlow")
#undef checkfSlow

#pragma push_macro("verifySlow")
#undef verifySlow

#pragma push_macro("UE_ENSURE_IMPL")
#undef UE_ENSURE_IMPL

#pragma push_macro("ensure")
#undef ensure

#pragma push_macro("ensureMsgf")
#undef ensureMsgf

#pragma push_macro("ensureAlways")
#undef ensureAlways

#pragma push_macro("ensureAlwaysMsgf")
#undef ensureAlwaysMsgf

#pragma push_macro("GET_ENUMERATOR_NAME_CHECKED")
#undef GET_ENUMERATOR_NAME_CHECKED

#pragma push_macro("GET_MEMBER_NAME_CHECKED")
#undef GET_MEMBER_NAME_CHECKED

#pragma push_macro("GET_MEMBER_NAME_STRING_CHECKED")
#undef GET_MEMBER_NAME_STRING_CHECKED

#pragma push_macro("GET_FUNCTION_NAME_CHECKED")
#undef GET_FUNCTION_NAME_CHECKED

#pragma push_macro("GET_FUNCTION_NAME_STRING_CHECKED")
#undef GET_FUNCTION_NAME_STRING_CHECKED

#pragma push_macro("LowLevelFatalError")
#undef LowLevelFatalError

#pragma push_macro("TestTrueExpr")
#undef TestTrueExpr

#pragma push_macro("DEFINE_LATENT_AUTOMATION_COMMAND")
#undef DEFINE_LATENT_AUTOMATION_COMMAND

#pragma push_macro("DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER")
#undef DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER

#pragma push_macro("DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER")
#undef DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER

#pragma push_macro("DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER")
#undef DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER

#pragma push_macro("DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER")
#undef DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER

#pragma push_macro("DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER")
#undef DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER

#pragma push_macro("DEFINE_EXPORTED_LATENT_AUTOMATION_COMMAND")
#undef DEFINE_EXPORTED_LATENT_AUTOMATION_COMMAND

#pragma push_macro("DEFINE_EXPORTED_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER")
#undef DEFINE_EXPORTED_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER

#pragma push_macro("DEFINE_ENGINE_LATENT_AUTOMATION_COMMAND")
#undef DEFINE_ENGINE_LATENT_AUTOMATION_COMMAND

#pragma push_macro("DEFINE_ENGINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER")
#undef DEFINE_ENGINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER

#pragma push_macro("ADD_LATENT_AUTOMATION_COMMAND")
#undef ADD_LATENT_AUTOMATION_COMMAND

#pragma push_macro("START_NETWORK_AUTOMATION_COMMAND")
#undef START_NETWORK_AUTOMATION_COMMAND

#pragma push_macro("END_NETWORK_AUTOMATION_COMMAND")
#undef END_NETWORK_AUTOMATION_COMMAND

#pragma push_macro("IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE")
#undef IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE

#pragma push_macro("IMPLEMENT_COMPLEX_AUTOMATION_TEST_PRIVATE")
#undef IMPLEMENT_COMPLEX_AUTOMATION_TEST_PRIVATE

#pragma push_macro("IMPLEMENT_NETWORKED_AUTOMATION_TEST_PRIVATE")
#undef IMPLEMENT_NETWORKED_AUTOMATION_TEST_PRIVATE

#pragma push_macro("IMPLEMENT_BDD_AUTOMATION_TEST_PRIVATE")
#undef IMPLEMENT_BDD_AUTOMATION_TEST_PRIVATE

#pragma push_macro("DEFINE_SPEC_PRIVATE")
#undef DEFINE_SPEC_PRIVATE

#pragma push_macro("BEGIN_DEFINE_SPEC_PRIVATE")
#undef BEGIN_DEFINE_SPEC_PRIVATE

#pragma push_macro("IMPLEMENT_SIMPLE_AUTOMATION_TEST")
#undef IMPLEMENT_SIMPLE_AUTOMATION_TEST

#pragma push_macro("IMPLEMENT_COMPLEX_AUTOMATION_TEST")
#undef IMPLEMENT_COMPLEX_AUTOMATION_TEST

#pragma push_macro("IMPLEMENT_COMPLEX_AUTOMATION_CLASS")
#undef IMPLEMENT_COMPLEX_AUTOMATION_CLASS

#pragma push_macro("IMPLEMENT_NETWORKED_AUTOMATION_TEST")
#undef IMPLEMENT_NETWORKED_AUTOMATION_TEST

#pragma push_macro("IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST")
#undef IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST

#pragma push_macro("IMPLEMENT_CUSTOM_COMPLEX_AUTOMATION_TEST")
#undef IMPLEMENT_CUSTOM_COMPLEX_AUTOMATION_TEST

#pragma push_macro("IMPLEMENT_BDD_AUTOMATION_TEST")
#undef IMPLEMENT_BDD_AUTOMATION_TEST

#pragma push_macro("DEFINE_SPEC")
#undef DEFINE_SPEC

#pragma push_macro("BEGIN_DEFINE_SPEC")
#undef BEGIN_DEFINE_SPEC

#pragma push_macro("END_DEFINE_SPEC")
#undef END_DEFINE_SPEC

#pragma push_macro("BEGIN_CUSTOM_COMPLEX_AUTOMATION_TEST")
#undef BEGIN_CUSTOM_COMPLEX_AUTOMATION_TEST

#pragma push_macro("END_CUSTOM_COMPLEX_AUTOMATION_TEST")
#undef END_CUSTOM_COMPLEX_AUTOMATION_TEST

#pragma push_macro("UTEST_EQUAL")
#undef UTEST_EQUAL

#pragma push_macro("UTEST_EQUAL_TOLERANCE")
#undef UTEST_EQUAL_TOLERANCE

#pragma push_macro("UTEST_EQUAL_INSENSITIVE")
#undef UTEST_EQUAL_INSENSITIVE

#pragma push_macro("UTEST_NOT_EQUAL")
#undef UTEST_NOT_EQUAL

#pragma push_macro("UTEST_SAME")
#undef UTEST_SAME

#pragma push_macro("UTEST_NOT_SAME")
#undef UTEST_NOT_SAME

#pragma push_macro("UTEST_TRUE")
#undef UTEST_TRUE

#pragma push_macro("UTEST_FALSE")
#undef UTEST_FALSE

#pragma push_macro("UTEST_VALID")
#undef UTEST_VALID

#pragma push_macro("UTEST_INVALID")
#undef UTEST_INVALID

#pragma push_macro("UTEST_NULL")
#undef UTEST_NULL

#pragma push_macro("UTEST_NOT_NULL")
#undef UTEST_NOT_NULL

#pragma push_macro("BYTESWAP_ORDER16_unsigned")
#undef BYTESWAP_ORDER16_unsigned

#pragma push_macro("BYTESWAP_ORDER32_unsigned")
#undef BYTESWAP_ORDER32_unsigned

#pragma push_macro("UE_BYTESWAP_INTRINSIC_PRIVATE_16")
#undef UE_BYTESWAP_INTRINSIC_PRIVATE_16

#pragma push_macro("UE_BYTESWAP_INTRINSIC_PRIVATE_32")
#undef UE_BYTESWAP_INTRINSIC_PRIVATE_32

#pragma push_macro("UE_BYTESWAP_INTRINSIC_PRIVATE_64")
#undef UE_BYTESWAP_INTRINSIC_PRIVATE_64

#pragma push_macro("INTEL_ORDER16")
#undef INTEL_ORDER16

#pragma push_macro("INTEL_ORDER32")
#undef INTEL_ORDER32

#pragma push_macro("INTEL_ORDERF")
#undef INTEL_ORDERF

#pragma push_macro("INTEL_ORDER64")
#undef INTEL_ORDER64

#pragma push_macro("INTEL_ORDER_TCHARARRAY")
#undef INTEL_ORDER_TCHARARRAY

#pragma push_macro("NETWORK_ORDER16")
#undef NETWORK_ORDER16

#pragma push_macro("NETWORK_ORDER32")
#undef NETWORK_ORDER32

#pragma push_macro("NETWORK_ORDERF")
#undef NETWORK_ORDERF

#pragma push_macro("NETWORK_ORDER64")
#undef NETWORK_ORDER64

#pragma push_macro("NETWORK_ORDER_TCHARARRAY")
#undef NETWORK_ORDER_TCHARARRAY

#pragma push_macro("LITERAL")
#undef LITERAL

#pragma push_macro("WhitelistCommandLines")
#undef WhitelistCommandLines

#pragma push_macro("STUBBED")
#undef STUBBED

#pragma push_macro("CLOCK_CYCLES")
#undef CLOCK_CYCLES

#pragma push_macro("UNCLOCK_CYCLES")
#undef UNCLOCK_CYCLES

#pragma push_macro("RETURN_VAL_IF_EXIT_REQUESTED")
#undef RETURN_VAL_IF_EXIT_REQUESTED

#pragma push_macro("PURE_VIRTUAL")
#undef PURE_VIRTUAL

#pragma push_macro("WARNING_LOCATION")
#undef WARNING_LOCATION

#pragma push_macro("PUSH_MACRO")
#undef PUSH_MACRO

#pragma push_macro("POP_MACRO")
#undef POP_MACRO

#pragma push_macro("ANONYMOUS_VARIABLE")
#undef ANONYMOUS_VARIABLE

#pragma push_macro("UE_DEPRECATED")
#undef UE_DEPRECATED

#pragma push_macro("UE_DEPRECATED_FORGAME")
#undef UE_DEPRECATED_FORGAME

#pragma push_macro("UE_STATIC_DEPRECATE")
#undef UE_STATIC_DEPRECATE

#pragma push_macro("UE_PTRDIFF_TO_INT32")
#undef UE_PTRDIFF_TO_INT32

#pragma push_macro("UE_PTRDIFF_TO_UINT32")
#undef UE_PTRDIFF_TO_UINT32

#pragma push_macro("UE_NONCOPYABLE")
#undef UE_NONCOPYABLE

#pragma push_macro("UE_GREATER_SORT")
#undef UE_GREATER_SORT

#pragma push_macro("UE_VERSION_NEWER_THAN")
#undef UE_VERSION_NEWER_THAN

#pragma push_macro("UE_VERSION_OLDER_THAN")
#undef UE_VERSION_OLDER_THAN

#pragma push_macro("ENUM_CLASS_FLAGS")
#undef ENUM_CLASS_FLAGS

#pragma push_macro("FRIEND_ENUM_CLASS_FLAGS")
#undef FRIEND_ENUM_CLASS_FLAGS

#pragma push_macro("ENUM_RANGE_BY_COUNT")
#undef ENUM_RANGE_BY_COUNT

#pragma push_macro("ENUM_RANGE_BY_FIRST_AND_LAST")
#undef ENUM_RANGE_BY_FIRST_AND_LAST

#pragma push_macro("ENUM_RANGE_BY_VALUES")
#undef ENUM_RANGE_BY_VALUES

#pragma push_macro("DEFINE_EXPRESSION_NODE_TYPE")
#undef DEFINE_EXPRESSION_NODE_TYPE

#pragma push_macro("MONOLITHIC_HEADER_BOILERPLATE")
#undef MONOLITHIC_HEADER_BOILERPLATE

#pragma push_macro("SET_WARN_COLOR")
#undef SET_WARN_COLOR

#pragma push_macro("SET_WARN_COLOR_AND_BACKGROUND")
#undef SET_WARN_COLOR_AND_BACKGROUND

#pragma push_macro("CLEAR_WARN_COLOR")
#undef CLEAR_WARN_COLOR

#pragma push_macro("LogRuntimeError")
#undef LogRuntimeError

#pragma push_macro("LogRuntimeWarning")
#undef LogRuntimeWarning

#pragma push_macro("ensureAsRuntimeWarning")
#undef ensureAsRuntimeWarning

#pragma push_macro("UE_STATIC_ASSERT_COMPLETE_TYPE")
#undef UE_STATIC_ASSERT_COMPLETE_TYPE

#pragma push_macro("DEFINE_TEXT_EXPRESSION_OPERATOR_NODE")
#undef DEFINE_TEXT_EXPRESSION_OPERATOR_NODE

#pragma push_macro("SCOPE_TIME_GUARD")
#undef SCOPE_TIME_GUARD

#pragma push_macro("SCOPE_TIME_GUARD_MS")
#undef SCOPE_TIME_GUARD_MS

#pragma push_macro("SCOPE_TIME_GUARD_NAMED")
#undef SCOPE_TIME_GUARD_NAMED

#pragma push_macro("SCOPE_TIME_GUARD_NAMED_MS")
#undef SCOPE_TIME_GUARD_NAMED_MS

#pragma push_macro("SCOPE_TIME_GUARD_DELEGATE")
#undef SCOPE_TIME_GUARD_DELEGATE

#pragma push_macro("SCOPE_TIME_GUARD_DELEGATE_MS")
#undef SCOPE_TIME_GUARD_DELEGATE_MS

#pragma push_macro("ENABLE_TIME_GUARDS")
#undef ENABLE_TIME_GUARDS

#pragma push_macro("CLEAR_TIME_GUARDS")
#undef CLEAR_TIME_GUARDS

#pragma push_macro("LIGHTWEIGHT_TIME_GUARD_BEGIN")
#undef LIGHTWEIGHT_TIME_GUARD_BEGIN

#pragma push_macro("LIGHTWEIGHT_TIME_GUARD_END")
#undef LIGHTWEIGHT_TIME_GUARD_END

#pragma push_macro("VARARG_DECL")
#undef VARARG_DECL

#pragma push_macro("VARARG_BODY")
#undef VARARG_BODY

#pragma push_macro("GET_VARARGS")
#undef GET_VARARGS

#pragma push_macro("GET_VARARGS_WIDE")
#undef GET_VARARGS_WIDE

#pragma push_macro("GET_VARARGS_ANSI")
#undef GET_VARARGS_ANSI

#pragma push_macro("GET_VARARGS_RESULT")
#undef GET_VARARGS_RESULT

#pragma push_macro("GET_VARARGS_RESULT_WIDE")
#undef GET_VARARGS_RESULT_WIDE

#pragma push_macro("GET_VARARGS_RESULT_ANSI")
#undef GET_VARARGS_RESULT_ANSI

#pragma push_macro("VARARG_EXTRA")
#undef VARARG_EXTRA

#pragma push_macro("IMPLEMENT_MODULE")
#undef IMPLEMENT_MODULE

#pragma push_macro("IMPLEMENT_GAME_MODULE")
#undef IMPLEMENT_GAME_MODULE

#pragma push_macro("IMPLEMENT_FOREIGN_ENGINE_DIR")
#undef IMPLEMENT_FOREIGN_ENGINE_DIR

#pragma push_macro("IMPLEMENT_LIVE_CODING_ENGINE_DIR")
#undef IMPLEMENT_LIVE_CODING_ENGINE_DIR

#pragma push_macro("IMPLEMENT_LIVE_CODING_PROJECT")
#undef IMPLEMENT_LIVE_CODING_PROJECT

#pragma push_macro("UE_LIST_ARGUMENT")
#undef UE_LIST_ARGUMENT

#pragma push_macro("UE_REGISTER_SIGNING_KEY")
#undef UE_REGISTER_SIGNING_KEY

#pragma push_macro("UE_REGISTER_ENCRYPTION_KEY")
#undef UE_REGISTER_ENCRYPTION_KEY

#pragma push_macro("IMPLEMENT_TARGET_NAME_REGISTRATION")
#undef IMPLEMENT_TARGET_NAME_REGISTRATION

#pragma push_macro("IMPLEMENT_APPLICATION")
#undef IMPLEMENT_APPLICATION

#pragma push_macro("IMPLEMENT_PRIMARY_GAME_MODULE")
#undef IMPLEMENT_PRIMARY_GAME_MODULE

#pragma push_macro("PER_MODULE_BOILERPLATE_ANYLINK")
#undef PER_MODULE_BOILERPLATE_ANYLINK

#pragma push_macro("SCOPED_ABTEST")
#undef SCOPED_ABTEST

#pragma push_macro("SCOPED_ABTEST_DOFIRSTTEST")
#undef SCOPED_ABTEST_DOFIRSTTEST

#pragma push_macro("COOK_STAT")
#undef COOK_STAT

#pragma push_macro("__TRACE_DECLARE_INLINE_COUNTER")
#undef __TRACE_DECLARE_INLINE_COUNTER

#pragma push_macro("TRACE_INT_VALUE")
#undef TRACE_INT_VALUE

#pragma push_macro("TRACE_FLOAT_VALUE")
#undef TRACE_FLOAT_VALUE

#pragma push_macro("TRACE_MEMORY_VALUE")
#undef TRACE_MEMORY_VALUE

#pragma push_macro("TRACE_DECLARE_INT_COUNTER")
#undef TRACE_DECLARE_INT_COUNTER

#pragma push_macro("TRACE_DECLARE_INT_COUNTER_EXTERN")
#undef TRACE_DECLARE_INT_COUNTER_EXTERN

#pragma push_macro("TRACE_DECLARE_FLOAT_COUNTER")
#undef TRACE_DECLARE_FLOAT_COUNTER

#pragma push_macro("TRACE_DECLARE_FLOAT_COUNTER_EXTERN")
#undef TRACE_DECLARE_FLOAT_COUNTER_EXTERN

#pragma push_macro("TRACE_DECLARE_MEMORY_COUNTER")
#undef TRACE_DECLARE_MEMORY_COUNTER

#pragma push_macro("TRACE_DECLARE_MEMORY_COUNTER_EXTERN")
#undef TRACE_DECLARE_MEMORY_COUNTER_EXTERN

#pragma push_macro("TRACE_COUNTER_SET")
#undef TRACE_COUNTER_SET

#pragma push_macro("TRACE_COUNTER_ADD")
#undef TRACE_COUNTER_ADD

#pragma push_macro("TRACE_COUNTER_SUBTRACT")
#undef TRACE_COUNTER_SUBTRACT

#pragma push_macro("TRACE_COUNTER_INCREMENT")
#undef TRACE_COUNTER_INCREMENT

#pragma push_macro("TRACE_COUNTER_DECREMENT")
#undef TRACE_COUNTER_DECREMENT

#pragma push_macro("TRACE_CPUPROFILER_SHUTDOWN")
#undef TRACE_CPUPROFILER_SHUTDOWN

#pragma push_macro("TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL_STR")
#undef TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL_STR

#pragma push_macro("TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL")
#undef TRACE_CPUPROFILER_EVENT_SCOPE_ON_CHANNEL

#pragma push_macro("TRACE_CPUPROFILER_EVENT_SCOPE_STR")
#undef TRACE_CPUPROFILER_EVENT_SCOPE_STR

#pragma push_macro("TRACE_CPUPROFILER_EVENT_SCOPE")
#undef TRACE_CPUPROFILER_EVENT_SCOPE

#pragma push_macro("TRACE_CPUPROFILER_EVENT_SCOPE_TEXT_ON_CHANNEL")
#undef TRACE_CPUPROFILER_EVENT_SCOPE_TEXT_ON_CHANNEL

#pragma push_macro("TRACE_CPUPROFILER_EVENT_SCOPE_TEXT")
#undef TRACE_CPUPROFILER_EVENT_SCOPE_TEXT

#pragma push_macro("CSV_CATEGORY_INDEX")
#undef CSV_CATEGORY_INDEX

#pragma push_macro("CSV_STAT_FNAME")
#undef CSV_STAT_FNAME

#pragma push_macro("CSV_SCOPED_TIMING_STAT")
#undef CSV_SCOPED_TIMING_STAT

#pragma push_macro("CSV_SCOPED_TIMING_STAT_GLOBAL")
#undef CSV_SCOPED_TIMING_STAT_GLOBAL

#pragma push_macro("CSV_SCOPED_TIMING_STAT_EXCLUSIVE")
#undef CSV_SCOPED_TIMING_STAT_EXCLUSIVE

#pragma push_macro("CSV_SCOPED_TIMING_STAT_EXCLUSIVE_CONDITIONAL")
#undef CSV_SCOPED_TIMING_STAT_EXCLUSIVE_CONDITIONAL

#pragma push_macro("CSV_SCOPED_WAIT_CONDITIONAL")
#undef CSV_SCOPED_WAIT_CONDITIONAL

#pragma push_macro("CSV_SCOPED_SET_WAIT_STAT")
#undef CSV_SCOPED_SET_WAIT_STAT

#pragma push_macro("CSV_SCOPED_SET_WAIT_STAT_IGNORE")
#undef CSV_SCOPED_SET_WAIT_STAT_IGNORE

#pragma push_macro("CSV_CUSTOM_STAT")
#undef CSV_CUSTOM_STAT

#pragma push_macro("CSV_CUSTOM_STAT_GLOBAL")
#undef CSV_CUSTOM_STAT_GLOBAL

#pragma push_macro("CSV_DEFINE_STAT")
#undef CSV_DEFINE_STAT

#pragma push_macro("CSV_DEFINE_STAT_GLOBAL")
#undef CSV_DEFINE_STAT_GLOBAL

#pragma push_macro("CSV_DECLARE_STAT_EXTERN")
#undef CSV_DECLARE_STAT_EXTERN

#pragma push_macro("CSV_CUSTOM_STAT_DEFINED")
#undef CSV_CUSTOM_STAT_DEFINED

#pragma push_macro("CSV_DEFINE_CATEGORY")
#undef CSV_DEFINE_CATEGORY

#pragma push_macro("CSV_DECLARE_CATEGORY_EXTERN")
#undef CSV_DECLARE_CATEGORY_EXTERN

#pragma push_macro("CSV_DEFINE_CATEGORY_MODULE")
#undef CSV_DEFINE_CATEGORY_MODULE

#pragma push_macro("CSV_DECLARE_CATEGORY_MODULE_EXTERN")
#undef CSV_DECLARE_CATEGORY_MODULE_EXTERN

#pragma push_macro("CSV_EVENT")
#undef CSV_EVENT

#pragma push_macro("CSV_EVENT_GLOBAL")
#undef CSV_EVENT_GLOBAL

#pragma push_macro("CSV_METADATA")
#undef CSV_METADATA

#pragma push_macro("TRACE_CSV_PROFILER_REGISTER_CATEGORY")
#undef TRACE_CSV_PROFILER_REGISTER_CATEGORY

#pragma push_macro("TRACE_CSV_PROFILER_INLINE_STAT")
#undef TRACE_CSV_PROFILER_INLINE_STAT

#pragma push_macro("TRACE_CSV_PROFILER_INLINE_STAT_EXCLUSIVE")
#undef TRACE_CSV_PROFILER_INLINE_STAT_EXCLUSIVE

#pragma push_macro("TRACE_CSV_PROFILER_DECLARED_STAT")
#undef TRACE_CSV_PROFILER_DECLARED_STAT

#pragma push_macro("TRACE_CSV_PROFILER_BEGIN_STAT")
#undef TRACE_CSV_PROFILER_BEGIN_STAT

#pragma push_macro("TRACE_CSV_PROFILER_END_STAT")
#undef TRACE_CSV_PROFILER_END_STAT

#pragma push_macro("TRACE_CSV_PROFILER_BEGIN_EXCLUSIVE_STAT")
#undef TRACE_CSV_PROFILER_BEGIN_EXCLUSIVE_STAT

#pragma push_macro("TRACE_CSV_PROFILER_END_EXCLUSIVE_STAT")
#undef TRACE_CSV_PROFILER_END_EXCLUSIVE_STAT

#pragma push_macro("TRACE_CSV_PROFILER_CUSTOM_STAT")
#undef TRACE_CSV_PROFILER_CUSTOM_STAT

#pragma push_macro("TRACE_CSV_PROFILER_EVENT")
#undef TRACE_CSV_PROFILER_EVENT

#pragma push_macro("TRACE_CSV_PROFILER_BEGIN_CAPTURE")
#undef TRACE_CSV_PROFILER_BEGIN_CAPTURE

#pragma push_macro("TRACE_CSV_PROFILER_END_CAPTURE")
#undef TRACE_CSV_PROFILER_END_CAPTURE

#pragma push_macro("TRACE_CSV_PROFILER_METADATA")
#undef TRACE_CSV_PROFILER_METADATA

#pragma push_macro("SCOPE_PROFILER_INCLUDER")
#undef SCOPE_PROFILER_INCLUDER

#pragma push_macro("SCOPE_PROFILER_EXCLUDER")
#undef SCOPE_PROFILER_EXCLUDER

#pragma push_macro("COUNT_INSTANCES")
#undef COUNT_INSTANCES

#pragma push_macro("COUNT_INSTANCES_AND_LOG")
#undef COUNT_INSTANCES_AND_LOG

#pragma push_macro("ACCUM_LOADTIME")
#undef ACCUM_LOADTIME

#pragma push_macro("SCOPED_ACCUM_LOADTIME")
#undef SCOPED_ACCUM_LOADTIME

#pragma push_macro("SCOPED_LOADTIMER_TEXT")
#undef SCOPED_LOADTIMER_TEXT

#pragma push_macro("SCOPED_LOADTIMER")
#undef SCOPED_LOADTIMER

#pragma push_macro("SCOPED_LOADTIMER_CNT")
#undef SCOPED_LOADTIMER_CNT

#pragma push_macro("ADD_CUSTOM_LOADTIMER_META")
#undef ADD_CUSTOM_LOADTIMER_META

#pragma push_macro("SCOPED_CUSTOM_LOADTIMER")
#undef SCOPED_CUSTOM_LOADTIMER

#pragma push_macro("SCOPED_ACCUM_LOADTIME_STAT")
#undef SCOPED_ACCUM_LOADTIME_STAT

#pragma push_macro("ACCUM_LOADTIMECOUNT_STAT")
#undef ACCUM_LOADTIMECOUNT_STAT

#pragma push_macro("TRACE_BOOKMARK")
#undef TRACE_BOOKMARK

#pragma push_macro("TRACE_BEGIN_FRAME")
#undef TRACE_BEGIN_FRAME

#pragma push_macro("TRACE_END_FRAME")
#undef TRACE_END_FRAME

#pragma push_macro("TRACE_PLATFORMFILE_BEGIN_OPEN")
#undef TRACE_PLATFORMFILE_BEGIN_OPEN

#pragma push_macro("TRACE_PLATFORMFILE_END_OPEN")
#undef TRACE_PLATFORMFILE_END_OPEN

#pragma push_macro("TRACE_PLATFORMFILE_FAIL_OPEN")
#undef TRACE_PLATFORMFILE_FAIL_OPEN

#pragma push_macro("TRACE_PLATFORMFILE_BEGIN_CLOSE")
#undef TRACE_PLATFORMFILE_BEGIN_CLOSE

#pragma push_macro("TRACE_PLATFORMFILE_END_CLOSE")
#undef TRACE_PLATFORMFILE_END_CLOSE

#pragma push_macro("TRACE_PLATFORMFILE_FAIL_CLOSE")
#undef TRACE_PLATFORMFILE_FAIL_CLOSE

#pragma push_macro("TRACE_PLATFORMFILE_BEGIN_READ")
#undef TRACE_PLATFORMFILE_BEGIN_READ

#pragma push_macro("TRACE_PLATFORMFILE_END_READ")
#undef TRACE_PLATFORMFILE_END_READ

#pragma push_macro("TRACE_PLATFORMFILE_BEGIN_WRITE")
#undef TRACE_PLATFORMFILE_BEGIN_WRITE

#pragma push_macro("TRACE_PLATFORMFILE_END_WRITE")
#undef TRACE_PLATFORMFILE_END_WRITE

#pragma push_macro("MALLOC_PROFILER")
#undef MALLOC_PROFILER

#pragma push_macro("FArchive_Serialize_BitfieldBool")
#undef FArchive_Serialize_BitfieldBool

#pragma push_macro("TRACE_LOADTIME_REQUEST_GROUP_SCOPE")
#undef TRACE_LOADTIME_REQUEST_GROUP_SCOPE

#pragma push_macro("UE_STATIC_ONLY")
#undef UE_STATIC_ONLY

#pragma push_macro("UE_DECLARE_INTERNAL_LINK_BASE")
#undef UE_DECLARE_INTERNAL_LINK_BASE

#pragma push_macro("UE_DECLARE_INTERNAL_LINK_SPECIALIZATION")
#undef UE_DECLARE_INTERNAL_LINK_SPECIALIZATION

#pragma push_macro("INTERNAL_LAYOUT_FIELD")
#undef INTERNAL_LAYOUT_FIELD

#pragma push_macro("INTERNAL_LAYOUT_FIELD_WITH_WRITER")
#undef INTERNAL_LAYOUT_FIELD_WITH_WRITER

#pragma push_macro("INTERNAL_LAYOUT_WRITE_MEMORY_IMAGE")
#undef INTERNAL_LAYOUT_WRITE_MEMORY_IMAGE

#pragma push_macro("INTERNAL_LAYOUT_TOSTRING")
#undef INTERNAL_LAYOUT_TOSTRING

#pragma push_macro("LAYOUT_FIELD")
#undef LAYOUT_FIELD

#pragma push_macro("LAYOUT_MUTABLE_FIELD")
#undef LAYOUT_MUTABLE_FIELD

#pragma push_macro("LAYOUT_FIELD_INITIALIZED")
#undef LAYOUT_FIELD_INITIALIZED

#pragma push_macro("LAYOUT_MUTABLE_FIELD_INITIALIZED")
#undef LAYOUT_MUTABLE_FIELD_INITIALIZED

#pragma push_macro("LAYOUT_ARRAY")
#undef LAYOUT_ARRAY

#pragma push_macro("LAYOUT_MUTABLE_BITFIELD")
#undef LAYOUT_MUTABLE_BITFIELD

#pragma push_macro("LAYOUT_BITFIELD")
#undef LAYOUT_BITFIELD

#pragma push_macro("LAYOUT_FIELD_WITH_WRITER")
#undef LAYOUT_FIELD_WITH_WRITER

#pragma push_macro("LAYOUT_MUTABLE_FIELD_WITH_WRITER")
#undef LAYOUT_MUTABLE_FIELD_WITH_WRITER

#pragma push_macro("LAYOUT_WRITE_MEMORY_IMAGE")
#undef LAYOUT_WRITE_MEMORY_IMAGE

#pragma push_macro("LAYOUT_TOSTRING")
#undef LAYOUT_TOSTRING

#pragma push_macro("LAYOUT_FIELD_EDITORONLY")
#undef LAYOUT_FIELD_EDITORONLY

#pragma push_macro("LAYOUT_ARRAY_EDITORONLY")
#undef LAYOUT_ARRAY_EDITORONLY

#pragma push_macro("LAYOUT_BITFIELD_EDITORONLY")
#undef LAYOUT_BITFIELD_EDITORONLY

#pragma push_macro("LAYOUT_FIELD_RAYTRACING")
#undef LAYOUT_FIELD_RAYTRACING

#pragma push_macro("LAYOUT_FIELD_INITIALIZED_RAYTRACING")
#undef LAYOUT_FIELD_INITIALIZED_RAYTRACING

#pragma push_macro("LAYOUT_ARRAY_RAYTRACING")
#undef LAYOUT_ARRAY_RAYTRACING

#pragma push_macro("INTERNAL_LAYOUT_INTERFACE_PREFIX_NonVirtual")
#undef INTERNAL_LAYOUT_INTERFACE_PREFIX_NonVirtual

#pragma push_macro("INTERNAL_LAYOUT_INTERFACE_PREFIX_Virtual")
#undef INTERNAL_LAYOUT_INTERFACE_PREFIX_Virtual

#pragma push_macro("INTERNAL_LAYOUT_INTERFACE_PREFIX_Abstract")
#undef INTERNAL_LAYOUT_INTERFACE_PREFIX_Abstract

#pragma push_macro("INTERNAL_LAYOUT_INTERFACE_PREFIX")
#undef INTERNAL_LAYOUT_INTERFACE_PREFIX

#pragma push_macro("INTERNAL_LAYOUT_INTERFACE_SUFFIX")
#undef INTERNAL_LAYOUT_INTERFACE_SUFFIX

#pragma push_macro("INTERNAL_LAYOUT_INTERFACE_INLINE_IMPL")
#undef INTERNAL_LAYOUT_INTERFACE_INLINE_IMPL

#pragma push_macro("INTERNAL_DECLARE_TYPE_LAYOUT_COMMON")
#undef INTERNAL_DECLARE_TYPE_LAYOUT_COMMON

#pragma push_macro("INTERNAL_DECLARE_INLINE_TYPE_LAYOUT")
#undef INTERNAL_DECLARE_INLINE_TYPE_LAYOUT

#pragma push_macro("INTERNAL_DECLARE_TYPE_LAYOUT")
#undef INTERNAL_DECLARE_TYPE_LAYOUT

#pragma push_macro("INTERNAL_DECLARE_LAYOUT_BASE")
#undef INTERNAL_DECLARE_LAYOUT_BASE

#pragma push_macro("INTERNAL_DECLARE_LAYOUT_EXPLICIT_BASES")
#undef INTERNAL_DECLARE_LAYOUT_EXPLICIT_BASES

#pragma push_macro("DECLARE_TYPE_LAYOUT")
#undef DECLARE_TYPE_LAYOUT

#pragma push_macro("DECLARE_INLINE_TYPE_LAYOUT")
#undef DECLARE_INLINE_TYPE_LAYOUT

#pragma push_macro("DECLARE_EXPORTED_TYPE_LAYOUT")
#undef DECLARE_EXPORTED_TYPE_LAYOUT

#pragma push_macro("DECLARE_TYPE_LAYOUT_EXPLICIT_BASES")
#undef DECLARE_TYPE_LAYOUT_EXPLICIT_BASES

#pragma push_macro("DECLARE_INLINE_TYPE_LAYOUT_EXPLICIT_BASES")
#undef DECLARE_INLINE_TYPE_LAYOUT_EXPLICIT_BASES

#pragma push_macro("DECLARE_EXPORTED_TYPE_LAYOUT_EXPLICIT_BASES")
#undef DECLARE_EXPORTED_TYPE_LAYOUT_EXPLICIT_BASES

#pragma push_macro("INTERNAL_IMPLEMENT_TYPE_LAYOUT_COMMON")
#undef INTERNAL_IMPLEMENT_TYPE_LAYOUT_COMMON

#pragma push_macro("INTERNAL_REGISTER_TYPE_LAYOUT")
#undef INTERNAL_REGISTER_TYPE_LAYOUT

#pragma push_macro("IMPLEMENT_UNREGISTERED_TEMPLATE_TYPE_LAYOUT")
#undef IMPLEMENT_UNREGISTERED_TEMPLATE_TYPE_LAYOUT

#pragma push_macro("IMPLEMENT_TEMPLATE_TYPE_LAYOUT")
#undef IMPLEMENT_TEMPLATE_TYPE_LAYOUT

#pragma push_macro("IMPLEMENT_TYPE_LAYOUT")
#undef IMPLEMENT_TYPE_LAYOUT

#pragma push_macro("IMPLEMENT_ABSTRACT_TYPE_LAYOUT")
#undef IMPLEMENT_ABSTRACT_TYPE_LAYOUT

#pragma push_macro("REGISTER_INLINE_TYPE_LAYOUT")
#undef REGISTER_INLINE_TYPE_LAYOUT

#pragma push_macro("DECLARE_TEMPLATE_INTRINSIC_TYPE_LAYOUT")
#undef DECLARE_TEMPLATE_INTRINSIC_TYPE_LAYOUT

#pragma push_macro("DECLARE_EXPORTED_TEMPLATE_INTRINSIC_TYPE_LAYOUT")
#undef DECLARE_EXPORTED_TEMPLATE_INTRINSIC_TYPE_LAYOUT

#pragma push_macro("IMPLEMENT_EXPORTED_INTRINSIC_TYPE_LAYOUT")
#undef IMPLEMENT_EXPORTED_INTRINSIC_TYPE_LAYOUT

#pragma push_macro("DECLARE_INTRINSIC_TYPE_LAYOUT")
#undef DECLARE_INTRINSIC_TYPE_LAYOUT

#pragma push_macro("ALIAS_TEMPLATE_TYPE_LAYOUT")
#undef ALIAS_TEMPLATE_TYPE_LAYOUT

#pragma push_macro("ALIAS_TYPE_LAYOUT")
#undef ALIAS_TYPE_LAYOUT

#pragma push_macro("SA_VALUE")
#undef SA_VALUE

#pragma push_macro("SA_ATTRIBUTE")
#undef SA_ATTRIBUTE

#pragma push_macro("SA_OPTIONAL_ATTRIBUTE")
#undef SA_OPTIONAL_ATTRIBUTE

#pragma push_macro("SA_FIELD_NAME")
#undef SA_FIELD_NAME

#pragma push_macro("STAT")
#undef STAT

#pragma push_macro("ANSI_TO_PROFILING")
#undef ANSI_TO_PROFILING

#pragma push_macro("DECLARE_SCOPE_CYCLE_COUNTER")
#undef DECLARE_SCOPE_CYCLE_COUNTER

#pragma push_macro("QUICK_SCOPE_CYCLE_COUNTER")
#undef QUICK_SCOPE_CYCLE_COUNTER

#pragma push_macro("SCOPE_CYCLE_COUNTER")
#undef SCOPE_CYCLE_COUNTER

#pragma push_macro("CONDITIONAL_SCOPE_CYCLE_COUNTER")
#undef CONDITIONAL_SCOPE_CYCLE_COUNTER

#pragma push_macro("RETURN_QUICK_DECLARE_CYCLE_STAT")
#undef RETURN_QUICK_DECLARE_CYCLE_STAT

#pragma push_macro("GET_STATID")
#undef GET_STATID

#pragma push_macro("SCOPE_SECONDS_ACCUMULATOR")
#undef SCOPE_SECONDS_ACCUMULATOR

#pragma push_macro("SCOPE_MS_ACCUMULATOR")
#undef SCOPE_MS_ACCUMULATOR

#pragma push_macro("DEFINE_STAT")
#undef DEFINE_STAT

#pragma push_macro("QUICK_USE_CYCLE_STAT")
#undef QUICK_USE_CYCLE_STAT

#pragma push_macro("DECLARE_CYCLE_STAT")
#undef DECLARE_CYCLE_STAT

#pragma push_macro("DECLARE_FLOAT_COUNTER_STAT")
#undef DECLARE_FLOAT_COUNTER_STAT

#pragma push_macro("DECLARE_DWORD_COUNTER_STAT")
#undef DECLARE_DWORD_COUNTER_STAT

#pragma push_macro("DECLARE_FLOAT_ACCUMULATOR_STAT")
#undef DECLARE_FLOAT_ACCUMULATOR_STAT

#pragma push_macro("DECLARE_DWORD_ACCUMULATOR_STAT")
#undef DECLARE_DWORD_ACCUMULATOR_STAT

#pragma push_macro("DECLARE_FNAME_STAT")
#undef DECLARE_FNAME_STAT

#pragma push_macro("DECLARE_PTR_STAT")
#undef DECLARE_PTR_STAT

#pragma push_macro("DECLARE_MEMORY_STAT")
#undef DECLARE_MEMORY_STAT

#pragma push_macro("DECLARE_MEMORY_STAT_POOL")
#undef DECLARE_MEMORY_STAT_POOL

#pragma push_macro("DECLARE_CYCLE_STAT_EXTERN")
#undef DECLARE_CYCLE_STAT_EXTERN

#pragma push_macro("DECLARE_FLOAT_COUNTER_STAT_EXTERN")
#undef DECLARE_FLOAT_COUNTER_STAT_EXTERN

#pragma push_macro("DECLARE_DWORD_COUNTER_STAT_EXTERN")
#undef DECLARE_DWORD_COUNTER_STAT_EXTERN

#pragma push_macro("DECLARE_FLOAT_ACCUMULATOR_STAT_EXTERN")
#undef DECLARE_FLOAT_ACCUMULATOR_STAT_EXTERN

#pragma push_macro("DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN")
#undef DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN

#pragma push_macro("DECLARE_FNAME_STAT_EXTERN")
#undef DECLARE_FNAME_STAT_EXTERN

#pragma push_macro("DECLARE_PTR_STAT_EXTERN")
#undef DECLARE_PTR_STAT_EXTERN

#pragma push_macro("DECLARE_MEMORY_STAT_EXTERN")
#undef DECLARE_MEMORY_STAT_EXTERN

#pragma push_macro("DECLARE_MEMORY_STAT_POOL_EXTERN")
#undef DECLARE_MEMORY_STAT_POOL_EXTERN

#pragma push_macro("DECLARE_STATS_GROUP")
#undef DECLARE_STATS_GROUP

#pragma push_macro("DECLARE_STATS_GROUP_VERBOSE")
#undef DECLARE_STATS_GROUP_VERBOSE

#pragma push_macro("DECLARE_STATS_GROUP_MAYBE_COMPILED_OUT")
#undef DECLARE_STATS_GROUP_MAYBE_COMPILED_OUT

#pragma push_macro("SET_CYCLE_COUNTER")
#undef SET_CYCLE_COUNTER

#pragma push_macro("INC_DWORD_STAT")
#undef INC_DWORD_STAT

#pragma push_macro("INC_FLOAT_STAT_BY")
#undef INC_FLOAT_STAT_BY

#pragma push_macro("INC_DWORD_STAT_BY")
#undef INC_DWORD_STAT_BY

#pragma push_macro("INC_DWORD_STAT_FNAME_BY")
#undef INC_DWORD_STAT_FNAME_BY

#pragma push_macro("INC_MEMORY_STAT_BY")
#undef INC_MEMORY_STAT_BY

#pragma push_macro("DEC_DWORD_STAT")
#undef DEC_DWORD_STAT

#pragma push_macro("DEC_FLOAT_STAT_BY")
#undef DEC_FLOAT_STAT_BY

#pragma push_macro("DEC_DWORD_STAT_BY")
#undef DEC_DWORD_STAT_BY

#pragma push_macro("DEC_DWORD_STAT_FNAME_BY")
#undef DEC_DWORD_STAT_FNAME_BY

#pragma push_macro("DEC_MEMORY_STAT_BY")
#undef DEC_MEMORY_STAT_BY

#pragma push_macro("SET_MEMORY_STAT")
#undef SET_MEMORY_STAT

#pragma push_macro("SET_DWORD_STAT")
#undef SET_DWORD_STAT

#pragma push_macro("SET_FLOAT_STAT")
#undef SET_FLOAT_STAT

#pragma push_macro("STAT_ADD_CUSTOMMESSAGE_NAME")
#undef STAT_ADD_CUSTOMMESSAGE_NAME

#pragma push_macro("STAT_ADD_CUSTOMMESSAGE_PTR")
#undef STAT_ADD_CUSTOMMESSAGE_PTR

#pragma push_macro("SET_CYCLE_COUNTER_FName")
#undef SET_CYCLE_COUNTER_FName

#pragma push_macro("INC_DWORD_STAT_FName")
#undef INC_DWORD_STAT_FName

#pragma push_macro("INC_FLOAT_STAT_BY_FName")
#undef INC_FLOAT_STAT_BY_FName

#pragma push_macro("INC_DWORD_STAT_BY_FName")
#undef INC_DWORD_STAT_BY_FName

#pragma push_macro("INC_MEMORY_STAT_BY_FName")
#undef INC_MEMORY_STAT_BY_FName

#pragma push_macro("DEC_DWORD_STAT_FName")
#undef DEC_DWORD_STAT_FName

#pragma push_macro("DEC_FLOAT_STAT_BY_FName")
#undef DEC_FLOAT_STAT_BY_FName

#pragma push_macro("DEC_DWORD_STAT_BY_FName")
#undef DEC_DWORD_STAT_BY_FName

#pragma push_macro("DEC_MEMORY_STAT_BY_FName")
#undef DEC_MEMORY_STAT_BY_FName

#pragma push_macro("SET_MEMORY_STAT_FName")
#undef SET_MEMORY_STAT_FName

#pragma push_macro("SET_DWORD_STAT_FName")
#undef SET_DWORD_STAT_FName

#pragma push_macro("SET_FLOAT_STAT_FName")
#undef SET_FLOAT_STAT_FName

#pragma push_macro("GET_STATFNAME")
#undef GET_STATFNAME

#pragma push_macro("GET_STATDESCRIPTION")
#undef GET_STATDESCRIPTION

#pragma push_macro("DECLARE_STAT_GROUP")
#undef DECLARE_STAT_GROUP

#pragma push_macro("DECLARE_STAT")
#undef DECLARE_STAT

#pragma push_macro("GET_STATISEVERYFRAME")
#undef GET_STATISEVERYFRAME

#pragma push_macro("STAT_GROUP_TO_FStatGroup")
#undef STAT_GROUP_TO_FStatGroup

#pragma push_macro("DECLARE_STATS_GROUP_SORTBYNAME")
#undef DECLARE_STATS_GROUP_SORTBYNAME

#pragma push_macro("checkStats")
#undef checkStats

#pragma push_macro("DECLARE_SCOPE_HIERARCHICAL_COUNTER")
#undef DECLARE_SCOPE_HIERARCHICAL_COUNTER

#pragma push_macro("DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC")
#undef DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC

#pragma push_macro("SCOPE_SECONDS_COUNTER_BASE")
#undef SCOPE_SECONDS_COUNTER_BASE

#pragma push_macro("SCOPE_SECONDS_COUNTER_RECURSION_SAFE_BASE")
#undef SCOPE_SECONDS_COUNTER_RECURSION_SAFE_BASE

#pragma push_macro("SCOPE_SECONDS_COUNTER")
#undef SCOPE_SECONDS_COUNTER

#pragma push_macro("SCOPE_SECONDS_COUNTER_RECURSION_SAFE")
#undef SCOPE_SECONDS_COUNTER_RECURSION_SAFE

#pragma push_macro("SCOPE_LOG_TIME")
#undef SCOPE_LOG_TIME

#pragma push_macro("SCOPE_LOG_TIME_IN_SECONDS")
#undef SCOPE_LOG_TIME_IN_SECONDS

#pragma push_macro("SCOPE_LOG_TIME_FUNC")
#undef SCOPE_LOG_TIME_FUNC

#pragma push_macro("SCOPE_LOG_TIME_FUNC_WITH_GLOBAL")
#undef SCOPE_LOG_TIME_FUNC_WITH_GLOBAL

#pragma push_macro("CONDITIONAL_SCOPE_LOG_TIME")
#undef CONDITIONAL_SCOPE_LOG_TIME

#pragma push_macro("CONDITIONAL_SCOPE_LOG_TIME_IN_SECONDS")
#undef CONDITIONAL_SCOPE_LOG_TIME_IN_SECONDS

#pragma push_macro("CONDITIONAL_SCOPE_LOG_TIME_FUNC")
#undef CONDITIONAL_SCOPE_LOG_TIME_FUNC

#pragma push_macro("CONDITIONAL_SCOPE_LOG_TIME_FUNC_WITH_GLOBAL")
#undef CONDITIONAL_SCOPE_LOG_TIME_FUNC_WITH_GLOBAL

#pragma push_macro("TRACE_STAT_INCREMENT")
#undef TRACE_STAT_INCREMENT

#pragma push_macro("TRACE_STAT_DECREMENT")
#undef TRACE_STAT_DECREMENT

#pragma push_macro("TRACE_STAT_ADD")
#undef TRACE_STAT_ADD

#pragma push_macro("TRACE_STAT_SET")
#undef TRACE_STAT_SET

#pragma push_macro("ALIGNOF")
#undef ALIGNOF

#pragma push_macro("ARE_TYPES_EQUAL")
#undef ARE_TYPES_EQUAL

#pragma push_macro("UE_PROJECTION")
#undef UE_PROJECTION

#pragma push_macro("UE_PROJECTION_MEMBER")
#undef UE_PROJECTION_MEMBER

#pragma push_macro("PROJECTION")
#undef PROJECTION

#pragma push_macro("PROJECTION_MEMBER")
#undef PROJECTION_MEMBER

#pragma push_macro("UE_TSHAREDPTR_STATIC_ASSERT_VALID_MODE")
#undef UE_TSHAREDPTR_STATIC_ASSERT_VALID_MODE

#pragma push_macro("IMPLEMENT_ALIGNED_STORAGE")
#undef IMPLEMENT_ALIGNED_STORAGE

#pragma push_macro("UE_ARRAY_COUNT")
#undef UE_ARRAY_COUNT

#pragma push_macro("ARRAY_COUNT")
#undef ARRAY_COUNT

#pragma push_macro("STRUCT_OFFSET")
#undef STRUCT_OFFSET

#pragma push_macro("VTABLE_OFFSET")
#undef VTABLE_OFFSET

#pragma push_macro("FGuardValue_Bitfield")
#undef FGuardValue_Bitfield

#pragma push_macro("TEMPLATE_PARAMETERS2")
#undef TEMPLATE_PARAMETERS2

#pragma push_macro("HAS_TRIVIAL_CONSTRUCTOR")
#undef HAS_TRIVIAL_CONSTRUCTOR

#pragma push_macro("IS_POD")
#undef IS_POD

#pragma push_macro("IS_EMPTY")
#undef IS_EMPTY

#pragma push_macro("Expose_TFormatSpecifier")
#undef Expose_TFormatSpecifier

#pragma push_macro("Expose_TNameOf")
#undef Expose_TNameOf

#pragma push_macro("GENERATE_MEMBER_FUNCTION_CHECK")
#undef GENERATE_MEMBER_FUNCTION_CHECK

#pragma push_macro("NAME_INTERNAL_TO_EXTERNAL")
#undef NAME_INTERNAL_TO_EXTERNAL

#pragma push_macro("NAME_EXTERNAL_TO_INTERNAL")
#undef NAME_EXTERNAL_TO_INTERNAL

#pragma push_macro("PREPROCESSOR_ENUM_PROTECT")
#undef PREPROCESSOR_ENUM_PROTECT

#pragma push_macro("DEFERRED_DEPENDENCY_CHECK")
#undef DEFERRED_DEPENDENCY_CHECK

#pragma push_macro("UE_ASYNC_PACKAGE_DEBUG")
#undef UE_ASYNC_PACKAGE_DEBUG

#pragma push_macro("UE_ASYNC_UPACKAGE_DEBUG")
#undef UE_ASYNC_UPACKAGE_DEBUG

#pragma push_macro("UE_ASYNC_PACKAGE_LOG")
#undef UE_ASYNC_PACKAGE_LOG

#pragma push_macro("UE_ASYNC_PACKAGE_CLOG")
#undef UE_ASYNC_PACKAGE_CLOG

#pragma push_macro("UE_ASYNC_PACKAGE_LOG_VERBOSE")
#undef UE_ASYNC_PACKAGE_LOG_VERBOSE

#pragma push_macro("UE_ASYNC_PACKAGE_CLOG_VERBOSE")
#undef UE_ASYNC_PACKAGE_CLOG_VERBOSE

#pragma push_macro("CHECK_IOSTATUS")
#undef CHECK_IOSTATUS

#pragma push_macro("TRACE_LOADTIME_START_ASYNC_LOADING")
#undef TRACE_LOADTIME_START_ASYNC_LOADING

#pragma push_macro("TRACE_LOADTIME_SUSPEND_ASYNC_LOADING")
#undef TRACE_LOADTIME_SUSPEND_ASYNC_LOADING

#pragma push_macro("TRACE_LOADTIME_RESUME_ASYNC_LOADING")
#undef TRACE_LOADTIME_RESUME_ASYNC_LOADING

#pragma push_macro("TRACE_LOADTIME_BEGIN_REQUEST")
#undef TRACE_LOADTIME_BEGIN_REQUEST

#pragma push_macro("TRACE_LOADTIME_END_REQUEST")
#undef TRACE_LOADTIME_END_REQUEST

#pragma push_macro("TRACE_LOADTIME_NEW_ASYNC_PACKAGE")
#undef TRACE_LOADTIME_NEW_ASYNC_PACKAGE

#pragma push_macro("TRACE_LOADTIME_BEGIN_LOAD_ASYNC_PACKAGE")
#undef TRACE_LOADTIME_BEGIN_LOAD_ASYNC_PACKAGE

#pragma push_macro("TRACE_LOADTIME_END_LOAD_ASYNC_PACKAGE")
#undef TRACE_LOADTIME_END_LOAD_ASYNC_PACKAGE

#pragma push_macro("TRACE_LOADTIME_DESTROY_ASYNC_PACKAGE")
#undef TRACE_LOADTIME_DESTROY_ASYNC_PACKAGE

#pragma push_macro("TRACE_LOADTIME_PACKAGE_SUMMARY")
#undef TRACE_LOADTIME_PACKAGE_SUMMARY

#pragma push_macro("TRACE_LOADTIME_ASYNC_PACKAGE_REQUEST_ASSOCIATION")
#undef TRACE_LOADTIME_ASYNC_PACKAGE_REQUEST_ASSOCIATION

#pragma push_macro("TRACE_LOADTIME_ASYNC_PACKAGE_LINKER_ASSOCIATION")
#undef TRACE_LOADTIME_ASYNC_PACKAGE_LINKER_ASSOCIATION

#pragma push_macro("TRACE_LOADTIME_ASYNC_PACKAGE_IMPORT_DEPENDENCY")
#undef TRACE_LOADTIME_ASYNC_PACKAGE_IMPORT_DEPENDENCY

#pragma push_macro("TRACE_LOADTIME_CREATE_EXPORT_SCOPE")
#undef TRACE_LOADTIME_CREATE_EXPORT_SCOPE

#pragma push_macro("TRACE_LOADTIME_SERIALIZE_EXPORT_SCOPE")
#undef TRACE_LOADTIME_SERIALIZE_EXPORT_SCOPE

#pragma push_macro("TRACE_LOADTIME_POSTLOAD_EXPORT_SCOPE")
#undef TRACE_LOADTIME_POSTLOAD_EXPORT_SCOPE

#pragma push_macro("TRACE_LOADTIME_CLASS_INFO")
#undef TRACE_LOADTIME_CLASS_INFO

#pragma push_macro("XFERSTRING")
#undef XFERSTRING

#pragma push_macro("XFERUNICODESTRING")
#undef XFERUNICODESTRING

#pragma push_macro("XFERTEXT")
#undef XFERTEXT

#pragma push_macro("DECLARE_CAST_BY_FLAG")
#undef DECLARE_CAST_BY_FLAG

#pragma push_macro("FIXUP_EXPR_OBJECT_POINTER")
#undef FIXUP_EXPR_OBJECT_POINTER

#pragma push_macro("CLASS_REDIRECT")
#undef CLASS_REDIRECT

#pragma push_macro("CLASS_REDIRECT_INSTANCES")
#undef CLASS_REDIRECT_INSTANCES

#pragma push_macro("STRUCT_REDIRECT")
#undef STRUCT_REDIRECT

#pragma push_macro("ENUM_REDIRECT")
#undef ENUM_REDIRECT

#pragma push_macro("PROPERTY_REDIRECT")
#undef PROPERTY_REDIRECT

#pragma push_macro("FUNCTION_REDIRECT")
#undef FUNCTION_REDIRECT

#pragma push_macro("PACKAGE_REDIRECT")
#undef PACKAGE_REDIRECT

#pragma push_macro("COMPARE_MEMBER")
#undef COMPARE_MEMBER

#pragma push_macro("DEFERRED_DEPENDENCY_ENSURE")
#undef DEFERRED_DEPENDENCY_ENSURE

#pragma push_macro("CompCheck")
#undef CompCheck

#pragma push_macro("DECLARE_OBJECT_FLAG")
#undef DECLARE_OBJECT_FLAG

#pragma push_macro("SCOPED_SAVETIMER")
#undef SCOPED_SAVETIMER

#pragma push_macro("STORE_INSTRUCTION_NAME")
#undef STORE_INSTRUCTION_NAME

#pragma push_macro("IMPLEMENT_FUNCTION")
#undef IMPLEMENT_FUNCTION

#pragma push_macro("IMPLEMENT_CAST_FUNCTION")
#undef IMPLEMENT_CAST_FUNCTION

#pragma push_macro("IMPLEMENT_VM_FUNCTION")
#undef IMPLEMENT_VM_FUNCTION

#pragma push_macro("ADD_COOK_STAT")
#undef ADD_COOK_STAT

#pragma push_macro("STREAMINGTOKEN_PARAM")
#undef STREAMINGTOKEN_PARAM

#pragma push_macro("DECLARE_CAST_BY_FLAG_FWD")
#undef DECLARE_CAST_BY_FLAG_FWD

#pragma push_macro("DECLARE_CAST_BY_FLAG_CAST")
#undef DECLARE_CAST_BY_FLAG_CAST

#pragma push_macro("IMPLEMENT_STRUCT")
#undef IMPLEMENT_STRUCT

#pragma push_macro("NET_CHECKSUM_OR_END")
#undef NET_CHECKSUM_OR_END

#pragma push_macro("NET_CHECKSUM")
#undef NET_CHECKSUM

#pragma push_macro("NET_CHECKSUM_CUSTOM")
#undef NET_CHECKSUM_CUSTOM

#pragma push_macro("NET_CHECKSUM_IGNORE")
#undef NET_CHECKSUM_IGNORE

#pragma push_macro("DECLARE_FIELD")
#undef DECLARE_FIELD

#pragma push_macro("IMPLEMENT_FIELD")
#undef IMPLEMENT_FIELD

#pragma push_macro("UPROPERTY")
#undef UPROPERTY

#pragma push_macro("UFUNCTION")
#undef UFUNCTION

#pragma push_macro("USTRUCT")
#undef USTRUCT

#pragma push_macro("UMETA")
#undef UMETA

#pragma push_macro("UPARAM")
#undef UPARAM

#pragma push_macro("UENUM")
#undef UENUM

#pragma push_macro("UDELEGATE")
#undef UDELEGATE

#pragma push_macro("RIGVM_METHOD")
#undef RIGVM_METHOD

#pragma push_macro("BODY_MACRO_COMBINE_INNER")
#undef BODY_MACRO_COMBINE_INNER

#pragma push_macro("BODY_MACRO_COMBINE")
#undef BODY_MACRO_COMBINE

#pragma push_macro("GENERATED_BODY_LEGACY")
#undef GENERATED_BODY_LEGACY

#pragma push_macro("GENERATED_BODY")
#undef GENERATED_BODY

#pragma push_macro("GENERATED_USTRUCT_BODY")
#undef GENERATED_USTRUCT_BODY

#pragma push_macro("GENERATED_UCLASS_BODY")
#undef GENERATED_UCLASS_BODY

#pragma push_macro("GENERATED_UINTERFACE_BODY")
#undef GENERATED_UINTERFACE_BODY

#pragma push_macro("GENERATED_IINTERFACE_BODY")
#undef GENERATED_IINTERFACE_BODY

#pragma push_macro("UCLASS")
#undef UCLASS

#pragma push_macro("UINTERFACE")
#undef UINTERFACE

#pragma push_macro("DECLARE_FUNCTION")
#undef DECLARE_FUNCTION

#pragma push_macro("DEFINE_FUNCTION")
#undef DEFINE_FUNCTION

#pragma push_macro("RELAY_CONSTRUCTOR")
#undef RELAY_CONSTRUCTOR

#pragma push_macro("COMPILED_IN_FLAGS")
#undef COMPILED_IN_FLAGS

#pragma push_macro("DECLARE_SERIALIZER")
#undef DECLARE_SERIALIZER

#pragma push_macro("IMPLEMENT_FARCHIVE_SERIALIZER")
#undef IMPLEMENT_FARCHIVE_SERIALIZER

#pragma push_macro("IMPLEMENT_FSTRUCTUREDARCHIVE_SERIALIZER")
#undef IMPLEMENT_FSTRUCTUREDARCHIVE_SERIALIZER

#pragma push_macro("DECLARE_FARCHIVE_SERIALIZER")
#undef DECLARE_FARCHIVE_SERIALIZER

#pragma push_macro("DECLARE_FSTRUCTUREDARCHIVE_SERIALIZER")
#undef DECLARE_FSTRUCTUREDARCHIVE_SERIALIZER

#pragma push_macro("DECLARE_CLASS")
#undef DECLARE_CLASS

#pragma push_macro("DEFINE_FORBIDDEN_DEFAULT_CONSTRUCTOR_CALL")
#undef DEFINE_FORBIDDEN_DEFAULT_CONSTRUCTOR_CALL

#pragma push_macro("DEFINE_DEFAULT_CONSTRUCTOR_CALL")
#undef DEFINE_DEFAULT_CONSTRUCTOR_CALL

#pragma push_macro("DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL")
#undef DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL

#pragma push_macro("DECLARE_VTABLE_PTR_HELPER_CTOR")
#undef DECLARE_VTABLE_PTR_HELPER_CTOR

#pragma push_macro("DEFINE_VTABLE_PTR_HELPER_CTOR")
#undef DEFINE_VTABLE_PTR_HELPER_CTOR

#pragma push_macro("DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER_DUMMY")
#undef DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER_DUMMY

#pragma push_macro("DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER")
#undef DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER

#pragma push_macro("DECLARE_CLASS_INTRINSIC_NO_CTOR")
#undef DECLARE_CLASS_INTRINSIC_NO_CTOR

#pragma push_macro("DECLARE_CLASS_INTRINSIC")
#undef DECLARE_CLASS_INTRINSIC

#pragma push_macro("DECLARE_CASTED_CLASS_INTRINSIC_WITH_API_NO_CTOR")
#undef DECLARE_CASTED_CLASS_INTRINSIC_WITH_API_NO_CTOR

#pragma push_macro("DECLARE_CASTED_CLASS_INTRINSIC_WITH_API")
#undef DECLARE_CASTED_CLASS_INTRINSIC_WITH_API

#pragma push_macro("DECLARE_CASTED_CLASS_INTRINSIC_NO_CTOR_NO_VTABLE_CTOR")
#undef DECLARE_CASTED_CLASS_INTRINSIC_NO_CTOR_NO_VTABLE_CTOR

#pragma push_macro("DECLARE_CASTED_CLASS_INTRINSIC_NO_CTOR")
#undef DECLARE_CASTED_CLASS_INTRINSIC_NO_CTOR

#pragma push_macro("DECLARE_CASTED_CLASS_INTRINSIC")
#undef DECLARE_CASTED_CLASS_INTRINSIC

#pragma push_macro("DECLARE_WITHIN_INTERNAL")
#undef DECLARE_WITHIN_INTERNAL

#pragma push_macro("DECLARE_WITHIN")
#undef DECLARE_WITHIN

#pragma push_macro("DECLARE_WITHIN_UPACKAGE")
#undef DECLARE_WITHIN_UPACKAGE

#pragma push_macro("IMPLEMENT_CLASS")
#undef IMPLEMENT_CLASS

#pragma push_macro("IMPLEMENT_INTRINSIC_CLASS")
#undef IMPLEMENT_INTRINSIC_CLASS

#pragma push_macro("IMPLEMENT_CORE_INTRINSIC_CLASS")
#undef IMPLEMENT_CORE_INTRINSIC_CLASS

#pragma push_macro("IMPLEMENT_DYNAMIC_CLASS")
#undef IMPLEMENT_DYNAMIC_CLASS

#pragma push_macro("SCOPED_SCRIPT_NATIVE_TIMER")
#undef SCOPED_SCRIPT_NATIVE_TIMER

#pragma push_macro("ZERO_INIT")
#undef ZERO_INIT

#pragma push_macro("PARAM_PASSED_BY_VAL")
#undef PARAM_PASSED_BY_VAL

#pragma push_macro("PARAM_PASSED_BY_VAL_ZEROED")
#undef PARAM_PASSED_BY_VAL_ZEROED

#pragma push_macro("PARAM_PASSED_BY_REF")
#undef PARAM_PASSED_BY_REF

#pragma push_macro("PARAM_PASSED_BY_REF_ZEROED")
#undef PARAM_PASSED_BY_REF_ZEROED

#pragma push_macro("P_GET_PROPERTY")
#undef P_GET_PROPERTY

#pragma push_macro("P_GET_PROPERTY_REF")
#undef P_GET_PROPERTY_REF

#pragma push_macro("P_GET_UBOOL")
#undef P_GET_UBOOL

#pragma push_macro("P_GET_UBOOL8")
#undef P_GET_UBOOL8

#pragma push_macro("P_GET_UBOOL16")
#undef P_GET_UBOOL16

#pragma push_macro("P_GET_UBOOL32")
#undef P_GET_UBOOL32

#pragma push_macro("P_GET_UBOOL64")
#undef P_GET_UBOOL64

#pragma push_macro("P_GET_UBOOL_REF")
#undef P_GET_UBOOL_REF

#pragma push_macro("P_GET_STRUCT")
#undef P_GET_STRUCT

#pragma push_macro("P_GET_STRUCT_REF")
#undef P_GET_STRUCT_REF

#pragma push_macro("P_GET_OBJECT")
#undef P_GET_OBJECT

#pragma push_macro("P_GET_OBJECT_REF")
#undef P_GET_OBJECT_REF

#pragma push_macro("P_GET_OBJECT_NO_PTR")
#undef P_GET_OBJECT_NO_PTR

#pragma push_macro("P_GET_OBJECT_REF_NO_PTR")
#undef P_GET_OBJECT_REF_NO_PTR

#pragma push_macro("P_GET_TARRAY")
#undef P_GET_TARRAY

#pragma push_macro("P_GET_TARRAY_REF")
#undef P_GET_TARRAY_REF

#pragma push_macro("P_GET_TMAP")
#undef P_GET_TMAP

#pragma push_macro("P_GET_TMAP_REF")
#undef P_GET_TMAP_REF

#pragma push_macro("P_GET_TSET")
#undef P_GET_TSET

#pragma push_macro("P_GET_TSET_REF")
#undef P_GET_TSET_REF

#pragma push_macro("P_GET_TINTERFACE")
#undef P_GET_TINTERFACE

#pragma push_macro("P_GET_TINTERFACE_REF")
#undef P_GET_TINTERFACE_REF

#pragma push_macro("P_GET_SOFTOBJECT")
#undef P_GET_SOFTOBJECT

#pragma push_macro("P_GET_SOFTOBJECT_REF")
#undef P_GET_SOFTOBJECT_REF

#pragma push_macro("P_GET_SOFTCLASS")
#undef P_GET_SOFTCLASS

#pragma push_macro("P_GET_SOFTCLASS_REF")
#undef P_GET_SOFTCLASS_REF

#pragma push_macro("P_GET_ARRAY")
#undef P_GET_ARRAY

#pragma push_macro("P_GET_ARRAY_REF")
#undef P_GET_ARRAY_REF

#pragma push_macro("P_GET_ENUM")
#undef P_GET_ENUM

#pragma push_macro("P_GET_ENUM_REF")
#undef P_GET_ENUM_REF

#pragma push_macro("P_THIS_CAST")
#undef P_THIS_CAST

#pragma push_macro("XFER")
#undef XFER

#pragma push_macro("XFERNAME")
#undef XFERNAME

#pragma push_macro("XFERPTR")
#undef XFERPTR

#pragma push_macro("XFER_OBJECT_POINTER")
#undef XFER_OBJECT_POINTER

#pragma push_macro("FUNC_DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE")
#undef FUNC_DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FourParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FourParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FiveParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FiveParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SixParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SixParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SevenParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SevenParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_EightParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_EightParams

#pragma push_macro("DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_NineParams")
#undef DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_NineParams

#pragma push_macro("CPP_ARRAY_DIM")
#undef CPP_ARRAY_DIM

#pragma push_macro("SCOPE_CYCLE_UOBJECT")
#undef SCOPE_CYCLE_UOBJECT

#pragma push_macro("UE_ASSET_LOG")
#undef UE_ASSET_LOG

#pragma push_macro("METADATA_PARAMS")
#undef METADATA_PARAMS

#pragma push_macro("IF_WITH_EDITOR")
#undef IF_WITH_EDITOR

#pragma push_macro("IF_WITH_EDITORONLY_DATA")
#undef IF_WITH_EDITORONLY_DATA

#pragma push_macro("INCREASE_ALLOC_COUNTER")
#undef INCREASE_ALLOC_COUNTER

#pragma push_macro("DECREASE_ALLOC_COUNTER")
#undef DECREASE_ALLOC_COUNTER

#pragma push_macro("ContextRedirect")
#undef ContextRedirect

#pragma push_macro("ContextGPU0")
#undef ContextGPU0

#pragma push_macro("DECLARE_ISBOUNDSHADER")
#undef DECLARE_ISBOUNDSHADER

#pragma push_macro("VALIDATE_BOUND_SHADER")
#undef VALIDATE_BOUND_SHADER

#pragma push_macro("COPY_SHADER")
#undef COPY_SHADER

#pragma push_macro("EXT_SHADER")
#undef EXT_SHADER

#pragma push_macro("PSO_IF_NOT_EQUAL_RETURN_FALSE")
#undef PSO_IF_NOT_EQUAL_RETURN_FALSE

#pragma push_macro("PSO_IF_MEMCMP_FAILS_RETURN_FALSE")
#undef PSO_IF_MEMCMP_FAILS_RETURN_FALSE

#pragma push_macro("PSO_IF_STRING_COMPARE_FAILS_RETURN_FALSE")
#undef PSO_IF_STRING_COMPARE_FAILS_RETURN_FALSE

#pragma push_macro("DEBUG_EXECUTE_COMMAND_LIST")
#undef DEBUG_EXECUTE_COMMAND_LIST

#pragma push_macro("DEBUG_EXECUTE_COMMAND_CONTEXT")
#undef DEBUG_EXECUTE_COMMAND_CONTEXT

#pragma push_macro("DEBUG_RHI_EXECUTE_COMMAND_LIST")
#undef DEBUG_RHI_EXECUTE_COMMAND_LIST

#pragma push_macro("EMBED_DXGI_ERROR_LIST")
#undef EMBED_DXGI_ERROR_LIST

#pragma push_macro("CONDITIONAL_SET_SRVS")
#undef CONDITIONAL_SET_SRVS

#pragma push_macro("CONDITIONAL_SET_CBVS")
#undef CONDITIONAL_SET_CBVS

#pragma push_macro("CONDITIONAL_SET_SAMPLERS")
#undef CONDITIONAL_SET_SAMPLERS

#pragma push_macro("DECLARE_SHADER_TRAITS")
#undef DECLARE_SHADER_TRAITS

#pragma push_macro("D3DERR")
#undef D3DERR

#pragma push_macro("MAKE_D3DHRESULT")
#undef MAKE_D3DHRESULT

#pragma push_macro("D3DFORMATCASE")
#undef D3DFORMATCASE

#pragma push_macro("MERGE_EXT")
#undef MERGE_EXT

#pragma push_macro("CASE_ERROR_NAME")
#undef CASE_ERROR_NAME

#pragma push_macro("VERIFYD3D12RESULT_LAMBDA")
#undef VERIFYD3D12RESULT_LAMBDA

#pragma push_macro("VERIFYD3D12RESULT_EX")
#undef VERIFYD3D12RESULT_EX

#pragma push_macro("VERIFYD3D12RESULT")
#undef VERIFYD3D12RESULT

#pragma push_macro("VERIFYD3D12CREATETEXTURERESULT")
#undef VERIFYD3D12CREATETEXTURERESULT

#pragma push_macro("checkComRefCount")
#undef checkComRefCount

#pragma push_macro("UPDATE_BITFLAGS")
#undef UPDATE_BITFLAGS

#pragma push_macro("DS_ELEMENT_TYPE")
#undef DS_ELEMENT_TYPE

#pragma push_macro("SerialAny_Case")
#undef SerialAny_Case

#pragma push_macro("DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE")
#undef DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE

#pragma push_macro("GETCURVE_REPORTERROR")
#undef GETCURVE_REPORTERROR

#pragma push_macro("GETCURVE_REPORTERROR_WITHPATHNAME")
#undef GETCURVE_REPORTERROR_WITHPATHNAME

#pragma push_macro("GETROW_REPORTERROR")
#undef GETROW_REPORTERROR

#pragma push_macro("GETROWOBJECT_REPORTERROR")
#undef GETROWOBJECT_REPORTERROR

#pragma push_macro("DECLARE_SCOPED_DELEGATE")
#undef DECLARE_SCOPED_DELEGATE

#pragma push_macro("STREAMABLERENDERASSET_NODEFAULT")
#undef STREAMABLERENDERASSET_NODEFAULT

#pragma push_macro("HIDE_ACTOR_TRANSFORM_FUNCTIONS")
#undef HIDE_ACTOR_TRANSFORM_FUNCTIONS

#pragma push_macro("DEPRECATED_CHARACTER_MOVEMENT_RPC")
#undef DEPRECATED_CHARACTER_MOVEMENT_RPC

#pragma push_macro("PARTICLE_PERF_STAT_INSTANCE_COUNT")
#undef PARTICLE_PERF_STAT_INSTANCE_COUNT

#pragma push_macro("PARTICLE_PERF_STAT_CYCLES")
#undef PARTICLE_PERF_STAT_CYCLES

#pragma push_macro("EFFECT_SETTINGS_NAME2")
#undef EFFECT_SETTINGS_NAME2

#pragma push_macro("EFFECT_SETTINGS_NAME1")
#undef EFFECT_SETTINGS_NAME1

#pragma push_macro("EFFECT_SETTINGS_NAME")
#undef EFFECT_SETTINGS_NAME

#pragma push_macro("EFFECT_PRESET_NAME2")
#undef EFFECT_PRESET_NAME2

#pragma push_macro("EFFECT_PRESET_NAME1")
#undef EFFECT_PRESET_NAME1

#pragma push_macro("EFFECT_PRESET_NAME")
#undef EFFECT_PRESET_NAME

#pragma push_macro("GET_EFFECT_SETTINGS")
#undef GET_EFFECT_SETTINGS

#pragma push_macro("EFFECT_PRESET_METHODS")
#undef EFFECT_PRESET_METHODS

#pragma push_macro("DECLARE_SOUNDNODE_ELEMENT")
#undef DECLARE_SOUNDNODE_ELEMENT

#pragma push_macro("DECLARE_SOUNDNODE_ELEMENT_PTR")
#undef DECLARE_SOUNDNODE_ELEMENT_PTR

#pragma push_macro("DEBUG_CALLSPACE")
#undef DEBUG_CALLSPACE

#pragma push_macro("UE_MAKEFOURCC")
#undef UE_MAKEFOURCC

#pragma push_macro("UE_mmioFOURCC")
#undef UE_mmioFOURCC

#pragma push_macro("SAFE_TRACEINDEX_DECREASE")
#undef SAFE_TRACEINDEX_DECREASE

#pragma push_macro("DATATABLE_CHANGE_SCOPE")
#undef DATATABLE_CHANGE_SCOPE

#pragma push_macro("CURVETABLE_CHANGE_SCOPE")
#undef CURVETABLE_CHANGE_SCOPE

#pragma push_macro("UE_DRAW_SERVER_DEBUG_ON_EACH_CLIENT")
#undef UE_DRAW_SERVER_DEBUG_ON_EACH_CLIENT

#pragma push_macro("IMPLEMENT_GPUSKINNING_VERTEX_FACTORY_TYPE_INTERNAL")
#undef IMPLEMENT_GPUSKINNING_VERTEX_FACTORY_TYPE_INTERNAL

#pragma push_macro("IMPLEMENT_GPUSKINNING_VERTEX_FACTORY_TYPE")
#undef IMPLEMENT_GPUSKINNING_VERTEX_FACTORY_TYPE

#pragma push_macro("IMPLEMENT_GPUSKINNING_VERTEX_FACTORY_PARAMETER_TYPE")
#undef IMPLEMENT_GPUSKINNING_VERTEX_FACTORY_PARAMETER_TYPE

#pragma push_macro("STRUCTTRACK_GETNUMKEYFRAMES")
#undef STRUCTTRACK_GETNUMKEYFRAMES

#pragma push_macro("STRUCTTRACK_GETTIMERANGE")
#undef STRUCTTRACK_GETTIMERANGE

#pragma push_macro("STRUCTTRACK_GETTRACKENDTIME")
#undef STRUCTTRACK_GETTRACKENDTIME

#pragma push_macro("STRUCTTRACK_GETKEYFRAMETIME")
#undef STRUCTTRACK_GETKEYFRAMETIME

#pragma push_macro("STRUCTTRACK_GETKEYFRAMEINDEX")
#undef STRUCTTRACK_GETKEYFRAMEINDEX

#pragma push_macro("STRUCTTRACK_SETKEYFRAMETIME")
#undef STRUCTTRACK_SETKEYFRAMETIME

#pragma push_macro("STRUCTTRACK_REMOVEKEYFRAME")
#undef STRUCTTRACK_REMOVEKEYFRAME

#pragma push_macro("STRUCTTRACK_DUPLICATEKEYFRAME")
#undef STRUCTTRACK_DUPLICATEKEYFRAME

#pragma push_macro("STRUCTTRACK_GETCLOSESTSNAPPOSITION")
#undef STRUCTTRACK_GETCLOSESTSNAPPOSITION

#pragma push_macro("DEBUG_REMOTEFUNCTION")
#undef DEBUG_REMOTEFUNCTION

#pragma push_macro("UpdatePerClientMinMaxAvg")
#undef UpdatePerClientMinMaxAvg

#pragma push_macro("BUILD_NETEMULATION_CONSOLE_COMMAND")
#undef BUILD_NETEMULATION_CONSOLE_COMMAND

#pragma push_macro("SCOPE_LOCK_REF")
#undef SCOPE_LOCK_REF

#pragma push_macro("CHECK_REPL_EQUALITY")
#undef CHECK_REPL_EQUALITY

#pragma push_macro("CHECK_REPL_VALIDITY")
#undef CHECK_REPL_VALIDITY

#pragma push_macro("COMPOSE_NET_GUID")
#undef COMPOSE_NET_GUID

#pragma push_macro("ALLOC_NEW_NET_GUID")
#undef ALLOC_NEW_NET_GUID

#pragma push_macro("PolyCheck")
#undef PolyCheck

#pragma push_macro("CHECK_VIRTUALTEXTURE_USAGE")
#undef CHECK_VIRTUALTEXTURE_USAGE

#pragma push_macro("WarnInvalidPhysicsOperations")
#undef WarnInvalidPhysicsOperations

#pragma push_macro("REPDATATYPE_SPECIALIZATION")
#undef REPDATATYPE_SPECIALIZATION

#pragma push_macro("LERP_PP")
#undef LERP_PP

#pragma push_macro("IF_PP")
#undef IF_PP

#pragma push_macro("IMPLEMENT_PLATFORM_INTERFACE_SINGLETON")
#undef IMPLEMENT_PLATFORM_INTERFACE_SINGLETON

#pragma push_macro("SHOWFLAG_ALWAYS_ACCESSIBLE")
#undef SHOWFLAG_ALWAYS_ACCESSIBLE

#pragma push_macro("SHOWFLAG_FIXED_IN_SHIPPING")
#undef SHOWFLAG_FIXED_IN_SHIPPING

#pragma push_macro("DISABLE_ENGINE_SHOWFLAG")
#undef DISABLE_ENGINE_SHOWFLAG

#pragma push_macro("DEBUGBROKENCONSTRAINTUPDATE")
#undef DEBUGBROKENCONSTRAINTUPDATE

#pragma push_macro("GENERATE_LOD_MODEL")
#undef GENERATE_LOD_MODEL

#pragma push_macro("SERIALIZE_CURVE")
#undef SERIALIZE_CURVE

#pragma push_macro("SERIALIZE_OPTION")
#undef SERIALIZE_OPTION

#pragma push_macro("SKIP_OLD_OPTION")
#undef SKIP_OLD_OPTION

#pragma push_macro("DECLARETEXTUREGROUPSTAT")
#undef DECLARETEXTUREGROUPSTAT

#pragma push_macro("ASSIGNTEXTUREGROUPSTATNAME")
#undef ASSIGNTEXTUREGROUPSTATNAME

#pragma push_macro("TEXT_TO_ENUM")
#undef TEXT_TO_ENUM

#pragma push_macro("TEXT_TO_MIPGENSETTINGS")
#undef TEXT_TO_MIPGENSETTINGS

#pragma push_macro("GROUPNAMES")
#undef GROUPNAMES

#pragma push_macro("TIMER_TEST_TEXT")
#undef TIMER_TEST_TEXT

#pragma push_macro("SCOPE_TIME_TO_VAR")
#undef SCOPE_TIME_TO_VAR

#pragma push_macro("CALCULATE_WEIGHTING")
#undef CALCULATE_WEIGHTING

#pragma push_macro("DO_ANIMSTAT_PROCESSING")
#undef DO_ANIMSTAT_PROCESSING

#pragma push_macro("IS_VALID_COLLISIONCHANNEL")
#undef IS_VALID_COLLISIONCHANNEL

#pragma push_macro("devCode")
#undef devCode

#pragma push_macro("SKY_DECLARE_BLUEPRINT_SETFUNCTION")
#undef SKY_DECLARE_BLUEPRINT_SETFUNCTION

#pragma push_macro("SKY_DECLARE_BLUEPRINT_SETFUNCTION_LINEARCOEFFICIENT")
#undef SKY_DECLARE_BLUEPRINT_SETFUNCTION_LINEARCOEFFICIENT

#pragma push_macro("CLOUD_DECLARE_BLUEPRINT_SETFUNCTION")
#undef CLOUD_DECLARE_BLUEPRINT_SETFUNCTION

#pragma push_macro("SETUPLODGROUP")
#undef SETUPLODGROUP

#pragma push_macro("TEXT_TO_SHADINGMODEL")
#undef TEXT_TO_SHADINGMODEL

#pragma push_macro("TEXT_TO_BLENDMODE")
#undef TEXT_TO_BLENDMODE

#pragma push_macro("SWAP_REFERENCE_TO")
#undef SWAP_REFERENCE_TO

#pragma push_macro("IF_INPUT_RETURN")
#undef IF_INPUT_RETURN

#pragma push_macro("COMPILER_OR_LOG_ERROR")
#undef COMPILER_OR_LOG_ERROR

#pragma push_macro("DECLARE_MATERIALUNIFORMEXPRESSION_TYPE")
#undef DECLARE_MATERIALUNIFORMEXPRESSION_TYPE

#pragma push_macro("IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE")
#undef IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE

#pragma push_macro("WORLD_TYPE_CASE")
#undef WORLD_TYPE_CASE

#pragma push_macro("STARTQUERYTIMER")
#undef STARTQUERYTIMER

#pragma push_macro("CAPTUREGEOMSWEEP")
#undef CAPTUREGEOMSWEEP

#pragma push_macro("CAPTURERAYCAST")
#undef CAPTURERAYCAST

#pragma push_macro("CAPTUREGEOMOVERLAP")
#undef CAPTUREGEOMOVERLAP

#pragma push_macro("SET_DRIVE_PARAM")
#undef SET_DRIVE_PARAM

#pragma push_macro("SUBSTEPPING_WARNING")
#undef SUBSTEPPING_WARNING

#pragma push_macro("ALLOCATE_VERTEX_DATA_TEMPLATE")
#undef ALLOCATE_VERTEX_DATA_TEMPLATE

#pragma push_macro("CHECK_SUCCESS")
#undef CHECK_SUCCESS

#pragma push_macro("CHECK_FAIL")
#undef CHECK_FAIL

#pragma push_macro("CHECK_NOT_NULL")
#undef CHECK_NOT_NULL

#pragma push_macro("MAKE_565")
#undef MAKE_565

#pragma push_macro("MAKE_8888")
#undef MAKE_8888

#pragma push_macro("UNORM16")
#undef UNORM16

#pragma push_macro("UNORM16_SRGB")
#undef UNORM16_SRGB

#pragma push_macro("AC_UnalignedSwap")
#undef AC_UnalignedSwap

#pragma push_macro("ECC_TO_BITFIELD")
#undef ECC_TO_BITFIELD

#pragma push_macro("CRC_TO_BITFIELD")
#undef CRC_TO_BITFIELD

#pragma push_macro("SCENE_QUERY_STAT_ONLY")
#undef SCENE_QUERY_STAT_ONLY

#pragma push_macro("SCENE_QUERY_STAT_NAME_ONLY")
#undef SCENE_QUERY_STAT_NAME_ONLY

#pragma push_macro("SCENE_QUERY_STAT")
#undef SCENE_QUERY_STAT

#pragma push_macro("MAKEFOURCC")
#undef MAKEFOURCC

#pragma push_macro("DIST_GET_RANDOM_VALUE")
#undef DIST_GET_RANDOM_VALUE

#pragma push_macro("DEFINE_GAME_DELEGATE")
#undef DEFINE_GAME_DELEGATE

#pragma push_macro("DEFINE_GAME_DELEGATE_TYPED")
#undef DEFINE_GAME_DELEGATE_TYPED

#pragma push_macro("SET_BONE_DATA")
#undef SET_BONE_DATA

#pragma push_macro("DECLARE_HIT_PROXY_STATIC")
#undef DECLARE_HIT_PROXY_STATIC

#pragma push_macro("DECLARE_HIT_PROXY")
#undef DECLARE_HIT_PROXY

#pragma push_macro("IMPLEMENT_HIT_PROXY_BASE")
#undef IMPLEMENT_HIT_PROXY_BASE

#pragma push_macro("IMPLEMENT_HIT_PROXY")
#undef IMPLEMENT_HIT_PROXY

#pragma push_macro("EVAL_CURVE")
#undef EVAL_CURVE

#pragma push_macro("FIND_POINT")
#undef FIND_POINT

#pragma push_macro("IMPLEMENT_MATERIAL_SHADER_TYPE")
#undef IMPLEMENT_MATERIAL_SHADER_TYPE

#pragma push_macro("TRACE_CLASS")
#undef TRACE_CLASS

#pragma push_macro("TRACE_OBJECT")
#undef TRACE_OBJECT

#pragma push_macro("TRACE_OBJECT_EVENT")
#undef TRACE_OBJECT_EVENT

#pragma push_macro("UNCONDITIONAL_TRACE_OBJECT_EVENT")
#undef UNCONDITIONAL_TRACE_OBJECT_EVENT

#pragma push_macro("TRACE_WORLD")
#undef TRACE_WORLD

#pragma push_macro("PARTICLE_INSTANCE_PREFETCH")
#undef PARTICLE_INSTANCE_PREFETCH

#pragma push_macro("DECLARE_PARTICLE")
#undef DECLARE_PARTICLE

#pragma push_macro("DECLARE_PARTICLE_CONST")
#undef DECLARE_PARTICLE_CONST

#pragma push_macro("DECLARE_PARTICLE_PTR")
#undef DECLARE_PARTICLE_PTR

#pragma push_macro("PARTICLE_ELEMENT")
#undef PARTICLE_ELEMENT

#pragma push_macro("TRAIL_EMITTER_CHECK_FLAG")
#undef TRAIL_EMITTER_CHECK_FLAG

#pragma push_macro("TRAIL_EMITTER_SET_FLAG")
#undef TRAIL_EMITTER_SET_FLAG

#pragma push_macro("TRAIL_EMITTER_GET_PREVNEXT")
#undef TRAIL_EMITTER_GET_PREVNEXT

#pragma push_macro("TRAIL_EMITTER_SET_PREVNEXT")
#undef TRAIL_EMITTER_SET_PREVNEXT

#pragma push_macro("TRAIL_EMITTER_IS_START")
#undef TRAIL_EMITTER_IS_START

#pragma push_macro("TRAIL_EMITTER_SET_START")
#undef TRAIL_EMITTER_SET_START

#pragma push_macro("TRAIL_EMITTER_IS_END")
#undef TRAIL_EMITTER_IS_END

#pragma push_macro("TRAIL_EMITTER_SET_END")
#undef TRAIL_EMITTER_SET_END

#pragma push_macro("TRAIL_EMITTER_IS_MIDDLE")
#undef TRAIL_EMITTER_IS_MIDDLE

#pragma push_macro("TRAIL_EMITTER_SET_MIDDLE")
#undef TRAIL_EMITTER_SET_MIDDLE

#pragma push_macro("TRAIL_EMITTER_IS_ONLY")
#undef TRAIL_EMITTER_IS_ONLY

#pragma push_macro("TRAIL_EMITTER_SET_ONLY")
#undef TRAIL_EMITTER_SET_ONLY

#pragma push_macro("TRAIL_EMITTER_IS_FORCEKILL")
#undef TRAIL_EMITTER_IS_FORCEKILL

#pragma push_macro("TRAIL_EMITTER_SET_FORCEKILL")
#undef TRAIL_EMITTER_SET_FORCEKILL

#pragma push_macro("TRAIL_EMITTER_IS_DEADTRAIL")
#undef TRAIL_EMITTER_IS_DEADTRAIL

#pragma push_macro("TRAIL_EMITTER_SET_DEADTRAIL")
#undef TRAIL_EMITTER_SET_DEADTRAIL

#pragma push_macro("TRAIL_EMITTER_IS_HEAD")
#undef TRAIL_EMITTER_IS_HEAD

#pragma push_macro("TRAIL_EMITTER_IS_HEADONLY")
#undef TRAIL_EMITTER_IS_HEADONLY

#pragma push_macro("TRAIL_EMITTER_GET_PREV")
#undef TRAIL_EMITTER_GET_PREV

#pragma push_macro("TRAIL_EMITTER_SET_PREV")
#undef TRAIL_EMITTER_SET_PREV

#pragma push_macro("TRAIL_EMITTER_GET_NEXT")
#undef TRAIL_EMITTER_GET_NEXT

#pragma push_macro("TRAIL_EMITTER_SET_NEXT")
#undef TRAIL_EMITTER_SET_NEXT

#pragma push_macro("BEAM2_TYPEDATA_SETFREQUENCY")
#undef BEAM2_TYPEDATA_SETFREQUENCY

#pragma push_macro("SRA_UPDATE_CALLBACK")
#undef SRA_UPDATE_CALLBACK

#pragma push_macro("VIEW_UNIFORM_BUFFER_MEMBER")
#undef VIEW_UNIFORM_BUFFER_MEMBER

#pragma push_macro("VIEW_UNIFORM_BUFFER_MEMBER_EX")
#undef VIEW_UNIFORM_BUFFER_MEMBER_EX

#pragma push_macro("VIEW_UNIFORM_BUFFER_MEMBER_ARRAY")
#undef VIEW_UNIFORM_BUFFER_MEMBER_ARRAY

#pragma push_macro("STEREO_LAYER_SHAPE_BOILERPLATE")
#undef STEREO_LAYER_SHAPE_BOILERPLATE

#pragma push_macro("CAN_TRACE_OBJECT")
#undef CAN_TRACE_OBJECT

#pragma push_macro("CANNOT_TRACE_OBJECT")
#undef CANNOT_TRACE_OBJECT

#pragma push_macro("MARK_OBJECT_TRACEABLE")
#undef MARK_OBJECT_TRACEABLE

#pragma push_macro("SET_OBJECT_TRACEABLE")
#undef SET_OBJECT_TRACEABLE

#pragma push_macro("GET_TRACE_OBJECT_VALUE")
#undef GET_TRACE_OBJECT_VALUE

#pragma push_macro("DISABLE_ENGINE_ACTOR_TRACE_FILTERING")
#undef DISABLE_ENGINE_ACTOR_TRACE_FILTERING

#pragma push_macro("DISABLE_ENGINE_WORLD_TRACE_FILTERING")
#undef DISABLE_ENGINE_WORLD_TRACE_FILTERING

#pragma push_macro("ANIM_MT_SCOPE_CYCLE_COUNTER")
#undef ANIM_MT_SCOPE_CYCLE_COUNTER

#pragma push_macro("TRACE_ANIM_TICK_RECORD")
#undef TRACE_ANIM_TICK_RECORD

#pragma push_macro("TRACE_SKELETAL_MESH")
#undef TRACE_SKELETAL_MESH

#pragma push_macro("TRACE_SKELETAL_MESH_COMPONENT")
#undef TRACE_SKELETAL_MESH_COMPONENT

#pragma push_macro("TRACE_SKELETALMESH_FRAME")
#undef TRACE_SKELETALMESH_FRAME

#pragma push_macro("TRACE_SCOPED_ANIM_GRAPH")
#undef TRACE_SCOPED_ANIM_GRAPH

#pragma push_macro("TRACE_SCOPED_ANIM_NODE")
#undef TRACE_SCOPED_ANIM_NODE

#pragma push_macro("TRACE_ANIM_NODE_VALUE")
#undef TRACE_ANIM_NODE_VALUE

#pragma push_macro("TRACE_ANIM_SEQUENCE_PLAYER")
#undef TRACE_ANIM_SEQUENCE_PLAYER

#pragma push_macro("TRACE_ANIM_STATE_MACHINE_STATE")
#undef TRACE_ANIM_STATE_MACHINE_STATE

#pragma push_macro("TRACE_ANIM_NOTIFY")
#undef TRACE_ANIM_NOTIFY

#pragma push_macro("TRACE_ANIM_SYNC_MARKER")
#undef TRACE_ANIM_SYNC_MARKER

#pragma push_macro("TRACE_ANIM_MONTAGE")
#undef TRACE_ANIM_MONTAGE

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_ZEROPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_ZEROPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_ONEPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_ONEPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_TWOPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_TWOPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_THREEPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_THREEPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_FOURPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_FOURPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_FIVEPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_FIVEPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_SIXPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_SIXPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_SEVENPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_SEVENPARAM

#pragma push_macro("DEFINE_CONTROL_CHANNEL_MESSAGE_EIGHTPARAM")
#undef DEFINE_CONTROL_CHANNEL_MESSAGE_EIGHTPARAM

#pragma push_macro("IMPLEMENT_CONTROL_CHANNEL_MESSAGE")
#undef IMPLEMENT_CONTROL_CHANNEL_MESSAGE

#pragma push_macro("UE_LOG_PACKET_NOTIFY_WARNING")
#undef UE_LOG_PACKET_NOTIFY_WARNING

#pragma push_macro("GRANULAR_NETWORK_MEMORY_TRACKING_INIT")
#undef GRANULAR_NETWORK_MEMORY_TRACKING_INIT

#pragma push_macro("GRANULAR_NETWORK_MEMORY_TRACKING_TRACK")
#undef GRANULAR_NETWORK_MEMORY_TRACKING_TRACK

#pragma push_macro("GRANULAR_NETWORK_MEMORY_TRACKING_CUSTOM_WORK")
#undef GRANULAR_NETWORK_MEMORY_TRACKING_CUSTOM_WORK

#pragma push_macro("NETWORK_PROFILER")
#undef NETWORK_PROFILER

#pragma push_macro("DOREPLIFETIME_WITH_PARAMS_FAST")
#undef DOREPLIFETIME_WITH_PARAMS_FAST

#pragma push_macro("DOREPLIFETIME_WITH_PARAMS_FAST_STATIC_ARRAY")
#undef DOREPLIFETIME_WITH_PARAMS_FAST_STATIC_ARRAY

#pragma push_macro("DOREPLIFETIME_WITH_PARAMS")
#undef DOREPLIFETIME_WITH_PARAMS

#pragma push_macro("DOREPLIFETIME")
#undef DOREPLIFETIME

#pragma push_macro("DOREPLIFETIME_DIFFNAMES")
#undef DOREPLIFETIME_DIFFNAMES

#pragma push_macro("DOREPLIFETIME_CONDITION")
#undef DOREPLIFETIME_CONDITION

#pragma push_macro("DOREPLIFETIME_CONDITION_NOTIFY")
#undef DOREPLIFETIME_CONDITION_NOTIFY

#pragma push_macro("DOREPLIFETIME_ACTIVE_OVERRIDE_FAST")
#undef DOREPLIFETIME_ACTIVE_OVERRIDE_FAST

#pragma push_macro("DOREPLIFETIME_ACTIVE_OVERRIDE_FAST_STATIC_ARRAY")
#undef DOREPLIFETIME_ACTIVE_OVERRIDE_FAST_STATIC_ARRAY

#pragma push_macro("DOREPLIFETIME_ACTIVE_OVERRIDE")
#undef DOREPLIFETIME_ACTIVE_OVERRIDE

#pragma push_macro("DOREPLIFETIME_CHANGE_CONDITION")
#undef DOREPLIFETIME_CHANGE_CONDITION

#pragma push_macro("DISABLE_REPLICATED_PROPERTY")
#undef DISABLE_REPLICATED_PROPERTY

#pragma push_macro("DISABLE_REPLICATED_PRIVATE_PROPERTY")
#undef DISABLE_REPLICATED_PRIVATE_PROPERTY

#pragma push_macro("DISABLE_REPLICATED_PROPERTY_FAST")
#undef DISABLE_REPLICATED_PROPERTY_FAST

#pragma push_macro("DISABLE_REPLICATED_PROPERTY_FAST_STATIC_ARRAY")
#undef DISABLE_REPLICATED_PROPERTY_FAST_STATIC_ARRAY

#pragma push_macro("DISABLE_ALL_CLASS_REPLICATED_PROPERTIES")
#undef DISABLE_ALL_CLASS_REPLICATED_PROPERTIES

#pragma push_macro("DISABLE_ALL_CLASS_REPLICATED_PROPERTIES_FAST")
#undef DISABLE_ALL_CLASS_REPLICATED_PROPERTIES_FAST

#pragma push_macro("RESET_REPLIFETIME_CONDITION")
#undef RESET_REPLIFETIME_CONDITION

#pragma push_macro("RESET_REPLIFETIME")
#undef RESET_REPLIFETIME

#pragma push_macro("RESET_REPLIFETIME_CONDITION_FAST")
#undef RESET_REPLIFETIME_CONDITION_FAST

#pragma push_macro("RESET_REPLIFETIME_CONDITION_FAST_STATIC_ARRAY")
#undef RESET_REPLIFETIME_CONDITION_FAST_STATIC_ARRAY

#pragma push_macro("RESET_REPLIFETIME_FAST")
#undef RESET_REPLIFETIME_FAST

#pragma push_macro("RESET_REPLIFETIME_FAST_STATIC_ARRAY")
#undef RESET_REPLIFETIME_FAST_STATIC_ARRAY

#pragma push_macro("RPC_VALIDATE")
#undef RPC_VALIDATE

#pragma push_macro("REDIRECT_TO_VLOG")
#undef REDIRECT_TO_VLOG

#pragma push_macro("REDIRECT_OBJECT_TO_VLOG")
#undef REDIRECT_OBJECT_TO_VLOG

#pragma push_macro("CONNECT_WITH_VLOG")
#undef CONNECT_WITH_VLOG

#pragma push_macro("CONNECT_OBJECT_WITH_VLOG")
#undef CONNECT_OBJECT_WITH_VLOG

#pragma push_macro("UE_VLOG")
#undef UE_VLOG

#pragma push_macro("UE_CVLOG")
#undef UE_CVLOG

#pragma push_macro("UE_VLOG_UELOG")
#undef UE_VLOG_UELOG

#pragma push_macro("UE_CVLOG_UELOG")
#undef UE_CVLOG_UELOG

#pragma push_macro("UE_VLOG_SEGMENT")
#undef UE_VLOG_SEGMENT

#pragma push_macro("UE_CVLOG_SEGMENT")
#undef UE_CVLOG_SEGMENT

#pragma push_macro("UE_VLOG_SEGMENT_THICK")
#undef UE_VLOG_SEGMENT_THICK

#pragma push_macro("UE_CVLOG_SEGMENT_THICK")
#undef UE_CVLOG_SEGMENT_THICK

#pragma push_macro("UE_VLOG_LOCATION")
#undef UE_VLOG_LOCATION

#pragma push_macro("UE_CVLOG_LOCATION")
#undef UE_CVLOG_LOCATION

#pragma push_macro("UE_VLOG_BOX")
#undef UE_VLOG_BOX

#pragma push_macro("UE_CVLOG_BOX")
#undef UE_CVLOG_BOX

#pragma push_macro("UE_VLOG_OBOX")
#undef UE_VLOG_OBOX

#pragma push_macro("UE_CVLOG_OBOX")
#undef UE_CVLOG_OBOX

#pragma push_macro("UE_VLOG_CONE")
#undef UE_VLOG_CONE

#pragma push_macro("UE_CVLOG_CONE")
#undef UE_CVLOG_CONE

#pragma push_macro("UE_VLOG_CYLINDER")
#undef UE_VLOG_CYLINDER

#pragma push_macro("UE_CVLOG_CYLINDER")
#undef UE_CVLOG_CYLINDER

#pragma push_macro("UE_VLOG_CAPSULE")
#undef UE_VLOG_CAPSULE

#pragma push_macro("UE_CVLOG_CAPSULE")
#undef UE_CVLOG_CAPSULE

#pragma push_macro("UE_VLOG_HISTOGRAM")
#undef UE_VLOG_HISTOGRAM

#pragma push_macro("UE_CVLOG_HISTOGRAM")
#undef UE_CVLOG_HISTOGRAM

#pragma push_macro("UE_VLOG_PULLEDCONVEX")
#undef UE_VLOG_PULLEDCONVEX

#pragma push_macro("UE_CVLOG_PULLEDCONVEX")
#undef UE_CVLOG_PULLEDCONVEX

#pragma push_macro("UE_VLOG_MESH")
#undef UE_VLOG_MESH

#pragma push_macro("UE_CVLOG_MESH")
#undef UE_CVLOG_MESH

#pragma push_macro("UE_VLOG_CONVEXPOLY")
#undef UE_VLOG_CONVEXPOLY

#pragma push_macro("UE_CVLOG_CONVEXPOLY")
#undef UE_CVLOG_CONVEXPOLY

#pragma push_macro("UE_VLOG_ARROW")
#undef UE_VLOG_ARROW

#pragma push_macro("UE_CVLOG_ARROW")
#undef UE_CVLOG_ARROW

#pragma push_macro("DECLARE_VLOG_EVENT")
#undef DECLARE_VLOG_EVENT

#pragma push_macro("DEFINE_VLOG_EVENT")
#undef DEFINE_VLOG_EVENT

#pragma push_macro("UE_VLOG_EVENTS")
#undef UE_VLOG_EVENTS

#pragma push_macro("UE_CVLOG_EVENTS")
#undef UE_CVLOG_EVENTS

#pragma push_macro("UE_VLOG_EVENT_WITH_DATA")
#undef UE_VLOG_EVENT_WITH_DATA

#pragma push_macro("UE_CVLOG_EVENT_WITH_DATA")
#undef UE_CVLOG_EVENT_WITH_DATA

#pragma push_macro("UE_IFVLOG")
#undef UE_IFVLOG

#pragma push_macro("TEXT_CONDITION")
#undef TEXT_CONDITION

#pragma push_macro("COLLAPSED_LOGF")
#undef COLLAPSED_LOGF

#pragma push_macro("DEFINE_ENUM_TO_STRING")
#undef DEFINE_ENUM_TO_STRING

#pragma push_macro("DECLARE_ENUM_TO_STRING")
#undef DECLARE_ENUM_TO_STRING

#pragma push_macro("EVOLUTION_TRAIT")
#undef EVOLUTION_TRAIT

#pragma push_macro("SCOPE_CYCLE_COUNTER_GJK")
#undef SCOPE_CYCLE_COUNTER_GJK

#pragma push_macro("MAX_CLAMP")
#undef MAX_CLAMP

#pragma push_macro("MIN_CLAMP")
#undef MIN_CLAMP

#pragma push_macro("RANGE_CLAMP")
#undef RANGE_CLAMP

#pragma push_macro("CHAOS_CHECK")
#undef CHAOS_CHECK

#pragma push_macro("CHAOS_ENSURE")
#undef CHAOS_ENSURE

#pragma push_macro("CHAOS_ENSURE_MSG")
#undef CHAOS_ENSURE_MSG

#pragma push_macro("CHAOS_LOG")
#undef CHAOS_LOG

#pragma push_macro("CHAOS_CLOG")
#undef CHAOS_CLOG

#pragma push_macro("CHAOS_PERF_TEST")
#undef CHAOS_PERF_TEST

#pragma push_macro("CHAOS_SCOPED_TIMER")
#undef CHAOS_SCOPED_TIMER

#pragma push_macro("PARTICLE_PROPERTY")
#undef PARTICLE_PROPERTY

#pragma push_macro("PROPERTY_TYPE")
#undef PROPERTY_TYPE

#pragma push_macro("SHAPE_PROPERTY")
#undef SHAPE_PROPERTY

#pragma push_macro("PARTICLE_PROPERTY_CHECKED")
#undef PARTICLE_PROPERTY_CHECKED

#pragma push_macro("CONSTRAINT_JOINT_PROPERPETY_IMPL")
#undef CONSTRAINT_JOINT_PROPERPETY_IMPL

#pragma push_macro("SCOPE_CYCLE_COUNTER_NAROWPHASE")
#undef SCOPE_CYCLE_COUNTER_NAROWPHASE

#pragma push_macro("CHAOS_COLLISION_STAT")
#undef CHAOS_COLLISION_STAT

#pragma push_macro("CHAOS_COLLISION_STAT_DISABLED")
#undef CHAOS_COLLISION_STAT_DISABLED

#pragma push_macro("MANAGED_ARRAY_TYPE")
#undef MANAGED_ARRAY_TYPE

#pragma push_macro("COPY_ON_WRITE_ATTRIBUTE")
#undef COPY_ON_WRITE_ATTRIBUTE

#pragma push_macro("SCOPE_LOG_GAMEPLAYTAGS")
#undef SCOPE_LOG_GAMEPLAYTAGS

#pragma push_macro("PARSE_FLOAT")
#undef PARSE_FLOAT

#pragma push_macro("PARSE_INT")
#undef PARSE_INT

#pragma push_macro("ADDKEYMAP")
#undef ADDKEYMAP

#pragma push_macro("MAP_OEM_VK_TO_SCAN")
#undef MAP_OEM_VK_TO_SCAN

#pragma push_macro("JSON_SERIALIZE")
#undef JSON_SERIALIZE

#pragma push_macro("JSON_SERIALIZE_ARRAY")
#undef JSON_SERIALIZE_ARRAY

#pragma push_macro("JSON_SERIALIZE_MAP")
#undef JSON_SERIALIZE_MAP

#pragma push_macro("JSON_SERIALIZE_SIMPLECOPY")
#undef JSON_SERIALIZE_SIMPLECOPY

#pragma push_macro("JSON_SERIALIZE_MAP_SAFE")
#undef JSON_SERIALIZE_MAP_SAFE

#pragma push_macro("JSON_SERIALIZE_SERIALIZABLE")
#undef JSON_SERIALIZE_SERIALIZABLE

#pragma push_macro("JSON_SERIALIZE_RAW_JSON_STRING")
#undef JSON_SERIALIZE_RAW_JSON_STRING

#pragma push_macro("JSON_SERIALIZE_ARRAY_SERIALIZABLE")
#undef JSON_SERIALIZE_ARRAY_SERIALIZABLE

#pragma push_macro("JSON_SERIALIZE_MAP_SERIALIZABLE")
#undef JSON_SERIALIZE_MAP_SERIALIZABLE

#pragma push_macro("JSON_SERIALIZE_OBJECT_SERIALIZABLE")
#undef JSON_SERIALIZE_OBJECT_SERIALIZABLE

#pragma push_macro("JSON_SERIALIZE_DATETIME_UNIX_TIMESTAMP")
#undef JSON_SERIALIZE_DATETIME_UNIX_TIMESTAMP

#pragma push_macro("JSON_SERIALIZE_DATETIME_UNIX_TIMESTAMP_MILLISECONDS")
#undef JSON_SERIALIZE_DATETIME_UNIX_TIMESTAMP_MILLISECONDS

#pragma push_macro("JSON_SERIALIZE_ENUM")
#undef JSON_SERIALIZE_ENUM

#pragma push_macro("HEIGHTDATA")
#undef HEIGHTDATA

#pragma push_macro("CHECK_JNI_METHOD")
#undef CHECK_JNI_METHOD

#pragma push_macro("VERSION_TEXT")
#undef VERSION_TEXT

#pragma push_macro("VERSION_STRINGIFY_2")
#undef VERSION_STRINGIFY_2

#pragma push_macro("VERSION_STRINGIFY")
#undef VERSION_STRINGIFY

#pragma push_macro("SAFE_CA_CALL")
#undef SAFE_CA_CALL

#pragma push_macro("CORE_AUDIO_ERR")
#undef CORE_AUDIO_ERR

#pragma push_macro("NEW_GLOBAL_PROPERTY")
#undef NEW_GLOBAL_PROPERTY

#pragma push_macro("NEW_OUTPUT_PROPERTY")
#undef NEW_OUTPUT_PROPERTY

#pragma push_macro("DECLARE_RPC")
#undef DECLARE_RPC

#pragma push_macro("MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER")
#undef MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER

#pragma push_macro("UE_MOVIESCENE_TODO_IMPL")
#undef UE_MOVIESCENE_TODO_IMPL

#pragma push_macro("UE_MOVIESCENE_TODO")
#undef UE_MOVIESCENE_TODO

#pragma push_macro("INITIALIZE_NAVQUERY_SIMPLE")
#undef INITIALIZE_NAVQUERY_SIMPLE

#pragma push_macro("INITIALIZE_NAVQUERY")
#undef INITIALIZE_NAVQUERY

#pragma push_macro("INITIALIZE_NAVQUERY_WLINKFILTER")
#undef INITIALIZE_NAVQUERY_WLINKFILTER

#pragma push_macro("TEXT_WEAKOBJ_NAME")
#undef TEXT_WEAKOBJ_NAME

#pragma push_macro("REGISTER_NET_ANALYTICS")
#undef REGISTER_NET_ANALYTICS

#pragma push_macro("CONDITIONAL_ON_PUSH_MODEL")
#undef CONDITIONAL_ON_PUSH_MODEL

#pragma push_macro("IS_PUSH_MODEL_ENABLED")
#undef IS_PUSH_MODEL_ENABLED

#pragma push_macro("PUSH_MAKE_BP_PROPERTIES_PUSH_MODEL")
#undef PUSH_MAKE_BP_PROPERTIES_PUSH_MODEL

#pragma push_macro("GET_PROPERTY_REP_INDEX")
#undef GET_PROPERTY_REP_INDEX

#pragma push_macro("GET_PROPERTY_REP_INDEX_STATIC_ARRAY_START")
#undef GET_PROPERTY_REP_INDEX_STATIC_ARRAY_START

#pragma push_macro("GET_PROPERTY_REP_INDEX_STATIC_ARRAY_END")
#undef GET_PROPERTY_REP_INDEX_STATIC_ARRAY_END

#pragma push_macro("GET_PROPERTY_REP_INDEX_STATIC_ARRAY_INDEX")
#undef GET_PROPERTY_REP_INDEX_STATIC_ARRAY_INDEX

#pragma push_macro("IS_PROPERTY_REPLICATED")
#undef IS_PROPERTY_REPLICATED

#pragma push_macro("CONDITIONAL_ON_OBJECT_NET_ID")
#undef CONDITIONAL_ON_OBJECT_NET_ID

#pragma push_macro("CONDITIONAL_ON_OBJECT_NET_ID_DYNAMIC")
#undef CONDITIONAL_ON_OBJECT_NET_ID_DYNAMIC

#pragma push_macro("CONDITIONAL_ON_REP_INDEX_AND_OBJECT_NET_ID")
#undef CONDITIONAL_ON_REP_INDEX_AND_OBJECT_NET_ID

#pragma push_macro("MARK_PROPERTY_DIRTY_UNSAFE")
#undef MARK_PROPERTY_DIRTY_UNSAFE

#pragma push_macro("MARK_PROPERTY_DIRTY")
#undef MARK_PROPERTY_DIRTY

#pragma push_macro("MARK_PROPERTY_DIRTY_STATIC_ARRAY_INDEX")
#undef MARK_PROPERTY_DIRTY_STATIC_ARRAY_INDEX

#pragma push_macro("MARK_PROPERTY_DIRTY_STATIC_ARRAY")
#undef MARK_PROPERTY_DIRTY_STATIC_ARRAY

#pragma push_macro("MARK_PROPERTY_DIRTY_FROM_NAME")
#undef MARK_PROPERTY_DIRTY_FROM_NAME

#pragma push_macro("MARK_PROPERTY_DIRTY_FROM_NAME_STATIC_ARRAY_INDEX")
#undef MARK_PROPERTY_DIRTY_FROM_NAME_STATIC_ARRAY_INDEX

#pragma push_macro("MARK_PROPERTY_DIRTY_FROM_NAME_STATIC_ARRAY")
#undef MARK_PROPERTY_DIRTY_FROM_NAME_STATIC_ARRAY

#pragma push_macro("GET_PROPERTY_REP_INDEX_STATIC_ARRAY")
#undef GET_PROPERTY_REP_INDEX_STATIC_ARRAY

#pragma push_macro("REPLICATED_BASE_CLASS")
#undef REPLICATED_BASE_CLASS

#pragma push_macro("UE_NET_TRACE_CREATE_COLLECTOR")
#undef UE_NET_TRACE_CREATE_COLLECTOR

#pragma push_macro("UE_NET_TRACE_DESTROY_COLLECTOR")
#undef UE_NET_TRACE_DESTROY_COLLECTOR

#pragma push_macro("UE_NET_TRACE_SCOPE")
#undef UE_NET_TRACE_SCOPE

#pragma push_macro("UE_NET_TRACE_OBJECT_SCOPE")
#undef UE_NET_TRACE_OBJECT_SCOPE

#pragma push_macro("UE_NET_TRACE_DYNAMIC_NAME_SCOPE")
#undef UE_NET_TRACE_DYNAMIC_NAME_SCOPE

#pragma push_macro("UE_NET_TRACE_NAMED_SCOPE")
#undef UE_NET_TRACE_NAMED_SCOPE

#pragma push_macro("UE_NET_TRACE_NAMED_OBJECT_SCOPE")
#undef UE_NET_TRACE_NAMED_OBJECT_SCOPE

#pragma push_macro("UE_NET_TRACE_NAMED_DYNAMIC_NAME_SCOPE")
#undef UE_NET_TRACE_NAMED_DYNAMIC_NAME_SCOPE

#pragma push_macro("UE_NET_TRACE_SET_SCOPE_NAME")
#undef UE_NET_TRACE_SET_SCOPE_NAME

#pragma push_macro("UE_NET_TRACE_SET_SCOPE_OBJECTID")
#undef UE_NET_TRACE_SET_SCOPE_OBJECTID

#pragma push_macro("UE_NET_TRACE_EXIT_NAMED_SCOPE")
#undef UE_NET_TRACE_EXIT_NAMED_SCOPE

#pragma push_macro("UE_NET_TRACE_OFFSET_SCOPE")
#undef UE_NET_TRACE_OFFSET_SCOPE

#pragma push_macro("UE_NET_TRACE")
#undef UE_NET_TRACE

#pragma push_macro("UE_NET_TRACE_DYNAMIC_NAME")
#undef UE_NET_TRACE_DYNAMIC_NAME

#pragma push_macro("UE_NET_TRACE_FLUSH_COLLECTOR")
#undef UE_NET_TRACE_FLUSH_COLLECTOR

#pragma push_macro("UE_NET_TRACE_BEGIN_BUNCH")
#undef UE_NET_TRACE_BEGIN_BUNCH

#pragma push_macro("UE_NET_TRACE_DISCARD_BUNCH")
#undef UE_NET_TRACE_DISCARD_BUNCH

#pragma push_macro("UE_NET_TRACE_POP_SEND_BUNCH")
#undef UE_NET_TRACE_POP_SEND_BUNCH

#pragma push_macro("UE_NET_TRACE_EVENTS")
#undef UE_NET_TRACE_EVENTS

#pragma push_macro("UE_NET_TRACE_END_BUNCH")
#undef UE_NET_TRACE_END_BUNCH

#pragma push_macro("UE_NET_TRACE_BUNCH_SCOPE")
#undef UE_NET_TRACE_BUNCH_SCOPE

#pragma push_macro("UE_NET_TRACE_ASSIGNED_GUID")
#undef UE_NET_TRACE_ASSIGNED_GUID

#pragma push_macro("UE_NET_TRACE_NETHANDLE_CREATED")
#undef UE_NET_TRACE_NETHANDLE_CREATED

#pragma push_macro("UE_NET_TRACE_NETHANDLE_DESTROYED")
#undef UE_NET_TRACE_NETHANDLE_DESTROYED

#pragma push_macro("UE_NET_TRACE_CONNECTION_CREATED")
#undef UE_NET_TRACE_CONNECTION_CREATED

#pragma push_macro("UE_NET_TRACE_CONNECTION_CLOSED")
#undef UE_NET_TRACE_CONNECTION_CLOSED

#pragma push_macro("UE_NET_TRACE_PACKET_DROPPED")
#undef UE_NET_TRACE_PACKET_DROPPED

#pragma push_macro("UE_NET_TRACE_PACKET_SEND")
#undef UE_NET_TRACE_PACKET_SEND

#pragma push_macro("UE_NET_TRACE_PACKET_RECV")
#undef UE_NET_TRACE_PACKET_RECV

#pragma push_macro("UE_NET_TRACE_END_SESSION")
#undef UE_NET_TRACE_END_SESSION

#pragma push_macro("UE_NET_TRACE_DO_IF")
#undef UE_NET_TRACE_DO_IF

#pragma push_macro("UE_NET_TRACE_INTERNAL_CREATE_COLLECTOR")
#undef UE_NET_TRACE_INTERNAL_CREATE_COLLECTOR

#pragma push_macro("UE_NET_TRACE_INTERNAL_DESTROY_COLLECTOR")
#undef UE_NET_TRACE_INTERNAL_DESTROY_COLLECTOR

#pragma push_macro("UE_NET_TRACE_INTERNAL_FLUSH_COLLECTOR")
#undef UE_NET_TRACE_INTERNAL_FLUSH_COLLECTOR

#pragma push_macro("UE_NET_TRACE_INTERNAL_BEGIN_BUNCH")
#undef UE_NET_TRACE_INTERNAL_BEGIN_BUNCH

#pragma push_macro("UE_NET_TRACE_INTERNAL_DISCARD_BUNCH")
#undef UE_NET_TRACE_INTERNAL_DISCARD_BUNCH

#pragma push_macro("UE_NET_TRACE_INTERNAL_POP_SEND_BUNCH")
#undef UE_NET_TRACE_INTERNAL_POP_SEND_BUNCH

#pragma push_macro("UE_NET_TRACE_INTERNAL_EVENTS")
#undef UE_NET_TRACE_INTERNAL_EVENTS

#pragma push_macro("UE_NET_TRACE_INTERNAL_END_BUNCH")
#undef UE_NET_TRACE_INTERNAL_END_BUNCH

#pragma push_macro("UE_NET_TRACE_INTERNAL_BUNCH_SCOPE")
#undef UE_NET_TRACE_INTERNAL_BUNCH_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_SCOPE")
#undef UE_NET_TRACE_INTERNAL_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_OBJECT_SCOPE")
#undef UE_NET_TRACE_INTERNAL_OBJECT_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_DYNAMIC_NAME_SCOPE")
#undef UE_NET_TRACE_INTERNAL_DYNAMIC_NAME_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_NAMED_SCOPE")
#undef UE_NET_TRACE_INTERNAL_NAMED_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_NAMED_OBJECT_SCOPE")
#undef UE_NET_TRACE_INTERNAL_NAMED_OBJECT_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_NAMED_DYNAMIC_NAME_SCOPE")
#undef UE_NET_TRACE_INTERNAL_NAMED_DYNAMIC_NAME_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_SET_SCOPE_NAME")
#undef UE_NET_TRACE_INTERNAL_SET_SCOPE_NAME

#pragma push_macro("UE_NET_TRACE_INTERNAL_SET_SCOPE_OBJECTID")
#undef UE_NET_TRACE_INTERNAL_SET_SCOPE_OBJECTID

#pragma push_macro("UE_NET_TRACE_INTERNAL_EXIT_NAMED_SCOPE")
#undef UE_NET_TRACE_INTERNAL_EXIT_NAMED_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL_OFFSET_SCOPE")
#undef UE_NET_TRACE_INTERNAL_OFFSET_SCOPE

#pragma push_macro("UE_NET_TRACE_INTERNAL")
#undef UE_NET_TRACE_INTERNAL

#pragma push_macro("UE_NET_TRACE_INTERNAL_DYNAMIC_NAME")
#undef UE_NET_TRACE_INTERNAL_DYNAMIC_NAME

#pragma push_macro("UE_NET_TRACE_INTERNAL_ASSIGNED_GUID")
#undef UE_NET_TRACE_INTERNAL_ASSIGNED_GUID

#pragma push_macro("UE_NET_TRACE_INTERNAL_NETHANDLE_CREATED")
#undef UE_NET_TRACE_INTERNAL_NETHANDLE_CREATED

#pragma push_macro("UE_NET_TRACE_INTERNAL_NETHANDLE_DESTROYED")
#undef UE_NET_TRACE_INTERNAL_NETHANDLE_DESTROYED

#pragma push_macro("UE_NET_TRACE_INTERNAL_CONNECTION_CREATED")
#undef UE_NET_TRACE_INTERNAL_CONNECTION_CREATED

#pragma push_macro("UE_NET_TRACE_INTERNAL_CONNECTION_CLOSED")
#undef UE_NET_TRACE_INTERNAL_CONNECTION_CLOSED

#pragma push_macro("UE_NET_TRACE_INTERNAL_PACKET_DROPPED")
#undef UE_NET_TRACE_INTERNAL_PACKET_DROPPED

#pragma push_macro("UE_NET_TRACE_INTERNAL_PACKET_SEND")
#undef UE_NET_TRACE_INTERNAL_PACKET_SEND

#pragma push_macro("UE_NET_TRACE_INTERNAL_PACKET_RECV")
#undef UE_NET_TRACE_INTERNAL_PACKET_RECV

#pragma push_macro("UE_NET_TRACE_INTERNAL_END_SESSION")
#undef UE_NET_TRACE_INTERNAL_END_SESSION

#pragma push_macro("CASE_ENUM_TO_STR")
#undef CASE_ENUM_TO_STR

#pragma push_macro("RETURN_IF_EQUAL")
#undef RETURN_IF_EQUAL

#pragma push_macro("ROTLEFT_64B")
#undef ROTLEFT_64B

#pragma push_macro("_TEST_EQUAL")
#undef _TEST_EQUAL

#pragma push_macro("_TEST_NOT_EQUAL")
#undef _TEST_NOT_EQUAL

#pragma push_macro("_TEST_NULL")
#undef _TEST_NULL

#pragma push_macro("_TEST_NOT_NULL")
#undef _TEST_NOT_NULL

#pragma push_macro("TEST_EQUAL")
#undef TEST_EQUAL

#pragma push_macro("TEST_NOT_EQUAL")
#undef TEST_NOT_EQUAL

#pragma push_macro("TEST_TRUE")
#undef TEST_TRUE

#pragma push_macro("TEST_FALSE")
#undef TEST_FALSE

#pragma push_macro("TEST_NULL")
#undef TEST_NULL

#pragma push_macro("TEST_NOT_NULL")
#undef TEST_NOT_NULL

#pragma push_macro("TEST_BECOMES_TRUE")
#undef TEST_BECOMES_TRUE

#pragma push_macro("MOCK_FUNC_NOT_IMPLEMENTED")
#undef MOCK_FUNC_NOT_IMPLEMENTED

#pragma push_macro("ARRAY")
#undef ARRAY

#pragma push_macro("ARRAYU64")
#undef ARRAYU64

#pragma push_macro("CASE_ENUM_SET")
#undef CASE_ENUM_SET

#pragma push_macro("MAP_TO_RESULTSTRING")
#undef MAP_TO_RESULTSTRING

#pragma push_macro("PrintCurlFeature")
#undef PrintCurlFeature

#pragma push_macro("EnumCase")
#undef EnumCase

#pragma push_macro("RESTRICT_SSL_TLS_PROTOCOL")
#undef RESTRICT_SSL_TLS_PROTOCOL

#pragma push_macro("DEFINE_COMMAND")
#undef DEFINE_COMMAND

#pragma push_macro("DECLARE_COMMAND")
#undef DECLARE_COMMAND

#pragma push_macro("IMPL_HEADER_FIELD_GETTER")
#undef IMPL_HEADER_FIELD_GETTER

#pragma push_macro("OPUS_CHECK_CTL")
#undef OPUS_CHECK_CTL

#pragma push_macro("LWSWEBSOCKET_ESTATE_TOSTRING")
#undef LWSWEBSOCKET_ESTATE_TOSTRING

#pragma push_macro("LOG_AND_GET_GL_QUERY_INT")
#undef LOG_AND_GET_GL_QUERY_INT

#pragma push_macro("INDEX_TO_VOID")
#undef INDEX_TO_VOID

#pragma push_macro("ASSERT_NO_GL_ERROR")
#undef ASSERT_NO_GL_ERROR

#pragma push_macro("LOG_GL_STRING")
#undef LOG_GL_STRING

#pragma push_macro("LOG_AND_GET_GL_INT_TEMP")
#undef LOG_AND_GET_GL_INT_TEMP

#pragma push_macro("CHECK_COMPRESSED_FORMAT")
#undef CHECK_COMPRESSED_FORMAT

#pragma push_macro("LOG_GL_DEBUG_FLAG")
#undef LOG_GL_DEBUG_FLAG

#pragma push_macro("GL_CHECK")
#undef GL_CHECK

#pragma push_macro("LOG_AND_GET_GL_INT")
#undef LOG_AND_GET_GL_INT

#pragma push_macro("GET_GL_INT")
#undef GET_GL_INT

#pragma push_macro("INTERFACE_BLOCK")
#undef INTERFACE_BLOCK

#pragma push_macro("VERIFY_EGL")
#undef VERIFY_EGL

#pragma push_macro("MACRO_TOKENIZER")
#undef MACRO_TOKENIZER

#pragma push_macro("MACRO_TOKENIZER2")
#undef MACRO_TOKENIZER2

#pragma push_macro("VERIFY_EGL_SCOPE_WITH_MSG_STR")
#undef VERIFY_EGL_SCOPE_WITH_MSG_STR

#pragma push_macro("VERIFY_EGL_SCOPE")
#undef VERIFY_EGL_SCOPE

#pragma push_macro("VERIFY_EGL_FUNC")
#undef VERIFY_EGL_FUNC

#pragma push_macro("QUERY_CHECK")
#undef QUERY_CHECK

#pragma push_macro("DEFINE_GL_ENTRYPOINTS")
#undef DEFINE_GL_ENTRYPOINTS

#pragma push_macro("GET_GL_ENTRYPOINTS")
#undef GET_GL_ENTRYPOINTS

#pragma push_macro("CHECK_GL_ENTRYPOINTS")
#undef CHECK_GL_ENTRYPOINTS

#pragma push_macro("ENUM_GL_ENTRYPOINTS")
#undef ENUM_GL_ENTRYPOINTS

#pragma push_macro("ENUM_GL_ENTRYPOINTS_OPTIONAL")
#undef ENUM_GL_ENTRYPOINTS_OPTIONAL

#pragma push_macro("ENUM_GL_ENTRYPOINTS_ALL")
#undef ENUM_GL_ENTRYPOINTS_ALL

#pragma push_macro("DECLARE_GL_ENTRYPOINTS")
#undef DECLARE_GL_ENTRYPOINTS

#pragma push_macro("CHECK_GL_ENTRYPOINTS_NULL")
#undef CHECK_GL_ENTRYPOINTS_NULL

#pragma push_macro("CHECK_GL_ENTRYPOINTS_OK")
#undef CHECK_GL_ENTRYPOINTS_OK

#pragma push_macro("ENUM_GL_ENTRYPOINTS_1_0")
#undef ENUM_GL_ENTRYPOINTS_1_0

#pragma push_macro("ENUM_GL_ENTRYPOINTS_1_1")
#undef ENUM_GL_ENTRYPOINTS_1_1

#pragma push_macro("ENUM_GL_ENTRYPOINTS_1_3")
#undef ENUM_GL_ENTRYPOINTS_1_3

#pragma push_macro("ENUM_GL_ENTRYPOINTS_1_4")
#undef ENUM_GL_ENTRYPOINTS_1_4

#pragma push_macro("ENUM_GL_ENTRYPOINTS_1_5")
#undef ENUM_GL_ENTRYPOINTS_1_5

#pragma push_macro("ENUM_GL_ENTRYPOINTS_2_0")
#undef ENUM_GL_ENTRYPOINTS_2_0

#pragma push_macro("ENUM_GL_ENTRYPOINTS_2_1")
#undef ENUM_GL_ENTRYPOINTS_2_1

#pragma push_macro("ENUM_GL_ENTRYPOINTS_3_0")
#undef ENUM_GL_ENTRYPOINTS_3_0

#pragma push_macro("ENUM_GL_ENTRYPOINTS_3_1")
#undef ENUM_GL_ENTRYPOINTS_3_1

#pragma push_macro("ENUM_GL_ENTRYPOINTS_3_2")
#undef ENUM_GL_ENTRYPOINTS_3_2

#pragma push_macro("ENUM_GL_ENTRYPOINTS_3_3")
#undef ENUM_GL_ENTRYPOINTS_3_3

#pragma push_macro("ENUM_GL_ENTRYPOINTS_4_0")
#undef ENUM_GL_ENTRYPOINTS_4_0

#pragma push_macro("ENUM_GL_ENTRYPOINTS_4_1")
#undef ENUM_GL_ENTRYPOINTS_4_1

#pragma push_macro("ENUM_GL_ENTRYPOINTS_4_2")
#undef ENUM_GL_ENTRYPOINTS_4_2

#pragma push_macro("ENUM_GL_ENTRYPOINTS_4_3")
#undef ENUM_GL_ENTRYPOINTS_4_3

#pragma push_macro("ENUM_GL_ENTRYPOINTS_4_4")
#undef ENUM_GL_ENTRYPOINTS_4_4

#pragma push_macro("ENUM_GL_ENTRYPOINTS_4_5")
#undef ENUM_GL_ENTRYPOINTS_4_5

#pragma push_macro("GET_GL_ENTRYPOINTS_DLL")
#undef GET_GL_ENTRYPOINTS_DLL

#pragma push_macro("ENUM_GL_ENTRYPOINTS_DLL")
#undef ENUM_GL_ENTRYPOINTS_DLL

#pragma push_macro("UGL_REQUIRED")
#undef UGL_REQUIRED

#pragma push_macro("UGL_OPTIONAL")
#undef UGL_OPTIONAL

#pragma push_macro("RHITHREAD_GLCOMMAND_PROLOGUE")
#undef RHITHREAD_GLCOMMAND_PROLOGUE

#pragma push_macro("RHITHREAD_GLCOMMAND_EPILOGUE_RETURN")
#undef RHITHREAD_GLCOMMAND_EPILOGUE_RETURN

#pragma push_macro("RHITHREAD_GLCOMMAND_EPILOGUE_GET_RETURN")
#undef RHITHREAD_GLCOMMAND_EPILOGUE_GET_RETURN

#pragma push_macro("RHITHREAD_GLCOMMAND_EPILOGUE")
#undef RHITHREAD_GLCOMMAND_EPILOGUE

#pragma push_macro("RHITHREAD_GLCOMMAND_EPILOGUE_NORETURN")
#undef RHITHREAD_GLCOMMAND_EPILOGUE_NORETURN

#pragma push_macro("SCOPE_CYCLE_COUNTER_DETAILED")
#undef SCOPE_CYCLE_COUNTER_DETAILED

#pragma push_macro("DETAILED_QUICK_SCOPE_CYCLE_COUNTER")
#undef DETAILED_QUICK_SCOPE_CYCLE_COUNTER

#pragma push_macro("GLAF_CHECK")
#undef GLAF_CHECK

#pragma push_macro("CHECK_EXPECTED_GL_THREAD")
#undef CHECK_EXPECTED_GL_THREAD

#pragma push_macro("VERIFY_GL")
#undef VERIFY_GL

#pragma push_macro("VERIFY_GL_SCOPE_WITH_MSG_STR")
#undef VERIFY_GL_SCOPE_WITH_MSG_STR

#pragma push_macro("VERIFY_GL_SCOPE")
#undef VERIFY_GL_SCOPE

#pragma push_macro("VERIFY_GL_FUNC")
#undef VERIFY_GL_FUNC

#pragma push_macro("glBlitFramebuffer")
#undef glBlitFramebuffer

#pragma push_macro("glTexImage2D")
#undef glTexImage2D

#pragma push_macro("glTexSubImage2D")
#undef glTexSubImage2D

#pragma push_macro("glCompressedTexImage2D")
#undef glCompressedTexImage2D

#pragma push_macro("REPORT_GL_DRAW_ARRAYS_EVENT_FOR_FRAME_DUMP")
#undef REPORT_GL_DRAW_ARRAYS_EVENT_FOR_FRAME_DUMP

#pragma push_macro("REPORT_GL_DRAW_ARRAYS_INSTANCED_EVENT_FOR_FRAME_DUMP")
#undef REPORT_GL_DRAW_ARRAYS_INSTANCED_EVENT_FOR_FRAME_DUMP

#pragma push_macro("REPORT_GL_DRAW_RANGE_ELEMENTS_EVENT_FOR_FRAME_DUMP")
#undef REPORT_GL_DRAW_RANGE_ELEMENTS_EVENT_FOR_FRAME_DUMP

#pragma push_macro("REPORT_GL_DRAW_ELEMENTS_INSTANCED_EVENT_FOR_FRAME_DUMP")
#undef REPORT_GL_DRAW_ELEMENTS_INSTANCED_EVENT_FOR_FRAME_DUMP

#pragma push_macro("REPORT_GL_CLEAR_EVENT_FOR_FRAME_DUMP")
#undef REPORT_GL_CLEAR_EVENT_FOR_FRAME_DUMP

#pragma push_macro("REPORT_GL_FRAMEBUFFER_BLIT_EVENT")
#undef REPORT_GL_FRAMEBUFFER_BLIT_EVENT

#pragma push_macro("REPORT_GL_END_BUFFER_EVENT_FOR_FRAME_DUMP")
#undef REPORT_GL_END_BUFFER_EVENT_FOR_FRAME_DUMP

#pragma push_macro("INITIATE_GL_FRAME_DUMP")
#undef INITIATE_GL_FRAME_DUMP

#pragma push_macro("INITIATE_GL_FRAME_DUMP_EVERY_X_CALLS")
#undef INITIATE_GL_FRAME_DUMP_EVERY_X_CALLS

#pragma push_macro("SCOPED_SCENE_READ_LOCK")
#undef SCOPED_SCENE_READ_LOCK

#pragma push_macro("SCOPED_SCENE_WRITE_LOCK")
#undef SCOPED_SCENE_WRITE_LOCK

#pragma push_macro("SCOPED_APEX_SCENE_READ_LOCK")
#undef SCOPED_APEX_SCENE_READ_LOCK

#pragma push_macro("SCOPED_APEX_SCENE_WRITE_LOCK")
#undef SCOPED_APEX_SCENE_WRITE_LOCK

#pragma push_macro("SCENE_LOCK_READ")
#undef SCENE_LOCK_READ

#pragma push_macro("SCENE_UNLOCK_READ")
#undef SCENE_UNLOCK_READ

#pragma push_macro("SCENE_LOCK_WRITE")
#undef SCENE_LOCK_WRITE

#pragma push_macro("SCENE_UNLOCK_WRITE")
#undef SCENE_UNLOCK_WRITE

#pragma push_macro("SCOPED_SCENE_READ_LOCK_INDEXED")
#undef SCOPED_SCENE_READ_LOCK_INDEXED

#pragma push_macro("SCOPED_SCENE_WRITE_LOCK_INDEXED")
#undef SCOPED_SCENE_WRITE_LOCK_INDEXED

#pragma push_macro("SQ_REPLAY_TEST")
#undef SQ_REPLAY_TEST

#pragma push_macro("BROADCAST_PROPERTY_CHANGED")
#undef BROADCAST_PROPERTY_CHANGED

#pragma push_macro("IMPLEMENT_COPY_RESOURCE_SHADER")
#undef IMPLEMENT_COPY_RESOURCE_SHADER

#pragma push_macro("IMPLEMENT_COPY_RESOURCE_SHADER_ALL_TYPES")
#undef IMPLEMENT_COPY_RESOURCE_SHADER_ALL_TYPES

#pragma push_macro("IMPLEMENT_ONECOLORVS")
#undef IMPLEMENT_ONECOLORVS

#pragma push_macro("EmitRDGWarningf")
#undef EmitRDGWarningf

#pragma push_macro("TEXT_TO_PIXELFORMAT")
#undef TEXT_TO_PIXELFORMAT

#pragma push_macro("DECLARE_GLOBAL_SHADER")
#undef DECLARE_GLOBAL_SHADER

#pragma push_macro("IMPLEMENT_GLOBAL_SHADER")
#undef IMPLEMENT_GLOBAL_SHADER

#pragma push_macro("RDG_ASYNC_COMPUTE_BUDGET_SCOPE")
#undef RDG_ASYNC_COMPUTE_BUDGET_SCOPE

#pragma push_macro("RDG_GPU_MASK_SCOPE")
#undef RDG_GPU_MASK_SCOPE

#pragma push_macro("IF_RDG_ENABLE_DEBUG")
#undef IF_RDG_ENABLE_DEBUG

#pragma push_macro("IF_RDG_GPU_SCOPES")
#undef IF_RDG_GPU_SCOPES

#pragma push_macro("IF_RDG_CPU_SCOPES")
#undef IF_RDG_CPU_SCOPES

#pragma push_macro("RDG_EVENT_NAME")
#undef RDG_EVENT_NAME

#pragma push_macro("RDG_EVENT_SCOPE")
#undef RDG_EVENT_SCOPE

#pragma push_macro("RDG_EVENT_SCOPE_CONDITIONAL")
#undef RDG_EVENT_SCOPE_CONDITIONAL

#pragma push_macro("RDG_GPU_STAT_SCOPE")
#undef RDG_GPU_STAT_SCOPE

#pragma push_macro("RDG_CSV_STAT_EXCLUSIVE_SCOPE")
#undef RDG_CSV_STAT_EXCLUSIVE_SCOPE

#pragma push_macro("RDG_CSV_STAT_EXCLUSIVE_SCOPE_CONDITIONAL")
#undef RDG_CSV_STAT_EXCLUSIVE_SCOPE_CONDITIONAL

#pragma push_macro("RDG_WAIT_FOR_TASKS_CONDITIONAL")
#undef RDG_WAIT_FOR_TASKS_CONDITIONAL

#pragma push_macro("RDG_WAIT_FOR_TASKS")
#undef RDG_WAIT_FOR_TASKS

#pragma push_macro("SCOPED_SUSPEND_RENDERING_THREAD")
#undef SCOPED_SUSPEND_RENDERING_THREAD

#pragma push_macro("LogRenderCommand")
#undef LogRenderCommand

#pragma push_macro("TASK_FUNCTION")
#undef TASK_FUNCTION

#pragma push_macro("TASKNAME_FUNCTION")
#undef TASKNAME_FUNCTION

#pragma push_macro("ENQUEUE_RENDER_COMMAND")
#undef ENQUEUE_RENDER_COMMAND

#pragma push_macro("SHADER_DECLARE_VTABLE")
#undef SHADER_DECLARE_VTABLE

#pragma push_macro("INTERNAL_DECLARE_SHADER_TYPE_COMMON")
#undef INTERNAL_DECLARE_SHADER_TYPE_COMMON

#pragma push_macro("DECLARE_EXPORTED_SHADER_TYPE")
#undef DECLARE_EXPORTED_SHADER_TYPE

#pragma push_macro("DECLARE_SHADER_TYPE")
#undef DECLARE_SHADER_TYPE

#pragma push_macro("DECLARE_SHADER_TYPE_EXPLICIT_BASES")
#undef DECLARE_SHADER_TYPE_EXPLICIT_BASES

#pragma push_macro("SHADER_TYPE_VTABLE")
#undef SHADER_TYPE_VTABLE

#pragma push_macro("IMPLEMENT_SHADER_TYPE")
#undef IMPLEMENT_SHADER_TYPE

#pragma push_macro("IMPLEMENT_SHADER_TYPE_WITH_DEBUG_NAME")
#undef IMPLEMENT_SHADER_TYPE_WITH_DEBUG_NAME

#pragma push_macro("IMPLEMENT_SHADER_TYPE2_WITH_TEMPLATE_PREFIX")
#undef IMPLEMENT_SHADER_TYPE2_WITH_TEMPLATE_PREFIX

#pragma push_macro("IMPLEMENT_SHADER_TYPE2")
#undef IMPLEMENT_SHADER_TYPE2

#pragma push_macro("IMPLEMENT_SHADER_TYPE3")
#undef IMPLEMENT_SHADER_TYPE3

#pragma push_macro("IMPLEMENT_SHADER_TYPE4_WITH_TEMPLATE_PREFIX")
#undef IMPLEMENT_SHADER_TYPE4_WITH_TEMPLATE_PREFIX

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSPS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSPS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSGSPS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSGSPS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSGS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSGS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDSPS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDSPS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDSGSPS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDSGSPS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDS

#pragma push_macro("IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDSGS")
#undef IMPLEMENT_SHADERPIPELINE_TYPE_VSHSDSGS

#pragma push_macro("IMPLEMENT_ALIGNED_TYPE")
#undef IMPLEMENT_ALIGNED_TYPE

#pragma push_macro("INTERNAL_UNIFORM_BUFFER_STRUCT_GET_STRUCT_METADATA")
#undef INTERNAL_UNIFORM_BUFFER_STRUCT_GET_STRUCT_METADATA

#pragma push_macro("INTERNAL_SHADER_PARAMETER_GET_STRUCT_METADATA")
#undef INTERNAL_SHADER_PARAMETER_GET_STRUCT_METADATA

#pragma push_macro("INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN")
#undef INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN

#pragma push_macro("INTERNAL_SHADER_PARAMETER_EXPLICIT")
#undef INTERNAL_SHADER_PARAMETER_EXPLICIT

#pragma push_macro("BEGIN_SHADER_PARAMETER_STRUCT")
#undef BEGIN_SHADER_PARAMETER_STRUCT

#pragma push_macro("END_SHADER_PARAMETER_STRUCT")
#undef END_SHADER_PARAMETER_STRUCT

#pragma push_macro("BEGIN_UNIFORM_BUFFER_STRUCT")
#undef BEGIN_UNIFORM_BUFFER_STRUCT

#pragma push_macro("BEGIN_UNIFORM_BUFFER_STRUCT_WITH_CONSTRUCTOR")
#undef BEGIN_UNIFORM_BUFFER_STRUCT_WITH_CONSTRUCTOR

#pragma push_macro("END_UNIFORM_BUFFER_STRUCT")
#undef END_UNIFORM_BUFFER_STRUCT

#pragma push_macro("IMPLEMENT_UNIFORM_BUFFER_STRUCT")
#undef IMPLEMENT_UNIFORM_BUFFER_STRUCT

#pragma push_macro("IMPLEMENT_UNIFORM_BUFFER_ALIAS_STRUCT")
#undef IMPLEMENT_UNIFORM_BUFFER_ALIAS_STRUCT

#pragma push_macro("IMPLEMENT_STATIC_UNIFORM_BUFFER_STRUCT")
#undef IMPLEMENT_STATIC_UNIFORM_BUFFER_STRUCT

#pragma push_macro("IMPLEMENT_STATIC_UNIFORM_BUFFER_SLOT")
#undef IMPLEMENT_STATIC_UNIFORM_BUFFER_SLOT

#pragma push_macro("SHADER_PARAMETER")
#undef SHADER_PARAMETER

#pragma push_macro("SHADER_PARAMETER_EX")
#undef SHADER_PARAMETER_EX

#pragma push_macro("SHADER_PARAMETER_ARRAY")
#undef SHADER_PARAMETER_ARRAY

#pragma push_macro("SHADER_PARAMETER_ARRAY_EX")
#undef SHADER_PARAMETER_ARRAY_EX

#pragma push_macro("SHADER_PARAMETER_TEXTURE")
#undef SHADER_PARAMETER_TEXTURE

#pragma push_macro("SHADER_PARAMETER_TEXTURE_ARRAY")
#undef SHADER_PARAMETER_TEXTURE_ARRAY

#pragma push_macro("SHADER_PARAMETER_SRV")
#undef SHADER_PARAMETER_SRV

#pragma push_macro("SHADER_PARAMETER_SRV_ARRAY")
#undef SHADER_PARAMETER_SRV_ARRAY

#pragma push_macro("SHADER_PARAMETER_UAV")
#undef SHADER_PARAMETER_UAV

#pragma push_macro("SHADER_PARAMETER_SAMPLER")
#undef SHADER_PARAMETER_SAMPLER

#pragma push_macro("SHADER_PARAMETER_SAMPLER_ARRAY")
#undef SHADER_PARAMETER_SAMPLER_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_TEXTURE")
#undef SHADER_PARAMETER_RDG_TEXTURE

#pragma push_macro("SHADER_PARAMETER_RDG_TEXTURE_ARRAY")
#undef SHADER_PARAMETER_RDG_TEXTURE_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_TEXTURE_SRV")
#undef SHADER_PARAMETER_RDG_TEXTURE_SRV

#pragma push_macro("SHADER_PARAMETER_RDG_TEXTURE_SRV_ARRAY")
#undef SHADER_PARAMETER_RDG_TEXTURE_SRV_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_TEXTURE_UAV")
#undef SHADER_PARAMETER_RDG_TEXTURE_UAV

#pragma push_macro("SHADER_PARAMETER_RDG_TEXTURE_UAV_ARRAY")
#undef SHADER_PARAMETER_RDG_TEXTURE_UAV_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER")
#undef SHADER_PARAMETER_RDG_BUFFER

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER_ARRAY")
#undef SHADER_PARAMETER_RDG_BUFFER_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER_SRV")
#undef SHADER_PARAMETER_RDG_BUFFER_SRV

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER_SRV_ARRAY")
#undef SHADER_PARAMETER_RDG_BUFFER_SRV_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER_UAV")
#undef SHADER_PARAMETER_RDG_BUFFER_UAV

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER_UAV_ARRAY")
#undef SHADER_PARAMETER_RDG_BUFFER_UAV_ARRAY

#pragma push_macro("SHADER_PARAMETER_RDG_UNIFORM_BUFFER")
#undef SHADER_PARAMETER_RDG_UNIFORM_BUFFER

#pragma push_macro("SHADER_PARAMETER_STRUCT")
#undef SHADER_PARAMETER_STRUCT

#pragma push_macro("SHADER_PARAMETER_STRUCT_ARRAY")
#undef SHADER_PARAMETER_STRUCT_ARRAY

#pragma push_macro("SHADER_PARAMETER_STRUCT_INCLUDE")
#undef SHADER_PARAMETER_STRUCT_INCLUDE

#pragma push_macro("SHADER_PARAMETER_STRUCT_REF")
#undef SHADER_PARAMETER_STRUCT_REF

#pragma push_macro("RDG_BUFFER_ACCESS")
#undef RDG_BUFFER_ACCESS

#pragma push_macro("RDG_BUFFER_ACCESS_DYNAMIC")
#undef RDG_BUFFER_ACCESS_DYNAMIC

#pragma push_macro("RDG_TEXTURE_ACCESS")
#undef RDG_TEXTURE_ACCESS

#pragma push_macro("RDG_TEXTURE_ACCESS_DYNAMIC")
#undef RDG_TEXTURE_ACCESS_DYNAMIC

#pragma push_macro("SHADER_PARAMETER_RDG_BUFFER_UPLOAD")
#undef SHADER_PARAMETER_RDG_BUFFER_UPLOAD

#pragma push_macro("RENDER_TARGET_BINDING_SLOTS")
#undef RENDER_TARGET_BINDING_SLOTS

#pragma push_macro("SHADER_USE_PARAMETER_STRUCT_INTERNAL")
#undef SHADER_USE_PARAMETER_STRUCT_INTERNAL

#pragma push_macro("SHADER_USE_PARAMETER_STRUCT")
#undef SHADER_USE_PARAMETER_STRUCT

#pragma push_macro("SHADER_USE_PARAMETER_STRUCT_WITH_LEGACY_BASE")
#undef SHADER_USE_PARAMETER_STRUCT_WITH_LEGACY_BASE

#pragma push_macro("SHADER_USE_ROOT_PARAMETER_STRUCT")
#undef SHADER_USE_ROOT_PARAMETER_STRUCT

#pragma push_macro("DECLARE_SHADER_PERMUTATION_IMPL")
#undef DECLARE_SHADER_PERMUTATION_IMPL

#pragma push_macro("SHADER_PERMUTATION_BOOL")
#undef SHADER_PERMUTATION_BOOL

#pragma push_macro("SHADER_PERMUTATION_INT")
#undef SHADER_PERMUTATION_INT

#pragma push_macro("SHADER_PERMUTATION_RANGE_INT")
#undef SHADER_PERMUTATION_RANGE_INT

#pragma push_macro("SHADER_PERMUTATION_SPARSE_INT")
#undef SHADER_PERMUTATION_SPARSE_INT

#pragma push_macro("SHADER_PERMUTATION_ENUM_CLASS")
#undef SHADER_PERMUTATION_ENUM_CLASS

#pragma push_macro("STRUCTMEMBER_VERTEXSTREAMCOMPONENT")
#undef STRUCTMEMBER_VERTEXSTREAMCOMPONENT

#pragma push_macro("IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE")
#undef IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE

#pragma push_macro("DECLARE_VERTEX_FACTORY_TYPE")
#undef DECLARE_VERTEX_FACTORY_TYPE

#pragma push_macro("IMPLEMENT_VERTEX_FACTORY_VTABLE")
#undef IMPLEMENT_VERTEX_FACTORY_VTABLE

#pragma push_macro("IMPLEMENT_VERTEX_FACTORY_TYPE")
#undef IMPLEMENT_VERTEX_FACTORY_TYPE

#pragma push_macro("IMPLEMENT_TEMPLATE_VERTEX_FACTORY_TYPE_EX")
#undef IMPLEMENT_TEMPLATE_VERTEX_FACTORY_TYPE_EX

#pragma push_macro("IMPLEMENT_VERTEX_FACTORY_TYPE_EX")
#undef IMPLEMENT_VERTEX_FACTORY_TYPE_EX

#pragma push_macro("SCOPED_GPU_EVENT")
#undef SCOPED_GPU_EVENT

#pragma push_macro("SCOPED_GPU_EVENT_COLOR")
#undef SCOPED_GPU_EVENT_COLOR

#pragma push_macro("SCOPED_GPU_EVENTF")
#undef SCOPED_GPU_EVENTF

#pragma push_macro("SCOPED_GPU_EVENTF_COLOR")
#undef SCOPED_GPU_EVENTF_COLOR

#pragma push_macro("SCOPED_CONDITIONAL_GPU_EVENT")
#undef SCOPED_CONDITIONAL_GPU_EVENT

#pragma push_macro("SCOPED_CONDITIONAL_GPU_EVENT_COLOR")
#undef SCOPED_CONDITIONAL_GPU_EVENT_COLOR

#pragma push_macro("SCOPED_CONDITIONAL_GPU_EVENTF")
#undef SCOPED_CONDITIONAL_GPU_EVENTF

#pragma push_macro("SCOPED_CONDITIONAL_GPU_EVENTF_COLOR")
#undef SCOPED_CONDITIONAL_GPU_EVENTF_COLOR

#pragma push_macro("BEGIN_GPU_EVENTF")
#undef BEGIN_GPU_EVENTF

#pragma push_macro("BEGIN_GPU_EVENTF_COLOR")
#undef BEGIN_GPU_EVENTF_COLOR

#pragma push_macro("STOP_GPU_EVENT")
#undef STOP_GPU_EVENT

#pragma push_macro("SCOPED_DRAW_EVENT")
#undef SCOPED_DRAW_EVENT

#pragma push_macro("SCOPED_DRAW_EVENT_COLOR")
#undef SCOPED_DRAW_EVENT_COLOR

#pragma push_macro("SCOPED_DRAW_EVENTF")
#undef SCOPED_DRAW_EVENTF

#pragma push_macro("SCOPED_DRAW_EVENTF_COLOR")
#undef SCOPED_DRAW_EVENTF_COLOR

#pragma push_macro("SCOPED_CONDITIONAL_DRAW_EVENT")
#undef SCOPED_CONDITIONAL_DRAW_EVENT

#pragma push_macro("SCOPED_CONDITIONAL_DRAW_EVENT_COLOR")
#undef SCOPED_CONDITIONAL_DRAW_EVENT_COLOR

#pragma push_macro("SCOPED_CONDITIONAL_DRAW_EVENTF")
#undef SCOPED_CONDITIONAL_DRAW_EVENTF

#pragma push_macro("SCOPED_CONDITIONAL_DRAW_EVENTF_COLOR")
#undef SCOPED_CONDITIONAL_DRAW_EVENTF_COLOR

#pragma push_macro("BEGIN_DRAW_EVENTF")
#undef BEGIN_DRAW_EVENTF

#pragma push_macro("BEGIN_DRAW_EVENTF_COLOR")
#undef BEGIN_DRAW_EVENTF_COLOR

#pragma push_macro("STOP_DRAW_EVENT")
#undef STOP_DRAW_EVENT

#pragma push_macro("SCOPED_RHI_DRAW_EVENT")
#undef SCOPED_RHI_DRAW_EVENT

#pragma push_macro("SCOPED_RHI_DRAW_EVENT_COLOR")
#undef SCOPED_RHI_DRAW_EVENT_COLOR

#pragma push_macro("SCOPED_RHI_DRAW_EVENTF")
#undef SCOPED_RHI_DRAW_EVENTF

#pragma push_macro("SCOPED_RHI_DRAW_EVENTF_COLOR")
#undef SCOPED_RHI_DRAW_EVENTF_COLOR

#pragma push_macro("SCOPED_RHI_CONDITIONAL_DRAW_EVENT")
#undef SCOPED_RHI_CONDITIONAL_DRAW_EVENT

#pragma push_macro("SCOPED_RHI_CONDITIONAL_DRAW_EVENT_COLOR")
#undef SCOPED_RHI_CONDITIONAL_DRAW_EVENT_COLOR

#pragma push_macro("SCOPED_RHI_CONDITIONAL_DRAW_EVENTF")
#undef SCOPED_RHI_CONDITIONAL_DRAW_EVENTF

#pragma push_macro("SCOPED_RHI_CONDITIONAL_DRAW_EVENTF_COLOR")
#undef SCOPED_RHI_CONDITIONAL_DRAW_EVENTF_COLOR

#pragma push_macro("SCOPED_COMPUTE_EVENT")
#undef SCOPED_COMPUTE_EVENT

#pragma push_macro("SCOPED_COMPUTE_EVENT_COLOR")
#undef SCOPED_COMPUTE_EVENT_COLOR

#pragma push_macro("SCOPED_COMPUTE_EVENTF")
#undef SCOPED_COMPUTE_EVENTF

#pragma push_macro("SCOPED_COMPUTE_EVENTF_COLOR")
#undef SCOPED_COMPUTE_EVENTF_COLOR

#pragma push_macro("SCOPED_CONDITIONAL_COMPUTE_EVENT")
#undef SCOPED_CONDITIONAL_COMPUTE_EVENT

#pragma push_macro("SCOPED_CONDITIONAL_COMPUTE_EVENT_COLOR")
#undef SCOPED_CONDITIONAL_COMPUTE_EVENT_COLOR

#pragma push_macro("SCOPED_CONDITIONAL_COMPUTE_EVENTF")
#undef SCOPED_CONDITIONAL_COMPUTE_EVENTF

#pragma push_macro("SCOPED_CONDITIONAL_COMPUTE_EVENTF_COLOR")
#undef SCOPED_CONDITIONAL_COMPUTE_EVENTF_COLOR

#pragma push_macro("DECLARE_GPU_STAT")
#undef DECLARE_GPU_STAT

#pragma push_macro("DECLARE_GPU_DRAWCALL_STAT")
#undef DECLARE_GPU_DRAWCALL_STAT

#pragma push_macro("DECLARE_GPU_DRAWCALL_STAT_EXTERN")
#undef DECLARE_GPU_DRAWCALL_STAT_EXTERN

#pragma push_macro("DECLARE_GPU_STAT_NAMED")
#undef DECLARE_GPU_STAT_NAMED

#pragma push_macro("DECLARE_GPU_DRAWCALL_STAT_NAMED")
#undef DECLARE_GPU_DRAWCALL_STAT_NAMED

#pragma push_macro("DECLARE_GPU_STAT_NAMED_EXTERN")
#undef DECLARE_GPU_STAT_NAMED_EXTERN

#pragma push_macro("DEFINE_GPU_STAT")
#undef DEFINE_GPU_STAT

#pragma push_macro("DEFINE_GPU_DRAWCALL_STAT")
#undef DEFINE_GPU_DRAWCALL_STAT

#pragma push_macro("SCOPED_GPU_STAT")
#undef SCOPED_GPU_STAT

#pragma push_macro("GPU_STATS_BEGINFRAME")
#undef GPU_STATS_BEGINFRAME

#pragma push_macro("GPU_STATS_ENDFRAME")
#undef GPU_STATS_ENDFRAME

#pragma push_macro("IMPLEMENT_ATMOSPHERE_TEXTURE_PARAM_SET")
#undef IMPLEMENT_ATMOSPHERE_TEXTURE_PARAM_SET

#pragma push_macro("SHADER_VARIATION")
#undef SHADER_VARIATION

#pragma push_macro("IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE")
#undef IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE

#pragma push_macro("IMPLEMENT_BASEPASS_VERTEXSHADER_ONLY_TYPE")
#undef IMPLEMENT_BASEPASS_VERTEXSHADER_ONLY_TYPE

#pragma push_macro("IMPLEMENT_BASEPASS_PIXELSHADER_TYPE")
#undef IMPLEMENT_BASEPASS_PIXELSHADER_TYPE

#pragma push_macro("IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE")
#undef IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE

#pragma push_macro("IMPLEMENT_CAPSULE_SHADOW_TYPE")
#undef IMPLEMENT_CAPSULE_SHADOW_TYPE

#pragma push_macro("IMPLEMENT_CAPSULE_APPLY_SHADER_TYPE")
#undef IMPLEMENT_CAPSULE_APPLY_SHADER_TYPE

#pragma push_macro("IMPLEMENT_GetDepthPassShaders")
#undef IMPLEMENT_GetDepthPassShaders

#pragma push_macro("VARIATION1")
#undef VARIATION1

#pragma push_macro("IMPLEMENT_REMOVE_OBJECTS_FROM_BUFFER_SHADER_TYPE")
#undef IMPLEMENT_REMOVE_OBJECTS_FROM_BUFFER_SHADER_TYPE

#pragma push_macro("IMPLEMENT_CONETRACE_CS_TYPE")
#undef IMPLEMENT_CONETRACE_CS_TYPE

#pragma push_macro("IMPLEMENT_CONETRACE_GLOBAL_CS_TYPE")
#undef IMPLEMENT_CONETRACE_GLOBAL_CS_TYPE

#pragma push_macro("VARIATION")
#undef VARIATION

#pragma push_macro("IMPLEMENT_GLOBALDF_COMPOSITE_CS_TYPE")
#undef IMPLEMENT_GLOBALDF_COMPOSITE_CS_TYPE

#pragma push_macro("GROUPSHARED_COMPLEX_TRANSFORM")
#undef GROUPSHARED_COMPLEX_TRANSFORM

#pragma push_macro("GROUPSHARED_TWO_FOR_ONE_TRANSFORM")
#undef GROUPSHARED_TWO_FOR_ONE_TRANSFORM

#pragma push_macro("GROUPSHARED_CONVOLUTION_WTEXTURE")
#undef GROUPSHARED_CONVOLUTION_WTEXTURE

#pragma push_macro("GET_COMPLEX_SHADER")
#undef GET_COMPLEX_SHADER

#pragma push_macro("GET_TWOFORONE_SHADER")
#undef GET_TWOFORONE_SHADER

#pragma push_macro("GET_GROUP_SHARED_TEXTURE_FILTER")
#undef GET_GROUP_SHARED_TEXTURE_FILTER

#pragma push_macro("IMPLEMENT_DENSITY_VERTEXSHADER_TYPE")
#undef IMPLEMENT_DENSITY_VERTEXSHADER_TYPE

#pragma push_macro("IMPLEMENT_DENSITY_PIXELSHADER_TYPE")
#undef IMPLEMENT_DENSITY_PIXELSHADER_TYPE

#pragma push_macro("IMPLEMENT_DENSITY_LIGHTMAPPED_SHADER_TYPE")
#undef IMPLEMENT_DENSITY_LIGHTMAPPED_SHADER_TYPE

#pragma push_macro("IMPLEMENT_MOBILE_SHADING_BASEPASS_LIGHTMAPPED_VERTEX_SHADER_TYPE")
#undef IMPLEMENT_MOBILE_SHADING_BASEPASS_LIGHTMAPPED_VERTEX_SHADER_TYPE

#pragma push_macro("IMPLEMENT_MOBILE_SHADING_BASEPASS_LIGHTMAPPED_PIXEL_SHADER_TYPE")
#undef IMPLEMENT_MOBILE_SHADING_BASEPASS_LIGHTMAPPED_PIXEL_SHADER_TYPE

#pragma push_macro("IMPLEMENT_MOBILE_SHADING_BASEPASS_LIGHTMAPPED_SHADER_TYPE")
#undef IMPLEMENT_MOBILE_SHADING_BASEPASS_LIGHTMAPPED_SHADER_TYPE

#pragma push_macro("IMPLEMENT_MOBILE_SCENE_CAPTURECOPY")
#undef IMPLEMENT_MOBILE_SCENE_CAPTURECOPY

#pragma push_macro("SET_SPEEDTREE_TABLE_FLOAT4V")
#undef SET_SPEEDTREE_TABLE_FLOAT4V

#pragma push_macro("FASTVRAM_CVAR")
#undef FASTVRAM_CVAR

#pragma push_macro("CANVAS_HEADER")
#undef CANVAS_HEADER

#pragma push_macro("CANVAS_LINE")
#undef CANVAS_LINE

#pragma push_macro("IMPLEMENT_MATERIAL_SHADER_SetParameters")
#undef IMPLEMENT_MATERIAL_SHADER_SetParameters

#pragma push_macro("IMPLEMENT_SHADOW_DEPTH_SHADERMODE_SHADERS")
#undef IMPLEMENT_SHADOW_DEPTH_SHADERMODE_SHADERS

#pragma push_macro("IMPLEMENT_SHADOWDEPTHPASS_PIXELSHADER_TYPE")
#undef IMPLEMENT_SHADOWDEPTHPASS_PIXELSHADER_TYPE

#pragma push_macro("IMPLEMENT_SHADOW_PROJECTION_PIXEL_SHADER")
#undef IMPLEMENT_SHADOW_PROJECTION_PIXEL_SHADER

#pragma push_macro("IMPLEMENT_MODULATED_SHADOW_PROJECTION_PIXEL_SHADER")
#undef IMPLEMENT_MODULATED_SHADOW_PROJECTION_PIXEL_SHADER

#pragma push_macro("IMPLEMENT_ONEPASS_POINT_SHADOW_PROJECTION_PIXEL_SHADER")
#undef IMPLEMENT_ONEPASS_POINT_SHADOW_PROJECTION_PIXEL_SHADER

#pragma push_macro("COPYMACRO")
#undef COPYMACRO

#pragma push_macro("IMPLEMENT_INJECTION_PIXELSHADER_TYPE")
#undef IMPLEMENT_INJECTION_PIXELSHADER_TYPE

#pragma push_macro("IMPLEMENT_RESOLVE_SHADER")
#undef IMPLEMENT_RESOLVE_SHADER

#pragma push_macro("ENTRY")
#undef ENTRY

#pragma push_macro("IMPLEMENT_POST_PROCESS_PARAM_SET")
#undef IMPLEMENT_POST_PROCESS_PARAM_SET

#pragma push_macro("UPDATE_HISTORY_FLAGS")
#undef UPDATE_HISTORY_FLAGS

#pragma push_macro("ANY_CAPTURE_RENDERED_RECENTLY")
#undef ANY_CAPTURE_RENDERED_RECENTLY

#pragma push_macro("ANY_HIGHRES_CAPTURE_RENDERED_RECENTLY")
#undef ANY_HIGHRES_CAPTURE_RENDERED_RECENTLY

#pragma push_macro("GET_STENCIL_BIT_MASK")
#undef GET_STENCIL_BIT_MASK

#pragma push_macro("STENCIL_LIGHTING_CHANNELS_MASK")
#undef STENCIL_LIGHTING_CHANNELS_MASK

#pragma push_macro("GET_STENCIL_MOBILE_SM_MASK")
#undef GET_STENCIL_MOBILE_SM_MASK

#pragma push_macro("SCOPED_DRAW_OR_COMPUTE_EVENT")
#undef SCOPED_DRAW_OR_COMPUTE_EVENT

#pragma push_macro("IMPLEMENT_MATERIALCHS_TYPE")
#undef IMPLEMENT_MATERIALCHS_TYPE

#pragma push_macro("IMPLEMENT_VIRTUALTEXTURE_SHADER_TYPE")
#undef IMPLEMENT_VIRTUALTEXTURE_SHADER_TYPE

#pragma push_macro("COMPARE_FIELD_BEGIN")
#undef COMPARE_FIELD_BEGIN

#pragma push_macro("COMPARE_FIELD")
#undef COMPARE_FIELD

#pragma push_macro("A")
#undef A

#pragma push_macro("TRACE_GPUPROFILER_DEFINE_EVENT_TYPE")
#undef TRACE_GPUPROFILER_DEFINE_EVENT_TYPE

#pragma push_macro("TRACE_GPUPROFILER_DECLARE_EVENT_TYPE_EXTERN")
#undef TRACE_GPUPROFILER_DECLARE_EVENT_TYPE_EXTERN

#pragma push_macro("TRACE_GPUPROFILER_EVENT_TYPE")
#undef TRACE_GPUPROFILER_EVENT_TYPE

#pragma push_macro("TRACE_GPUPROFILER_BEGIN_FRAME")
#undef TRACE_GPUPROFILER_BEGIN_FRAME

#pragma push_macro("TRACE_GPUPROFILER_BEGIN_EVENT")
#undef TRACE_GPUPROFILER_BEGIN_EVENT

#pragma push_macro("TRACE_GPUPROFILER_END_EVENT")
#undef TRACE_GPUPROFILER_END_EVENT

#pragma push_macro("TRACE_GPUPROFILER_END_FRAME")
#undef TRACE_GPUPROFILER_END_FRAME

#pragma push_macro("RHI_DRAW_CALL_INC")
#undef RHI_DRAW_CALL_INC

#pragma push_macro("RHI_DRAW_CALL_STATS")
#undef RHI_DRAW_CALL_STATS

#pragma push_macro("RHISTAT")
#undef RHISTAT

#pragma push_macro("FRHICOMMAND_MACRO")
#undef FRHICOMMAND_MACRO

#pragma push_macro("ALLOC_COMMAND")
#undef ALLOC_COMMAND

#pragma push_macro("ALLOC_COMMAND_CL")
#undef ALLOC_COMMAND_CL

#pragma push_macro("SCOPED_GPU_MASK")
#undef SCOPED_GPU_MASK

#pragma push_macro("SCOPED_UNIFORM_BUFFER_GLOBAL_BINDINGS")
#undef SCOPED_UNIFORM_BUFFER_GLOBAL_BINDINGS

#pragma push_macro("INTERNAL_DECORATOR_COMPUTE")
#undef INTERNAL_DECORATOR_COMPUTE

#pragma push_macro("IMPLEMENT_DDPSPI_SETTING_WITH_RETURN_TYPE")
#undef IMPLEMENT_DDPSPI_SETTING_WITH_RETURN_TYPE

#pragma push_macro("IMPLEMENT_DDPSPI_SETTING")
#undef IMPLEMENT_DDPSPI_SETTING

#pragma push_macro("GEOMETRY_SHADER")
#undef GEOMETRY_SHADER

#pragma push_macro("TESSELLATION_SHADER")
#undef TESSELLATION_SHADER

#pragma push_macro("RTACTION_MAKE_MASK")
#undef RTACTION_MAKE_MASK

#pragma push_macro("DUMP_TRANSITION")
#undef DUMP_TRANSITION

#pragma push_macro("UE_LOG_RIGVMMEMORY")
#undef UE_LOG_RIGVMMEMORY

#pragma push_macro("BREAK_WHEN_AUDIBLE")
#undef BREAK_WHEN_AUDIBLE

#pragma push_macro("BREAK_WHEN_TOO_LOUD")
#undef BREAK_WHEN_TOO_LOUD

#pragma push_macro("CHECK_SAMPLE")
#undef CHECK_SAMPLE

#pragma push_macro("CHECK_SAMPLE2")
#undef CHECK_SAMPLE2

#pragma push_macro("UI_COMMAND_EXT")
#undef UI_COMMAND_EXT

#pragma push_macro("UI_COMMAND")
#undef UI_COMMAND

#pragma push_macro("APPEND_FONT")
#undef APPEND_FONT

#pragma push_macro("APPEND_EDITOR_FONT")
#undef APPEND_EDITOR_FONT

#pragma push_macro("APPEND_RANGE")
#undef APPEND_RANGE

#pragma push_macro("RETURN_TRUE_IF_CHAR_WITHIN_RANGE")
#undef RETURN_TRUE_IF_CHAR_WITHIN_RANGE

#pragma push_macro("REGISTER_UNICODE_BLOCK_RANGE")
#undef REGISTER_UNICODE_BLOCK_RANGE

#pragma push_macro("AddToNextFocusableWidgetCondidateDebugResults")
#undef AddToNextFocusableWidgetCondidateDebugResults

#pragma push_macro("SLATE_CROSS_THREAD_CHECK")
#undef SLATE_CROSS_THREAD_CHECK

#pragma push_macro("DRAG_DROP_OPERATOR_TYPE")
#undef DRAG_DROP_OPERATOR_TYPE

#pragma push_macro("UE_TRACE_SLATE_APPLICATION_TICK_AND_DRAW_WIDGETS")
#undef UE_TRACE_SLATE_APPLICATION_TICK_AND_DRAW_WIDGETS

#pragma push_macro("UE_TRACE_SLATE_WIDGET_ADDED")
#undef UE_TRACE_SLATE_WIDGET_ADDED

#pragma push_macro("UE_TRACE_SLATE_WIDGET_DEBUG_INFO")
#undef UE_TRACE_SLATE_WIDGET_DEBUG_INFO

#pragma push_macro("UE_TRACE_SLATE_WIDGET_REMOVED")
#undef UE_TRACE_SLATE_WIDGET_REMOVED

#pragma push_macro("UE_TRACE_SCOPED_SLATE_WIDGET_PAINT")
#undef UE_TRACE_SCOPED_SLATE_WIDGET_PAINT

#pragma push_macro("UE_TRACE_SLATE_WIDGET_UPDATED")
#undef UE_TRACE_SLATE_WIDGET_UPDATED

#pragma push_macro("UE_TRACE_SLATE_WIDGET_INVALIDATED")
#undef UE_TRACE_SLATE_WIDGET_INVALIDATED

#pragma push_macro("UE_TRACE_SLATE_ROOT_INVALIDATED")
#undef UE_TRACE_SLATE_ROOT_INVALIDATED

#pragma push_macro("UE_TRACE_SLATE_ROOT_CHILDORDER_INVALIDATED")
#undef UE_TRACE_SLATE_ROOT_CHILDORDER_INVALIDATED

#pragma push_macro("SLATE_METADATA_TYPE")
#undef SLATE_METADATA_TYPE

#pragma push_macro("SNew")
#undef SNew

#pragma push_macro("SAssignNew")
#undef SAssignNew

#pragma push_macro("SLATE_BEGIN_ARGS")
#undef SLATE_BEGIN_ARGS

#pragma push_macro("SLATE_USER_ARGS")
#undef SLATE_USER_ARGS

#pragma push_macro("HACK_SLATE_SLOT_ARGS")
#undef HACK_SLATE_SLOT_ARGS

#pragma push_macro("SLATE_END_ARGS")
#undef SLATE_END_ARGS

#pragma push_macro("SLATE_ATTRIBUTE")
#undef SLATE_ATTRIBUTE

#pragma push_macro("SLATE_ARGUMENT")
#undef SLATE_ARGUMENT

#pragma push_macro("SLATE_ARGUMENT_DEFAULT")
#undef SLATE_ARGUMENT_DEFAULT

#pragma push_macro("SLATE_STYLE_ARGUMENT")
#undef SLATE_STYLE_ARGUMENT

#pragma push_macro("SLATE_SUPPORTS_SLOT")
#undef SLATE_SUPPORTS_SLOT

#pragma push_macro("SLATE_SUPPORTS_SLOT_WITH_ARGS")
#undef SLATE_SUPPORTS_SLOT_WITH_ARGS

#pragma push_macro("SLATE_NAMED_SLOT")
#undef SLATE_NAMED_SLOT

#pragma push_macro("SLATE_DEFAULT_SLOT")
#undef SLATE_DEFAULT_SLOT

#pragma push_macro("SLATE_EVENT")
#undef SLATE_EVENT

#pragma push_macro("SCOPE_CYCLE_SWIDGET")
#undef SCOPE_CYCLE_SWIDGET

#pragma push_macro("IMPLEMENT_SLATE_VERTEXMATERIALSHADER_TYPE")
#undef IMPLEMENT_SLATE_VERTEXMATERIALSHADER_TYPE

#pragma push_macro("IMPLEMENT_SLATE_MATERIALSHADER_TYPE")
#undef IMPLEMENT_SLATE_MATERIALSHADER_TYPE

#pragma push_macro("SLATE_DRAW_EVENT")
#undef SLATE_DRAW_EVENT

#pragma push_macro("IMPLEMENT_SLATE_PIXELSHADER_TYPE")
#undef IMPLEMENT_SLATE_PIXELSHADER_TYPE

#pragma push_macro("UE_TRACE_EVENT_DEFINE")
#undef UE_TRACE_EVENT_DEFINE

#pragma push_macro("UE_TRACE_EVENT_BEGIN")
#undef UE_TRACE_EVENT_BEGIN

#pragma push_macro("UE_TRACE_EVENT_BEGIN_EXTERN")
#undef UE_TRACE_EVENT_BEGIN_EXTERN

#pragma push_macro("UE_TRACE_EVENT_FIELD")
#undef UE_TRACE_EVENT_FIELD

#pragma push_macro("UE_TRACE_EVENT_END")
#undef UE_TRACE_EVENT_END

#pragma push_macro("UE_TRACE_LOG")
#undef UE_TRACE_LOG

#pragma push_macro("UE_TRACE_LOG_SCOPED")
#undef UE_TRACE_LOG_SCOPED

#pragma push_macro("UE_TRACE_LOG_SCOPED_T")
#undef UE_TRACE_LOG_SCOPED_T

#pragma push_macro("UE_TRACE_CHANNEL")
#undef UE_TRACE_CHANNEL

#pragma push_macro("UE_TRACE_CHANNEL_EXTERN")
#undef UE_TRACE_CHANNEL_EXTERN

#pragma push_macro("UE_TRACE_CHANNEL_MODULE_EXTERN")
#undef UE_TRACE_CHANNEL_MODULE_EXTERN

#pragma push_macro("UE_TRACE_CHANNEL_DEFINE")
#undef UE_TRACE_CHANNEL_DEFINE

#pragma push_macro("UE_TRACE_CHANNELEXPR_IS_ENABLED")
#undef UE_TRACE_CHANNELEXPR_IS_ENABLED

#pragma push_macro("TRACE_PRIVATE_FIELD")
#undef TRACE_PRIVATE_FIELD

#pragma push_macro("TRACE_PRIVATE_CHANNEL_DECLARE")
#undef TRACE_PRIVATE_CHANNEL_DECLARE

#pragma push_macro("TRACE_PRIVATE_CHANNEL_IMPL")
#undef TRACE_PRIVATE_CHANNEL_IMPL

#pragma push_macro("TRACE_PRIVATE_CHANNEL")
#undef TRACE_PRIVATE_CHANNEL

#pragma push_macro("TRACE_PRIVATE_CHANNEL_MODULE_EXTERN")
#undef TRACE_PRIVATE_CHANNEL_MODULE_EXTERN

#pragma push_macro("TRACE_PRIVATE_CHANNEL_DEFINE")
#undef TRACE_PRIVATE_CHANNEL_DEFINE

#pragma push_macro("TRACE_PRIVATE_CHANNEL_EXTERN")
#undef TRACE_PRIVATE_CHANNEL_EXTERN

#pragma push_macro("TRACE_PRIVATE_CHANNELEXPR_IS_ENABLED")
#undef TRACE_PRIVATE_CHANNELEXPR_IS_ENABLED

#pragma push_macro("TRACE_PRIVATE_EVENT_DEFINE")
#undef TRACE_PRIVATE_EVENT_DEFINE

#pragma push_macro("TRACE_PRIVATE_EVENT_BEGIN")
#undef TRACE_PRIVATE_EVENT_BEGIN

#pragma push_macro("TRACE_PRIVATE_EVENT_BEGIN_EXTERN")
#undef TRACE_PRIVATE_EVENT_BEGIN_EXTERN

#pragma push_macro("TRACE_PRIVATE_EVENT_BEGIN_IMPL")
#undef TRACE_PRIVATE_EVENT_BEGIN_IMPL

#pragma push_macro("TRACE_PRIVATE_EVENT_FIELD")
#undef TRACE_PRIVATE_EVENT_FIELD

#pragma push_macro("TRACE_PRIVATE_EVENT_END")
#undef TRACE_PRIVATE_EVENT_END

#pragma push_macro("TRACE_PRIVATE_LOG_PRELUDE")
#undef TRACE_PRIVATE_LOG_PRELUDE

#pragma push_macro("TRACE_PRIVATE_LOG_EPILOG")
#undef TRACE_PRIVATE_LOG_EPILOG

#pragma push_macro("TRACE_PRIVATE_LOG")
#undef TRACE_PRIVATE_LOG

#pragma push_macro("TRACE_PRIVATE_LOG_SCOPED")
#undef TRACE_PRIVATE_LOG_SCOPED

#pragma push_macro("TRACE_PRIVATE_LOG_SCOPED_T")
#undef TRACE_PRIVATE_LOG_SCOPED_T

#pragma push_macro("IMPLEMENT_TYPED_UMG_LIST")
#undef IMPLEMENT_TYPED_UMG_LIST

#pragma push_macro("BIND_UOBJECT_ATTRIBUTE")
#undef BIND_UOBJECT_ATTRIBUTE

#pragma push_macro("BIND_UOBJECT_DELEGATE")
#undef BIND_UOBJECT_DELEGATE

#pragma push_macro("OPTIONAL_BINDING")
#undef OPTIONAL_BINDING

#pragma push_macro("PROPERTY_BINDING")
#undef PROPERTY_BINDING

#pragma push_macro("BITFIELD_PROPERTY_BINDING")
#undef BITFIELD_PROPERTY_BINDING

#pragma push_macro("PROPERTY_BINDING_IMPLEMENTATION")
#undef PROPERTY_BINDING_IMPLEMENTATION

#pragma push_macro("GAME_SAFE_OPTIONAL_BINDING")
#undef GAME_SAFE_OPTIONAL_BINDING

#pragma push_macro("GAME_SAFE_BINDING_IMPLEMENTATION")
#undef GAME_SAFE_BINDING_IMPLEMENTATION

#pragma push_macro("OPTIONAL_BINDING_CONVERT")
#undef OPTIONAL_BINDING_CONVERT

#pragma push_macro("UA_SYSTEM_ERROR")
#undef UA_SYSTEM_ERROR

#pragma push_macro("SOUND_CONVERT_CHECK")
#undef SOUND_CONVERT_CHECK

#pragma push_macro("DEBUG_AUDIO_CHECK_AUDIO_THREAD")
#undef DEBUG_AUDIO_CHECK_AUDIO_THREAD

#pragma push_macro("DEBUG_AUDIO_CHECK_MAIN_THREAD")
#undef DEBUG_AUDIO_CHECK_MAIN_THREAD

#pragma push_macro("DEBUG_AUDIO_CHECK")
#undef DEBUG_AUDIO_CHECK

#pragma push_macro("DEBUG_AUDIO_CHECK_MSG")
#undef DEBUG_AUDIO_CHECK_MSG

#pragma push_macro("AUDIO_VOICE_CHECK_ERROR")
#undef AUDIO_VOICE_CHECK_ERROR

#pragma push_macro("AUDIO_VOICE_CHECK_SUSPEND")
#undef AUDIO_VOICE_CHECK_SUSPEND

#pragma push_macro("VOICE_CHECK_INITIALIZATION")
#undef VOICE_CHECK_INITIALIZATION

#pragma push_macro("UA_DEVICE_PLATFORM_ERROR")
#undef UA_DEVICE_PLATFORM_ERROR

#pragma push_macro("AU_DEVICE_PARAM_ERROR")
#undef AU_DEVICE_PARAM_ERROR

#pragma push_macro("AU_DEVICE_WARNING")
#undef AU_DEVICE_WARNING

#pragma push_macro("VectorIntShuffle")
#undef VectorIntShuffle

#pragma push_macro("VK_DYNAMICAPI_TO_VULKANRHI")
#undef VK_DYNAMICAPI_TO_VULKANRHI

#pragma push_macro("VULKAN_REPORT_LOG")
#undef VULKAN_REPORT_LOG

#pragma push_macro("VKSWITCHCASE")
#undef VKSWITCHCASE

#pragma push_macro("AppendBitFieldName")
#undef AppendBitFieldName

#pragma push_macro("VULKAN_SET_DEBUG_NAME")
#undef VULKAN_SET_DEBUG_NAME

#pragma push_macro("VERIFYVULKANRESULT_INIT")
#undef VERIFYVULKANRESULT_INIT

#pragma push_macro("LLM_SCOPE_VULKAN")
#undef LLM_SCOPE_VULKAN

#pragma push_macro("LLM_PLATFORM_SCOPE_VULKAN")
#undef LLM_PLATFORM_SCOPE_VULKAN

#pragma push_macro("LLM_TRACK_VULKAN_HIGH_LEVEL_ALLOC")
#undef LLM_TRACK_VULKAN_HIGH_LEVEL_ALLOC

#pragma push_macro("LLM_TRACK_VULKAN_HIGH_LEVEL_FREE")
#undef LLM_TRACK_VULKAN_HIGH_LEVEL_FREE

#pragma push_macro("LLM_TRACK_VULKAN_SPARE_MEMORY_GPU")
#undef LLM_TRACK_VULKAN_SPARE_MEMORY_GPU

#pragma push_macro("ENUM_VK_ENTRYPOINTS_INSTANCE")
#undef ENUM_VK_ENTRYPOINTS_INSTANCE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_SURFACE_INSTANCE")
#undef ENUM_VK_ENTRYPOINTS_SURFACE_INSTANCE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_BASE")
#undef ENUM_VK_ENTRYPOINTS_BASE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_OPTIONAL_BASE")
#undef ENUM_VK_ENTRYPOINTS_OPTIONAL_BASE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_OPTIONAL_INSTANCE")
#undef ENUM_VK_ENTRYPOINTS_OPTIONAL_INSTANCE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_ALL")
#undef ENUM_VK_ENTRYPOINTS_ALL

#pragma push_macro("DECLARE_VK_ENTRYPOINTS")
#undef DECLARE_VK_ENTRYPOINTS

#pragma push_macro("VULKAN_LOGMEMORY")
#undef VULKAN_LOGMEMORY

#pragma push_macro("VULKAN_FILL_TRACK_INFO")
#undef VULKAN_FILL_TRACK_INFO

#pragma push_macro("VULKAN_FREE_TRACK_INFO")
#undef VULKAN_FREE_TRACK_INFO

#pragma push_macro("VULKAN_TRACK_STRING")
#undef VULKAN_TRACK_STRING

#pragma push_macro("VKSWITCH")
#undef VKSWITCH

#pragma push_macro("LRUPRINT")
#undef LRUPRINT

#pragma push_macro("LRUPRINT_DEBUG")
#undef LRUPRINT_DEBUG

#pragma push_macro("VKERRORCASE")
#undef VKERRORCASE

#pragma push_macro("DEFINE_VK_ENTRYPOINTS")
#undef DEFINE_VK_ENTRYPOINTS

#pragma push_macro("CHECK_VK_ENTRYPOINTS")
#undef CHECK_VK_ENTRYPOINTS

#pragma push_macro("GET_VK_ENTRYPOINTS")
#undef GET_VK_ENTRYPOINTS

#pragma push_macro("GETINSTANCE_VK_ENTRYPOINTS")
#undef GETINSTANCE_VK_ENTRYPOINTS

#pragma push_macro("CLEAR_VK_ENTRYPOINTS")
#undef CLEAR_VK_ENTRYPOINTS

#pragma push_macro("ENUM_VK_ENTRYPOINTS_PLATFORM_BASE")
#undef ENUM_VK_ENTRYPOINTS_PLATFORM_BASE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE")
#undef ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE

#pragma push_macro("ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE")
#undef ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE

#pragma push_macro("VULKAN_SIGNAL_UNIMPLEMENTED")
#undef VULKAN_SIGNAL_UNIMPLEMENTED

#pragma push_macro("VULKAN_TRACK_OBJECT_CREATE")
#undef VULKAN_TRACK_OBJECT_CREATE

#pragma push_macro("VULKAN_TRACK_OBJECT_DELETE")
#undef VULKAN_TRACK_OBJECT_DELETE

#pragma push_macro("VERIFYVULKANRESULT")
#undef VERIFYVULKANRESULT

#pragma push_macro("VERIFYVULKANRESULT_EXPANDED")
#undef VERIFYVULKANRESULT_EXPANDED

#pragma push_macro("XAUDIO2_GOTO_CLEANUP_ON_FAIL")
#undef XAUDIO2_GOTO_CLEANUP_ON_FAIL

#pragma push_macro("XAUDIO2_RETURN_ON_FAIL")
#undef XAUDIO2_RETURN_ON_FAIL

#pragma push_macro("NVAFTERMATH_ON_ERROR")
#undef NVAFTERMATH_ON_ERROR

#pragma push_macro("D3D11_STATE_CACHE_VERIFY")
#undef D3D11_STATE_CACHE_VERIFY

#pragma push_macro("D3D11_STATE_CACHE_VERIFY_PRE")
#undef D3D11_STATE_CACHE_VERIFY_PRE

#pragma push_macro("D3D11_STATE_CACHE_VERIFY_POST")
#undef D3D11_STATE_CACHE_VERIFY_POST

#pragma push_macro("CACHE_NV_AFTERMATH_ENABLED")
#undef CACHE_NV_AFTERMATH_ENABLED

#pragma push_macro("START_NV_AFTERMATH")
#undef START_NV_AFTERMATH

#pragma push_macro("STOP_NV_AFTERMATH")
#undef STOP_NV_AFTERMATH

#pragma push_macro("VERIFYD3D11RESULT_EX")
#undef VERIFYD3D11RESULT_EX

#pragma push_macro("VERIFYD3D11RESULT")
#undef VERIFYD3D11RESULT

#pragma push_macro("VERIFYD3D11RESULT_NOEXIT")
#undef VERIFYD3D11RESULT_NOEXIT

#pragma push_macro("VERIFYD3D11SHADERRESULT")
#undef VERIFYD3D11SHADERRESULT

#pragma push_macro("VERIFYD3D11CREATETEXTURERESULT")
#undef VERIFYD3D11CREATETEXTURERESULT

#pragma push_macro("VERIFYD3D11RESIZEVIEWPORTRESULT")
#undef VERIFYD3D11RESIZEVIEWPORTRESULT

#pragma push_macro("CLEANUP_ON_FAIL")
#undef CLEANUP_ON_FAIL

#pragma push_macro("RETURN_FALSE_ON_FAIL")
#undef RETURN_FALSE_ON_FAIL

#pragma push_macro("PI")
#undef PI
