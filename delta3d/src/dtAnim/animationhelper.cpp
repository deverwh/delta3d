/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2007, Alion Science and Technology
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
 * Bradley Anderegg 03/30/2007
 * David Guthrie
 */

#include <dtAnim/animationhelper.h>

#include <dtAnim/animationchannel.h>
#include <dtAnim/animationgameactor.h>
#include <dtAnim/animationsequence.h>
#include <dtAnim/animationwrapper.h>
#include <dtAnim/animnodebuilder.h>
#include <dtAnim/cal3ddatabase.h>
#include <dtAnim/cal3dmodeldata.h>
#include <dtAnim/cal3dmodelwrapper.h>
#include <dtAnim/ical3ddriver.h>
#include <dtAnim/posesequence.h>

#include <dtCore/hotspotattachment.h>

#include <dtCore/actorproperty.h>
#include <dtCore/actorproxy.h>
#include <dtCore/datatype.h>
#include <dtCore/project.h>
#include <dtCore/exceptionenum.h>
#include <dtCore/object.h>
#include <dtCore/resourceactorproperty.h>

#include <dtGame/gameactorproxy.h>
#include <dtGame/gameactor.h>

#include <dtUtil/log.h>
#include <dtUtil/mathdefines.h>

#include <osg/MatrixTransform>
#include <osg/Texture2D>

