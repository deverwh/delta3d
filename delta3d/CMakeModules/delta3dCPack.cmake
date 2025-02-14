SET(CPACK_BINARY_TBZ2 "ON")
SET(CPACK_PACKAGE_VENDOR "Caper Holdings LLC")
SET(CPACK_BINARY_PACKAGEMAKER "OFF")

SET(CPACK_PACKAGE_VERSION_MAJOR ${delta3d_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${delta3d_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${delta3d_VERSION_PATCH})

if (APPLE)
  SET(CPACK_BINARY_PACKAGEMAKER "OFF")
  SET(CPACK_SYSTEM_NAME "OSX")
endif ()

include(CPack)
