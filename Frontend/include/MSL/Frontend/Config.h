/*
 * Copyright 2016 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

/**
 * @file
 * @brief Configuration macros for the project.
 */

#if defined(_WIN32)
#	define MSL_WINDOWS 1
#elif defined(linux)
#	define MSL_LINUX 1
#elif defined(__APPLE__)
#	define MSL_APPLE
#endif

#if defined(_MSC_VER)
#	define MSL_MSC 1
#elif defined(__clang__)
#	define MSL_CLANG 1
#elif defined(__GNUC__)
#	define MSL_GCC 1
#else
#error Unknown compiler.
#endif

/**
 * @brief Define for whether the platform is Windows.
 */
#ifndef MSL_WINDOWS
#	define MSL_WINDOWS 0
#endif

/**
 * @brief Define for whether the platform is Linux.
 */
#ifndef MSL_LINUX
#	define MSL_LINUX 0
#endif

/**
 * @brief Define for whether the platform is Apple.
 */
#ifndef MSL_APPLE
#	define MSL_APPLE 0
#endif

/**
 * @brief Define for whether the compler is Microsoft's C compiler.
 */
#ifndef MSL_MSC
#	define MSL_MSC 0
#endif

/**
 * @brief Define for whether the compiler is LLVM clang.
 */
#ifndef MSL_CLANG
#	define MSL_CLANG 0
#endif

/**
 * @def MSL_GCC
 * @brief Define for whether the compiler is GCC.
 */
#ifndef MSL_GCC
#	define MSL_GCC 0
#endif

/**
 * @brief Macro defined to whether or not the system is 64-bit.
 */
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__arm64__)
#define MSL_64BIT 1
#else
#define MSL_64BIT 0
#endif

/**
 * @brief Define for whether or not this is a debug build.
 */
#ifdef NDEBUG
#define MSL_DEBUG 0
#else
#define MSL_DEBUG 1
#endif

/**
 * @brief Macro for an unused variable.
 *
 * This can be used to work around compiler warnings.
 * @param x The unused variable.
 */
#define MSL_UNUSED(x) (void)(&x)