namespace dtAnim
{
/////////////////////////////////////////////////////////////////////////////////
// CLASS CODE
/////////////////////////////////////////////////////////////////////////////////
const std::string AnimationHelper::PROPERTY_SKELETAL_MESH("Skeletal Mesh");

const dtGame::ActorComponent::ACType AnimationHelper::TYPE(new dtCore::ActorType("AnimationActorComponent", "ActorComponents",
       "Encapsulates skeletal mesh support and control.",
       dtGame::ActorComponent::BaseActorComponentType));

/////////////////////////////////////////////////////////////////////////////////
AnimationHelper::AnimationHelper()
   : BaseClass(TYPE)
   , mAutoRegisterWithGMComponent(true)
   , mLoadModelAsynchronously(false)
   , mEnableAttachingNodeToDrawable(true)
   , mGroundClamp(false)
   , mEnableCommands(false)
   , mLastUpdateTime(0.0)
   , mNode(NULL)
   , mAnimator(NULL)
   , mSequenceMixer(new SequenceMixer())
   , mAttachmentController(new AttachmentController())
{
   ModelLoadedSignal.connect_slot(this, &AnimationHelper::OnLoadCompleted);
   ModelUnloadedSignal.connect_slot(this, &AnimationHelper::OnUnloadCompleted);
}

/////////////////////////////////////////////////////////////////////////////////
AnimationHelper::~AnimationHelper()
{
}

//////////////////////////////////////////////////////////////////////
void AnimationHelper::OnEnteredWorld()
{
   BaseClass::OnEnteredWorld();
   if (!GetSkeletalMesh().IsEmpty())
   {
      LoadSkeletalMesh();
   }
}

//////////////////////////////////////////////////////////////////////
void AnimationHelper::OnRemovedFromWorld()
{
   BaseClass::OnRemovedFromWorld();
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::Update(float dt)
{
   if (mAnimator.valid())
   {
      mLastUpdateTime = mSequenceMixer->GetRootSequence().GetElapsedTime();
      CollectCommands(mLastUpdateTime, mLastUpdateTime + dt);

      mSequenceMixer->Update(dt);
      mAnimator->Update(dt);
      mAttachmentController->Update(*GetModelWrapper());
   }

   if (IsLoadingAsynchronously())
   {
      Cal3DDatabase& database = Cal3DDatabase::GetInstance();

      // See if the data is ready yet
      Cal3DModelData* modelData = database.GetModelData(mAsyncFile);

      if (modelData)
      {
         // Now that we have the data, create the model wrapper
         CalModel* model = new CalModel(modelData->GetCoreModel());
         dtAnim::Cal3DModelWrapper* newWrapper = new Cal3DModelWrapper(model);

         mAnimator = new Cal3DAnimator(newWrapper);
         mNode = database.GetNodeBuilder().CreateNode(newWrapper);

         RegisterAnimations(*modelData);
         CreateAttachments(*modelData);

         SetupPoses(*modelData);

         // Done loading, clear the file to load string
         mAsyncFile.clear();

         // Notify observers that the model has been loaded
         ModelLoadedSignal(this);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::PlayAnimation(const std::string& pAnim)
{
   const Animatable* anim = mSequenceMixer->GetRegisteredAnimation(pAnim);

   if (anim != NULL)
   {
      dtCore::RefPtr<Animatable> clonedAnim = anim->Clone(mAnimator->GetWrapper());
      mSequenceMixer->PlayAnimation(clonedAnim.get());
   }
   else
   {
      LOG_ERROR("Cannot play animation '" + pAnim +
            "' because it has not been registered with the SequenceMixer.")
   }
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::ClearAnimation(const std::string& pAnim, float fadeOutTime)
{
   mSequenceMixer->ClearAnimation(pAnim, fadeOutTime);
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::ClearAll(float fadeOut)
{
   mSequenceMixer->ClearActiveAnimations(fadeOut);
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::CreateAttachments(const Cal3DModelData& modelData)
{
   const Cal3DModelData::AttachmentArray& attachments = modelData.GetAttachments();

   Cal3DModelData::AttachmentArray::const_iterator i,iend;
   i = attachments.begin();
   iend = attachments.end();
   for (; i != iend; ++i)
   {
      dtUtil::HotSpotDefinition hsd = i->first;
      dtCore::RefPtr<dtCore::HotSpotAttachment> newAttachment = new dtCore::HotSpotAttachment(hsd);
      if (!i->second.empty())
      {
         std::string pathToLoad = i->second;
         if (pathToLoad.find(dtCore::ResourceDescriptor::DESCRIPTOR_SEPARATOR) != std::string::npos)
         {
            try
            {
               pathToLoad = dtCore::Project::GetInstance().GetResourcePath(dtCore::ResourceDescriptor(pathToLoad));
            }
            catch (const dtCore::ProjectFileNotFoundException& pfe)
            {
               LOG_ERROR(std::string("Error loading attached resource for \"") + modelData.GetFilename() + "\" character file: " + i->second  + " : " + pfe.ToString());
            }
         }
         dtCore::RefPtr<dtCore::Object> obj = new dtCore::Object(hsd.mName);
         obj->LoadFile(pathToLoad, true);
         newAttachment->AddChild(obj);
      }
      mAttachmentController->AddAttachment(*newAttachment, i->first);
   }
}

/////////////////////////////////////////////////////////////////////////////////
DT_IMPLEMENT_ACCESSOR(AnimationHelper, bool, LoadModelAsynchronously);
/////////////////////////////////////////////////////////////////////////////////
DT_IMPLEMENT_ACCESSOR(AnimationHelper, bool, EnableAttachingNodeToDrawable);
/////////////////////////////////////////////////////////////////////////////////
DT_IMPLEMENT_ACCESSOR_GETTER(AnimationHelper, dtCore::ResourceDescriptor, SkeletalMesh);
/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::SetSkeletalMesh(const dtCore::ResourceDescriptor& rd)
{
   if (rd != mSkeletalMesh)
   {
      mSkeletalMesh = rd;

      dtCore::Project& proj = dtCore::Project::GetInstance();
      if (!rd.IsEmpty() && (GetIsInGM() || proj.GetEditMode()))
      {
         LoadSkeletalMesh();
      }
      else
      {
         LoadModel("");
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::LoadSkeletalMesh()
{
   try
   {
      dtCore::Project& proj = dtCore::Project::GetInstance();
      std::string path = proj.GetResourcePath(mSkeletalMesh);
      if (mLoadModelAsynchronously && !proj.GetEditMode())
      {
         LoadModelAsynchronously(path);
      }
      else
      {
         LoadModel(path, false);
      }
   }
   catch (const dtUtil::Exception& ex)
   {
      ex.LogException(dtUtil::Log::LOG_ERROR);
      // don't throw, it could break the code setting the property.
   }
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::UnloadModel()
{
   mAnimator = NULL;
   mSequenceMixer->ClearRegisteredAnimations();
   mAttachmentController->Clear();
   ModelUnloadedSignal(this);
   // Set node to null after so that it can be accessed in the callback.
   mNode = NULL;
}

/////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::LoadModel(const std::string& pFilename, bool immediate)
{
   if (!pFilename.empty())
   {
      if (mNode.valid())
      {
         UnloadModel();
      }
      Cal3DDatabase& database = Cal3DDatabase::GetInstance();
      dtCore::RefPtr<Cal3DModelWrapper> newModel = database.Load(pFilename);

      if (newModel.valid())
      {
         mAnimator = new Cal3DAnimator(newModel.get());
         mNode = database.GetNodeBuilder().CreateNode(newModel.get(), immediate);

         const Cal3DModelData*  modelData = database.GetModelData(*newModel);
         if (modelData == NULL)
         {
            LOG_ERROR("ModelData not found for Character XML '" + pFilename + "'");
            return false;
         }

         RegisterAnimations(*modelData);
         CreateAttachments(*modelData);

         SetupPoses(*modelData);

         // Notify observers that the model has been loaded
         ModelLoadedSignal(this);
      }
      else
      {
         LOG_ERROR("Unable to load skeletal resource: " + pFilename);
         return false;
      }
   }
   else
   {
      UnloadModel();
  }

   return true;
}

/////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::LoadModelAsynchronously(const std::string& pFilename)
{
   if (!pFilename.empty())
   {
      if (mNode.valid())
      {
         UnloadModel();
      }

      Cal3DDatabase& database = Cal3DDatabase::GetInstance();

      database.LoadAsynchronously(pFilename);

      // Store the filename so that we can poll for load completion
      mAsyncFile = pFilename;

   }
   else
   {
      UnloadModel();
   }

   return true;
}

/////////////////////////////////////////////////////////////////////////////
void AnimationHelper::AttachNodeToDrawable(osg::Group* parent)
{
   osg::Node* node = GetNode();
   if (node != NULL)
   {
      if (parent == NULL)
      {
         dtGame::GameActorProxy* gap;
         GetOwner(gap);
         if (gap != NULL)
         {
            // This really should make a drawable and add it.
            gap->GetDrawable()->GetOSGNode()->asGroup()->addChild(node);
         }
      }
      else
      {
         parent->addChild(node);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
void AnimationHelper::DetachNodeFromDrawable()
{
   osg::Node* node = GetNode();
   if (node != NULL)
   {
      for (unsigned i = 0; i < node->getNumParents(); ++i)
      {
         // This really should make a drawable and add it.
         node->getParent(i)->removeChild(node);
      }
   }
}


/////////////////////////////////////////////////////////////////////////////
void AnimationHelper::OnLoadCompleted(AnimationHelper*)
{
   if (mEnableAttachingNodeToDrawable)
   {
      AttachNodeToDrawable();
   }
}

/////////////////////////////////////////////////////////////////////////////
void AnimationHelper::OnUnloadCompleted(AnimationHelper*)
{
   if (mEnableAttachingNodeToDrawable)
   {
      DetachNodeFromDrawable();
   }
}


/////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::IsLoadingAsynchronously() const
{
   return !mAsyncFile.empty();
}


/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::BuildPropertyMap()
{
   static const std::string ANIMATION_MODEL_GROUP("AnimationModel");

   static const std::string PROPERTY_SKELETAL_MESH_DESC
      ("The model resource that defines the skeletal mesh");
   AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::SKELETAL_MESH,
      PROPERTY_SKELETAL_MESH, PROPERTY_SKELETAL_MESH,
      dtCore::ResourceActorProperty::SetDescFuncType(this, &AnimationHelper::SetSkeletalMesh),
      dtCore::ResourceActorProperty::GetDescFuncType(this, &AnimationHelper::GetSkeletalMesh),
      PROPERTY_SKELETAL_MESH_DESC, ANIMATION_MODEL_GROUP));
}

/////////////////////////////////////////////////////////////////////////////////
osg::Node* AnimationHelper::GetNode()
{
   return mNode.get();
}

/////////////////////////////////////////////////////////////////////////////////
const osg::Node* AnimationHelper::GetNode() const
{
   return mNode.get();
}

/////////////////////////////////////////////////////////////////////////////////
Cal3DAnimator* AnimationHelper::GetAnimator()
{
   return mAnimator.get();
}

/////////////////////////////////////////////////////////////////////////////////
const Cal3DModelWrapper* AnimationHelper::GetModelWrapper() const
{
   if (!mAnimator.valid())
   {
      return NULL;
   }
   return mAnimator->GetWrapper();
}

/////////////////////////////////////////////////////////////////////////////////
Cal3DModelWrapper* AnimationHelper::GetModelWrapper()
{
   if (!mAnimator.valid())
   {
      return NULL;
   }
   return mAnimator->GetWrapper();
}

/////////////////////////////////////////////////////////////////////////////////
const Cal3DAnimator* AnimationHelper::GetAnimator() const
{
   return mAnimator.get();
}

/////////////////////////////////////////////////////////////////////////////////
SequenceMixer& AnimationHelper::GetSequenceMixer()
{
   return *mSequenceMixer;
}

/////////////////////////////////////////////////////////////////////////////////
const SequenceMixer& AnimationHelper::GetSequenceMixer() const
{
   return *mSequenceMixer;
}

/////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::GetGroundClamp() const
{
   return mGroundClamp;
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::SetGroundClamp(bool b)
{
   mGroundClamp = b;
}

/////////////////////////////////////////////////////////////////////////////////
AttachmentController& AnimationHelper::GetAttachmentController()
{
   return *mAttachmentController;
}

/////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::SetAttachmentController(AttachmentController& newController)
{
   mAttachmentController = &newController;
}

////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::RegisterAnimations(const Cal3DModelData& sourceData)
{
   const Cal3DModelData::AnimatableArray& animatables = sourceData.GetAnimatables();

   Cal3DModelData::AnimatableArray::const_iterator iter = animatables.begin();
   Cal3DModelData::AnimatableArray::const_iterator end = animatables.end();

   for (;iter != end; ++iter)
   {
      mSequenceMixer->RegisterAnimation((*iter).get());
   }
}

////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::SetCommandCallbacksEnabled(bool enable)
{
   mEnableCommands = enable;

   // Clear any commands that may have been collected if
   // commands are being disabled. This will prevent them
   // from being executed on the next call to ExecuteCommands.
   if (!enable && !mCommands.empty())
   {
      mCommands.clear();
   }
}

////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::IsCommandCallbacksEnabled() const
{
   return mEnableCommands;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::SetSendEventCallback(AnimEventCallback callback)
{
   // The event callback is about to change.
   // Go through all existing registered commands and remove
   // those set to the initially set Send Event Callback.
   ClearAnimationEventCallbacks();

   mSendEventCallback = callback;

   // Go through all registered animations an create appropriate event
   // callbacks for animations that have event names associated.
   return CreateAnimationEventCallbacks();
}

////////////////////////////////////////////////////////////////////////////////
AnimEventCallback AnimationHelper::GetSendEventCallback() const
{
   return mSendEventCallback;
}

////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::RegisterCommandCallback(const std::string& animName,
   float timeOffset, AnimCommandCallback& command)
{
   return RegisterCommandCallback_Internal(animName, timeOffset, command) != NULL;
}

////////////////////////////////////////////////////////////////////////////////
AnimationHelper::TimeOffsetCommand* AnimationHelper::RegisterCommandCallback_Internal(
   const std::string& animName, float timeOffset, AnimCommandCallback& command)
{
   dtCore::RefPtr<TimeOffsetCommand> mCmdEntry;

   // Determine if a command has already been registered.
   bool cmdExists = false;
   AnimCommandArray commands;
   if (GetCommandCallbacks(animName, timeOffset, commands) > 0)
   {
      AnimCommandArray::iterator curIter = commands.begin();
      AnimCommandArray::iterator endIter = commands.end();
      for (; curIter != endIter; ++curIter)
      {
         if(&command == curIter->get())
         {
            cmdExists = true;
            break;
         }
      }
   }

   // Add the callback only if it is not currently registered for the current time.
   if (!cmdExists)
   {
      if(timeOffset < 0.0f)
      {
         timeOffset = GetAnimationDuration(animName);
      }

      mCmdEntry = new TimeOffsetCommand(timeOffset, command);

      // If insertion fails...
      if (mCommandMap.insert(std::make_pair(animName, mCmdEntry.get())) == mCommandMap.end())
      {
         // ...ensure no reference is returned or kept.
         mCmdEntry = NULL;
      }
   }

   return mCmdEntry.get();
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::UnregisterCommandCallback(const std::string& animName,
   float timeOffset, AnimCommandCallback* command)
{
   unsigned count  = 0;

   TimeOffsetCommand* curStruct = NULL;
   CommandMap::iterator curIter = mCommandMap.lower_bound(animName);
   CommandMap::iterator endIter = mCommandMap.upper_bound(animName);
   for (; curIter != endIter; ++curIter)
   {
      curStruct = curIter->second.get();
      if (timeOffset == curStruct->mOffset)
      {
         // Unregister if it is a specific command has been specified.
         if(command == NULL || command == curStruct->mCommand.get())
         {
            // If a command was specified, make sure it is not still
            // in the queue, if it collected.
            if(command != NULL)
            {
               RemoveCommandFromQueue(*command);
            }

            CommandMap::iterator tmpIter = curIter;
            mCommandMap.erase(tmpIter);
            ++count;

            // The end may have changed, so update the end variable.
            curIter = mCommandMap.lower_bound(animName);
            endIter = mCommandMap.upper_bound(animName);
         }
      }
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::UnregisterCommandCallbacks(const std::string& animName)
{
   size_t count = mCommandMap.size();

   // Ensure any of the commands that may have been queued are removed
   // from the queue so that they are not accidentally executed.
   CommandMap::iterator frontIter = mCommandMap.lower_bound(animName);
   CommandMap::iterator curIter = frontIter;
   CommandMap::iterator endIter = mCommandMap.upper_bound(animName);
   for (; curIter != endIter; ++curIter)
   {
      RemoveCommandFromQueue(*curIter->second->mCommand);
   }

   // Remove the section for the specified animation.
   mCommandMap.erase(frontIter, endIter);

   // Return how many were removed.
   return count - mCommandMap.size();
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::CollectCommandCallbacks(Animatable& anim,
   double startTime, double endTime, AnimCommandArray& outArray)
{
   unsigned count = 0;

   // Only operate on the animation if is active or is going to be active.
   // NOTE: The going-to-be-active check assumes this method is called before
   // the animation actually has a chance to Update and mark itself active.
   double animStartTime = anim.GetInsertTime() + double(anim.GetStartTime());
   if (anim.IsActive() || (animStartTime < endTime && animStartTime >= startTime))
   {
      double animSpeed = anim.GetSpeed();
      if (animSpeed == 0.0)
      {
         animSpeed = 1.0;
      }

      // Adjust the start and end times if this is a looping animation channel.
      double diff = endTime - startTime;
      startTime = double(anim.ConvertToRelativeTimeInAnimationScope(startTime));
      endTime = startTime + diff;

      // Ensure floating point error does not cause math to fall short of
      // the end time. Snap to the end time if the pre-calculated end time is very close
      if (anim.GetEndTime() > 0.0f)
      {
         float diffToEnd = anim.GetEndTime() - anim.GetElapsedTime();
         if (dtUtil::Abs(diffToEnd - diff) < 0.01)
         {
            // Nudge the end time to have the time range encompass
            // any end event time trigger. It should not matter how
            // much passed the end the time range is.
            endTime += 1.0; // One second is a good nudge for now :)
         }
      }

      // Declare variables for the subsequent loop.
      double offset = 0.0f;
      AnimCommandCallback* curCallback = NULL;
      AnimReferenceCommandCallback* specialCallback = NULL;
      TimeOffsetCommand* curStruct = NULL;
      CommandMap::iterator curIter = mCommandMap.lower_bound(anim.GetName());
      CommandMap::iterator endIter = mCommandMap.upper_bound(anim.GetName());

      for (; curIter != endIter; ++curIter)
      {
         curStruct = curIter->second.get();
         offset = double(curStruct->mOffset) / animSpeed;

         // Start time should only be inclusive if the range is at or before
         // the offset 0.0, which means this if the first call to collect commands.
         // Otherwise, check with in the range of time exclusive of the start but
         // inclusive of the end.
         if ((startTime <= 0.0 && offset == 0.0)
            || (offset > startTime && offset <= endTime))
         {
            // Determine if the callback to be triggered requires
            // a reference to the animatable triggering it.
            curCallback = curStruct->mCommand.get();
            specialCallback = dynamic_cast<AnimReferenceCommandCallback*>(curCallback);
            if (specialCallback != NULL && specialCallback->GetArg1() == NULL)
            {
               // Set the animatable to be passed to the command's callback.
               specialCallback->SetArg1(&anim);

               // Ensure it has a reference to the affected animatable, in case the
               // animatable may go out of scope (such as the end of an animation).
               curStruct->mAnim = &anim;
            }

            // Finally, add it to the list.
            outArray.push_back(curCallback);
            ++count;
         }
      } // End loop
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::GetCommandCallbacks(const std::string& animName,
   float timeOffset, AnimCommandArray& outArray)
{
   return GetCommandCallbacks(animName, timeOffset, timeOffset, outArray);
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::GetCommandCallbacks(const std::string& animName,
   float startTime, float endTime, AnimCommandArray& outArray)
{
   unsigned count = 0;

   float offset = 0.0f;
   const TimeOffsetCommand* curStruct = NULL;
   CommandMap::const_iterator curIter = mCommandMap.lower_bound(animName);
   CommandMap::const_iterator endIter = mCommandMap.upper_bound(animName);
   for (; curIter != endIter; ++curIter)
   {
      curStruct = curIter->second.get();
      offset = curStruct->mOffset;

      if (offset >= startTime && offset <= endTime)
      {
         outArray.push_back(curStruct->mCommand.get());
         ++count;
      }
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::CollectCommands(double startTime, double endTime)
{
   unsigned collectCount = 0;

   // Prepare for this frame's command collection.
   mCommands.clear();

   if(mEnableCommands)
   {
      // Acquire all animations that are currently updating.
      const AnimationSequence::AnimationContainer& animArray
         = mSequenceMixer->GetRootSequence().GetChildAnimations();

      // Declare variables for the subsequent loop.
      Animatable* anim = NULL;
      AnimationSequence::AnimationContainer::const_iterator curIter = animArray.begin();
      AnimationSequence::AnimationContainer::const_iterator endIter = animArray.end();

      // Collect commands that should trigger for the specified time range.
      for (; curIter != endIter; ++curIter)
      {
         // Get the commands for the current animatable.
         anim = curIter->get();
         collectCount += CollectCommandCallbacks(*anim, startTime, endTime, mCommands);
      }
   }

   // Inform how many commands were collected.
   return collectCount;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::ExecuteCommands()
{
   unsigned commandCount = mCommands.size();

   AnimCommandArray::iterator curIter = mCommands.begin();
   AnimCommandArray::iterator endIter = mCommands.end();
   for (; curIter != endIter; ++curIter)
   {
      curIter->get()->operator()();
   }

   // All queued up commands have been executed.
   // Now clear the queue for the next frame.
   mCommands.clear();

   return commandCount;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::GetCollectedCommandCount() const
{
   return unsigned(mCommands.size());
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::ClearAnimationEventCallbacks()
{
   unsigned count = 0;

   // Remove only command callbacks that were automatically created
   // by this object.
   TimeOffsetCommand* curStruct = NULL;
   CommandMap::iterator curIter = mCommandMap.begin();
   for (; curIter != mCommandMap.end();)
   {
      curStruct = curIter->second.get();

      if(curStruct->mIsEventCallback)
      {
         RemoveCommandFromQueue(*(curStruct->mCommand.get()));

         CommandMap::iterator tmp = curIter;
         ++curIter;
         mCommandMap.erase(tmp);
         ++count;
      }
      else
      {
         ++curIter;
      }
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::CreateAnimationEventCallbacks()
{
   // Keep a grand total of all event commands created.
   unsigned count = 0;

   // Get all the prototype animatables that were loaded.
   typedef std::vector<const Animatable*> ConstAnimArray;
   ConstAnimArray animArray;
   mSequenceMixer->GetRegisteredAnimations(animArray);

   // Create event command callbacks for all animations
   // that have event names specified.
   ConstAnimArray::const_iterator curIter = animArray.begin();
   ConstAnimArray::const_iterator endIter = animArray.end();
   for (; curIter != endIter; ++curIter)
   {
      count += CreateAnimationEventCallbacks(*(*curIter));
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::CreateAnimationEventCallbacks(const Animatable& anim)
{
   // Keep track of how many event commands are created.
   unsigned count = 0;

   // Only register event commands if a valid event callback has been set.
   if (mSendEventCallback.valid())
   {
      typedef dtUtil::Command1<void, const std::string&> EventCommand;

      // For all the registered events on the specified animatable...
      float curOffset = 0.0f; // for readability
      std::string curEvent; // for readability
      const Animatable::TimeEventMap& teMap = anim.GetTimeEventMap();
      Animatable::TimeEventMap::const_iterator curIter = teMap.begin();
      Animatable::TimeEventMap::const_iterator endIter = teMap.end();
      for (; curIter != endIter; ++curIter)
      {
         // (capture current variables for readability sake)
         curEvent = curIter->first;
         curOffset = curIter->second;

         // ...create a command to fire its event...
         dtCore::RefPtr<EventCommand> cmd
            = new EventCommand(mSendEventCallback, curEvent);

         // ...and register it under its name with a specific time offset.
         TimeOffsetCommand* curStruct
            = RegisterCommandCallback_Internal(anim.GetName(), curOffset, *cmd);
         if (curStruct != NULL)
         {
            // Flag the new struct to indicate that it holds
            // a reference to the currently set Send Event Callback.
            curStruct->mIsEventCallback = true;
            ++count;
         }
      }
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////
// Helper Predicate
struct RemoveCommandPred
{
   RemoveCommandPred(AnimCommandCallback& commandToRemove)
      : mPtrToRemove(&commandToRemove)
   {
   }

   bool operator() (dtCore::RefPtr<AnimCommandCallback>& value)
   {
      return value.get() == mPtrToRemove;
   }

   AnimCommandCallback* mPtrToRemove;
};

////////////////////////////////////////////////////////////////////////////////
unsigned AnimationHelper::RemoveCommandFromQueue(AnimCommandCallback& commandToRemove)
{
   size_t count = mCommands.size();

   RemoveCommandPred pred(commandToRemove);
   AnimCommandArray::iterator eraseBeginIter
      = std::remove_if(mCommands.begin(), mCommands.end(), pred);

   if(eraseBeginIter != mCommands.end())
   {
      mCommands.erase(eraseBeginIter, mCommands.end());
   }

   return unsigned(count - mCommands.size());
}

////////////////////////////////////////////////////////////////////////////////
float AnimationHelper::GetAnimationDuration(const std::string& animName) const
{
   float duration = 0.0f;

   const Animatable* anim = mSequenceMixer->GetRegisteredAnimation(animName);
   if (anim != NULL)
   {
      duration = anim->CalculateDuration();
   }

   return duration;
}

////////////////////////////////////////////////////////////////////////////////
PoseController* AnimationHelper::GetPoseController()
{
   PoseController* controller = NULL;

   if ( ! mPoseSequence.valid())
   {
      Cal3DModelWrapper* model = GetModelWrapper();
      if (model != NULL && NULL != Cal3DDatabase::GetInstance().GetPoseMeshDatabase(*model))
      {
         mPoseSequence = new PoseSequence;
         mPoseSequence->SetName("DefaultPoseSequence");
         mSequenceMixer->RegisterAnimation(mPoseSequence.get());
      }
   }

   if (mPoseSequence.valid())
   {
      controller = mPoseSequence->GetPoseController();
   }

   return controller;
}

////////////////////////////////////////////////////////////////////////////////
void AnimationHelper::SetPosesEnabled(bool enabled)
{
   if (IsPosesEnabled() != enabled)
   {
      if (mPoseSequence.valid())
      {
         if (enabled)
         {
            mPoseSequence = static_cast<PoseSequence*>(mPoseSequence->Clone(GetModelWrapper()).get());
            mSequenceMixer->PlayAnimation(mPoseSequence);
         }
         else
         {
            float blendTime = mPoseSequence->GetPoseController()->GetBlendTime();
            mSequenceMixer->ClearAnimation(mPoseSequence->GetName(), blendTime);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::IsPosesEnabled() const
{
   return mPoseSequence.valid() && mSequenceMixer->IsAnimationPlaying(mPoseSequence->GetName());
}

////////////////////////////////////////////////////////////////////////////////
bool AnimationHelper::SetupPoses(const dtAnim::Cal3DModelData& modelData)
{
   // Characters may not have pose meshes defined.
   // Avoid extra work and error reports if this is the case.
   if (modelData.GetPoseMeshFilename().empty())
   {
      return false;
   }

   bool success = false;

   Cal3DDatabase& database = Cal3DDatabase::GetInstance();

   dtCore::TransformableActorProxy* actor = NULL;
   GetOwner(actor);

   dtCore::Transformable* drawable = NULL;
   if (actor != NULL)
   {
      drawable = actor->GetDrawable()->AsTransformable();
   }

   Cal3DModelWrapper* model = GetModelWrapper();
   if (drawable != NULL)
   {
      PoseMeshDatabase* poseDatabase = database.GetPoseMeshDatabase(*model);

      if (poseDatabase == NULL)
      {
         std::string modelName;
         if (model->GetCalModel() != NULL)
         {
            modelName = model->GetCalModel()->getCoreModel()->getName();
         }

         LOG_ERROR("Cannot setup PoseController for model \"" + modelName 
            + "\" because its PoseMeshDatabase is not available.");
      }
      else
      {
         PoseController* poseController = GetPoseController();
         
         poseController->SetPoseMeshDatabase(poseDatabase);
         poseController->SetPoseDrawable(drawable);
         poseController->SetModelWrapper(model);

         // Attempt setting some defaults. This may have to be removed later.
         /*poseController->AddPoseControl("Poses_LeftEye", 0);
         poseController->AddPoseControl("Poses_RightEye", 0);
         poseController->AddPoseControl("Poses_Head", 1, true);
         poseController->AddPoseControl("Poses_Torso", 2);*/

         SetPosesEnabled(true);

         success = true;
      }
   }

   return success;
}


} // namespace dtAnim
