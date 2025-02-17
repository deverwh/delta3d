/* 
* Delta3D Open Source Game and Simulation Engine 
* Copyright (C) 2007 MOVES Institute 
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
* Erik Johnson
*/

#ifndef chardrawable_h__
#define chardrawable_h__

#include <dtCore/transformable.h>
#include <dtAnim/export.h>

class CalModel;

/// @cond DOXYGEN_SHOULD_SKIP_THIS
namespace osg
{
   class Node;
}
/// @endcond


namespace dtAnim
{
   class CoreModel;
   class Cal3DAnimator;
   class Cal3DModelWrapper;

   /// A "view" of the cal3d animation state.
   /** Simple class that wraps up a dtAnim::Model so it can be added to the
   * Scene.  
   *
   * Usage:
   * @code
   *   dtCore::RefPtr<dtAnim::CharDrawable> char = new dtAnim::CharDrawable();
   *   dtCore::RefPtr<dtAnim::CoreModel> core = new dtAnim::CoreModel();
   *   core->LoadSkeleton(...);
   *   core->LoadAnimation(...);
   *   char->Create( *core );
   *   myScene->AddChild( *char );
   * @endcode
   */
   class DT_ANIM_EXPORT CharDrawable : public dtCore::Transformable
   {
   public:
      CharDrawable(Cal3DModelWrapper* wrapper);
      ~CharDrawable();

      void OnMessage(dtCore::Base::MessageData* data);

      Cal3DModelWrapper* GetCal3DWrapper();

      /** 
        * Get the Node representing the geometry.
        * @note This Node comes from
        * the AnimNodeBuilder. There is no knowledge of the returned Node's
        * hierarchy in this class.  Use with caution.  GetOSGNode() is the "normal"
        * method for getting a handle to the geometry.
        * @see GetOSGNode()
        */
      osg::Node* GetNode() const { return mNode.get(); }

      /// change the data this class is viewing.
      void SetCal3DWrapper(Cal3DModelWrapper* wrapper);

      /** 
       * Delete and rebuild all the SubMeshDrawables required, based on the CalRenderer.
       * @return : the new Node which contains the newly scaled geometry.
       * @note: This will generate brand new geometry.  The node previously 
       * returned by GetNode() will be invalid after calling this.
       */
      osg::Node* RebuildSubmeshes();

   protected:
      dtCore::RefPtr<Cal3DAnimator> mAnimator;
      dtCore::RefPtr<osg::Node>     mNode;
      int mLastMeshCount;


   private:
      CharDrawable();
   };
}
#endif // chardrawable_h__
