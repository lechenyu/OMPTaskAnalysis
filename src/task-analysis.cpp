#include "callback.h"
#include "data_structure.h"
#include <cstddef>
#include <iostream>
#include <omp-tools.h>
#include <assert.h>
#include <vector>

struct dpst DPST;

char node_char[6] = {'R','F','A','f','S','W'};
static int node_index = 0;

ompt_get_task_info_t ompt_get_task_info;
ompt_set_callback_t ompt_set_callback;

/**
 * @brief  Create a new tree node
 * @note   
 * @retval The newly created tree node
 */
tree_node* newtreeNode()
{
    // Allocate memory for new node
    tree_node* node = (tree_node*) malloc(sizeof(tree_node));
    node->children_list_head = NULL;
    node->children_list_tail = NULL;
    node->next_sibling = NULL;
    node->corresponding_task_id = -2;
    node->number_of_child = 0;
    node->is_parent_nth_child = 0;
    node->preceeding_taskwait = -1;

    node->index = node_index;
    node_index ++;
    return node;
}


/**
 * @brief  Insert a tree node (not a step node) under parent
 * @note   
 * @param  nodeType: root,async,future or finish
 * @param  *parent: parent node, the inserted node will become a child of parent
 * @retval the newly inserted node
 */
tree_node* insert_tree_node(enum node_type nodeType, tree_node *parent){
    tree_node *node = newtreeNode();   
    node->this_node_type = nodeType;
    
    if(nodeType == ROOT){ 
        node->depth = 0;
        node->parent = NULL;
        DPST.root = node;
    }
    else{
        // each task corresponds to an async or a future tree node
        // assert(parent);
        node->parent = parent;
        node->depth = node->parent->depth + 1;
        node->is_parent_nth_child = parent->number_of_child;
        if(nodeType == FINISH){
            node->corresponding_task_id = parent->corresponding_task_id;
        }
        parent->number_of_child += 1;

        if(node->parent->children_list_head == NULL){
            node->parent->children_list_head = node;
            node->parent->children_list_tail = node;
        }
        else{
            node->parent->children_list_tail->next_sibling = node;
            node->preceeding_taskwait = node->parent->children_list_tail->preceeding_taskwait;
            node->parent->children_list_tail = node;
        }   
    }

    if(node->depth > DPST.height){
        DPST.height = node->depth;
    }
    return node;
}


/**
 * @brief  Insert a new leaf(step) node to the task_node
 * @note   
 * @param  *task_node: the node that will have a new leaf
 * @retval the newly inserted leaf(step) node
 */
tree_node* insert_leaf(tree_node *task_node){
    // HASSERT(task_node);
    tree_node *new_step = newtreeNode();   
    new_step->this_node_type = STEP;
    new_step->parent = task_node;
    new_step->depth = task_node->depth + 1;
    new_step->is_parent_nth_child = task_node->number_of_child;
    new_step->corresponding_task_id = task_node->corresponding_task_id;

    task_node->number_of_child += 1;
    
    if(task_node->children_list_head == NULL){
        task_node->children_list_head = new_step;
        task_node->children_list_tail = new_step;
    }
    else{
        task_node->children_list_tail->next_sibling = new_step;
        new_step->preceeding_taskwait = task_node->children_list_tail->preceeding_taskwait;
        task_node->children_list_tail = new_step;
    }

    if(new_step->depth > DPST.height){
        DPST.height = new_step->depth;
    }
    return new_step;
}


/**
 * @brief  check if node1 precedes node2 in dpst
 * @param  node1: previous step node
 * @param  node2: current step node
 * @retval true if node1 precdes node2 by tree edges 
 */
bool precede_dpst(tree_node* node1, tree_node* node2){
  bool node1_precede_node2 = true;
  bool node1_not_precede_node2 = false;

    if(node1->parent->index == node2->parent->index){
        // node1 and node2 are step nodes with same parent
        if(node1->is_parent_nth_child <= node2->is_parent_nth_child){
          return node1_precede_node2;
        }
        else{
          return node1_not_precede_node2;
        }
    }
    
    // need to guarantee prev_node is to the left of current_node
    tree_node* node1_last_node;
    tree_node* node2_last_node;

    while (node1->depth != node2->depth)
    {
        node1_last_node = node1;
        node2_last_node = node2;
        if (node1->depth > node2->depth)
        {
            node1 = node1->parent;
        }
        else{
            node2 = node2->parent;
        }
    }

    while(node1->index != node2->index){
        node1_last_node = node1;
        node2_last_node = node2;
        node1 = node1->parent;
        node2 = node2->parent;
    }; // end

    if(node1_last_node->is_parent_nth_child < node2_last_node->is_parent_nth_child){
        // node1 is to the left of node 2
        if(
             (node1_last_node->this_node_type == ASYNC && node2_last_node->preceeding_taskwait < 0)
          || (node1_last_node->this_node_type == ASYNC && node2_last_node->preceeding_taskwait < node1_last_node->is_parent_nth_child)
        ){
          return node1_not_precede_node2;
        }
        else{
            return node1_precede_node2;
        }
    }

    // node 1 is to the right of node 2
    // return false because "node1 doesn't precede node2 by DPST"
    return node1_not_precede_node2;
}


