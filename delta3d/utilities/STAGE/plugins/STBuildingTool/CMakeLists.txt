
SET(LIB_NAME     STBuildingTool)

file(GLOB LIB_PUBLIC_HEADERS "*.h")
file(GLOB LIB_C_SOURCES "*.cpp")

INCLUDE_DIRECTORIES(  ${CMAKE_SOURCE_DIR}/utilities/STAGE/inc
            ${QT_INCLUDE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR} # for ui*.h files
)

qt4_wrap_cpp(MOC_SOURCES ${LIB_PUBLIC_HEADERS})

set(LIB_SOURCES
   ${LIB_C_SOURCES}
   ${UI_SOURCES}
   ${RCC_SOURCES}
   ${MOC_SOURCES}
)

SET(LIB_DEPS 
     STAGE
     ${DTUTIL_LIBRARY}
     ${DTCORE_LIBRARY}
     ${DTGAME_LIBRARY}
     ${DTABC_LIBRARY}
     ${DTACTORS_LIBRARY}
     ${QT_QTCORE_LIBRARY}
     ${QT_QTGUI_LIBRARY}
     ${QT_QTOPENGL_LIBRARY}
 )


SETUP_PLUGIN_WITH_OUTPUT_DIRS(${LIB_NAME} DT_BUILDING_TOOL_PLUGIN stplugins)

