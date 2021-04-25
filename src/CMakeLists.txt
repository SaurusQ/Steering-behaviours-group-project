configure_file(mainConfig.h.in mainConfig.h)

include_directories(../libs/glfw-3.3/deps)
include_directories(../libs/glfw-3.3/include)
include_directories(../libs/glm)

add_executable(main ${SOURCES})




if(UNIX)
	if(POLICY CMP0072)
		cmake_policy(SET CMP0072 NEW)
	endif()
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
    target_link_libraries(main glfw ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
endif()


if(WIN32)
    target_link_libraries(main glfw gdi32 opengl32)
endif()

if(MSVC)
	target_link_libraries(main glfw gdi32 opengl32)
endif()


target_include_directories(main PUBLIC 
    "${PROJECT_BINARY_DIR}" 
    ${EXTRA_INCLUDES})