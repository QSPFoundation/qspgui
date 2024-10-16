cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

set(APP_VERSION CACHE STRING "0.0.0")

project(qspgui VERSION ${APP_VERSION} HOMEPAGE_URL "https://qsp.org")

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(PROJECT_COMPILER_FLAGS "-Wall")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-char-subscripts -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	set(PROJECT_COMPILER_FLAGS "/W4")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
	set(PROJECT_COMPILER_FLAGS "")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
else()
	message(FATAL_ERROR "Compiler is not supported")
endif()

# Download and unpack qsp at configure time
configure_file(CMakeLists-qsp.txt.in "${CMAKE_BINARY_DIR}/qsp-download/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/qsp-download"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/qsp-download"
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(qsp_force_shared_crt ON CACHE BOOL "" FORCE)

# Add qsp directly to our build
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory("${CMAKE_BINARY_DIR}/qsp-src"
				 "${CMAKE_BINARY_DIR}/qsp-build"
)

if (USE_INSTALLED_WX)
	find_package(wxWidgets REQUIRED base core adv aui html)
	include(${wxWidgets_USE_FILE})
else()
	# Download and unpack wxWidgets at configure time
	configure_file(CMakeLists-wxWidgets.txt.in "${CMAKE_BINARY_DIR}/wxWidgets-download/CMakeLists.txt")
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-download"
	)
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-download"
	)
	# Patch wxWidgets
	execute_process(COMMAND git checkout --recurse-submodules -- .
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-src"
	)
	execute_process(COMMAND git apply --ignore-whitespace --reject "${CMAKE_CURRENT_SOURCE_DIR}/build_wx/wxPatch.diff"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-src"
	)

	# Disable wxWidgets features we don't need
	set(wxUSE_ACTIVEX OFF)
	set(wxUSE_LIBTIFF OFF)
	set(wxUSE_REGEX OFF)
	set(wxUSE_SECRETSTORE OFF)
	set(wxUSE_LIBSDL OFF)
	set(wxUSE_LIBMSPACK OFF)
	set(wxUSE_GSTREAMER OFF)
	set(wxUSE_OPENGL OFF)
	set(wxUSE_RICHTEXT OFF)
	set(wxUSE_RIBBON OFF)
	set(wxUSE_MEDIACTRL OFF)
	set(wxUSE_PROPGRID OFF)
	set(wxUSE_XRC OFF)
	set(wxUSE_DEBUGREPORT OFF)
	set(wxUSE_SOCKETS OFF)
	set(wxUSE_PROTOCOL OFF)
	set(wxUSE_URL OFF)
	set(wxUSE_FS_INET OFF)
	set(wxUSE_STC OFF)
	set(wxUSE_WEBVIEW OFF)

	set(wxBUILD_MONOLITHIC OFF)
	set(wxBUILD_SAMPLES OFF)
	set(wxBUILD_TESTS OFF)
	set(wxBUILD_DEMOS OFF)
	set(wxBUILD_BENCHMARKS OFF)
	set(wxBUILD_OPTIMISE ON)
	set(wxBUILD_STRIPPED_RELEASE ON)
	set(wxBUILD_SHARED OFF)

	if (MSVC)
		set(wxBUILD_USE_STATIC_RUNTIME ON)
	else()
		set(wxBUILD_USE_STATIC_RUNTIME OFF)
	endif()

	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(wxWidgets_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add wxWidgets directly to our build
	set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
	add_subdirectory("${CMAKE_BINARY_DIR}/wxWidgets-src"
					 "${CMAKE_BINARY_DIR}/wxWidgets-build"
	)
endif()

set(QSPGUI_SOURCES
	qspgui/animwin.cpp
	qspgui/app.cpp
	qspgui/callbacks_gui.cpp
	qspgui/comtools.cpp
	qspgui/frame.cpp
	qspgui/imgcanvas.cpp
	qspgui/initevent.cpp
	qspgui/inputbox.cpp
	qspgui/inputdlg.cpp
	qspgui/listbox.cpp
	qspgui/msgdlg.cpp
	qspgui/textbox.cpp
	qspgui/transhelper.cpp
	qspgui/sound/sound_engine.c
)
if (WIN32)
	list(APPEND QSPGUI_SOURCES qspgui/rsc/res.rc)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/misc/common/icons/logo.ico" "${CMAKE_BINARY_DIR}/misc/icons/logo.ico" COPYONLY)
	add_executable(qspgui WIN32 ${QSPGUI_SOURCES})
	target_compile_definitions(qspgui PRIVATE _CRT_SECURE_NO_WARNINGS)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT qspgui)
