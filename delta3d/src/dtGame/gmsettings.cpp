/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2010, Alion Science and Technology
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
 * Curtiss Murphy
 */

#include <prefix/dtgameprefix.h>

#include <dtGame/gmsettings.h>

namespace dtGame
{

   ///////////////////////////////////////////////////////////////////////////////
   GMSettings::GMSettings()
      : mServerRole(true)
      , mClientRole(true)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////
   GMSettings::~GMSettings()
   {
   }

   ///////////////////////////////////////////////////////////////////////////////
   bool GMSettings::IsServerRole() const
   {
      return mServerRole;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GMSettings::SetServerRole(bool newValue)
   {
      mServerRole = newValue;
   }

   ///////////////////////////////////////////////////////////////////////////////
   bool GMSettings::IsClientRole() const
   {
      return mClientRole;
   }

   ///////////////////////////////////////////////////////////////////////////////
   void GMSettings::SetClientRole(bool newValue)
   {
      mClientRole = newValue;
   }


} // namespace dtGame
