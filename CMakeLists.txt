cmake_minimum_required(VERSION 3.16)
project(unicornfish)

if (APPLE)
    include_directories(/usr/local/include)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(
	${PROJECT_NAME}
	STATIC
	src/unicornfish.cpp
	src/unicornfish_broker.cpp
	src/unicornfish_client.cpp
	src/unicornfish_corona_database.cpp
	src/unicornfish_corona_job.cpp
	src/unicornfish_corona_scene_file.cpp
	src/unicornfish_curl.cpp
	src/unicornfish_frame.cpp
	src/unicornfish_majordomo.cpp
	src/unicornfish_message.cpp
	src/unicornfish_socket.cpp
	src/unicornfish_worker.cpp
   )

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD            99
    C_STANDARD_REQUIRED   ON
    CXX_EXTENSIONS        OFF
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
)

if(${CMAKE_VERSION} VERSION_GREATER "3.16.0")
    message("Viperfish using precompiled headers")
    target_precompile_headers(${PROJECT_NAME} PRIVATE src/unicornfish_pch.cpp)
endif()

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
