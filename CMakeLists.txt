cmake_minimum_required(VERSION 3.5.0)
if (NOT DEFINED GUI_BASE_DIR)
	if (DEFINED ENV{GUI_BASE_DIR})
		set(GUI_BASE_DIR $ENV{GUI_BASE_DIR})
	else()
		set(GUI_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../plugin-GUI)
	endif()
endif()

get_filename_component(PROJECT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
get_filename_component(PLUGIN_NAME ${PROJECT_FOLDER} NAME)

set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture for Mac OS X" FORCE)

project(OE_PLUGIN_${PLUGIN_NAME})
set(CMAKE_SHARED_LIBRARY_PREFIX "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug)
	endif()
endif()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
	OEPLUGIN
	"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
	$<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	)


set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false "${SOURCE_PATH}/*.cpp" "${SOURCE_PATH}/*.h")
set(GUI_COMMONLIB_DIR ${GUI_BASE_DIR}/installed_libs)

set(CONFIGURATION_FOLDER $<$<CONFIG:Debug>:Debug>$<$<NOT:$<CONFIG:Debug>>:Release>)

list(APPEND CMAKE_PREFIX_PATH ${GUI_COMMONLIB_DIR} ${GUI_COMMONLIB_DIR}/${CONFIGURATION_FOLDER})

if (APPLE)
	add_library(${PLUGIN_NAME} MODULE ${SRC_FILES})
else()
	add_library(${PLUGIN_NAME} SHARED ${SRC_FILES})
endif()

target_compile_features(${PLUGIN_NAME} PUBLIC cxx_auto_type cxx_generalized_initializers cxx_std_17)
target_include_directories(${PLUGIN_NAME} PUBLIC ${GUI_BASE_DIR}/JuceLibraryCode ${GUI_BASE_DIR}/JuceLibraryCode/modules ${GUI_BASE_DIR}/Plugins/Headers ${GUI_COMMONLIB_DIR}/include)

set(GUI_BIN_DIR ${GUI_BASE_DIR}/Build/${CONFIGURATION_FOLDER})

if (NOT CMAKE_LIBRARY_ARCHITECTURE)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "x86")
	endif()
endif()

#Libraries and compiler options
if(MSVC)
	target_link_libraries(${PLUGIN_NAME} ${GUI_BIN_DIR}/open-ephys.lib)
	target_compile_options(${PLUGIN_NAME} PRIVATE /sdl- /W0)
	
	install(TARGETS ${PLUGIN_NAME} RUNTIME DESTINATION ${GUI_BIN_DIR}/plugins  CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
	install(FILES $<TARGET_PDB_FILE:${PLUGIN_NAME}> DESTINATION  ${GUI_BIN_DIR}/plugins OPTIONAL)

	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/windows)
elseif(LINUX)
	target_link_libraries(${PLUGIN_NAME} GL X11 Xext Xinerama asound dl freetype pthread rt)
	set_property(TARGET ${PLUGIN_NAME} APPEND_STRING PROPERTY LINK_FLAGS
		"-fvisibility=hidden -fPIC -rdynamic -Wl,-rpath='$ORIGIN/../shared' -Wl,-rpath='$ORIGIN/../shared-api8'")
	target_compile_options(${PLUGIN_NAME} PRIVATE -fPIC -rdynamic)
	target_compile_options(${PLUGIN_NAME} PRIVATE -O3) #enable optimization for linux debug
	
	install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION ${GUI_BIN_DIR}/plugins)
	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/linux)
elseif(APPLE)
	set_target_properties(${PLUGIN_NAME} PROPERTIES BUNDLE TRUE)
	set_property(TARGET ${PLUGIN_NAME} APPEND_STRING PROPERTY LINK_FLAGS
	"-undefined dynamic_lookup -rpath @loader_path/../../../../shared-api8")

	add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD COMMAND
	install_name_tool -change "/usr/local/lib/libtensorflow.2.dylib" "@rpath/libtensorflow.2.dylib"
	$<TARGET_FILE:${PLUGIN_NAME}>)

	install(TARGETS ${PLUGIN_NAME} DESTINATION $ENV{HOME}/Library/Application\ Support/open-ephys/plugins-api8)
	set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/macos)
endif()

#create filters for vs and xcode

foreach( src_file IN ITEMS ${SRC_FILES})
	get_filename_component(src_path "${src_file}" PATH)
	file(RELATIVE_PATH src_path_rel "${SOURCE_PATH}" "${src_path}")
	string(REPLACE "/" "\\" group_name "${src_path_rel}")
	source_group("${group_name}" FILES "${src_file}")
endforeach()

#copy the dlls when installing
if (MSVC)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/windows/bin/ DESTINATION ${GUI_BIN_DIR}/shared CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
elseif(LINUX)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/linux/bin/ DESTINATION ${GUI_BIN_DIR}/shared)
elseif(APPLE)
	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/macos/bin/ DESTINATION $ENV{HOME}/Library/Application\ Support/open-ephys/shared-api8)
endif()


#additional libraries, if needed
find_library(TF_LIBRARIES NAMES libtensorflow tensorflow)
find_path(TF_INCLUDE_DIRS tensorflow/c/c_api.h)

target_include_directories(${PLUGIN_NAME} PUBLIC ${TF_INCLUDE_DIRS})
target_link_libraries(${PLUGIN_NAME} ${TF_LIBRARIES})
target_compile_definitions(${PLUGIN_NAME} PRIVATE TF $<$<PLATFORM_ID:Windows>:_SCL_SECURE_NO_WARNINGS>)