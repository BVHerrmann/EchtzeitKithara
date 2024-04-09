get_filename_component( _PLUGIN ${CMAKE_CURRENT_LIST_DIR} NAME )

message( "Plugin - ${_PLUGIN}" )

find_package( OpenCVStatic REQUIRED )

file( GLOB LIB_SRC
    "*.h"
    "*.cpp"
)

if ( WIN32 )
	set( _VER_DESCRIPTION "${PROJECT_NAME} Plugin" )
	set( _VER_FILENAME "${_PLUGIN}.dll" )
	configure_file(
		${CMAKE_SOURCE_DIR}/cmake/common/VersionInfo.rc.in
		${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc
		@ONLY )
	list( APPEND PLUGIN_SRC ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc )
endif ( )

add_library ( ${_PLUGIN} SHARED ${LIB_SRC} )
set_property( TARGET ${_PLUGIN} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" )
target_include_directories( ${_PLUGIN} PUBLIC "../CrystalT2" )
target_link_libraries( ${_PLUGIN} OpenCVStatic Kithara )

# install
install( TARGETS ${_PLUGIN}
	RUNTIME DESTINATION . COMPONENT Runtime
	LIBRARY DESTINATION . COMPONENT Runtime )
if ( WIN32 )
	install( FILES $<TARGET_PDB_FILE:${_PLUGIN}> DESTINATION pdb COMPONENT Debug )
endif ( )
