enum node_type{
    ROOT,
    FINISH,
    ASYNC,
    FUTURE,
    STEP
};

typedef struct tree_node{
    int index;
    int corresponding_task_id;
    enum node_type this_node_type;
    int depth;
    int number_of_child;
    int is_parent_nth_child;
    int preceeding_taskwait;
    struct tree_node *parent;
    struct tree_node *children_list_head;
    struct tree_node *children_list_tail;
    struct tree_node *next_sibling;
} tree_node;


typedef struct dpst{
    struct tree_node *root;
    struct tree_node *current_step_node;
    int height;
} dpst;

tree_node* newtreeNode();
void printDPST();
tree_node* find_lca(tree_node *node1,tree_node *node2);
tree_node* find_lca_left_child(tree_node *node1,tree_node *node2);
tree_node* get_current_step_node();
tree_node* insert_tree_node(enum node_type nodeType, tree_node *parent);
tree_node* insert_leaf(tree_node *task_node);
void update_node_parent(tree_node *node, tree_node* new_parent);

extern struct dpst DPST;

typedef struct task_t{
  int id;
  int parent_id;
  tree_node* node_in_dpst;
  tree_node* belong_to_finish;
  tree_node* current_finish;
} task_t;

