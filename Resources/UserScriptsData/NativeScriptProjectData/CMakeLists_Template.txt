cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(PROJECT_NAME PLACE_HOLDER)

project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags based on USE_STATIC_RUNTIME
if(MSVC)
    # Disable C4251 warning
    add_compile_options(/wd4251)

    if(USE_STATIC_RUNTIME)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    endif()

    # Always add /MP for multi-processor compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	# Define that engine should be used as shared library
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D FEBASICAPPLICATION_SHARED /D FOCAL_ENGINE_SHARED")
endif()

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB Connection_To_Engine_SRC
	"SubSystems/FocalEngine/Resources/UserScriptsData/FENativeScriptConnector.cpp"
    "SubSystems/FocalEngine/Resources/UserScriptsData/FENativeScriptConnector.h"
)

file(GLOB Main_SRC PLACE_HOLDER)

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

add_library(${PROJECT_NAME}
	SHARED
		${Main_SRC}
		${Connection_To_Engine_SRC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        FEBasicApplication.lib
        FocalEngine.lib
)

# FocalEngine is headers-only, we need to add its include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${FOCAL_ENGINE_INCLUDES})
target_compile_definitions(${PROJECT_NAME} PRIVATE FOCAL_ENGINE_HEADERS_ONLY)

source_group("Source Files/" FILES ${Main_SRC})
source_group("Source Files/Engine/" FILES ${Connection_To_Engine_SRC})

# set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication/SubSystems
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/glew2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/ThirdParty
    ${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/ThirdParty/entt
)

add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Build type: $<CONFIG>" > $<IF:$<CONFIG:Debug>,${CMAKE_CURRENT_BINARY_DIR}/BuildManagement/Debug_Build_Flag.txt,${CMAKE_CURRENT_BINARY_DIR}/BuildManagement/Release_Build_Flag.txt>
  COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_LIST_DIR}/BuildManagement/EnsureBuildCompletion.cmake"
)