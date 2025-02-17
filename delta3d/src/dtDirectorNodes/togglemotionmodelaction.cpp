/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009 MOVES Institute
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

#include <prefix/dtdirectornodesprefix.h>
#include <dtDirectorNodes/togglemotionmodelaction.h>

#include <dtCore/motionmodel.h>

#include <dtCore/stringactorproperty.h>

#include <dtDirector/director.h>

namespace dtDirector
{
   /////////////////////////////////////////////////////////////////////////////
   ToggleMotionModelAction::ToggleMotionModelAction()
      : ActionNode()
   {
      mModelName = "";

      AddAuthor("Jeff P. Houde");
   }

   /////////////////////////////////////////////////////////////////////////////
   ToggleMotionModelAction::~ToggleMotionModelAction()
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void ToggleMotionModelAction::Init(const NodeType& nodeType, DirectorGraph* graph)
   {
      ActionNode::Init(nodeType, graph);

      // Create multiple inputs for different operations.
      mInputs.clear();
      mInputs.push_back(InputLink(this, "TurnOn", "Turns the motion model on."));
      mInputs.push_back(InputLink(this, "TurnOff", "Turns the motion model off."));
      mInputs.push_back(InputLink(this, "Toggle", "Toggles the motion model."));

      mOutputs.push_back(OutputLink(this, "Failed", "Activates when the motion model could not be found."));
   }

   /////////////////////////////////////////////////////////////////////////////
   void ToggleMotionModelAction::BuildPropertyMap()
   {
      ActionNode::BuildPropertyMap();

      dtCore::StringActorProperty* modelProp = new dtCore::StringActorProperty(
         "Motion Model", "Motion Model",
         dtCore::StringActorProperty::SetFuncType(this, &ToggleMotionModelAction::SetModelName),
         dtCore::StringActorProperty::GetFuncType(this, &ToggleMotionModelAction::GetModelName),
         "The name of the motion model to toggle.");
      AddProperty(modelProp);

      // This will expose the properties in the editor and allow
      // them to be connected to ValueNodes.
      mValues.push_back(ValueLink(this, modelProp, false, true, true, false));
   }

   /////////////////////////////////////////////////////////////////////////////
   bool ToggleMotionModelAction::Update(float simDelta, float delta, int input, bool firstUpdate)
   {
      bool onOrOff = false;
      bool shouldToggle = false;

      int count = GetPropertyCount("Motion Model");

      switch (input)
      {
      case INPUT_TOGGLE:
         {
            // Set to true and continue without break
            shouldToggle = true;
         }
      case INPUT_TURN_ON:
         {
            if (!shouldToggle)
            {
               // Set to true and continue without break
               onOrOff = true;
            }
         }
      case INPUT_TURN_OFF:
         {
            for (int index = 0; index < count; index++)
            {
               std::string modelName = GetString("Motion Model");
               if (!modelName.empty())
               {
                  dtCore::MotionModel* model = dtCore::MotionModel::GetInstance(modelName);
                  if (model)
                  {
                     if (shouldToggle)
                     {
                        onOrOff = !model->IsEnabled();
                     }

                     model->SetEnabled(onOrOff);
                  }
               }
            }
         }
         break;
      }

      return ActionNode::Update(simDelta, delta, input, firstUpdate);
   }

   ////////////////////////////////////////////////////////////////////////////////
   void ToggleMotionModelAction::SetModelName(const std::string& value)
   {
      mModelName = value;
      mName = mModelName;
   }

   ////////////////////////////////////////////////////////////////////////////////
   std::string ToggleMotionModelAction::GetModelName() const
   {
      return mModelName;
   }
}

////////////////////////////////////////////////////////////////////////////////
