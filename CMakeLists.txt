cmake_minimum_required(VERSION 3.22)
project(cmail
    VERSION 0.0.1
    LANGUAGES C
)

find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/payload.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/attach.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dispatch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/envelope.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/job.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/job_queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/worker.c
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    CURL::libcurl
)
