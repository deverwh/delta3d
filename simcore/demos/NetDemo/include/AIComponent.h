/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
* Bradley Anderegg
*/
#ifndef NETDEMO_AICOMPONENT
#define NETDEMO_AICOMPONENT

#include <DemoExport.h>
#include <dtAI/baseaicomponent.h>

namespace NetDemo
{

   class NETDEMO_EXPORT AIComponent : public dtAI::BaseAIComponent
   {
   public:

      typedef dtAI::BaseAIComponent BaseClass;
      static const std::string DEFAULT_NAME;

      /// Constructor
      AIComponent(const std::string& name = DEFAULT_NAME);

      /**
      * Handles incoming messages
      */
      virtual void ProcessMessage(const dtGame::Message& message);

      /**
      * Called when this component is added to the game manager
      */
      virtual void OnAddedToGM();

      virtual void OnRemovedFromGM();


   protected:

      /// Destructor
      virtual ~AIComponent();

      virtual void CleanUp();

   private:

   };
}//namespace NetDemo

#endif //NETDEMO_AICOMPONENT
