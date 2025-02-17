/* -*-c++-*-
 * Delta3D Simulation Training And Game Editor (STAGE)
 * STAGE - This source file (.h & .cpp) - Using 'The MIT License'
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
 * Curtiss Murphy
 */

#include <prefix/dtqtprefix.h>
#include <dtQt/basepropertyeditor.h>

#include <dtCore/deltadrawable.h>

#include <dtCore/actorproxy.h>
#include <dtCore/datatype.h>

#include <dtCore/librarymanager.h>
#include <dtCore/map.h>

#include <dtQt/dynamicabstractcontrol.h>
#include <dtQt/dynamiccontainercontrol.h>
#include <dtQt/dynamicgroupcontrol.h>
#include <dtQt/dynamiclabelcontrol.h>
//#include <dtEditQt/dynamicnamecontrol.h>
#include <dtQt/propertyeditormodel.h>
#include <dtQt/propertyeditortreeview.h>

#include <dtUtil/log.h>
#include <dtUtil/tree.h>

#include <QtCore/QStringList>

#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QHeaderView>
#include <QtGui/QGroupBox>
#include <QtGui/QTreeView>
#include <QtGui/QAction>
#include <QtGui/QHeaderView>

#include <osg/Referenced>

#include <vector>
#include <cmath>

namespace dtQt
{
   /////////////////////////////////////////////////////////////////////////////
   BasePropertyEditor::BasePropertyEditor(QMainWindow* parent)
      : QDockWidget(parent)
      , mControlFactory(new DynamicControlFactory)
   {
      LOG_INFO("Initializing BasePropertyEditor");
      propertyModel = NULL;
      propertyTree = NULL;
      mRootControl = NULL;

      lastScrollBarLocation = 0; // top
      lastHeaderPosition = 100; // rough guess on a good size.

      mBaseGroupBoxName = tr("Actor Properties");

      setWindowTitle(tr("Property Editor"));

      setupUI();
   }

   /////////////////////////////////////////////////////////////////////////////
   BasePropertyEditor::~BasePropertyEditor()
   {
      delete mRootControl;
      mRootControl = NULL;
   }

