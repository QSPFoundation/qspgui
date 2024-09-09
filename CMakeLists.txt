cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(qspgui VERSION 5.9.0)

if(WIN32)
	if(NOT MSVC OR MSVC_TOOLSET_VERSION LESS 110)
		message(FATAL_ERROR "Only Visual Studio 2012 and later is supported for Windows targets")
	endif()
elseif(APPLE OR NOT UNIX)
	message(FATAL_ERROR "Only Windows and Linux targets are supported")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(CMAKE_INCLUDE_CURRENT_DIR True)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCC)
	set(PROJECT_COMPILER_FLAGS "-Wall")
	set(PROJECT_LINKER_FLAGS "-no-pie")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-char-subscripts -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
endif()
if(MSVC)
	set(PROJECT_COMPILER_FLAGS "/W4")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

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

# Add oniguruma directly to our build
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory("${CMAKE_BINARY_DIR}/qsp-src"
				 "${CMAKE_BINARY_DIR}/qsp-build"
)
#add_library(qsp)

install(TARGETS qsp DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime)

install(
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake
	DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(FILES
		${CMAKE_SOURCE_DIR}/cmake/Modules/Findqsp.cmake
	DESTINATION
		"${CMAKE_INSTALL_DATAROOTDIR}/cmake/Modules"
)

if(USE_INSTALLED_WX)
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
	execute_process(COMMAND git apply --ignore-whitespace --reject "${CMAKE_SOURCE_DIR}/build_wx/wxPatch.diff"
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

	set(wxBUILD_MONOLITHIC OFF)
	set(wxBUILD_SAMPLES OFF)
	set(wxBUILD_TESTS OFF)
	set(wxBUILD_DEMOS OFF)
	set(wxBUILD_BENCHMARKS OFF)
	set(wxBUILD_OPTIMISE ON)
	set(wxBUILD_STRIPPED_RELEASE ON)

	if(WIN32)
		set(wxBUILD_SHARED OFF)
		set(wxBUILD_USE_STATIC_RUNTIME ON)
	else()
		set(wxBUILD_SHARED ON)
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

add_library(fmod SHARED IMPORTED)
if(WIN32)
	set(FMOD_DLL "${CMAKE_SOURCE_DIR}/misc/win32/fmodex.dll")
	set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/misc/win32/fmodex_vc.lib")
	set_target_properties(fmod PROPERTIES IMPORTED_LOCATION ${FMOD_DLL} IMPORTED_IMPLIB ${FMOD_LIBRARY})
else()
	set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86;x86_64)")
	if("${TARGET_ARCH}" STREQUAL "x86")
		set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/misc/linux_${TARGET_ARCH}/libfmodex.so")
	else()
		set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/misc/linux_${TARGET_ARCH}/libfmodex64.so")
	endif()
	set_target_properties(fmod PROPERTIES
		IMPORTED_NO_SONAME TRUE
		IMPORTED_LOCATION ${FMOD_LIBRARY})
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
)
if(WIN32)
	list(APPEND QSPGUI_SOURCES qspgui/rsc/res.rc)
	configure_file("${CMAKE_SOURCE_DIR}/misc/common/icons/logo.ico" "${CMAKE_BINARY_DIR}/misc/icons/logo.ico" COPYONLY)
	add_executable(qspgui WIN32 ${QSPGUI_SOURCES})
	target_compile_definitions(qspgui PRIVATE _CRT_SECURE_NO_WARNINGS)
	set(RESOURCES_DIR "${CMAKE_INSTALL_PREFIX}/bin")
	install(FILES "${FMOD_DLL}" TYPE BIN)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT qspgui)
else()
	add_executable(qspgui ${QSPGUI_SOURCES})
	set(RESOURCES_DIR "${CMAKE_INSTALL_PREFIX}/share/qspgui")
	install(FILES "${FMOD_LIBRARY}" TYPE LIB)
endif()
target_compile_definitions(qspgui PRIVATE _UNICODE)
target_include_directories(qspgui PRIVATE qsp/bindings qsp/bindings/default)
target_compile_options(qspgui PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_options(qspgui PRIVATE ${PROJECT_LINKER_FLAGS})
target_link_libraries(qspgui PRIVATE qsp fmod wxbase wxcore wxadv wxaui wxhtml)
install(TARGETS qspgui qsp)
if (NOT USE_INSTALLED_WX)
	install(TARGETS wxbase wxcore wxadv wxaui wxhtml)
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/misc/common/langs" DESTINATION "${RESOURCES_DIR}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/misc/common/sound" DESTINATION "${RESOURCES_DIR}")
