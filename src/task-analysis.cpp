#include "callback.h"
#include <cstddef>
#include <iostream>
#include <omp-tools.h>

static void ompt_ta_task_create(ompt_data_t *encountering_task_data,
                                const ompt_frame_t *encountering_task_frame,
                                ompt_data_t *new_task_data, int flags,
                                int has_dependences, const void *codeptr_ra) {
  std::cout << "task " << (uintptr_t)encountering_task_data << " created"
            << std::endl;
}
static int ompt_ta_initialize(ompt_function_lookup_t lookup, int device_num,
                              ompt_data_t *tool_data) {

  ompt_set_callback_t ompt_set_callback =
      (ompt_set_callback_t)lookup("ompt_set_callback");
  if (ompt_set_callback == NULL) {
    std::cerr << "Could not set callback, exiting..." << std::endl;
    std::exit(1);
  }

  SET_CALLBACK(task_create);
  return 1; // success
}

static void ompt_ta_finalize(ompt_data_t *tool_data) {}

extern "C" ompt_start_tool_result_t *
ompt_start_tool(unsigned int omp_version, const char *runtime_version) {
  static ompt_start_tool_result_t start_tool_result = {
      &ompt_ta_initialize, &ompt_ta_finalize, {0}};
  std::cout << "OMPT Task Analsis Enabled" << std::endl;
  return &start_tool_result;
}