   /////////////////////////////////////////////////////////////////////////////
   DynamicControlFactory& BasePropertyEditor::GetDynamicControlFactory()
   {
      return *mControlFactory;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::SetReadOnly(bool readOnly)
   {
      if (propertyModel)
      {
         propertyModel->SetReadOnly(readOnly);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool BasePropertyEditor::IsReadOnly() const
   {
      if (propertyModel)
      {
         return propertyModel->IsReadOnly();
      }

      return false;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::OnContainersSelected(const std::vector<dtCore::RefPtr<dtCore::PropertyContainer> >& selection)
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::setupUI()
   {
      // create the main widget and the scroll and setup our editor docking window.
      mainAreaWidget = new QWidget(this);
      QGridLayout* mainAreaLayout = new QGridLayout(mainAreaWidget);
      setWidget(mainAreaWidget);

      // build the dynamic property area
      actorPropBox = new QGroupBox(mBaseGroupBoxName, mainAreaWidget);
      actorPropBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      mainAreaLayout->addWidget(actorPropBox, 0, 0);
      dynamicControlLayout = new QGridLayout(actorPropBox);

      // create the base model and root properties for our tree
      propertyModel = new PropertyEditorModel(this);
      mRootControl = new DynamicGroupControl("root");

      // Left here.  This is the code you need here if you want to NOT recreate the
      // tree everytime.  See the comment a few methods down.
      //propertyTree = new BasePropertyEditorTreeView(propertyModel, actorPropBox);
      //propertyTree->setMinimumSize(100, 100);
      //propertyTree->setRoot(GetRootControl());
      //dynamicControlLayout->addWidget(propertyTree);

      refreshSelectedActors();
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::HandlePropertyContainersSelected(PropertyContainerRefPtrVector& propCons)
   {
      PropertyContainerRefPtrVector::const_iterator iter;

      // Mark the current expansion of properties so we can restore it later.
      markCurrentExpansion();

      // clear our selected list.
      mSelectedPC.clear();

      // copy passed in actors to our internal list.
      for (iter = propCons.begin(); iter != propCons.end(); ++iter)
      {
         dtCore::RefPtr<dtCore::PropertyContainer> propCon = (*iter);
         mSelectedPC.push_back(propCon);
         LOG_INFO("Selected property containers found a property container");
      }

      // turn off screen updates so that we don't watchf it draw
      setUpdatesEnabled(false);
      mainAreaWidget->setUpdatesEnabled(false);
      actorPropBox->setUpdatesEnabled(false);

      OnContainersSelected(mSelectedPC);
      refreshSelectedActors();

      // turn them back on, so it looks right
      setUpdatesEnabled(true);
      mainAreaWidget->setUpdatesEnabled(true);
      actorPropBox->setUpdatesEnabled(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::refreshSelectedActors()
   {
      CommitCurrentEdits();

      delete propertyTree;
      GetRootControl()->removeAllChildren(propertyModel);
      propertyTree = new PropertyEditorTreeView(propertyModel, actorPropBox);
      dynamicControlLayout->addWidget(propertyTree);
      propertyTree->setRoot(GetRootControl());

      resetGroupBoxLabel();

      buildDynamicControls();

      propertyTree->reset();

      // we deleted the tree, so we have  to reset some sizes
      //actorPropBox->setMinimumSize(actorPropBox->sizeHint());
      //mainAreaWidget->setMinimumSize(mainAreaWidget->sizeHint());
      propertyTree->setMinimumSize(80, 80);
      propertyTree->update();

      // Now, go back and try to re-expand items and restore our scroll position
      restorePreviousExpansion();

      propertyTree->show();
   }

   /////////////////////////////////////////////////////////////////////////////
   QString BasePropertyEditor::GetGroupBoxLabelText(const QString& baseGroupBoxName)
   {
      if (mSelectedPC.empty())
      {
         return baseGroupBoxName;
      }
      else if (mSelectedPC.size() == 1)
      {
         // set the name in the group box.
         dtCore::RefPtr<dtCore::PropertyContainer> selectedProxy = mSelectedPC[0];
         QString label;
         label = baseGroupBoxName + " ('" + tr("Property Container") + "' selected)";
         return label;
      }

      //  put the count of selections
      QString label = baseGroupBoxName + " (" + QString::number(mSelectedPC.size()) + " selected)";
      return label;
   }

   /////////////////////////////////////////////////////////////////////////////
   std::string BasePropertyEditor::GetContainerGroupName(dtCore::PropertyContainer* propertyContainer)
   {
      return "Property Container";
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::resetGroupBoxLabel()
   {
      actorPropBox->setTitle(GetGroupBoxLabelText(mBaseGroupBoxName));
   }

   ////////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::buildDynamicControls()
   {
      int PCCount = (int)mSelectedPC.size();
      if (PCCount == 0)
      {
         return;
      }

      DynamicGroupControl* parent = GetRootControl();

      // Use our first selected item as our base.
      dtCore::RefPtr<dtCore::PropertyContainer> basePC = mSelectedPC[0];
      std::vector<dtCore::ActorProperty*> propList;
      basePC->GetPropertyList(propList);

      // Walk the properties that belong to the base container.
      int propCount = (int)propList.size();
      for (int propIndex = 0; propIndex < propCount; ++propIndex)
      {
         dtCore::ActorProperty* baseProp = propList[propIndex];

         // Test all other selected containers to see if they also
         // contain the same property.  If they do not, we will
         // not show this property in the editor.
         bool propertyMatch = true;

         for (int PCIndex = 1; PCIndex < PCCount; ++PCIndex)
         {
            dtCore::PropertyContainer* propCon = mSelectedPC[PCIndex];
            if (propCon)
            {
               dtCore::ActorProperty* linkedProp = propCon->GetProperty(baseProp->GetName());

               // If this container does not contain the base property, has
               // a different group name, or has a different data type then
               // they do not match, so we should not show this property.
               if (!linkedProp || linkedProp->GetGroupName() != baseProp->GetGroupName() ||
                  linkedProp->GetDataType() != baseProp->GetDataType())
               {
                  propertyMatch = false;
                  break;
               }
            }
         }

         // If this property is a match between all selected containers,
         // show the property within the editor.
         if (propertyMatch)
         {
            try
            {
               // first create the control.  Sometimes the controls aren't creatable, so
               // check that first before we do other work.  Excepts if it fails
               DynamicAbstractControl* newControl = mControlFactory->CreateDynamicControl(*baseProp);
               if (newControl == NULL)
               {
                  LOG_ERROR("Object Factory failed to create a control for property: " + baseProp->GetDataType().GetName());
               }
               else
               {
                  newControl->SetTreeView(propertyTree);
                  newControl->SetDynamicControlFactory(mControlFactory.get());

                  connect(newControl, SIGNAL(PropertyAboutToChange(dtCore::PropertyContainer&, dtCore::ActorProperty&,
                     const std::string&, const std::string&)),
                     this, SLOT(PropertyAboutToChangeFromControl(dtCore::PropertyContainer&, dtCore::ActorProperty&,
                     const std::string&, const std::string&)));

                  connect(newControl, SIGNAL(PropertyChanged(dtCore::PropertyContainer&, dtCore::ActorProperty&)),
                     this, SLOT(PropertyChangedFromControl(dtCore::PropertyContainer&, dtCore::ActorProperty&)));

                  // Now add all other linked properties to this control.
                  for (int PCIndex = 1; PCIndex < PCCount; ++PCIndex)
                  {
                     dtCore::RefPtr<dtCore::PropertyContainer> propCon = mSelectedPC[PCIndex];
                     if (propCon.valid())
                     {
                        dtCore::ActorProperty* linkedProp = propCon->GetProperty(baseProp->GetName());
                        newControl->AddLinkedProperty(propCon, linkedProp);
                     }
                  }

                  // Work with the group.  Requires finding an existing group or creating one,
                  // and eventually adding our new control to that group control
                  const std::string& groupName = baseProp->GetGroupName();
                  if (!groupName.empty())
                  {
                     // find our group
                     DynamicGroupControl* groupControl = parent->getChildGroupControl(QString(groupName.c_str()));

                     // if no group, then create one.
                     if (groupControl == NULL)
                     {
                        groupControl = new DynamicGroupControl(groupName);
                        groupControl->InitializeData(parent, propertyModel, basePC, NULL);
                        parent->addChildControlSorted(groupControl, propertyModel);
                     }

                     // add our new control to the group.
                     newControl->InitializeData(groupControl, propertyModel, basePC, baseProp);
                     groupControl->addChildControl(newControl, propertyModel);
                  }
                  else
                  {
                     // there's no group, so use the root.
                     newControl->InitializeData(parent, propertyModel, basePC, baseProp);
                     parent->addChildControl(newControl, propertyModel);
                  }
               }
            }
            catch (dtUtil::Exception& ex)
            {
               LOG_ERROR("Failed to create a control for property: " + baseProp->GetDataType().GetName() +
                  " with error: " + ex.What());
            }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   DynamicGroupControl* BasePropertyEditor::GetRootControl()
   {
      return mRootControl;
   }

   /////////////////////////////////////////////////////////////////////////////
   PropertyEditorModel& BasePropertyEditor::GetPropertyEditorModel()
   {
      return *propertyModel;
   }

   /////////////////////////////////////////////////////////////////////////////
   PropertyEditorTreeView& BasePropertyEditor::GetPropertyEditorView()
   {
      return *propertyTree;
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::markCurrentExpansion()
   {
      // clear out previous marks
      expandedTreeNames.clear();

      if (propertyTree != NULL && GetRootControl() != NULL)
      {
         // start recursion
         recurseMarkCurrentExpansion(GetRootControl(), expandedTreeNames);

         // also store the last location of the scroll bar... so that they go back
         // to where they were next time.
         lastScrollBarLocation = propertyTree->verticalScrollBar()->value();//sliderPosition();
         lastHeaderPosition = propertyTree->header()->sectionSize(0);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::recurseMarkCurrentExpansion(DynamicAbstractControl* parent,
      dtUtil::tree<QString>& currentTree)
   {
      for (int i = 0; i < parent->getChildCount(); ++i)
      {
         DynamicAbstractControl* child = parent->getChild(i);

         // if we have children, then we could potentially be expanded...
         if (child->getChildCount() > 0)
         {
            QModelIndex index = propertyModel->IndexOf(child);
            if (propertyTree->isExpanded(index))
            {
               // add it to our list
               dtUtil::tree<QString>& insertedItem = currentTree.
                  insert(child->getDisplayName()).tree_ref();

               // recurse on the child with the new tree
               recurseMarkCurrentExpansion(child, insertedItem);
            }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::restorePreviousExpansion()
   {
      recurseRestorePreviousExpansion(GetRootControl(), expandedTreeNames);

      // Put the scroll bar back where it was last time
      propertyTree->verticalScrollBar()->setValue(lastScrollBarLocation);//setSliderPosition(lastScrollBarLocation);
      // reset the little header scroll bar
      propertyTree->header()->resizeSection(0, lastHeaderPosition);
      lastScrollBarLocation = propertyTree->verticalScrollBar()->value();//sliderPosition();
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::recurseRestorePreviousExpansion(DynamicAbstractControl* parent,
      dtUtil::tree<QString>& currentTree)
   {
      // walk through the children...
      for (dtUtil::tree<QString>::const_iterator iter = currentTree.in(); iter != currentTree.end(); ++iter)
      {
         QString name = (*iter);

         // Try to find a control with this name in our propertyModel
         for (int i = 0; i < parent->getChildCount(); ++i)
         {
            DynamicAbstractControl* child = parent->getChild(i);
            // found a match!  expand it
            if (child->getDisplayName() == name)
            {
               QModelIndex childIndex = propertyModel->IndexOf(child);
               propertyTree->setExpanded(childIndex, true);

               // recurse over the children of this object
               recurseRestorePreviousExpansion(child, iter.tree_ref());
            }
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::ActorPropertyChanged(dtCore::PropertyContainer& propCon, dtCore::ActorProperty& property)
   {
      propertyTree->viewport()->update();

      // Update each of the property widgets.
      if (GetRootControl())
      {
         GetRootControl()->OnPropertyChanged(propCon, property);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::ProxyNameChanged(dtCore::BaseActorObject& propCon, std::string oldName)
   {
      UpdateTitle();
   }

   /////////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::PropertyChangedFromControl(dtCore::PropertyContainer& propCon, dtCore::ActorProperty& prop)
   {
      ActorPropertyChanged(propCon, prop);
   }

   /////////////////////////////////////////////////////////////////////////////
   void BasePropertyEditor::UpdateTitle()
   {
      resetGroupBoxLabel();
      propertyTree->viewport()->update();
   }

} // namespace dtQt
