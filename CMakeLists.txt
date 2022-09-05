cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME VULKVULK)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED) 
 
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} 
    VulkApp/main.cpp
    src/Core/core.h  
    src/Core/App.cpp                    src/Core/App.h
    src/GameAsset/gameObject.cpp        src/GameAsset/gameObject.h
    src/Render/window.cpp               src/Render/window.h
    src/Render/pipeline.cpp             src/Render/pipeline.h
    src/Render/swapChain.cpp            src/Render/swapChain.h
    src/Render/device.cpp               src/Render/device.h
    src/Render/model.cpp                src/Render/model.h
    src/Render/renderer.cpp             src/Render/renderer.h
    src/Render/simpleRenderSystem.cpp   src/Render/simpleRenderSystem.h
    src/Render/camera.cpp               src/Render/camera.h
    src/IO/keyboard_movement.cpp        src/IO/keyboard_movement.h


)

include(Dependency.cmake)


target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS} )
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIBS_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS} ${Vulkan_LIBRARIES})

add_dependencies(${PROJECT_NAME} ${DEP_LIST})
