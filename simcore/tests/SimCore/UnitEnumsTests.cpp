/* -*-c++-*-
 * Simulation Core - ToolTests (.h & .cpp) - Using 'The MIT License'
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
 * @author David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include <SimCore/UnitEnums.h>

#include <UnitTestMain.h>

using dtCore::RefPtr;

namespace SimCore
{
   class UnitEnumsTests : public CPPUNIT_NS::TestFixture
   {
      CPPUNIT_TEST_SUITE(UnitEnumsTests);

         CPPUNIT_TEST(TestLength);
         CPPUNIT_TEST(TestAngle);

      CPPUNIT_TEST_SUITE_END();

   public:

      UnitEnumsTests()
      {
      }

      ~UnitEnumsTests()
      {
      }

      void setUp()
      {

      }

      void tearDown()
      {

      }

      void TestLength()
      {
         double value = UnitOfLength::Convert(UnitOfLength::NAUTICAL_MILE, UnitOfLength::FOOT, 1.0);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(6076.11549, value, 0.01);

         value = UnitOfLength::Convert(UnitOfLength::METER, UnitOfLength::YARD, 1.0);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0936133, value, 0.0001);
      }

      void TestAngle()
      {
         double value = UnitOfAngle::Convert(UnitOfAngle::MIL, UnitOfAngle::RADIAN, 10.0);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(0.009817477, value, 0.0001);

         value = UnitOfAngle::Convert(UnitOfAngle::DEGREE, UnitOfAngle::RADIAN, 2.0);
         CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * osg::PI / 180.0, value, 0.0001);
      }

   private:

   };

   CPPUNIT_TEST_SUITE_REGISTRATION(UnitEnumsTests);
}
