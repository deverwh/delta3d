/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
* @author Matthew W. Campbell
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/MessageType.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/actorproxyicon.h>
#include <dtCore/project.h>

#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>

#include <dtCore/shadergroup.h>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>
#include <dtUtil/datapathutils.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/functor.h>
#include <dtUtil/nodemask.h>
#include <dtCore/transform.h>

#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/invokable.h>

#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/StateSet>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgUtil/GLObjectsVisitor>

#include <iostream>

#include <dtPhysics/physicsreaderwriter.h>
#include <dtPhysics/geometry.h>
#include <dtPhysics/physicsmaterialactor.h>

namespace SimCore
{
   namespace Actors
   {
      static const std::string LOAD_NODE_TERRAIN_TIMER;
      // time between checks for loaded terrain.
      static const float LOAD_NODE_TIMER_TIMEOUT = 0.1;

      ///////////////////////////////////////////////////////////////////////////////
      LoadNodeTask::LoadNodeTask()
      : mUseFileCaching(true)
      , mComplete(false)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////
      LoadNodeTask::~LoadNodeTask()
      {
      }

      ///////////////////////////////////////////////////////////////////////////////
      void LoadNodeTask::operator()()
      {
         if (!mFileToLoad.empty())
         {
            dtCore::RefPtr<osgDB::ReaderWriter::Options> options;
            if (mLoadOptions.valid())
            {
               options = mLoadOptions;
            }
            else
            {
               options = new osgDB::ReaderWriter::Options;
            }

            if (mUseFileCaching)
            {
               options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
            }
            else
            {
               options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_NONE);
            }

            options->setOptionString("loadMaterialsToStateSet");

            mLoadedNode = osgDB::readNodeFile(mFileToLoad, options.get());
            mComplete = true;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////
      osg::Node* LoadNodeTask::GetLoadedNode()
      {
         return mLoadedNode;
      }

      ///////////////////////////////////////////////////////////////////////////////
      const osg::Node* LoadNodeTask::GetLoadedNode() const
      {
         return mLoadedNode;
      }

      ///////////////////////////////////////////////////////////////////////////////
      bool LoadNodeTask::IsComplete() const
      {
         return mComplete;
      }

      ///////////////////////////////////////////////////////////////////////////////
      void LoadNodeTask::ResetData()
      {
         mLoadedNode = NULL;
         mComplete = false;
      }

      DT_IMPLEMENT_ACCESSOR(LoadNodeTask, bool, UseFileCaching);
      DT_IMPLEMENT_ACCESSOR(LoadNodeTask, std::string, FileToLoad);
      DT_IMPLEMENT_ACCESSOR(LoadNodeTask, dtCore::RefPtr<osgDB::ReaderWriter::Options>, LoadOptions);

      ///////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////
      const std::string TerrainActor::DEFAULT_NAME = "Terrain";

      ///////////////////////////////////////////////////////////////////////////////
      TerrainActorProxy::TerrainActorProxy()
      {
         SetClassName("SimCore::Actors::TerrainActorProxy");
         SetHideDTCorePhysicsProps(true);
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActorProxy::OnRemovedFromWorld()
      {
         dtGame::GameActorProxy::OnRemovedFromWorld();
      }


      ///////////////////////////////////////////////////////////////////////////////
      void TerrainActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();
         TerrainActor* ta = NULL;
         GetDrawable(ta);

         static const dtUtil::RefString GROUP_("Terrain");

         static const dtUtil::RefString PROPERTY_TERRAIN_PHYSICS_MODE("Terrain Physics Mode");
         static const dtUtil::RefString PROPERTY_TERRAIN_PHYSICS_MODE_DESC("The mode for the physics, DISABLED disables all physics, DEFERRED"
                  " uses the cull visitor to load in the background for paged terrains, and IMMEDIATE loads the physics immediately based on the"
                  " properties on this actor.");

         AddProperty(new dtCore::EnumActorProperty<TerrainPhysicsMode>(
                 PROPERTY_TERRAIN_PHYSICS_MODE, PROPERTY_TERRAIN_PHYSICS_MODE,
            dtCore::EnumActorProperty<TerrainPhysicsMode>::SetFuncType(ta, &TerrainActor::SetTerrainPhysicsMode),
            dtCore::EnumActorProperty<TerrainPhysicsMode>::GetFuncType(ta, &TerrainActor::GetTerrainPhysicsMode),
            PROPERTY_TERRAIN_PHYSICS_MODE_DESC, GROUP_));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TERRAIN,
            "TerrainMesh", "TerrainMesh",
            dtCore::ResourceActorProperty::SetFuncType(ta, &TerrainActor::LoadFile),
            "Loads in a terrain mesh for this object", GROUP_));