int get_dpst_height(){
    return DPST.height;
}



tree_node* search_node_by_index(int index){
  std::vector<tree_node*> node_vector;
  node_vector.push_back(DPST.root);

  tree_node* node = NULL;

  while(!node_vector.empty()){
    node = node_vector.back();
    if(node->index == index){
      return node;
    }

    tree_node *child = node->children_list_head;
    while (child != NULL)
    {
      node_vector.insert(node_vector.begin(),child);
      child = child->next_sibling;
    }

    node_vector.pop_back();
    
  }

  printf("Error: cannot find node with index %d \n", index);
  assert(0);
  return node;
}


bool precede_dpst_index(int a, int b){
  tree_node* node1 = search_node_by_index(a);
  tree_node* node2 = search_node_by_index(b);
  return precede_dpst(node1,node2);
}


void printDPST(){
    tree_node *node_array[100] = {NULL};
    tree_node *tmp_array[100] = {NULL};
    node_array[0] = DPST.root;
    int depth = 0;

    while (node_array[0] != NULL)
    {
        printf("depth %d:   ",depth);
        int tmp_index = 0;
        int i = 0;
        while (i < 100)
        {
            tree_node *node = node_array[i];
            if(node == NULL){
                //printf("   ");
            }
            else{
                printf("%c (i:%d) ",node_char[node->this_node_type],node->index);
                if(node->parent != NULL){
                    printf("(p:%d)    ",node->parent->index);
                }
                tree_node *child = node->children_list_head;
                while (child != NULL)
                {
                    tmp_array[tmp_index] = child;
                    tmp_index++;
                    child = child->next_sibling;
                }
            }

            node_array[i] = NULL;
            i++;
        }
        printf("\n");

        depth++;
        int j = 0;
        while(j < 100){
            node_array[j] = tmp_array[j];
            tmp_array[j] = NULL;
            j++;
        }
    }
}


static int task_id_counter = 1;


static void ompt_ta_parallel_begin
(
  ompt_data_t *encountering_task_data,
  const ompt_frame_t *encountering_task_frame,
  ompt_data_t *parallel_data,
  unsigned int requested_parallelism,
  int flags,
  const void *codeptr_ra
)
{
  assert(encountering_task_data->ptr != NULL);
  // insert a FINISH node because of the implicit barrier
  task_t* current_task = (task_t*) encountering_task_data->ptr;
  tree_node* current_task_node = current_task->node_in_dpst;

  // 1. Update DPST
  tree_node* new_finish_node;
  if(current_task->current_finish == NULL){
    new_finish_node = insert_tree_node(FINISH,current_task_node);
  }
  else{
    new_finish_node = insert_tree_node(FINISH,current_task->current_finish->node_in_dpst);
  }
  insert_leaf(new_finish_node);
  insert_leaf(current_task_node);

  // 2. Set current task's current_finish to this finish
  finish_t* finish = (finish_t*) malloc(sizeof(finish_t));
  finish->node_in_dpst = new_finish_node;

  if(current_task->current_finish == NULL){
    finish->parent = NULL;
  }
  else{
    finish->parent = current_task->current_finish;
  }

  current_task->current_finish = finish;


  parallel_data->ptr = encountering_task_data->ptr;
}


static void ompt_ta_parallel_end
(
  ompt_data_t *parallel_data,
  ompt_data_t *encountering_task_data,
  int flags,
  const void *codeptr_ra
)
{
  assert(encountering_task_data->ptr != NULL);
  printf("parall end callback \n");

  // set current_task->current_finish to either NULL or another finish (nested finish)
  task_t* current_task = (task_t*) encountering_task_data->ptr;
  finish_t* the_finish = current_task->current_finish;

  if(the_finish->parent == NULL){
    current_task->current_finish = NULL;
  }
  else{
    current_task->current_finish = the_finish->parent;
  }
}


