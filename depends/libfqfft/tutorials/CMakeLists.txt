# Tutorials
add_executable(
  pn_m
  
  pn_m.cpp
)
target_link_libraries(
  pn_m

  ff
)
target_include_directories(
  pn_m

  PUBLIC
  ${DEPENDS_DIR}/libff/
)
add_custom_command(
  TARGET pn_m
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy $<TARGET_FILE:pn_m>  ${CMAKE_BINARY_DIR}
  )

add_executable(
  pn_eval
  
  pn_eval.cpp
)
target_link_libraries(
  pn_eval

  ff
)
target_include_directories(
  pn_eval

  PUBLIC
  ${DEPENDS_DIR}/libff/
)

message(CMAKE_COMMAND=${CMAKE_COMMAND})
message(CMAKE_ROOT=${CMAKE_ROOT})
message(PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR})
message(CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR})
message(CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR})

add_custom_command(
  TARGET pn_eval
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy $<TARGET_FILE:pn_eval>  ${CMAKE_BINARY_DIR}
  )

add_executable(
  lagrange_pn_eval

  lagrange_pn_eval.cpp
)
target_link_libraries(
  lagrange_pn_eval

  ff
)
target_include_directories(
  lagrange_pn_eval

  PUBLIC
  ${DEPENDS_DIR}/libff/
)
add_custom_command(
  TARGET lagrange_pn_eval
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy $<TARGET_FILE:lagrange_pn_eval>  ${CMAKE_BINARY_DIR}
  )

