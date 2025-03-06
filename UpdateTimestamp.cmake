string(TIMESTAMP ENGINE_BUILD_TIMESTAMP \"%Y%m%d%H%M%S\" UTC)
configure_file(${ENGINE_FOLDER}/ResourceManager/Timestamp.h.in ${ENGINE_FOLDER}/ResourceManager/Timestamp.h @ONLY)