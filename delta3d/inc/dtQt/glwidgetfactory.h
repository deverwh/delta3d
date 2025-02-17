/* -*-c++-*-
* SimulationCore
* Copyright 2009 MOVES Institute
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
* Erik Johnson
*/

#ifndef glwidgetfactory_h__
#define glwidgetfactory_h__

#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
#include <QtOpenGL/QGLWidget>
#include <osg/Referenced>
DT_DISABLE_WARNING_END

#include <dtUtil/breakoverride.h>

namespace dtQt
{
   class OSGAdapterWidget;

   /** Abstract factory interface used to create specialized OSGAdapterWidget
     * for custom application requirements.
     * @see QtGuiWindowSystemWrapper::SetGLWidgetFactory()
     */
   class GLWidgetFactory : public osg::Referenced
   {
      public:
         GLWidgetFactory()
         {
         }

         ///Override to generate a custom OSGAdapterWidget
         virtual OSGAdapterWidget* CreateWidget(const QGLFormat& /*format*/, bool /*drawOnSeparateThread*/, QWidget* /*parent*/ = NULL,
                                                const QGLWidget* /*shareWidget*/ = NULL, Qt::WindowFlags /*f*/ = NULL)
         {
            return NULL;
         }

         /// This is the old, deprecated version.  Deprecated 4/5/2010.
         /// Overwrite OSGAdapterWidget* CreateWidget(const QGLFormat&,bool,QWidget*,const QGLWidget*,Qt::WindowFlags) instead
         BREAK_OVERRIDE(CreateWidget(bool,  QWidget*,
                                     const QGLWidget*, Qt::WindowFlags))

      protected:
         ~GLWidgetFactory()
         {
         }

   };
}
#endif // glwidgetfactory_h__
