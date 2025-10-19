find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_library(FocalEnginePython MODULE PythonBindings.cpp)

target_include_directories(FocalEnginePython PRIVATE
    ${PROJECT_SOURCE_DIR}/external/pybind11/include
    ${Python3_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(FocalEnginePython PRIVATE
    FocalEngine
    FEBasicApplication
    Python3::Python
)

# Proper Python extension naming
set_target_properties(FocalEnginePython PROPERTIES PREFIX "")
if (WIN32)
    set_target_properties(FocalEnginePython PROPERTIES SUFFIX ".pyd")
endif()

add_custom_command(TARGET FocalEnginePython POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:FocalEnginePython>"
		"$<TARGET_FILE_DIR:FocalEngine>"
	COMMENT "Copying FocalEnginePython.pyd to output directory"
)