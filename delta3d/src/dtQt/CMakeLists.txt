SET(LIB_NAME     dtQt)


SET(HEADER_PATH ${DELTA3D_SOURCE_DIR}/inc/${LIB_NAME})
SET(SOURCE_PATH ${DELTA3D_SOURCE_DIR}/src/${LIB_NAME})
file(GLOB LIB_PUBLIC_HEADERS "${HEADER_PATH}/*.h")

# These files are listed in alphabetical order.
set(LIB_SOURCES 

baselibrarylisteditor.cpp
basepropertyeditor.cpp
deltastepper.cpp
docbrowser.cpp
docbrowserxmlreader.cpp
dialoglistselection.cpp
dynamicabstractcontrol.cpp
dynamicabstractparentcontrol.cpp
dynamicactorcontrol.cpp
dynamicarraycontrol.cpp
dynamicbitmaskcontrol.cpp
dynamicboolcontrol.cpp
dynamicbyteelementcontrol.cpp
dynamiccolorrgbacontrol.cpp
dynamiccolorelementcontrol.cpp
dynamiccontainercontrol.cpp
dynamiccontainerselectorcontrol.cpp
dynamicdoublecontrol.cpp
dynamicenumcontrol.cpp
dynamicfloatcontrol.cpp
dynamicgameeventcontrol.cpp
dynamicgroupcontrol.cpp
dynamicintcontrol.cpp
dynamiclabelcontrol.cpp
dynamiclongcontrol.cpp
dynamicpropertycontainercontrol.cpp
dynamicresourcecontrol.cpp
dynamicstringcontrol.cpp
dynamicvectorelementcontrol.cpp
librarypathseditor.cpp
nodetreepanel.cpp
osgadapterwidget.cpp
osggraphicswindowqt.cpp
projectcontextdialog.cpp
propertyeditordelegate.cpp
propertyeditormodel.cpp
propertyeditortreeview.cpp
resourceselectorwidget.cpp
qtguiwindowsystemwrapper.cpp
viewwindow.cpp

)

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR} # for ui*.h files
                   )
				   
qt4_wrap_ui(UI_SOURCES nodetreepanel.ui)
QT4_WRAP_CPP(MOC_SOURCES ${LIB_PUBLIC_HEADERS})

QT4_ADD_RESOURCES(RCC_SOURCES
                  dtqt.qrc
)

ADD_PRECOMPILED_HEADER(${LIB_NAME} prefix/dtqtprefix.h prefix/dtqtprefix.cpp LIB_SOURCES)

SET(LIB_SOURCES 
    ${LIB_SOURCES}
    ${MOC_SOURCES}
    ${RCC_SOURCES}
    ${UI_SOURCES}
    )


set(LIB_EXTERNAL_DEPS
  QT_QTCORE_LIBRARY
  QT_QTGUI_LIBRARY
  QT_QTOPENGL_LIBRARY
  QT_QTWEBKIT_LIBRARY
 )

set(LIB_DEPS
  ${DTCORE_LIBRARY}
)

DELTA3D_ADD_LIBRARY(${LIB_NAME} DT_QT_LIBRARY)

if (X11_FOUND)
   TARGET_LINK_LIBRARIES( ${LIB_NAME}
               ${X11_LIBRARIES})
endif (X11_FOUND)





