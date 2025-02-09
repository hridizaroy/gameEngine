# CMakeList.txt : CMake project for gameEngine, include source and define
# project specific logic here.
#

include_directories(
  "${PROJECT_SOURCE_DIR}/third-party/glfw-3.4.bin.WIN64/include"
  "${PROJECT_SOURCE_DIR}/third-party/vulkan/Include"
  "${PROJECT_SOURCE_DIR}/third-party/ImGui"
)

set(IMGUI_SRC
	imgui/imconfig.h
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_internal.h
	imgui/imgui_widgets.cpp
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
	imgui/imgui_impl_glfw.h
	imgui/imgui_impl_glfw.cpp
	imgui/imgui_impl_vulkan.h
	imgui/imgui_impl_vulkan.cpp
)

# Add source to this project's executable.
add_executable (gameEngine "engine.cpp" "engine.h" "main.cpp" "instance.h"
				"config.h" "logging.h" "device.h" "queue_families.h"
				"frame.h" "shaders.h" "pipeline.h" "app.h" "app.cpp"
				"render_structs.h" "scene.h" "scene.cpp" "commands.h" "swapchain.h"
				"framebuffer.h" "sync.h" "mesh.h" "buffers.h" "buffers.cpp"
				"SceneData.h" "SceneData.cpp" "descriptors.h"
				${IMGUI_SRC})

target_link_libraries(gameEngine 
  "${PROJECT_SOURCE_DIR}/third-party/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib"
  "${PROJECT_SOURCE_DIR}/third-party/vulkan/Lib/vulkan-1.lib"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gameEngine PROPERTY CXX_STANDARD 20)
endif()
