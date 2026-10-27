# Applications include this file and call fe_require_plugin before they add engine, because engine links plugins that are in its Plugins folder when it is added.
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
# Usage: fe_require_plugin(PluginName REPOSITORY Owner/Repository VERSION 1.0.0 [SHA256 Hash])
function(fe_require_plugin PLUGIN_NAME)
	cmake_parse_arguments(PLUGIN "" "REPOSITORY;VERSION;SHA256" "" ${ARGN})
	if(NOT PLUGIN_REPOSITORY OR NOT PLUGIN_VERSION)
		message(FATAL_ERROR "fe_require_plugin: REPOSITORY and VERSION are required for plugin ${PLUGIN_NAME}")
	endif()

	set(PluginFolder "${FE_PLUGINS_DIRECTORY}/${PLUGIN_NAME}")
	fe_read_plugin_manifest_version("${PluginFolder}/${PLUGIN_NAME}.json" InstalledVersion)
	if(NOT InstalledVersion STREQUAL PLUGIN_VERSION)
		set(ArchiveName "${PLUGIN_NAME}_${PLUGIN_VERSION}.zip")
		set(ArchiveURL "https://github.com/${PLUGIN_REPOSITORY}/releases/download/v${PLUGIN_VERSION}/${ArchiveName}")
		# Same '.' prefix as InstallPluginPackage uses, engine skips such folders, so partially downloaded plugin is never linked.
		set(ArchivePath "${FE_PLUGINS_DIRECTORY}/.Downloading_${ArchiveName}")
		set(TemporaryFolder "${FE_PLUGINS_DIRECTORY}/.Downloading_${PLUGIN_NAME}")

		message(STATUS "Downloading plugin ${PLUGIN_NAME} ${PLUGIN_VERSION} from ${ArchiveURL}")
		set(DownloadArguments TLS_VERIFY ON STATUS DownloadStatus)
		if(PLUGIN_SHA256)
			list(APPEND DownloadArguments EXPECTED_HASH SHA256=${PLUGIN_SHA256})
		endif()

		file(DOWNLOAD "${ArchiveURL}" "${ArchivePath}" ${DownloadArguments})
		list(GET DownloadStatus 0 DownloadResult)
		if(NOT DownloadResult EQUAL 0)
			list(GET DownloadStatus 1 DownloadError)
			file(REMOVE "${ArchivePath}")
			message(FATAL_ERROR "fe_require_plugin: Could not download ${ArchiveURL}: ${DownloadError}")
		endif()

		file(REMOVE_RECURSE "${TemporaryFolder}")
		file(ARCHIVE_EXTRACT INPUT "${ArchivePath}" DESTINATION "${TemporaryFolder}")
		file(REMOVE "${ArchivePath}")

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
			message(FATAL_ERROR "fe_require_plugin: ${ArchiveName} should contain ${PLUGIN_NAME}.json with version ${PLUGIN_VERSION}, but found version \"${DownloadedVersion}\"")
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