static void ompt_ta_implicit_task(
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  unsigned int actual_parallelism,
  unsigned int index,
  int flags
)
{
  if(flags & ompt_task_initial){
    if(endpoint == ompt_scope_begin){

      printf("initial task begins, should only appear once !! \n");

      // DPST operation
      tree_node* root = insert_tree_node(ROOT,NULL);
      insert_leaf(root);

      task_t* main_ti = (task_t*) malloc(sizeof(task_t));
      main_ti->belong_to_finish = NULL;
      main_ti->id = 0;
      main_ti->parent_id = -1;
      main_ti->node_in_dpst = root;

      finish_t* main_finish_placeholder = (finish_t*) malloc(sizeof(finish_t));
      main_finish_placeholder->node_in_dpst = root;
      main_finish_placeholder->parent = NULL;
      main_ti->current_finish = main_finish_placeholder;

      task_data->ptr = (void*) main_ti;
    
    }
  }
  else{
    if(endpoint == ompt_scope_begin){
      assert(parallel_data->ptr != NULL);

      // try to use ompt_get_task_info
      // int ancestor_level = 0;
      // int flags_2;
      // ompt_data_t *task_data_2;
      // ompt_frame_t *task_frame_2;
      // ompt_data_t *parallel_data_2;
      // int thread_num_2;
      // int result = ompt_get_task_info(ancestor_level,&flags_2,&task_data_2,&task_frame_2,&parallel_data_2,&thread_num_2);
      // if(result == 2){
      //   task_t* task = (task_t*) parallel_data_2->ptr;
      //   printf("current task node's index is %d \n", task->node_in_dpst->index);
      //   task_t* same_task = (task_t*) task_data_2->ptr;
      //   // printf("same task node's node index %d \n", same_task->node_in_dpst->index);
      // }


      // A. Get encountering_task information
      task_t* current_task = (task_t*) parallel_data->ptr;
      tree_node* current_task_node = current_task->node_in_dpst;

      // B. DPST operation
        tree_node* new_task_node;
        tree_node* parent_node;

        if(current_task->current_finish != NULL){
          // 1. if current task's current_finish is not null, parent_node should be that finish's node
          parent_node = current_task->current_finish->node_in_dpst;
        }
        else{
          // 2. if current task's current_finish is null, parent_node should be current task's node
          parent_node = current_task_node;
        }

        new_task_node = insert_tree_node(ASYNC, parent_node);

        insert_leaf(new_task_node->parent);
        insert_leaf(new_task_node);
        new_task_node->corresponding_task_id = task_id_counter;


      // C. Update task data for new task
        task_t* ti = (task_t*) malloc(sizeof(task_t));
        ti->id = task_id_counter;
        ti->node_in_dpst = new_task_node;

        // set new task's belong_to_finish
        if(current_task->current_finish != NULL){
          ti->belong_to_finish = current_task->current_finish;
        }
        else{
          ti->belong_to_finish = current_task->belong_to_finish;
        }
        
        task_data->ptr = (void*) ti;

      // D. Update global variable
        task_id_counter ++;
    }
  }

}


