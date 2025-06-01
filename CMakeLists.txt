cmake_minimum_required(VERSION 3.8)

project(btk2gpx VERSION 1.0 DESCRIPTION "Converts BTK files from the Bushnell Backtrack D-Tour into GPX files which can be imported into Garmin Connect.")

add_executable (btk2gpx "btk2gpx.c")

if (WIN32)
	target_link_libraries(btk2gpx PRIVATE kernel32)
endif()

install(TARGETS btk2gpx RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DEB")

set(CPACK_DEBIAN_PACKAGE_RELEASE "1")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")

set(CPACK_PACKAGE_NAME "btk2gpx")
set(CPACK_PACKAGE_CONTACT "Rob Dixon <rob@kmr.uk.com>")
set(CPACK_STRIP_FILES "btk2gpx")

include(CPack)
