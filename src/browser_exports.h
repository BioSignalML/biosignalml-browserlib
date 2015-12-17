/*****************************************************************************
 *                                                                           *
 *  BioSignalML Browser in C++                                               *
 *                                                                           *
 *  Copyright (c) 2014-2015  David Brooks                                    *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#ifndef BROWSER_EXPORTS_H
#define BROWSER_EXPORTS_H

#include <QtGlobal>

#ifdef BROWSER_STATIC_DEFINE
#  define BROWSER_EXPORT
#  define BROWSER_NO_EXPORT
#else
#  ifndef BROWSER_EXPORT
#    ifdef browserlib_EXPORTS   /* We are building the library */
#      define BROWSER_EXPORT Q_DECL_EXPORT
#    else                       /* We are using the library */
#      define BROWSER_EXPORT Q_DECL_IMPORT
#    endif
#  endif

#  ifndef BROWSER_NO_EXPORT
#    ifdef WIN32
#      define BROWSER_NO_EXPORT 
#    else
#      define BROWSER_NO_EXPORT __attribute__((visibility("hidden")))
#    endif
#  endif
#endif

#ifndef BROWSER_DEPRECATED
#  ifdef WIN32
#    define BROWSER_DEPRECATED __declspec(deprecated)
#  else
#    define BROWSER_DEPRECATED __attribute__ ((__deprecated__))
#  endif
#endif

#ifndef BROWSER_DEPRECATED_EXPORT
#  define BROWSER_DEPRECATED_EXPORT BROWSER_EXPORT BROWSER_DEPRECATED
#endif

#ifndef BROWSER_DEPRECATED_NO_EXPORT
#  define BROWSER_DEPRECATED_NO_EXPORT BROWSER_NO_EXPORT BROWSER_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define BROWSER_NO_DEPRECATED
#endif

#endif