elseif (APPLE)
	add_executable(qspgui MACOSX_BUNDLE ${QSPGUI_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/misc/macos/icon.icns")
	set_target_properties(qspgui PROPERTIES
		MACOSX_BUNDLE_BUNDLE_NAME "QSP Classic"
		MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
		MACOSX_BUNDLE_COPYRIGHT "(c) QSP Foundation"
		MACOSX_BUNDLE_GUI_IDENTIFIER "org.qsp.qspgui"
		MACOSX_BUNDLE_ICON_FILE "icon.icns"
		MACOSX_BUNDLE_INFO_STRING "QSP Classic"
		MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
		RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/misc/macos/icon.icns")
elseif (UNIX)
	add_executable(qspgui ${QSPGUI_SOURCES})
else()
	message(FATAL_ERROR "Platform is not supported")
endif()

target_compile_definitions(qspgui PRIVATE _UNICODE)
target_include_directories(qspgui PRIVATE qsp/bindings qsp/bindings/default)
target_compile_options(qspgui PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_options(qspgui PRIVATE ${PROJECT_LINKER_FLAGS})
target_link_libraries(qspgui PRIVATE qsp wxbase wxcore wxadv wxaui wxhtml)

# Installation

if (WIN32)
	set(BUNDLE_NAME qspgui.exe)
	set(BINDIR ${CMAKE_INSTALL_BINDIR})
	set(LIBDIR ${CMAKE_INSTALL_LIBDIR})
	set(RLIBDIR ${CMAKE_INSTALL_BINDIR})
	set(HEADERDIR ${CMAKE_INSTALL_INCLUDEDIR})
	set(DATADIR ${CMAKE_INSTALL_BINDIR})
	set(INSTALL_RPATH "")
elseif (APPLE)
	set(BUNDLE_NAME qspgui.app)
	set(BINDIR ${BUNDLE_NAME}/Contents/MacOS)
	set(LIBDIR ${BUNDLE_NAME}/Contents/Frameworks)
	set(RLIBDIR ${BUNDLE_NAME}/Contents/Frameworks)
	set(HEADERDIR ${BUNDLE_NAME}/Contents/Frameworks)
	set(DATADIR ${BUNDLE_NAME}/Contents/Resources)
	set(INSTALL_RPATH "@loader_path;@loader_path/../../${RLIBDIR}")
elseif (UNIX)
	set(BUNDLE_NAME qspgui)
	set(BINDIR ${CMAKE_INSTALL_BINDIR})
	set(LIBDIR ${CMAKE_INSTALL_LIBDIR})
	set(RLIBDIR ${CMAKE_INSTALL_LIBDIR})
	set(HEADERDIR ${CMAKE_INSTALL_INCLUDEDIR})
	set(DATADIR ${CMAKE_INSTALL_DATADIR}/qspgui)
	set(INSTALL_RPATH "$ORIGIN;$ORIGIN/../${RLIBDIR}")
else()
	message(FATAL_ERROR "Platform is not supported")
endif()

set_target_properties(qspgui qsp PROPERTIES INSTALL_RPATH "${INSTALL_RPATH}")
install(TARGETS qspgui qsp
		COMPONENT Main
		RUNTIME DESTINATION ${BINDIR}
		LIBRARY DESTINATION ${RLIBDIR}
		ARCHIVE DESTINATION ${LIBDIR}
		PRIVATE_HEADER DESTINATION ${HEADERDIR}
		PUBLIC_HEADER DESTINATION ${HEADERDIR}
		BUNDLE DESTINATION .)
if (NOT USE_INSTALLED_WX)
	set_target_properties(wxbase wxcore wxadv wxaui wxhtml PROPERTIES INSTALL_RPATH "${INSTALL_RPATH}")
	install(TARGETS wxbase wxcore wxadv wxaui wxhtml
			COMPONENT Libs
			RUNTIME DESTINATION ${BINDIR}
			LIBRARY DESTINATION ${RLIBDIR}
			ARCHIVE DESTINATION ${LIBDIR}
			PRIVATE_HEADER DESTINATION ${HEADERDIR}
			PUBLIC_HEADER DESTINATION ${HEADERDIR})
endif()

if (APPLE)
	set(DEP_DIRS "${CMAKE_INSTALL_PREFIX}/${RLIBDIR}")
	install(CODE
			"include(BundleUtilities)
			fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/${BUNDLE_NAME}\" \"\" \"${DEP_DIRS}\")"
			COMPONENT Main)
elseif (UNIX)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/linux/qspgui.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications" COMPONENT Main)
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/common/icons/qsp.svg" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps" COMPONENT Main)
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/misc/common/langs" DESTINATION "${DATADIR}" COMPONENT Main)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/misc/common/sound" DESTINATION "${DATADIR}" COMPONENT Main)

# Packaging

if (WIN32)
	set(CPACK_PACKAGE_NAME "QSP Classic")
else()
	set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
endif()
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QSP Classic player")
set(CPACK_PACKAGE_VENDOR "QSP Foundation")
set(CPACK_PACKAGE_CONTACT "byte@qsp.org")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_VERBATIM_VARIABLES ON)
set(CPACK_STRIP_FILES ON)
set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE
)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CPACK_PACKAGE_EXECUTABLES qspgui "QSP Classic")
set(CPACK_CREATE_DESKTOP_LINKS qspgui)
set(CPACK_NSIS_INSTALLED_ICON_NAME "${BINDIR}\\qspgui.exe")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
     WriteRegStr HKCR '.qsp' '' 'QSPGame'
     WriteRegStr HKCR '.gam' '' 'QSPGame'
     WriteRegStr HKCR 'QSPGame' '' 'QSP Game'
     WriteRegStr HKCR 'QSPGame\\shell' '' 'open'
     WriteRegStr HKCR 'QSPGame\\DefaultIcon' \\
                      '' '$INSTDIR\\${BINDIR}\\qspgui.exe,0'
     WriteRegStr HKCR 'QSPGame\\shell\\open\\command' \\
                      '' '$INSTDIR\\${BINDIR}\\qspgui.exe \"%1\"'
     System::Call \\
       'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
     ")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
     DeleteRegKey HKCR '.qsp'
     DeleteRegKey HKCR '.gam'
     DeleteRegKey HKCR 'QSPGame'
     ")

set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL Main)

include(CPack)

cpack_add_component(Main DISPLAY_NAME "QSP Player" REQUIRED)
