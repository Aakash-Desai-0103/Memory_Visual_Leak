#ifndef GC_SIMULATOR_H
#define GC_SIMULATOR_H
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------
// Data Structures
// ------------------------------------------------------

typedef struct Object Object;
typedef struct RefNode RefNode;

struct RefNode {
    Object *to;
    RefNode *next;
};

struct Object {
    int id;
    char *name;     // dynamically allocated string (must be freed)
    int marked;
    RefNode *refs;
    Object *next;
};

// ------------------------------------------------------
// Global Variables
// ------------------------------------------------------

extern Object *heap_head;
extern Object *roots[50];
extern int root_count;
extern int total_objects_allocated;
extern int total_objects_freed;
extern size_t total_memory_allocated;
extern size_t total_memory_freed;
void force_leak_scenario(int scenario_id);


// ------------------------------------------------------
// Function Prototypes
// ------------------------------------------------------

int get_menu_choice(void);
Object *create_object(int id, const char *name);
Object *find_object_by_name(const char *name);
void add_reference(Object *from, Object *to);
void clear_references(Object *from);
void gc_mark(Object *root);
void gc_mark_all(Object **roots, int root_count);
void gc_sweep(void);
void run_gc(Object **roots, int root_count);
void write_dot(const char *filename, Object **roots, int root_count);
void final_cleanup(void);
void reset_simulator_state();
void export_snapshot(void);



#endif
