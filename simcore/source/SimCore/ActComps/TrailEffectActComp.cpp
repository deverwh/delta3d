/* -*-c++-*-
* Simulation Core
* Copyright 2010, Alion Science and Technology
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
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
* @author Chris Rodgers
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osg/NodeVisitor>
#include <dtCore/particlesystem.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/propertymacros.h>
#include <dtGame/basemessages.h>
#include <dtGame/gameactor.h>
#include <dtGame/invokable.h>
#include <dtGame/messagetype.h>
#include <dtUtil/datapathutils.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/Utilities.h>
#include <SimCore/ActComps/TrailEffectActComp.h>
#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Components/ParticleManagerComponent.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CONSTANTS
      //////////////////////////////////////////////////////////////////////////
      const dtGame::ActorComponent::ACType TrailEffectActComp::TYPE(new dtCore::ActorType("TrailEffectActComp", "ActorComponents",
            "Creates A trail of particles following a drawable.", dtGame::ActorComponent::BaseActorComponentType));
      const dtUtil::RefString TrailEffectActComp::PROPERTY_TRAIL_PARTICLES("Trail Particles");
      const dtUtil::RefString TrailEffectActComp::PROPERTY_TRAIL_ATTACHED("Trail Attached");
      const dtUtil::RefString TrailEffectActComp::PROPERTY_TRAIL_ATTACH_NODE_NAME("Trail Attach Node Name");
      const dtUtil::RefString TrailEffectActComp::PROPERTY_TRAIL_CLAMP_INTERVAL("Trail Clamp Interval");
      const dtUtil::RefString TrailEffectActComp::PROPERTY_TRAIL_ENABLE_DISTANCE("Trail Enable Distance");

      const float TrailEffectActComp::DEFAULT_TRAIL_CLAMP_INTERVAL = 1.0f;
      const float TrailEffectActComp::DEFAULT_TRAIL_ENABLE_DISTANCE = 1.0f;



      //////////////////////////////////////////////////////////////////////////
      // HELPER FUNCTIONS
      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::TerrainActor* GetTerrainActor(TrailEffectActComp& comp)
      {
         SimCore::Actors::TerrainActor* terrain = NULL;

         dtCore::Transform xform;
         dtGame::GameActorProxy* actor = NULL;
         comp.GetOwner(actor);
         if(actor != NULL)
         {
            dtGame::GameManager* gm = actor->GetGameManager();
            if(gm != NULL)
            {
               dtGame::GameActorProxy* terrainA = NULL;
               gm->FindActorByType(*SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, terrainA);

               if(terrainA != NULL)
               {
                  terrainA->GetDrawable(terrain);
               }
            }
         }

         return terrain;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Components::ParticleManagerComponent* GetParticleComponent(dtGame::GameManager& gm)
      {
         SimCore::Components::ParticleManagerComponent* comp = NULL;
         gm.GetComponentByName(SimCore::Components::ParticleManagerComponent::DEFAULT_NAME, comp);

         return comp;
      }

      //////////////////////////////////////////////////////////////////////////
      void GetOwnerPosition(TrailEffectActComp& comp, osg::Vec3& outPos)
      {
         dtGame::GameActorProxy* actor = NULL;
         comp.GetOwner(actor);
         if(actor != NULL)
         {
            outPos = actor->GetTranslation();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool GetClampPosition(TrailEffectActComp& comp, const osg::Vec3& startPoint, osg::Vec3& outPoint)
      {
         bool success = false;

         dtCore::Transform xform;
         xform.SetTranslation(startPoint);
//         SimCore::Utils::KeepBodyOnGround(xform, 0.0f, 0.0f, 0.0f, 0.0f);

         SimCore::Actors::TerrainActor* terrain =  GetTerrainActor(comp);
         if(terrain != NULL)
         {
            success = SimCore::Utils::KeepTransformOnGround(xform, *terrain, 0.0f, 0.0, 0.0f);
         }

         xform.GetTranslation(outPoint);

         return success;
      }



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      TrailEffectActComp::TrailEffectActComp()
         : BaseClass(TYPE)
         , mOwnerIsPlatform(false)
         , mClampTimer(0.0f)
      {
         SetDefaults();
      }

      //////////////////////////////////////////////////////////////////////////
      TrailEffectActComp::TrailEffectActComp(const ActorComponent::ACType& actType) // for derived classes to call
         : BaseClass(actType)
         , mOwnerIsPlatform(false)
         , mClampTimer(0.0f)
      {
         SetDefaults();
      }

      //////////////////////////////////////////////////////////////////////////
      TrailEffectActComp::~TrailEffectActComp()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::SetDefaults()
      {
         SetTrailClampInterval(DEFAULT_TRAIL_CLAMP_INTERVAL);
         SetTrailEnableDistance(DEFAULT_TRAIL_ENABLE_DISTANCE);
         SetTrailAttached(false);
      }



      //////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter and Setter method body for each property
      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(TrailEffectActComp, float, TrailClampInterval);
      DT_IMPLEMENT_ACCESSOR(TrailEffectActComp, float, TrailEnableDistance);
      DT_IMPLEMENT_ACCESSOR(TrailEffectActComp, bool, TrailAttached);
      DT_IMPLEMENT_ACCESSOR(TrailEffectActComp, std::string, TrailAttachNodeName);
      DT_IMPLEMENT_ACCESSOR_GETTER(TrailEffectActComp, dtCore::ResourceDescriptor, TrailParticlesFile); // Setter is implemented below

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::SetTrailParticlesFile(const dtCore::ResourceDescriptor& file)
      {
         mTrailParticlesFile = file;
      }



      //////////////////////////////////////////////////////////////////////////
      // SPECIAL METHODS
      //////////////////////////////////////////////////////////////////////////
      bool TrailEffectActComp::LoadParticles()
      {
         bool success = false;

         if( ! mTrailParticlesFile.GetResourceIdentifier().empty())
         {
            std::string res;
            try
            {
               res = dtCore::Project::GetInstance().GetResourcePath(mTrailParticlesFile.GetResourceIdentifier());
            }
            catch(std::exception& ex)
            {
               std::ostringstream oss;
               oss << "Exception: " << ex.what() << "\n";
               LOG_ERROR(oss.str());
            }
            catch(...)
            {
               std::ostringstream oss;
               oss << "Unknown Exception finding: " << mTrailParticlesFile.GetResourceIdentifier() << "\n";
               LOG_ERROR(oss.str());
            }

            if( ! res.empty())
            {
               mParticles = new dtCore::ParticleSystem("TrailEffect");

               if(mParticles->LoadFile(res) == NULL)
               {
                  mParticles = NULL;

                  std::ostringstream oss;
                  oss << "Could not successfully load particle file \"" << res << "\"\n";
                  LOG_WARNING(oss.str());
               }
               else
               {
                  success = true;
               }
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      class FindNodeVisitor : public osg::NodeVisitor
      {
      public:
         dtCore::RefPtr<osg::Group> mFoundNode;

         FindNodeVisitor(const std::string& nodeName)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
         {}

         virtual void apply(osg::Group& node)
         {
            if( ! mFoundNode.valid())
            {
               if(node.getName() == mNodeName)
               {
                  mFoundNode = &node;
               }

               traverse(node);
            }
         }

         private:
            std::string mNodeName;
      };

      //////////////////////////////////////////////////////////////////////////
      bool TrailEffectActComp::AttachParticles()
      {
         bool success = false;

         if(mParticles.valid())
         {
            dtGame::GameActorProxy* actor = NULL;
            GetOwner(actor);

            if(actor != NULL)
            {
               osg::Group* rootNode = actor->GetDrawable()->GetOSGNode()->asGroup();
               osg::Node* particleRoot = mParticles->GetLoadedParticleSystemRoot();

               if(mTrailAttached)
               {
                  if( ! mTrailAttachNodeName.empty())
                  {
                     // Find the specified node by name.
                     dtCore::RefPtr<FindNodeVisitor> visitor = new FindNodeVisitor(mTrailAttachNodeName);
                     rootNode->accept(*visitor);

                     // Attach to the found node.
                     if(visitor->mFoundNode.valid())
                     {
                        visitor->mFoundNode->addChild(particleRoot);
                        success = true;
                     }
                     else
                     {
                        std::ostringstream oss;
                        oss << "Could not attach trail effect to actor \""
                           << actor->GetName() << "\" because the specified node \""
                           << mTrailAttachNodeName << "\" could not be found on the actor.\n";
                        LOG_WARNING(oss.str());
                     }
                  }

                  // Attached to the root node of the parent if the specified node was not found.
                  if( ! success && rootNode != NULL)
                  {
                     if(actor->GetDrawable()->AddChild(mParticles.get()))
                     {
                        dtCore::Transform xform;
                        mParticles->SetTransform(xform,dtCore::Transformable::REL_CS);
                     }
                  }
               }
               else // not attached
               {
                  dtGame::GameManager* gm = actor->GetGameManager();
                  if(gm != NULL)
                  {
                     gm->GetScene().AddChild(mParticles.get());
                     success = true;
                  }
                  else
                  {
                     std::ostringstream oss;
                     oss << "could not add trail effect to the scene because the Game Manager could not be accessed.\n";
                     LOG_WARNING(oss.str());
                  }
               }
            }

            // Register the particles with the Particle Manager Component
            // so that the particles can be tracked and affected by the
            // global wind force.
            if(mParticles.valid() && actor != NULL)
            {
               SimCore::Components::ParticleManagerComponent* comp = GetParticleComponent(*actor->GetGameManager());
               if(comp != NULL)
               {
                  // Set the flags to ensure the particles are affected by wind.
                  SimCore::Components::ParticleInfoAttributeFlags flags;
                  flags.mEnableWind = true;
                  flags.mAddWindToAllLayers = true;

                  // Register the particles.
                  if( ! comp->Register(*mParticles, &flags))
                  {
                     LOG_WARNING("Could not register Trail Effect particles.");
                  }
               }
               else
               {
                  LOG_WARNING("Could not access the Particle Manager Component to register Trail Effect particles.");
               }
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      int TrailEffectActComp::DetachParticles()
      {
         int count = 0;

         if(mParticles.valid())
         {
            osg::Group* parent = NULL;
            osg::Node* particleNode = mParticles->GetLoadedParticleSystemRoot();

            // Ensure the particles are not attached to the actor directly.
            dtGame::GameActorProxy* actor = NULL;
            GetOwner(actor);
            if(actor != NULL)
            {
               actor->GetDrawable()->RemoveChild(mParticles.get());
            }

            dtGame::GameManager* gm = actor->GetGameManager();
            // Ensure the particles are unregistered from the Particle Manager
            SimCore::Components::ParticleManagerComponent* comp = GetParticleComponent(*gm);
            if(comp != NULL)
            {
               comp->Unregister(*mParticles);
            }

            // Ensure the particles are not attached directly to the scene.
            if(gm != NULL)
            {
               gm->GetScene().RemoveChild(mParticles.get());
            }

            // Ensure the particle effect is not attached to the owner's sub-nodes.
            do 
            {
               parent = particleNode->getNumParents() > 0 ? particleNode->getParent(0) : NULL;
               if(parent != NULL)
               {
                  parent->removeChild(particleNode);
                  ++count;
               }
            } while(parent != NULL);
         }

         return count;
      }

      //////////////////////////////////////////////////////////////////////////
      bool TrailEffectActComp::IsTickable() const
      {
         return ! mTrailAttached;
      }

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::SetEnabled(bool enable)
      {
         if(mParticles.valid())
         {
            mParticles->SetEnabled(enable);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool TrailEffectActComp::IsEnabled() const
      {
         return mParticles.valid() && mParticles->IsEnabled();
      }

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::Update(float timeDelta)
      {
         using namespace SimCore::Actors;

         if(IsTickable())
         {
            osg::Vec3 pos;
            GetOwnerPosition(*this, pos);

            if(mClampTimer >= 0.0f)
            {
               mClampTimer -= timeDelta;

               if(mClampTimer <= 0.0f)
               {
                  mClampTimer = mTrailClampInterval;

                  osg::Vec3 clampPoint;
                  bool clampSuccess = GetClampPosition(*this, pos, clampPoint);
                  if(clampSuccess)
                  {
                     mClampPoint = clampPoint;

                     float distance = dtUtil::Abs(pos.z() - mClampPoint.z());

                     // Enable/disable the effect based on the distance to the ground
                     // and by the damage state if the actor is a Platform type.
                     if(mOwnerIsPlatform)
                     {
                        // Get the damage state of the entity.
                        dtGame::GameActorProxy* actor = NULL;
                        GetOwner(actor);
                        BaseEntityActorProxy::DamageStateEnum* damage
                           = &static_cast<Platform*>(actor->GetDrawable())->GetDamageState();

                        SetEnabled(distance < mTrailEnableDistance
                           && damage != &BaseEntityActorProxy::DamageStateEnum::DESTROYED);
                     }
                     else
                     {
                        SetEnabled(distance < mTrailEnableDistance);
                     }
                  }
                  else
                  {
                     SetEnabled(false);
                  }
               }
            }

            pos.z() = mClampPoint.z();
            SetParticlePosition(pos);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         Update(tickMessage.GetDeltaSimTime());
      }

      //////////////////////////////////////////////////////////////////////////
      // OVERRIDE METHODS
      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         BaseClass::OnTickLocal(tickMessage);

         Update(tickMessage.GetDeltaSimTime());
      }

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         // Register tick handlers only if the particle effect is valid.
         if(LoadParticles() && IsTickable())
         {
            RegisterForTick();
         }

         AttachParticles();

         // Disable the effect if the owner actor is a platform type
         // with a damage state of destroyed.
         using namespace SimCore::Actors;
         dtGame::GameActorProxy* actor = NULL;
         GetOwner(actor);
         if (actor != NULL)
         {
            Platform* platform = NULL;
            actor->GetDrawable(actor);
            if(platform != NULL)
            {
               mOwnerIsPlatform = true;
               SetEnabled(platform->GetDamageState() != BaseEntityActorProxy::DamageStateEnum::DESTROYED);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();

         UnregisterForTick();

         DetachParticles();
      }


      //////////////////////////////////////////////////////////////////////////
      void TrailEffectActComp::BuildPropertyMap()
      {
         typedef dtCore::PropertyRegHelper<TrailEffectActComp&, TrailEffectActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Trail Effect");

         // FLOAT PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            TrailClampInterval,
            PROPERTY_TRAIL_CLAMP_INTERVAL,
            PROPERTY_TRAIL_CLAMP_INTERVAL,
            "Time interval between ground clamp attempts of the effect.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            TrailEnableDistance,
            PROPERTY_TRAIL_ENABLE_DISTANCE,
            PROPERTY_TRAIL_ENABLE_DISTANCE,
            "Max distance above the surface in which the trail effect remains enabled",
            PropRegType, propRegHelper);

         // BOOLEAN PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            TrailAttached,
            PROPERTY_TRAIL_ATTACHED,
            PROPERTY_TRAIL_ATTACHED,
            "Flag to determine if the trail effect is attachable to the parent actor.",
            PropRegType, propRegHelper);

         // STRING PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            TrailAttachNodeName,
            PROPERTY_TRAIL_ATTACH_NODE_NAME,
            PROPERTY_TRAIL_ATTACH_NODE_NAME,
            "Name of the node on the parent actor to attach to",
            PropRegType, propRegHelper);

         // FILE PROPERTIES
         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(
            dtCore::DataType::PARTICLE_SYSTEM,
            TrailParticlesFile,
            PROPERTY_TRAIL_PARTICLES,
            PROPERTY_TRAIL_PARTICLES,
            "Particle system that is used for the trail effect.",
            PropRegType, propRegHelper);
      }

      //////////////////////////////////////////////////////////////////////////
      // SPECIAL METHODS
      //////////////////////////////////////////////////////////////////////////

      void TrailEffectActComp::SetParticlePosition(const osg::Vec3& pos)
      {
         if(mParticles.valid())
         {
            dtCore::Transform xform;
            mParticles->GetTransform(xform);
            xform.SetTranslation(pos);
            mParticles->SetTransform(xform);
         }
      }

   } // ActComps namespace
} // SimCore namespace
