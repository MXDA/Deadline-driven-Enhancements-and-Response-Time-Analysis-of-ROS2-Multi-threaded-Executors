cmake_minimum_required(VERSION 3.5)
project(priority_executor)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-reorder)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(simple_timer REQUIRED)

add_library(priority_executor src/priority_executor.cpp)
target_include_directories(priority_executor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(priority_executor
  rmw
  rclcpp
  rcl
  simple_timer 
)

add_library(test_nodes src/test_nodes.cpp)
target_include_directories(test_nodes PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(test_nodes
  rmw
  rclcpp
  rcl
  simple_timer
  std_msgs
)
target_link_libraries(test_nodes
  primes_workload
)

add_library(primes_workload src/primes_workload.cpp)
target_include_directories(primes_workload PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(primes_workload
  simple_timer
)


#add_executable(test_publisher src/test_publisher.cpp)
#target_include_directories(test_publisher PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>)
#target_link_libraries(test_publisher
#  priority_executor
#  test_nodes
#  default_executor
#)
#ament_target_dependencies(
#  test_publisher
#  rclcpp
#  std_msgs
#  std_srvs
#  simple_timer 
#)

#install(TARGETS test_publisher priority_executor
#  DESTINATION lib/${PROJECT_NAME})

add_library(default_executor src/default_executor.cpp)
target_include_directories(default_executor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(default_executor
  rclcpp
  simple_timer
)
target_link_libraries(default_executor
  primes_workload
)

#add_executable(f1tenth_publisher src/f1tenth_test.cpp)
#target_include_directories(test_publisher PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>)
#target_link_libraries(f1tenth_publisher
#  priority_executor
#  test_nodes
#  default_executor
#)
#ament_target_dependencies(f1tenth_publisher
#  rclcpp
#  std_msgs
#  std_srvs
#  simple_timer 
#)


add_executable(multi_test src/multi_test.cpp)
target_include_directories(multi_test PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(multi_test
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(multi_test
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)

add_executable(multi_arb src/multi_arb.cpp)
target_include_directories(multi_arb PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(multi_arb
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(multi_arb
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)


add_executable(multi_static1 src/multi_static1.cpp)
target_include_directories(multi_static1 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(multi_static1
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(multi_static1
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)
# add_executable(test1 src/test1.cpp)
# target_include_directories(test_publisher PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>)
#target_link_libraries(test1
#  priority_executor
#  test_nodes
#  default_executor
#)
#ament_target_dependencies(test1
#  rclcpp
#  std_msgs
#  std_srvs
#  simple_timer
#)

#add_executable(multi_test1 src/multi_test1.cpp)
#target_include_directories(test_publisher PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>)
#target_link_libraries(multi_test1
#  priority_executor
#  test_nodes
#  default_executor
#)
#ament_target_dependencies(multi_test1
#  rclcpp
#  std_msgs
#  std_srvs
#  simple_timer
#)

#add_executable(usage_example src/usage_example.cpp)
#target_include_directories(usage_example PUBLIC
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  $<INSTALL_INTERFACE:include>)
#target_link_libraries(usage_example
#  priority_executor
#  test_nodes
#  default_executor
#)
#ament_target_dependencies(usage_example
#  rclcpp
#  std_msgs
#  std_srvs
#  simple_timer 
#)

add_executable(dy_2 src/dy_2.cpp)
target_include_directories(dy_2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(dy_2
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(dy_2
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)
add_executable(st_2 src/st_2.cpp)
target_include_directories(st_2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(st_2
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(st_2
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)

add_executable(muex_test src/muex_test.cpp)
target_include_directories(muex_test PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(muex_test
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(muex_test
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)

add_executable(muex_static1 src/muex_static1.cpp)
target_include_directories(muex_static1 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(muex_static1
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(muex_static1
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)
add_executable(arb_test src/arb_test.cpp)
target_include_directories(arb_test PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(arb_test
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(arb_test
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)
add_executable(arb_static src/arb_static.cpp)
target_include_directories(arb_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(arb_static
  priority_executor
  test_nodes
  default_executor
)
ament_target_dependencies(arb_static
  rclcpp
  std_msgs
  std_srvs
  simple_timer 
)
install(TARGETS arb_test arb_static multi_test multi_arb muex_test multi_static1 muex_static1 dy_2 st_2 priority_executor
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