static void ompt_ta_sync_region(
  ompt_sync_region_t kind,
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  const void *codeptr_ra)
{
  if(kind == ompt_sync_region_taskgroup && endpoint == ompt_scope_begin){
    assert(task_data->ptr != NULL);

    task_t* current_task = (task_t*) task_data->ptr;
    tree_node* current_task_node = current_task->node_in_dpst;

    // 1. Update DPST
    tree_node* new_finish_node;
    if(current_task->current_finish == NULL){
      new_finish_node = insert_tree_node(FINISH,current_task_node);
    }
    else{
      new_finish_node = insert_tree_node(FINISH,current_task->current_finish->node_in_dpst);
    }
    insert_leaf(new_finish_node);
    insert_leaf(current_task_node);

    // 2. Set current task's current_finish to this finish
    finish_t* new_finish = (finish_t*) malloc(sizeof(finish_t));
    new_finish->node_in_dpst = new_finish_node;

    if(current_task->current_finish == NULL){
      new_finish->parent = NULL;
    }
    else{
      new_finish->parent = current_task->current_finish;
    }

    current_task->current_finish = new_finish;

  }
  else if (kind == ompt_sync_region_taskgroup && endpoint == ompt_scope_end )
  {
    assert(task_data->ptr != NULL);

    // set current_task->current_finish to either NULL or another finish (nested finish)
    task_t* current_task = (task_t*) task_data->ptr;
    finish_t* finish = current_task->current_finish;

    if(finish->parent == NULL){
      current_task->current_finish = NULL;
    }
    else{
      current_task->current_finish = finish->parent;
    }
    
  }
  else if(kind == ompt_sync_region_taskwait && endpoint == ompt_scope_begin){
    assert(task_data->ptr != NULL);
    task_t* current_task = (task_t*) task_data->ptr;

    // insert a single node (type STEP), mark this as a taskwait step
    tree_node* new_taskwait_node;
    if(current_task->current_finish == NULL){
      new_taskwait_node = insert_tree_node(TASKWAIT, current_task->node_in_dpst);
      new_taskwait_node->preceeding_taskwait = new_taskwait_node->is_parent_nth_child;
      insert_leaf(current_task->node_in_dpst);
    }
    else{
      new_taskwait_node = insert_tree_node(TASKWAIT, current_task->current_finish->node_in_dpst);
      new_taskwait_node->preceeding_taskwait = new_taskwait_node->is_parent_nth_child;
      insert_leaf(current_task->current_finish->node_in_dpst);
    }

  }
  
}


static void ompt_ta_task_create(ompt_data_t *encountering_task_data,
                                const ompt_frame_t *encountering_task_frame,
                                ompt_data_t *new_task_data, int flags,
                                int has_dependences, const void *codeptr_ra) 
{
  assert(encountering_task_data->ptr != NULL);

  // A. Get encountering_task information
    task_t* current_task = (task_t*) encountering_task_data->ptr;
    tree_node* current_task_node = current_task->node_in_dpst;

  // B. DPST operation
    tree_node* new_task_node;
    tree_node* parent_node;

    if(current_task->current_finish != NULL){
      // 1. if current task's current_finish is not null, parent_node should be that finish's node
      parent_node = current_task->current_finish->node_in_dpst;
    }
    else{
      // 2. if current task's current_finish is null, parent_node should be current task's node
      parent_node = current_task_node;
    }

    new_task_node = insert_tree_node(ASYNC, parent_node);

    insert_leaf(new_task_node->parent);
    insert_leaf(new_task_node);
    new_task_node->corresponding_task_id = task_id_counter;

  // C. Update task data
    task_t* ti = (task_t*) malloc(sizeof(task_t));
    ti->id = task_id_counter;
    ti->node_in_dpst = new_task_node;

    // set new task's belong_to_finish
    if(current_task->current_finish != NULL){
      ti->belong_to_finish = current_task->current_finish;
    }
    else{
      ti->belong_to_finish = current_task->belong_to_finish;
    }
    
    new_task_data->ptr = (void*) ti;

  // D. Update global variable
    task_id_counter ++;
}


static int ompt_ta_initialize(ompt_function_lookup_t lookup, int device_num,
                              ompt_data_t *tool_data) {
                          
  ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
  if (ompt_set_callback == NULL) {
    std::cerr << "Could not set callback, exiting..." << std::endl;
    std::exit(1);
  }

  ompt_get_task_info = (ompt_get_task_info_t) lookup("ompt_get_task_info");
  if(ompt_get_task_info == NULL){
    std::cerr << "Could not get task info, exiting..." << std::endl;
    std::exit(1);
  }

  SET_CALLBACK(task_create);
  SET_CALLBACK(parallel_begin);
  SET_CALLBACK(implicit_task);
  SET_CALLBACK(sync_region);
  SET_CALLBACK(parallel_end);

  return 1; // success
}


static void ompt_ta_finalize(ompt_data_t *tool_data) 
{
  printDPST();

  // // taskwait reachability test
  // assert(precede_dpst_index(12,17));
  // assert(precede_dpst_index(15,17));
  // assert(precede_dpst_index(9,17));

  // // General reachability test
  // assert(!precede_dpst_index(12,20));
  // assert(precede_dpst_index(2,15));
  // assert(!precede_dpst_index(15,19));

}


extern "C" ompt_start_tool_result_t *
ompt_start_tool(unsigned int omp_version, const char *runtime_version) {
  static ompt_start_tool_result_t start_tool_result = {
      &ompt_ta_initialize, &ompt_ta_finalize, {0}};
  std::cout << "OMPT Task Analsis Enabled" << std::endl;
  return &start_tool_result;
}