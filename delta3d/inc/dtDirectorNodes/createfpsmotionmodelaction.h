/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008 MOVES Institute
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
 * Author: Jeff P. Houde
 */

#ifndef CREATE_FPS_MOTION_MODEL_ACTION_NODE
#define CREATE_FPS_MOTION_MODEL_ACTION_NODE

////////////////////////////////////////////////////////////////////////////////

#include <dtDirector/actionnode.h>
#include <dtDirectorNodes/nodelibraryexport.h>
#include <dtCore/refptr.h>
#include <dtCore/collisionmotionmodel.h>
namespace dtDirector
{
   ////////////////////////////////////////////////////////////////////////////////
   class NODE_LIBRARY_EXPORT CreateFPSMotionModelAction: public ActionNode
   {
   public:

      /**
       * Constructor.
       */
      CreateFPSMotionModelAction();

      /**
       * Initializes the Node.
       *
       * @param[in]  nodeType  The node type.
       * @param[in]  graph     The graph that owns this node.
       */
      virtual void Init(const NodeType& nodeType, DirectorGraph* graph);

      /**
       * This method is called in init, which instructs the node
       * to create its properties.  Methods implementing this should
       * be sure to call their parent class's buildPropertyMap method to
       * ensure all properties in the proxy inheritance hierarchy are
       * correctly added to the property map.
       *
       * @see GetDeprecatedProperty to handle old properties that need
       *       to be removed.
       */
      virtual void BuildPropertyMap();

      /**
       * Updates the node.
       * @note  Parent implementation will auto activate any trigger
       *        with the "Out" label by default.
       *
       * @param[in]  simDelta     The simulation time step.
       * @param[in]  delta        The real time step.
       * @param[in]  input        The index to the input that is active.
       * @param[in]  firstUpdate  True if this input was just activated,
       *
       * @return     True if the current node should remain active.
       */
      virtual bool Update(float simDelta, float delta, int input, bool firstUpdate);     

      /**
       * Accessors for property values.
       */
      void SetCurrentActor(const dtCore::UniqueId& value);
      dtCore::UniqueId GetCurrentActor();

      void SetModelName(const std::string& value);
      std::string GetModelName() const;

      void SetHeight(float value);
      float GetHeight() const;

      void SetRadius(float value);
      float GetRadius() const;

      void SetStepUpHeight(float value);
      float GetStepUpHeight() const;

      void SetWalkSpeed(float value);
      float GetWalkSpeed() const;

      void SetSidestepSpeed(float value);
      float GetSidestepSpeed() const;

      void SetTurnSpeed(float value);
      float GetTurnSpeed() const;

      void SetJumpSpeed(float value);
      float GetJumpSpeed() const;

      void SetSlideSpeed(float value);
      float GetSlideSpeed() const;

      void SetSmoothingSpeed(float value);
      float GetSmoothingSpeed() const;

      void SetSlideThreshold(float value);
      float GetSlideThreshold() const;

      void SetUseWASD(bool value);
      bool GetUseWASD() const;

      void SetUseArrows(bool value);
      bool GetUseArrows() const;

      void SetAllowJump(bool value);
      bool GetAllowJump() const;

      void SetTorsoCollisionMask(unsigned int value);
      unsigned int GetTorsoCollisionMask() const;

      void SetFeetCollisionMask(unsigned int value);
      unsigned int GetFeetCollisionMask() const;
      void GetCollisionMaskList(std::vector<std::string>& names, std::vector<unsigned int>& values);

   protected:

      /**
       * Destructor.
       */
      ~CreateFPSMotionModelAction();

   private:

      dtCore::UniqueId mActor;
      std::string mModelName;
      float mHeight;
      float mRadius;
      float mStepUpHeight;
      float mWalkSpeed;
      float mSidestepSpeed;
      float mTurnSpeed;
      float mJumpSpeed;
      float mSlideSpeed;
      float mSlideThreshold;
      float mSmoothingSpeed;
      bool  mUseWASD;
      bool  mUseArrows;
      bool  mAllowJump;
      unsigned int mTorsoMask;
      unsigned int mFeetMask;
      dtCore::RefPtr<dtCore::CollisionMotionModel> mCreatedMotionModel;
   };
}

////////////////////////////////////////////////////////////////////////////////

#endif // CREATE_FPS_MOTION_MODEL_ACTION_NODE
