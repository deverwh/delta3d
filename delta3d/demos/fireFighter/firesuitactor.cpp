/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
 * William E. Johnson II
 */

#include <fireFighter/firesuitactor.h>
#include <dtCore/gameeventmanager.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagefactory.h>
#include <dtGame/messagetype.h>

using dtCore::RefPtr;

////////////////////////////////////////////////////
FireSuitActorProxy::FireSuitActorProxy()
{

}

FireSuitActorProxy::~FireSuitActorProxy()
{

}

void FireSuitActorProxy::BuildPropertyMap()
{
   GameItemActorProxy::BuildPropertyMap();
}

void FireSuitActorProxy::BuildInvokables()
{
   GameItemActorProxy::BuildInvokables();
}

////////////////////////////////////////////////////
FireSuitActor::FireSuitActor(dtGame::GameActorProxy& parent)
   : GameItemActor(parent)
{
   mItemIndex = 0;
}

FireSuitActor::~FireSuitActor()
{

}

void FireSuitActor::Activate(bool enable)
{
   GameItemActor::Activate(enable);

   const std::string& name = "DonFireFighterEnsemble";

   // No event, peace out
   if (!IsActivated())
   {
      return;
   }

   dtCore::GameEvent* event = dtCore::GameEventManager::GetInstance().FindEvent(name);
   if (event == NULL)
   {
      throw dtUtil::Exception("Failed to find the game event: " + name, __FILE__, __LINE__);
   }

   dtGame::GameManager& mgr = *GetGameActorProxy().GetGameManager();
   RefPtr<dtGame::Message> msg =
      mgr.GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_GAME_EVENT);

   dtGame::GameEventMessage& gem = static_cast<dtGame::GameEventMessage&>(*msg);
   gem.SetGameEvent(*event);
   mgr.SendMessage(gem);
}
