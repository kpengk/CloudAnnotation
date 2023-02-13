cmake_minimum_required (VERSION 3.17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(target_name CloudAnnotation)

find_package(Qt6 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(VTK REQUIRED)

if(VTK_FOUND)
	message (STATUS "VTK_VERSION: ${VTK_VERSION}")
	message(STATUS " VTK_FOUND =  ${VTK_FOUND}" )
	message(STATUS " VTK_MAJOR_VERSION = ${VTK_MAJOR_VERSION}" )
	message(STATUS " VTK_MINOR_VERSION = ${VTK_MINOR_VERSION}" )
	message(STATUS " VTK_BUILD_VERSION = ${VTK_BUILD_VERSION}" )
	message(STATUS " VTK_LIBRARY_DIRS  = ${VTK_LIBRARY_DIRS}" )
	message(STATUS " VTK_LIBRARIES  = ${VTK_LIBRARIES}" )
else()
	message("VTK NOT FOUND: ${VTK_NOT_FOUND_MESSAGE}")
	return()
endif()


# translations
set(TS_FILES translations/cloud_annotation_zh_CN.ts)
qt6_add_translation(QM_FILES ${TS_FILES})
qt6_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})


include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/ui_templates )

file( GLOB source_list *.h *.hpp *.cpp )
file( GLOB ui_list *.ui )
file( GLOB qrc_list *.qrc )

if( MSVC )
	# App icon with MSVC
	set( rc_list images/icon/ca_icon.rc )

	#to get rid of the (system) console
	qt_add_executable( ${target_name} MANUAL_FINALIZATION ${source_list} ${ui_list} ${qrc_list} ${rc_list} ${TS_FILES} )
elseif( APPLE )
	qt_add_executable( ${target_name} MACOSX_BUNDLE ${source_list} ${ui_list} ${qrc_list} ${TS_FILES} )
else()
	qt_add_executable( ${target_name} ${header_list} ${ui_list} ${qrc_list} ${rc_list} ${TS_FILES} )
	target_compile_definitions( ${target_name} PRIVATE QT_NO_KEYWORDS )
endif()

target_include_directories(${target_name} PRIVATE ../include )
target_link_libraries(${target_name} PRIVATE ${VTK_LIBRARIES} Qt6::Widgets)

set_target_properties(${target_name} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

qt_finalize_executable(${target_name})

vtk_module_autoinit(TARGETS ${target_name} MODULES ${VTK_LIBRARIES})
