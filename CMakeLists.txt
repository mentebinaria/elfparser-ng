cmake_minimum_required(VERSION 3.0)

# Options
option(test "Build all tests." OFF)
option(debug "Build with debug flags." OFF)
option(qt "Build with Qt GUI." ON)
option(windows "Enable Windows build." OFF)

#define project
if (qt)
    project(elfparser-ng CXX)
else()
    project(elfparser-cli-ng CXX)
endif()

# project variables
set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "6")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION_COUNT 3)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_VENDOR " elfparser-ng (c) 2021-2022 Mente Binária // elfparser (c) 2014 Jacob Baines")
set(PROJECT_COPYRIGHT_YEAR "2022")
set(PROJECT_DOMAIN_FIRST "elfparser-ng")
set(PROJECT_DOMAIN "${PROJECT_DOMAIN_FIRST}.${PROJECT_DOMAIN_SECOND}")

# compiler flags
if (debug)
  set(CMAKE_CXX_FLAGS "-g -DDEBUG")
else()
    if (APPLE)
        set(CMAKE_CXX_FLAGS "-O2 -fno-stack-protector -march=native")
    else()
        if(windows)
            if (tests)
                set(CMAKE_CXX_FLAGS "/EHsc")
            elseif()
                set(CMAKE_CXX_FLAGS "-O2 /EHsc")
            endif()
        else()
            set(CMAKE_CXX_FLAGS "-O2 -fno-stack-protector -march=native")
        endif()
    endif()
endif()

#package locations
set(CMAKE_MODULE_PATH ${${PROJECT_NAME}_SOURCE_DIR}/CMake)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.46 COMPONENTS program_options iostreams system filesystem regex REQUIRED)

# includes
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM lib/hash-lib)

# preprocessor definitions
if (test)
    add_definitions(-DUNIT_TESTS)
endif()
if (APPLE)
    add_definitions(-DAPPLE)
endif()
if (windows)
    add_definitions(-DWINDOWS)
endif()

# QT configuration
if (qt)
    # set preprocessor to select code that gets used
    add_definitions(-DQT_GUI)

    # from the QT5 cmake guide
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    if (APPLE)
        set(CMAKE_PREFIX_PATH "~/Qt/5.3/clang_64")
    else()
        if(windows)
            set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.3.2\\5.3\\msvc2013_64")
        else()
            set(CMAKE_PREFIX_PATH "~/Qt/5.2.0/gcc_64")
        endif()
    endif()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)

    # let QT generate files
    qt5_wrap_ui(UI_HEADERS src/ui/mainwindow.ui  src/ui/about.ui)
    set(EXTRA_SOURCES src/ui/mainwindow.cpp src/ui/QHexView-ng.cpp)
    set(EXTRA_SOURCES ${EXTRA_SOURCES} ${UI_HEADERS})

    if (APPLE)
        # tell the compiler to do the GUI subsystem
        set(GUI_TYPE MACOSX_BUNDLE)
        set(BUILD_TYPE MACOSX_BUNDLE)
    endif()

endif()

# compilation units
add_executable(${PROJECT_NAME} ${GUI_TYPE}
               src/main.cpp
               src/elfparser.cpp
               src/programheaders.cpp
               src/sectionheaders.cpp
               src/segment.cpp
               src/symbols.cpp
               src/dynamicsection.cpp
               src/abstract_elfheader.cpp
               src/abstract_programheader.cpp
               src/abstract_sectionheader.cpp
               src/abstract_segments.cpp
               src/abstract_symbol.cpp
               src/abstract_dynamic.cpp
               src/initarray.cpp
               src/segment_types/segment_type.cpp
               src/segment_types/note_segment.cpp
               src/segment_types/comment_segment.cpp
               src/segment_types/debuglink_segment.cpp
               src/segment_types/interp_segment.cpp
               src/segment_types/strtable_segment.cpp
               src/segment_types/readonly_segment.cpp
               src/datastructures/search_node.cpp
               src/datastructures/search_tree.cpp
               src/ui/inttablewidget.cpp
               lib/hash-lib/sha1.cpp
               lib/hash-lib/sha256.cpp
               lib/hash-lib/md5.cpp

               ${EXTRA_SOURCES}
               ${RESOURCE_ADDED})

if(APPLE)
    if (qt)
        set(ICONS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/src/ui/assets")
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_COPYRIGHT "${PROJECT_COPYRIGHT_YEAR} ${PROJECT_VENDOR}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_DOMAIN_SECOND}.${PROJECT_DOMAIN_FIRST}")
        set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources")
        set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${MACOSX_BUNDLE_RESOURCES})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MACOSX_BUNDLE_ICON} ${MACOSX_BUNDLE_RESOURCES})
        set(MACOSX_APP_FILE "${${PROJECT_NAME}_SOURCE_DIR}/src/ui/assets/bug.png")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MACOSX_APP_FILE} ${MACOSX_BUNDLE_RESOURCES})
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Frameworks)
    endif()
