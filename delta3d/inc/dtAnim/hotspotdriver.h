/* 
* Delta3D Open Source Game and Simulation Engine 
* Copyright (C) 2004-2005 MOVES Institute 
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
* John Grant
*/

#ifndef __DTANIM_HOT_SPOT_DRIVER_H__
#define __DTANIM_HOT_SPOT_DRIVER_H__

#include <dtAnim/ical3ddriver.h>
#include <vector>
#include <dtCore/refptr.h>
#include <dtAnim/export.h>

namespace dtCore
{
   class HotSpotAttachment;
}


namespace dtAnim
{

   /// updates the body offset value for HotSpot instances.
   class DT_ANIM_EXPORT HotSpotDriver : public dtAnim::ICal3DDriver
   {
   public:
      typedef std::vector<dtCore::RefPtr<dtCore::HotSpotAttachment> > HotSpotContainer;

      HotSpotDriver(const dtAnim::Cal3DModelWrapper* model);

      void Update(double dt);
      void SetWrapper(dtAnim::Cal3DModelWrapper* model);

      void AddHotSpot(dtCore::HotSpotAttachment* spot);
      void RemoveHotSpot(const dtCore::HotSpotAttachment* spot);
      const HotSpotContainer& GetHotSpots() const;

   protected:
      ~HotSpotDriver();

   private:
      HotSpotDriver();  ///< not implemented by design
      HotSpotDriver(const HotSpotDriver&);  ///< not implemented by design
      HotSpotDriver& operator=(const HotSpotDriver&);  ///< not implemented by design

      const dtAnim::Cal3DModelWrapper* mModel;
      HotSpotContainer mHotSpots;
   };
}

#endif // __DTANIM_HOT_SPOT_DRIVER_H__
