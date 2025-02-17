/* -*-c++-*-
 * testAAR - export (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2005-2008, Alion Science and Technology Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Christopher DuBuc
 */

#ifndef DELTA_TEST_AAR_EXPORT_H
#define DELTA_TEST_AAR_EXPORT_H

/**
 * This is modeled from the DT_EXPORT macro found in dtCore/export.h.  
 * We define another due to conflicts with using the DT_EXPORT while  
 * trying to import Delta3D symbols.  The SIMPLE_DEMO_EXPORT macro should be used
 * in front of any classes that are to be exported.
 * Also note that SIMPLE_DEMO_LIBRARY should be defined in the compiler 
 * preprocessor #defines.
 */
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  ifdef TEST_AAR_LIBRARY
#    define TEST_AAR_EXPORT __declspec(dllexport)
#  else
#    define TEST_AAR_EXPORT __declspec(dllimport)
#  endif
#else
#  define TEST_AAR_EXPORT
#endif

#endif
