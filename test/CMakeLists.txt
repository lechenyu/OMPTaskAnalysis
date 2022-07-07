set(TEST_CASES "fib.cpp" "taskwait.cpp" "taskgroup.cpp")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(TEST_CASE_LIST "")
foreach(SRC ${TEST_CASES})
  get_filename_component(TEST_CASE_NAME ${SRC} NAME_WE)
  list(APPEND ${TEST_CASE_LIST} ${TEST_CASE_NAME})
  add_executable(${TEST_CASE_NAME} ${SRC})
endforeach()

install(TARGETS ${TEST_CASE_LIST} DESTINATION test)