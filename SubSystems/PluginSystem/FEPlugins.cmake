# Applications include this file and call fe_require_plugin or fe_optional_plugin before they add engine, because engine links plugins that are in its Plugins folder when it is added.
include_guard(GLOBAL)

if(CMAKE_VERSION VERSION_LESS 3.19)
	message(FATAL_ERROR "FEPlugins.cmake needs CMake 3.19 or newer")
endif()

get_filename_component(FE_PLUGINS_ENGINE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(FE_PLUGINS_DIRECTORY "${FE_PLUGINS_ENGINE_DIRECTORY}/Plugins")
find_package(Git QUIET)

# OUTPUT_VERSION is empty if manifest is missing or has no Version.
function(fe_read_plugin_manifest_version MANIFEST_PATH OUTPUT_VERSION)
	set(${OUTPUT_VERSION} "" PARENT_SCOPE)
	if(NOT EXISTS "${MANIFEST_PATH}")
		return()
	endif()

	file(READ "${MANIFEST_PATH}" ManifestText)
	string(JSON ManifestVersion ERROR_VARIABLE JsonError GET "${ManifestText}" Version)
	if(NOT JsonError)
		set(${OUTPUT_VERSION} "${ManifestVersion}" PARENT_SCOPE)
	endif()
endfunction()

# Downloads plugin from GitHub release with tag v<VERSION>, release asset <PluginName>_<VERSION>.zip should contain Build folder of plugin.
# Plugin is downloaded again only when VERSION differs from Version in manifest of installed plugin, SHA256 of archive is checked if it is given.
# With REQUIRED problems stop configure, without it they are warnings. Installed plugin is kept if requested version could not be installed.
# Usage: fe_download_plugin(PluginName [REQUIRED] REPOSITORY Owner/Repository VERSION 1.0.0 [SHA256 Hash])
function(fe_download_plugin PLUGIN_NAME)
	cmake_parse_arguments(PLUGIN "REQUIRED" "REPOSITORY;VERSION;SHA256" "" ${ARGN})
	if(PLUGIN_REQUIRED)
		set(FunctionName "fe_require_plugin")
		set(MessageMode FATAL_ERROR)
	else()
		set(FunctionName "fe_optional_plugin")
		set(MessageMode WARNING)
	endif()

	if(NOT PLUGIN_REPOSITORY OR NOT PLUGIN_VERSION)
		message(FATAL_ERROR "${FunctionName}: REPOSITORY and VERSION are required for plugin ${PLUGIN_NAME}")
	endif()

	set(PluginFolder "${FE_PLUGINS_DIRECTORY}/${PLUGIN_NAME}")
	fe_read_plugin_manifest_version("${PluginFolder}/${PLUGIN_NAME}.json" InstalledVersion)
	if(NOT InstalledVersion STREQUAL PLUGIN_VERSION)
		set(ArchiveName "${PLUGIN_NAME}_${PLUGIN_VERSION}.zip")
		set(ArchiveURL "https://github.com/${PLUGIN_REPOSITORY}/releases/download/v${PLUGIN_VERSION}/${ArchiveName}")
		# Same '.' prefix as InstallPluginPackage uses, engine skips such folders, so partially downloaded plugin is never linked.
		set(ArchivePath "${FE_PLUGINS_DIRECTORY}/.Downloading_${ArchiveName}")
		set(TemporaryFolder "${FE_PLUGINS_DIRECTORY}/.Downloading_${PLUGIN_NAME}")

		set(KeptVersionMessage "")
		if(InstalledVersion)
			set(KeptVersionMessage ", installed version ${InstalledVersion} is kept")
		endif()

		message(STATUS "Downloading plugin ${PLUGIN_NAME} ${PLUGIN_VERSION} from ${ArchiveURL}")
		file(DOWNLOAD "${ArchiveURL}" "${ArchivePath}" TLS_VERIFY ON STATUS DownloadStatus)
		list(GET DownloadStatus 0 DownloadResult)
		if(NOT DownloadResult EQUAL 0)
			list(GET DownloadStatus 1 DownloadError)
			file(REMOVE "${ArchivePath}")
			message(${MessageMode} "${FunctionName}: Could not download ${ArchiveURL}: ${DownloadError}${KeptVersionMessage}")
			return()
		endif()

		# Not EXPECTED_HASH of file(DOWNLOAD), its mismatch always stops configure.
		if(PLUGIN_SHA256)
			file(SHA256 "${ArchivePath}" ArchiveSHA256)
			string(TOLOWER "${PLUGIN_SHA256}" ExpectedSHA256)
			if(NOT ArchiveSHA256 STREQUAL ExpectedSHA256)
				file(REMOVE "${ArchivePath}")
				message(${MessageMode} "${FunctionName}: SHA256 of ${ArchiveName} is ${ArchiveSHA256}, but ${ExpectedSHA256} was expected${KeptVersionMessage}")
				return()
			endif()
		endif()

		# Not file(ARCHIVE_EXTRACT), it always stops configure if archive is damaged.
		file(REMOVE_RECURSE "${TemporaryFolder}")
		file(MAKE_DIRECTORY "${TemporaryFolder}")
		execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${ArchivePath}" WORKING_DIRECTORY "${TemporaryFolder}" RESULT_VARIABLE ExtractResult OUTPUT_QUIET ERROR_VARIABLE ExtractError ERROR_STRIP_TRAILING_WHITESPACE)
		file(REMOVE "${ArchivePath}")
		if(NOT ExtractResult EQUAL 0)
			file(REMOVE_RECURSE "${TemporaryFolder}")
			message(${MessageMode} "${FunctionName}: Could not extract ${ArchiveName}: ${ExtractError}${KeptVersionMessage}")
			return()
		endif()

		# Archive could contain files of Build folder at its root or Build folder itself.
		set(ExtractedFolder "${TemporaryFolder}")
		file(GLOB ExtractedEntries "${TemporaryFolder}/*")
		list(LENGTH ExtractedEntries ExtractedEntriesCount)
		if(ExtractedEntriesCount EQUAL 1 AND IS_DIRECTORY "${ExtractedEntries}")
			set(ExtractedFolder "${ExtractedEntries}")
		endif()

		# With different version in manifest plugin would be downloaded again on every configure.
		fe_read_plugin_manifest_version("${ExtractedFolder}/${PLUGIN_NAME}.json" DownloadedVersion)
		if(NOT DownloadedVersion STREQUAL PLUGIN_VERSION)
			file(REMOVE_RECURSE "${TemporaryFolder}")
			message(${MessageMode} "${FunctionName}: ${ArchiveName} should contain ${PLUGIN_NAME}.json with version ${PLUGIN_VERSION}, but found version \"${DownloadedVersion}\"${KeptVersionMessage}")
			return()
		endif()

		file(REMOVE_RECURSE "${PluginFolder}")
		file(RENAME "${ExtractedFolder}" "${PluginFolder}")
		file(REMOVE_RECURSE "${TemporaryFolder}")
		message(STATUS "Plugin ${PLUGIN_NAME} ${PLUGIN_VERSION} is installed to ${PluginFolder}")
	endif()

	# Plugin binaries are built with headers of engine submodule of plugin, with other engine headers they could fail to link or crash.
	if(NOT EXISTS "${PluginFolder}/EngineCommit.txt")
		message(WARNING "Plugin ${PLUGIN_NAME} ${PLUGIN_VERSION} has no EngineCommit.txt, so it is not known if it was built with the same engine commit")
		return()
	endif()

	if(NOT GIT_FOUND)
		return()
	endif()

	file(STRINGS "${PluginFolder}/EngineCommit.txt" PluginEngineCommit LIMIT_COUNT 1)
	execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${FE_PLUGINS_ENGINE_DIRECTORY}" rev-parse HEAD OUTPUT_VARIABLE EngineCommit OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE GitResult ERROR_QUIET)
	if(GitResult EQUAL 0 AND NOT EngineCommit STREQUAL PluginEngineCommit)
		message(WARNING "Plugin ${PLUGIN_NAME} ${PLUGIN_VERSION} was built with engine commit ${PluginEngineCommit}, but engine is at commit ${EngineCommit}, plugin could fail to link or crash. Use version of plugin that was built with this engine commit.")
	endif()
endfunction()

# Configure stops if plugin could not be installed.
# Usage: fe_require_plugin(PluginName REPOSITORY Owner/Repository VERSION 1.0.0 [SHA256 Hash])
function(fe_require_plugin PLUGIN_NAME)
	fe_download_plugin(${PLUGIN_NAME} REQUIRED ${ARGN})
endfunction()

# Problems are reported as warnings and configure continues, application should check at runtime if plugin is loaded.
# RESULT_VARIABLE is TRUE if plugin is installed after the call, it could be older version if requested one could not be installed.
# Usage: fe_optional_plugin(PluginName REPOSITORY Owner/Repository VERSION 1.0.0 [SHA256 Hash] [RESULT_VARIABLE Variable])
function(fe_optional_plugin PLUGIN_NAME)
	cmake_parse_arguments(OPTIONAL_PLUGIN "" "RESULT_VARIABLE" "" ${ARGN})
	fe_download_plugin(${PLUGIN_NAME} ${OPTIONAL_PLUGIN_UNPARSED_ARGUMENTS})
	if(OPTIONAL_PLUGIN_RESULT_VARIABLE)
		if(EXISTS "${FE_PLUGINS_DIRECTORY}/${PLUGIN_NAME}/${PLUGIN_NAME}.json")
			set(${OPTIONAL_PLUGIN_RESULT_VARIABLE} TRUE PARENT_SCOPE)
		else()
			set(${OPTIONAL_PLUGIN_RESULT_VARIABLE} FALSE PARENT_SCOPE)
		endif()
	endif()
endfunction()

# After each build copies Debug or Release folder of statically linked plugins that declared RUNTIME_NEXT_TO_EXECUTABLE next to executable.
# Call it after engine is added, because plugins are linked when engine is added.
# Usage: fe_copy_plugin_runtime(TargetName)
function(fe_copy_plugin_runtime TARGET_NAME)
	get_property(PluginFolders GLOBAL PROPERTY FE_RUNTIME_NEXT_TO_EXECUTABLE_PLUGIN_FOLDERS)
	if(NOT PluginFolders)
		return()
	endif()

	# ';' would split command argument.
	string(REPLACE ";" "|" PluginFolders "${PluginFolders}")
	add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} "-DPLUGIN_FOLDERS=${PluginFolders}" "-DCONFIGURATION=$<IF:$<CONFIG:Debug>,Debug,Release>" "-DDESTINATION_DIRECTORY=$<TARGET_FILE_DIR:${TARGET_NAME}>" -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CopyPluginRuntime.cmake"
		VERBATIM
	)
endfunction()