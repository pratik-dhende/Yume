function(add_copy_yume_assets_target DEST_DIR TARGET)
    set(ASSET_SRC "${CMAKE_CURRENT_SOURCE_DIR}/Yume/assets")

    if(NOT EXISTS ${ASSET_SRC})
        message(FATAL_ERROR "Source directory ${ASSET_SRC} does not exist!")
    endif()

    if(NOT DEST_DIR)
        message(FATAL_ERROR "Destination directory must be provided to copy assets.")
    endif()

    add_custom_target(${TARGET} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSET_SRC} ${DEST_DIR}
        COMMENT "Copying assets from ${ASSET_SRC} to ${DEST_DIR}"
    )
endfunction()