endif()
if (windows)
    if (qt)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
endif()


# linking comp / libs
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
if (qt)
    target_link_libraries(${PROJECT_NAME}  ${Boost_LIBRARIES} Qt5::Widgets)
endif()
####
# Testing silliness
####
if (test)
    if (APPLE)
        add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)
    endif()
    if(WINDOWS) # VS2012 doesn't support correctly the tuples yet
        add_definitions( /D _VARIADIC_MAX=10 )
    endif()
    add_subdirectory(lib/gtest-1.6.0)
    enable_testing()
    include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    # Unit test compilation  this seems really inefficient...
    add_executable(${PROJECT_NAME}_test
                    src/elfparser.cpp
                    src/programheaders.cpp
                    src/sectionheaders.cpp
                    src/segment.cpp
                    src/symbols.cpp
                    src/dynamicsection.cpp
                    src/abstract_elfheader.cpp
                    src/abstract_programheader.cpp
                    src/abstract_sectionheader.cpp
                    src/abstract_segments.cpp
                    src/abstract_symbol.cpp
                    src/abstract_dynamic.cpp
                    src/initarray.cpp
                    src/segment_types/segment_type.cpp
                    src/segment_types/note_segment.cpp
                    src/segment_types/comment_segment.cpp
                    src/segment_types/debuglink_segment.cpp
                    src/segment_types/interp_segment.cpp
                    src/segment_types/strtable_segment.cpp
                    src/segment_types/readonly_segment.cpp
                    src/datastructures/search_node.cpp
                    src/datastructures/search_tree.cpp
                    lib/hash-lib/sha1.cpp
                    lib/hash-lib/sha256.cpp
                    lib/hash-lib/md5.cpp
                    src/tests/ls_tests.cpp
                    src/tests/tiny_tests.cpp
                    )

    target_link_libraries(${PROJECT_NAME}_test gtest gtest_main ${Boost_LIBRARIES})
endif()

# CPACK stuff
if (APPLE)
    if (qt)
        install(TARGETS elfparser-ng BUNDLE DESTINATION . RUNTIME DESTINATION bin)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/bundle")
        include(DeployQt5)
        install_qt5_executable("elfparser.app" "qcocoa")
        set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
        set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MAJOR}")
        set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
        set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
        set(CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
        set(CMAKE_INSTALL_PREFIX "")
        set(CPACK_GENERATOR "DragNDrop")
        set(CPACK_DMG_FORMAT "UDBZ")
        set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
        set(CPACK_SYSTEM_NAME "OSX")
        include(InstallRequiredSystemLibraries)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BIN_INSTALL_DIR}")
        include(CPack)
    endif()
else()
    if (windows)
        INSTALL(DIRECTORY ./build/Release/ DESTINATION .)
        INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/ui/assets/bug.png DESTINATION .)
        INSTALL(TARGETS ${PROJECT_NAME} DESTINATION .)
        SET(CPACK_INSTALL_DIRECTORY "elfparser-ng")
        SET(CPACK_GENERATOR "NSIS")
        SET(CPACK_PACKAGE_DESCRIPTION "elfparser-ng")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Binary static Analysis Tool")
        SET(CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
        SET(CPACK_PACKAGE_INSTALL_DIRECTORY "elfparser-ng")
        SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
        SET(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
        SET(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
        SET(CPACK_NSIS_DISPLAY_NAME "elfparser-ng")
        SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        SET(CPACK_NSIS_CREATE_ICONS "CreateShortCut '\$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${PROJECT_NAME}.lnk' '\$INSTDIR\\\\${PROJECT_NAME}.exe'")
        SET(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
        INCLUDE(CPack)
    else()
        INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
        if (qt)
            INSTALL(FILES ${CMAKE_SOURCE_DIR}/src/ui/assets/bug.png DESTINATION /usr/local/share/elfparser-ng/)
        endif()
        SET(CPACK_SET_DESTDIR ON)
        SET(CPACK_GENERATOR "DEB")
        SET(CPACK_PACKAGE_DESCRIPTION "elfparser-ng")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Binary static analysis tool")
        SET(CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
        SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
        SET(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
        SET(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
        SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CMAKE_SYSTEM_PROCESSOR}_${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
        SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
        SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1.3), libgcc1 (>= 1:4.1.1), libstdc++6 (>= 4.2.1), libQt5Widgets5 (>= 5.2.0)")
        SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
        SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
        INCLUDE(CPack)
    endif()
endif()
