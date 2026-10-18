# Builds the configuration that was not built, so both plugin DLLs are always up to date.
# Expects BUILT_CONFIGURATION, BINARY_DIR and TARGET_NAME to be passed with -D.

# Nested build will run this script again, it should not start another build.
if(DEFINED ENV{FE_PLUGIN_NESTED_BUILD})
    return()
endif()

if(BUILT_CONFIGURATION STREQUAL "Debug")
    set(OTHER_CONFIGURATION Release)
else()
    set(OTHER_CONFIGURATION Debug)
endif()

message("${BUILT_CONFIGURATION} build was conducted, building ${OTHER_CONFIGURATION}.")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env FE_PLUGIN_NESTED_BUILD=1 ${CMAKE_COMMAND} --build "${BINARY_DIR}" --target ${TARGET_NAME} --config ${OTHER_CONFIGURATION}
    RESULT_VARIABLE BuildResult
)

if(BuildResult)
    message(FATAL_ERROR "${OTHER_CONFIGURATION} build failed with error ${BuildResult}")
endif()