         static const dtUtil::RefString PROPERTY_LOAD_TERRAIN_MESH_WITH_CACHING("LoadTerrainMeshWithCaching");
         AddProperty(new dtCore::BooleanActorProperty(
            PROPERTY_LOAD_TERRAIN_MESH_WITH_CACHING, PROPERTY_LOAD_TERRAIN_MESH_WITH_CACHING,
            dtCore::BooleanActorProperty::SetFuncType(ta, &TerrainActor::SetLoadTerrainMeshWithCaching),
            dtCore::BooleanActorProperty::GetFuncType(ta, &TerrainActor::GetLoadTerrainMeshWithCaching),
            "Enables OSG caching when loading the terrain mesh.", GROUP_));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::STATIC_MESH,
            "PhysicsModel", "PhysicsModel",
            dtCore::ResourceActorProperty::SetFuncType(ta, &TerrainActor::SetPhysicsModelFile),
            "Loads a SINGLE physics model file to use for collision.", GROUP_));

         static const dtUtil::RefString PROPERTY_PHYSICSDIRECTORY("PhysicsDirectory");
         AddProperty(new dtCore::StringActorProperty(PROPERTY_PHYSICSDIRECTORY, PROPERTY_PHYSICSDIRECTORY,
            dtCore::StringActorProperty::SetFuncType(ta, &TerrainActor::SetPhysicsDirectory),
            dtCore::StringActorProperty::GetFuncType(ta, &TerrainActor::GetPhysicsDirectory),
            "The directory name of MULTIPLE physics model files to use for collision within the Terrains folder in your map project.", GROUP_));

      }

      /////////////////////////////////////////////////////////////////////////////
      dtCore::ActorProxyIcon* TerrainActorProxy::GetBillBoardIcon()
      {
         if (!mBillBoardIcon.valid())
         {
            mBillBoardIcon = new dtCore::ActorProxyIcon("billboards/staticmesh.png");
         }

         return mBillBoardIcon.get();
      }

      ///////////////////////////////////////////////////////////////////
      void TerrainActorProxy::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();

         dtCore::RefPtr<dtPhysics::PhysicsActComp> physAC = new dtPhysics::PhysicsActComp();

         AddComponent(*physAC);

         // Adding this physics object AFTER the build actor components because we don't want the properties to be
         // accessible.
         // Add our initial body.
         dtPhysics::PhysicsObject* pobj = new dtPhysics::PhysicsObject("Terrain");
         pobj->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);
         pobj->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
         pobj->SetCollisionGroup(SimCore::CollisionGroup::GROUP_TERRAIN);
         physAC->AddPhysicsObject(*pobj);
      }

      ///////////////////////////////////////////////////////////////////
      void TerrainActorProxy::BuildInvokables()
      {
         dtGame::GameActorProxy::BuildInvokables();

         static const dtUtil::RefString LOAD_CHECK_TIMER_INVOKABLE("LOAD_CHECK_TIMER_INVOKABLE");
         AddInvokable(*new dtGame::Invokable(LOAD_CHECK_TIMER_INVOKABLE,
            dtUtil::MakeFunctor(&TerrainActorProxy::HandleNodeLoaded, this)));

         RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_TIMER_ELAPSED, LOAD_CHECK_TIMER_INVOKABLE);
      }

      ///////////////////////////////////////////////////////////////////
      void TerrainActorProxy::HandleNodeLoaded(const dtGame::TimerElapsedMessage& timerElapsed)
      {
         TerrainActor* ta = NULL;
         GetDrawable(ta);
         if (ta != NULL)
         {
            if (ta->CheckForTerrainLoaded())
            {
               ta->GetOSGNode()->setNodeMask(dtUtil::NodeMask::TERRAIN_GEOMETRY | dtUtil::NodeMask::NON_TRANSPARENT_GEOMETRY | dtUtil::NodeMask::SHADOW_RECEIVE);
               GetGameManager()->ClearTimer(LOAD_NODE_TERRAIN_TIMER, this);
               ta->SetupTerrainPhysics();
            }
         }
      }


      ///////////////////////////////////////////////////////////////
      void TerrainActor::RemovedFromScene(dtCore::Scene* scene)
      {
         scene->ResetDatabasePager();
      }


      ///////////////////////////////////////////////////////////////
      TerrainActor::TerrainActor(dtGame::GameActorProxy& owner)
      : IGActor(owner)
      , mTerrainPhysicsMode(&SimCore::TerrainPhysicsMode::DEFERRED)
      , mNeedToLoad(false)
      , mLoadTerrainMeshWithCaching(false)
      {
         SetName(DEFAULT_NAME);
      }

      /////////////////////////////////////////////////////////////////////////////
      TerrainActor::~TerrainActor()
      {
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::OnEnteredWorld()
      {
         dtGame::GameActor::OnEnteredWorld();
         LoadFile(mLoadedFile);
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::AddedToScene(dtCore::Scene* scene)
      {
         IGActor::AddedToScene(scene);
         //Actually load the file, even if it's empty string so that if someone were to
         //load a mesh, remove it from the scene, then try to clear the mesh, this actor will still
         //work.
         if (GetGameActorProxy().IsInSTAGE())
         {
            LoadFile(mLoadedFile);
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetPhysicsModelFile(const std::string& filename)
      {
         mCollisionResourceString = filename;
      }

      /////////////////////////////////////////////////////////////////////////////
      const std::string& TerrainActor::GetPhysicsModelFile() const
      {
         return mCollisionResourceString;
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetPhysicsDirectory( const std::string& filename )
      {
         mPhysicsDirectory = filename;
      }

      /////////////////////////////////////////////////////////////////////////////
      std::string TerrainActor::GetPhysicsDirectory() const
      {
         return mPhysicsDirectory;
      }

      /////////////////////////////////////////////////////////////////////////////
      TerrainPhysicsMode& TerrainActor::GetTerrainPhysicsMode() const
      {
         return *mTerrainPhysicsMode;
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetTerrainPhysicsMode(TerrainPhysicsMode& physicsMode)
      {
         mTerrainPhysicsMode = &physicsMode;
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::SetupTerrainPhysics()
      {
         GetComponent(mHelper);

         if (!mTerrainNode.valid())
         {
            return;
         }

         // For STAGE
         if (GetGameActorProxy().GetGameManager() == NULL)
         {
            return;
         }

         if (mTerrainPhysicsMode == &SimCore::TerrainPhysicsMode::IMMEDIATE)
         {
            dtCore::Transform xform;
            GetTransform(xform);

            dtPhysics::PhysicsComponent* comp = NULL;
            GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, comp);

            if (comp != NULL)
            {
               bool loadSuccess = false;

               osg::Vec3 pos;
               xform.GetTranslation(pos);

               mCollisionResourceString = dtUtil::FindFileInPathList( mCollisionResourceString.c_str() );
               if (!mCollisionResourceString.empty())
               {

                  LoadMeshFromFile(mCollisionResourceString, std::string());

                  loadSuccess = true;
               }

               //next, if a physics directory is specified, we will load all files in that directory
               if (!mPhysicsDirectory.empty())
               {
                  try
                  {
                     std::string fullDirPath = dtCore::Project::GetInstance().GetContext() + "/Terrains/" + mPhysicsDirectory;

                     if(dtUtil::FileUtils::GetInstance().DirExists(fullDirPath))
                     {
                        dtUtil::FileExtensionList extensionList;
                        extensionList.push_back(".dtphys");

                        const dtUtil::DirectoryContents& filesInDir = dtUtil::FileUtils::GetInstance().DirGetFiles(fullDirPath, extensionList);
                        dtUtil::DirectoryContents::const_iterator iter = filesInDir.begin();
                        dtUtil::DirectoryContents::const_iterator iterEnd = filesInDir.end();

                        for( ;iter != iterEnd; ++iter)
                        {
                           const std::string& curFile = *iter;
                           std::string fileWithPath = fullDirPath + "/" + curFile;

                           //double check this isn't the same one we loaded above
                           if(!mCollisionResourceString.empty() && dtUtil::FileUtils::GetInstance().IsSameFile(mCollisionResourceString, fileWithPath))
                           {
                              //don't load file
                           }
                           else
                           {
                              LoadMeshFromFile(fileWithPath, curFile);
                           }
                        }

                        LOG_INFO("Loaded " + dtUtil::ToString(filesInDir.size()) + " physics model files from directory '" + fullDirPath + "'." );
                        loadSuccess = true;
                     }
                     else
                     {
                        LOG_ERROR("Attempting to load physics mesh from file, cannot load directory '" + fullDirPath + "'.");
                     }

                  }
                  catch (dtUtil::Exception& e)
                  {
                     e.LogException(dtUtil::Log::LOG_ERROR);
                  }
               }
               if (!loadSuccess && mTerrainNode.valid())
               {
                  //if we didn't find a pre-baked static mesh but we did have a renderable terrain node
                  //then just bake a static collision mesh with that
                  mHelper->GetMainPhysicsObject()->SetTransform(xform);
                  const dtPhysics::MaterialActor* matAct = mHelper->LookupMaterialActor();
                  if (matAct != NULL)
                  {
                     mHelper->GetMainPhysicsObject()->SetMaterial(matAct->GetMaterial());
                  }
                  mHelper->GetMainPhysicsObject()->Create(mTerrainNode.get());
                  loadSuccess = true;
               }

               if (!loadSuccess)
               {
                  LOG_ERROR("Could not find valid terrain mesh or pre-baked collision mesh to create collision data for terrain.");
               }
            }
            else
            {
               LOG_ERROR("No Physics Component exists in the Game Manager.");
            }


            //Set the helper name to match the actor name.
            mHelper->SetName(GetName());
         }

         dtCore::RefPtr<dtGame::Message> terrainMessage;
         GetGameActorProxy().GetGameManager()->GetMessageFactory().CreateMessage(SimCore::MessageType::INFO_TERRAIN_LOADED, terrainMessage);
         terrainMessage->SetAboutActorId(GetUniqueId());
         GetGameActorProxy().GetGameManager()->SendMessage(*terrainMessage);
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::LoadMeshFromFile(const std::string& fileToLoad, const std::string& materialType)
      {
         if (dtUtil::FileUtils::GetInstance().FileExists(fileToLoad))
         {
            std::string filename = dtUtil::FindFileInPathList(fileToLoad);
            if (!filename.empty())
            {
               dtCore::RefPtr<dtPhysics::VertexData> vertData = new dtPhysics::VertexData;

               if (dtPhysics::PhysicsReaderWriter::LoadTriangleDataFile(*vertData, fileToLoad))
               {
                  dtCore::Transform geometryWorld;
                  GetTransform(geometryWorld);

                  dtCore::RefPtr<dtPhysics::PhysicsObject> newTile = new dtPhysics::PhysicsObject(fileToLoad);
                  newTile->SetTransform(geometryWorld);
                  newTile->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
                  newTile->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);

                  dtCore::RefPtr<dtPhysics::Geometry> geom = dtPhysics::Geometry::CreateConcaveGeometry(geometryWorld, *vertData, 0);
                  newTile->CreateFromGeometry(*geom);

                  newTile->SetCollisionGroup(SimCore::CollisionGroup::GROUP_TERRAIN);
                  const dtPhysics::MaterialActor* matAct = mHelper->LookupMaterialActor();
                  if (matAct != NULL)
                  {
                     newTile->SetMaterial(matAct->GetMaterial());
                  }

                  mHelper->AddPhysicsObject(*newTile);
               }
               else
               {
                  LOG_ERROR("Unable to load physics mesh file '" + fileToLoad + "'.");
               }
            }
         }
         else
         {
            LOG_ERROR("Unable to find physics mesh file '" + fileToLoad + "'.");
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void TerrainActor::LoadFile(const std::string& fileName)
      {
         // Don't do anything if the filenames are the same unless we still need to load
         if (!mNeedToLoad && mLoadedFile == fileName)
         {
            return;
         }

         //Don't actually load the file unless we are in the scene.
         if (GetSceneParent() != NULL)
         {
            //We should always clear the geometry.  If LoadFile fails, we should have no geometry.
            if (GetMatrixNode()->getNumChildren() != 0)
            {
               GetMatrixNode()->removeChild(0, GetMatrixNode()->getNumChildren());
            }
            // If the terrain changes, unload the physics.
            dtPhysics::PhysicsActComp* pac = NULL;
            GetComponent(pac);
            if (pac != NULL)
            {
               pac->CleanUp();
            }

            // Empty string is not an error, just has no geometry.
            if (!fileName.empty())
            {
               if (mLoadNodeTask.valid())
               {
                  // If you try to change the terrain while it's loading
                  // then tough, we just have to block until it's done.
                  mLoadNodeTask->WaitUntilComplete();
                  mLoadNodeTask->ResetData();
               }
               else
               {
                  mLoadNodeTask = new LoadNodeTask();
               }

               mLoadNodeTask->SetUseFileCaching(mLoadTerrainMeshWithCaching);
               mLoadNodeTask->SetFileToLoad(fileName);

               dtCore::RefPtr<osgDB::ReaderWriter::Options> options;
               if (!options.valid())
               {
                  options = new osgDB::ReaderWriter::Options;
                  mLoadNodeTask->SetLoadOptions(options);
               }

               options->setOptionString("loadMaterialsToStateSet");

               // Can't load in the background if we aren't in the GM.
               if (!GetGameActorProxy().IsInGM())
               {
                  dtUtil::ThreadPool::AddTask(*mLoadNodeTask, dtUtil::ThreadPool::IMMEDIATE);
                  dtUtil::ThreadPool::ExecuteTasks();
                  if (CheckForTerrainLoaded())
                  {
                     SetupTerrainPhysics();
                  }
               }
               else
               {
                  dtUtil::ThreadPool::AddTask(*mLoadNodeTask, dtUtil::ThreadPool::IO);
                  // This timer is repeating, so it must be cleared when it's over.
                  GetGameActorProxy().GetGameManager()->SetTimer(LOAD_NODE_TERRAIN_TIMER, &GetGameActorProxy(), LOAD_NODE_TIMER_TIMEOUT, true, true);
               }
            }

            // go ahead and start this because even if the loading fails later
            // it still tried, and we don't want any code thinking it hasn't attempted to load yet.
            mNeedToLoad = false;

         }
         else
         {
            mNeedToLoad = true;
         }
         mLoadedFile = fileName;
      }

      ///////////////////////////////////////////////////////////////////
      bool TerrainActor::CheckForTerrainLoaded()
      {
         if (!mLoadNodeTask.valid())
         {
            // It's done already or nothing is happening.
            return mTerrainNode.valid();
         }

         if (!mLoadNodeTask->IsComplete())
         {
            return false;
         }

         // It is "complete" so wait to make sure the task clears the thread pool.
         mLoadNodeTask->WaitUntilComplete();

         if (mLoadNodeTask->GetLoadedNode() != NULL)
         {
            mTerrainNode = mLoadNodeTask->GetLoadedNode();

            //osg::StateSet* ss =
            mTerrainNode->getOrCreateStateSet();
            //ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_TERRAIN, "TerrainBin");

            // Run a visitor to switch to VBO's instead of DrawArrays (the OSG default)
            // Turning this on had a catastrophic impact on performance. OFF is better for now.
            //osgUtil::GLObjectsVisitor nodeVisitor(osgUtil::GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS);
            //mTerrainNode->accept(nodeVisitor);
            GetMatrixNode()->addChild(mTerrainNode.get());

            if (!GetShaderGroup().empty())
            {
               // TODO - Figure out why we have to do this next hack.
               // Hack Explanation - if the actor was a prototype, then setting the shader doesn't seem
               // to take effect. So, we set it again. However, we have to set it back to empty string
               // or the SetShaderGroup won't do anything.
               std::string shaderToSet = GetShaderGroup();
               SetShaderGroup(""); // clear the shader so that it will accept the new setting
               SetShaderGroup(shaderToSet);
            }

            mLoadNodeTask = NULL;
         }

         return true;

      }

      ///////////////////////////////////////////////////////////////////
      void TerrainActor::SetLoadTerrainMeshWithCaching(bool enable)
      {
         mLoadTerrainMeshWithCaching = enable;
      }

      ///////////////////////////////////////////////////////////////////
      bool TerrainActor::GetLoadTerrainMeshWithCaching()
      {
         return mLoadTerrainMeshWithCaching;
      }

   }
}
