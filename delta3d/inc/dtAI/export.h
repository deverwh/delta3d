/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2004-2005 MOVES Institute
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef DELTA_AI_EXPORT
#define DELTA_AI_EXPORT

// export.h: Export definitions.  Modeled after osg/Export.
//
//////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#   ifdef DT_AI_LIBRARY
#      define DT_AI_EXPORT __declspec(dllexport)
#   else
#      define DT_AI_EXPORT __declspec(dllimport)
#   endif
#else
#   ifdef DT_AI_LIBRARY
#      define DT_AI_EXPORT __attribute__ ((visibility("default")))
#   else
#      define DT_AI_EXPORT
#   endif
#endif

#ifdef _MSC_VER
#   pragma warning (disable: 4251)
#   pragma warning(disable : 4355) // 'this' used in initializer list
#endif

#endif // DELTA_AI_EXPORT
