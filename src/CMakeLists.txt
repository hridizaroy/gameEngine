# CMakeList.txt : CMake project for gameEngine, include source and define
# project specific logic here.
#

include_directories(
  "${PROJECT_SOURCE_DIR}/third-party/glfw-3.4.bin.WIN64/include"
  "${PROJECT_SOURCE_DIR}/third-party/vulkan/Include"
)


# Add source to this project's executable.
add_executable (gameEngine "engine.cpp" "engine.h" "main.cpp" "instance.h" "config.h" "logging.h" "device.h" "queue_families.h" "frame.h" "shaders.h" "pipeline.h" "app.h" "app.cpp" "render_structs.h" "scene.h" "scene.cpp")

target_link_libraries(gameEngine 
  "${PROJECT_SOURCE_DIR}/third-party/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib"
  "${PROJECT_SOURCE_DIR}/third-party/vulkan/Lib/vulkan-1.lib"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gameEngine PROPERTY CXX_STANDARD 20)
endif()
