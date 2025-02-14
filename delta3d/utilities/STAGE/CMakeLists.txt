ADD_SUBDIRECTORY(plugins/STBuildingTool)

ADD_SUBDIRECTORY(plugins/STDirectorCinematicEditor)
ADD_SUBDIRECTORY(plugins/STDirectorTool)
ADD_SUBDIRECTORY(plugins/STFenceTool)
ADD_SUBDIRECTORY(plugins/STGameStart)
ADD_SUBDIRECTORY(plugins/STLinkedPointsActorTool)
ADD_SUBDIRECTORY(plugins/STMapHierarchyEditor)
ADD_SUBDIRECTORY(plugins/STMapStatistics)
ADD_SUBDIRECTORY(plugins/STPluginDialog)
ADD_SUBDIRECTORY(plugins/STMassImporter)
ADD_SUBDIRECTORY(plugins/STRandomActorGenerator)
ADD_SUBDIRECTORY(plugins/STRTSCamera)

SET(APP_NAME STAGE)

SET(SOURCE_PATH ${CMAKE_SOURCE_DIR}/utilities/${APP_NAME})
file(GLOB PROG_HEADERS "inc/dtEditQt/*.h")
file(GLOB PROG_SOURCES "src/*.cpp" "*.rc")
list(REMOVE_ITEM PROG_SOURCES ${SOURCE_PATH}/src/precomp.cpp)

INCLUDE_DIRECTORIES(${SOURCE_PATH}/inc ${QT_INCLUDE_DIR}
                    ${ALUT_INCLUDE_DIR} #for dtAudio
                    ${OPENAL_INCLUDE_DIR} #for dtAudio
                    ${CAL3D_INCLUDE_DIR} #for dtAnim
                    ${CMAKE_CURRENT_BINARY_DIR} # for ui*.h files
 )

qt4_wrap_cpp(MOC_SOURCES ${PROG_HEADERS})

QT4_ADD_RESOURCES(RCC_SOURCES
                  stage.qrc
                  docs/help/help.qrc
)

QT4_WRAP_UI(UI_SOURCES externaltooldialog.ui externaltoolargeditor.ui positiondialog.ui)

SOURCE_GROUP("Generated Files" REGULAR_EXPRESSION qrc_.*cxx|moc_.*cxx)

ADD_PRECOMPILED_HEADER(${APP_NAME} prefix/stageprefix.h prefix/stageprefix.cpp PROG_SOURCES)

SET(APP_EXTERNAL_DEPS
                    QT_QTCORE_LIBRARY
                    QT_QTGUI_LIBRARY
                    QT_QTOPENGL_LIBRARY
                    OSGFX_LIBRARY
                    AGL_LIBRARY
)

SET(APP_DEPS
                      ${DTUTIL_LIBRARY}
                      ${DTCORE_LIBRARY}
                      ${DTANIM_LIBRARY}
                      ${DTAUDIO_LIBRARY}
                      ${DTABC_LIBRARY}
                      ${DTGAME_LIBRARY}
                      ${DTQT_LIBRARY}
                      ${DTACTORS_LIBRARY}
)
SET(PROG_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/STAGE.plist.in)

DELTA3D_ADD_PROGRAM(${APP_NAME} MACOSX_BUNDLE DT_EDITQT_LIBRARY)

IF (MSVC)
  SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ENDIF (MSVC)

