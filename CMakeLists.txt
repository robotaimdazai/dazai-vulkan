# CMakeList.txt : CMake project for Dazai Vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Dazai Vulkan")

include_directories("vendor/include")
link_directories("vendor/lib")

# Use file globbing to collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Add source to this project's executable.
add_executable (DazaiVulkan ${SOURCES})

target_link_libraries(DazaiVulkan PRIVATE glfw3 vulkan-1)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DazaiVulkan PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
