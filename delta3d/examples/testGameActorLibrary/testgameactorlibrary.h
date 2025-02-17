/* -*-c++-*-
 * testGameActorLibrary - testgameactorlibrary (.h & .cpp) - Using 'The MIT License'
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
 * William E. Johnson II
 */

#ifndef TEST_GAME_ACTOR_LIBRARY_H_
#define TEST_GAME_ACTOR_LIBRARY_H_

#include <dtCore/actorpluginregistry.h>
#include "export.h"

class DT_EXAMPLE_EXPORT TestGameActorLibrary : public dtCore::ActorPluginRegistry
{
   public:
      static dtCore::RefPtr<dtCore::ActorType> TEST1_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST2_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_GAME_ACTOR_CRASH_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_PLAYER_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_TASK_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_COUNTER_TASK_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_TANK_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_JET_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_CULTURAL_FEATURE_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_SENSOR_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_HELICOPTER_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_ENVIRONMENT_GAME_ACTOR_TYPE;
      static dtCore::RefPtr<dtCore::ActorType> TEST_GAME_PROPERTY_TYPE;

      /// Constructor
      TestGameActorLibrary();

      void RegisterActorTypes();
};

#endif // TEST_GAME_ACTOR_LIBRARY_H_
