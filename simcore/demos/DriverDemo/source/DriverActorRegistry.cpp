/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
* 
* See the .h file for complete licensing information.
* 
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
* 
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>
#include <DriverActorRegistry.h>
#include <SimCore/Actors/EntityActorRegistry.h>

//#ifdef AGEIA_PHYSICS
#include <HoverVehicleActor.h>
#include <HoverTargetActor.h>
#include <HoverExplodingTargetActor.h>
#include <dtPhysics/physicsactcomp.h>
//#include <NxAgeiaWorldComponent.h>
//#endif

#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

using dtCore::RefPtr;

namespace DriverDemo
{

   RefPtr<dtCore::ActorType> DriverActorRegistry::HOVER_VEHICLE_ACTOR_TYPE(
      new dtCore::ActorType("HoverActor", "DriverDemo", "A floaty drivable vehicle for Driver Demo", 
      SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));
   RefPtr<dtCore::ActorType> DriverActorRegistry::HOVER_TARGET_ACTOR_TYPE(
      new dtCore::ActorType("HoverTargetActor", "DriverDemo", "A floaty shootable target object for Driver Demo",
      SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));
   RefPtr<dtCore::ActorType> DriverActorRegistry::HOVER_EXPLODING_TARGET_ACTOR_TYPE(
      new dtCore::ActorType("HoverExplodingTargetActor", "DriverDemo", "A floaty shootable target that explodes for Driver Demo",
      SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

   ///////////////////////////////////////////////////////////////////////////
   extern "C" DRIVER_DEMO_EXPORT dtCore::ActorPluginRegistry* CreatePluginRegistry()
   {
       return new DriverActorRegistry;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" DRIVER_DEMO_EXPORT void DestroyPluginRegistry(dtCore::ActorPluginRegistry *registry)
   {
       delete registry;
   }

   ///////////////////////////////////////////////////////////////////////////
   DriverActorRegistry::DriverActorRegistry() :
      dtCore::ActorPluginRegistry("This library holds actors from the Driver Demo")
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   void DriverActorRegistry::RegisterActorTypes()
   {
      mActorFactory->RegisterType<HoverVehicleActorProxy>(HOVER_VEHICLE_ACTOR_TYPE.get());
      mActorFactory->RegisterType<HoverTargetActorProxy>(HOVER_TARGET_ACTOR_TYPE.get());
      mActorFactory->RegisterType<HoverExplodingTargetActorProxy>(HOVER_EXPLODING_TARGET_ACTOR_TYPE.get());
   }
}
