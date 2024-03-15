cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_NAME AudioProcessingProject)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

##---FFMpeg
find_package(PkgConfig REQUIRED)

find_package(SDL2 REQUIRED)

# Finding opengl, glfw, glew, and glut
find_package(glfw3 3.4 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

if(__APPLE__)

    # This is making sure we find the SDL libraries on mac
    include_directories(${SDL2_INCLUDE_DIRS})

    # This is making sure that opencv on mac has been found!
    if(OpenCV_Found)
        # Additional Include Directories
        include_directories( ${OpenCV_INCLUDE_DIRS} )

        # Additional Library Directories
        link_directories( ${OpenCV_LIB_DIR} )

        # Additional Dependencies
        # target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
    else()
        message("Opencv is not found on mac!")
        message("
            Make sure you installed opencv correctly!
            - Check homebrew
            - Check if opencv is installed correctly with homebrew.
        ")
    endif()


    target_link_libraries(${PROJECT_NAME} pthread)
    # Linking the libraries for Cocoa, OpenGL, IOKit, and glfw
    # Need this for opengl
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    # target_link_libraries(${PROJECT_NAME} glfw ${OPENGL_gl_LIBRARY})
    # target_link_libraries(${PROJECT_NAME} GLEW::GLEW) # This helps us get rid of the "___glewGenBuffers" reference error

endif(__APPLE__)

add_subdirectory(dependencies/formatCpp)

add_executable(${PROJECT_NAME} main.cpp)

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

target_link_libraries(${PROJECT_NAME}
    PkgConfig::LIBAV
    ${SDL2_LIBRARIES}
    glfw
    ${OPENGL_gl_LIBRARY}
    GLEW::GLEW
    FormatCpp::FormatCpp
)
