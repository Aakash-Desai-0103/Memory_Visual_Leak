#include "gc_simulator.h"

// ------------------------------------------------------
// Global variable definitions
// ------------------------------------------------------
Object *heap_head = NULL;
Object *roots[50];
int root_count = 0;
int total_objects_allocated = 0;
int total_objects_freed = 0;
size_t total_memory_allocated = 0;
size_t total_memory_freed = 0;

// ------------------------------------------------------
// Utility: Safe Menu Input
// ------------------------------------------------------
int get_menu_choice() {
    int choice;
    while (1) {
        printf("Enter choice: ");
        if (scanf("%d", &choice) == 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {} // clear buffer

            if (choice < 1 || choice > 9) {
                printf("Invalid choice! Please enter a number between 1 and 9.\n");
                continue;
            }
            return choice;
        }

        printf("Invalid input. Please enter a numeric choice (1–9).\n");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}

// ------------------------------------------------------
// Object Management
// ------------------------------------------------------
Object *create_object(int id, const char *name) {
    if (find_object_by_name(name)) {
        printf("Error: Object '%s' already exists.\n", name);
        return NULL;
    }

    Object *obj = (Object *)malloc(sizeof(Object));
    if (!obj) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    obj->name = malloc(strlen(name) + 1);
    if (!obj->name) {
        printf("Memory allocation failed for object name.\n");
        free(obj);
        exit(1);
    }
    strcpy(obj->name, name);

    obj->id = id;
    obj->marked = 0;
    obj->refs = NULL;
    obj->next = heap_head;
    heap_head = obj;

    total_objects_allocated++;
    total_memory_allocated += sizeof(Object) + strlen(name) + 1;

    printf("Created object '%s' (approx %zu bytes)\n", name, sizeof(Object) + strlen(name) + 1);
    return obj;
}

Object *find_object_by_name(const char *name) {
    Object *cur = heap_head;
    while (cur) {
        if (strcmp(cur->name, name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void add_reference(Object *from, Object *to) {
    if (!from || !to) return;

    if (from == to) {
        printf("Error: Cannot create self-reference for object '%s'.\n", from->name);
        return;
    }

    for (RefNode *r = from->refs; r; r = r->next) {
        if (r->to == to) {
            printf("Warning: Reference from '%s' to '%s' already exists.\n", from->name, to->name);
            return;
        }
    }

    RefNode *node = malloc(sizeof(RefNode));
    if (!node) {
        printf("Memory allocation failed for reference.\n");
        exit(1);
    }

    node->to = to;
    node->next = from->refs;
    from->refs = node;

    total_memory_allocated += sizeof(RefNode);
    printf("Reference created: %s -> %s\n", from->name, to->name);
}

void clear_references(Object *from) {
    RefNode *r = from->refs;
    while (r) {
        RefNode *tmp = r;
        r = r->next;
        total_memory_freed += sizeof(RefNode);
        free(tmp);
    }
    from->refs = NULL;
}

// ------------------------------------------------------
// Garbage Collection Logic
// ------------------------------------------------------
void gc_mark(Object *root) {
    if (!root || root->marked) return;
    root->marked = 1;
    for (RefNode *ref = root->refs; ref; ref = ref->next)
        gc_mark(ref->to);
}

void gc_mark_all(Object **roots, int root_count) {
    for (int i = 0; i < root_count; i++)
        if (roots[i]) gc_mark(roots[i]);
}

void gc_sweep() {
    int collected_count = 0;
    Object **cur = &heap_head;

    while (*cur) {
        if (!(*cur)->marked) {
            Object *unreached = *cur;
            *cur = unreached->next;

            clear_references(unreached);
            printf("[GC] Collecting unreachable object: %s\n", unreached->name);

            total_objects_freed++;
            collected_count++;

            total_memory_freed += sizeof(Object) + strlen(unreached->name) + 1;

            free(unreached->name);
            free(unreached);
        } else {
            (*cur)->marked = 0;
            cur = &((*cur)->next);
        }
    }

    printf("[GC] Cycle complete -> %d object(s) collected.\n", collected_count);
    printf("[GC] Memory freed this cycle: %zu bytes\n", total_memory_freed);
    printf("[GC] Current memory in use: %zu bytes\n\n",
           total_memory_allocated - total_memory_freed);
}

void run_gc(Object **roots, int root_count) {
    if (!heap_head) {
        printf("No objects in heap. Nothing to collect.\n");
        return;
    }

    printf("\nRunning garbage collector...\n");
    printf("Before GC: %zu bytes in use\n", total_memory_allocated - total_memory_freed);

    gc_mark_all(roots, root_count);
    gc_sweep();

    printf("After GC: %zu bytes in use\n\n", total_memory_allocated - total_memory_freed);
}

// ------------------------------------------------------
// Visualization (DOT)
// ------------------------------------------------------
void write_dot(const char *filename, Object **roots, int root_count) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Error: Could not create DOT file.\n");
        return;
    }

    fprintf(f, "digraph MemoryGraph {\n");
    fprintf(f, "  node [shape=circle, style=filled, color=black];\n\n");

    fprintf(f, "  subgraph cluster_legend {\n");
    fprintf(f, "    label=\"Legend\";\n");
    fprintf(f, "    key_alive [label=\"Reachable (green)\", fillcolor=green];\n");
    fprintf(f, "    key_garbage [label=\"Unreachable (red)\", fillcolor=red];\n");
    fprintf(f, "    key_root [label=\"Root (light blue)\", shape=doublecircle, fillcolor=lightblue];\n");
    fprintf(f, "  }\n\n");

    Object *cur = heap_head;
    while (cur) {
        const char *color = cur->marked ? "green" : "red";
        fprintf(f, "  \"%s\" [label=\"%s\", fillcolor=%s];\n", cur->name, cur->name, color);

        for (RefNode *r = cur->refs; r; r = r->next)
            fprintf(f, "  \"%s\" -> \"%s\";\n", cur->name, r->to->name);

        cur = cur->next;
    }

    // roots highlighted
    for (int i = 0; i < root_count; i++)
        if (roots[i])
            fprintf(f, "  \"%s\" [shape=doublecircle, fillcolor=lightblue];\n", roots[i]->name);

    fprintf(f, "}\n");
    fclose(f);

    printf("DOT file generated: %s\n", filename);
    printf("To visualize: dot -Tpng %s -o heap_visual.png\n\n", filename);
}

// ------------------------------------------------------
// Final Cleanup
// ------------------------------------------------------
void final_cleanup() {
    if (!heap_head) {
        printf("\n[Cleanup] No remaining objects to free.\n");
        return;
    }

    printf("\n[Cleanup] Freeing remaining objects...\n");
    Object *cur = heap_head;

    while (cur) {
        Object *next = cur->next;

        clear_references(cur);
        printf("Freeing survivor: %s\n", cur->name);

        total_objects_freed++;
        total_memory_freed += sizeof(Object) + strlen(cur->name) + 1;

        free(cur->name);
        free(cur);

        cur = next;
    }

    heap_head = NULL;
    printf("All memory freed successfully.\n");
}

// ------------------------------------------------------
// Force Leak Scenarios (WITH RESET AFTER COMPLETION)
// ------------------------------------------------------
void force_leak_scenario(int scenario_id) {

    // -------- Reset before starting new scenario --------
    reset_simulator_state();

    printf("\n[Force Leak] Creating scenario %d...\n", scenario_id);

    int scenario_id_counter = 1000;
    Object *scenario_created[128];
    int scenario_obj_count = 0;

    // ------------------------- SCENARIO 1 -------------------------
    if (scenario_id == 1) {
        Object *A = create_object(scenario_id_counter++, "A");
        Object *B = create_object(scenario_id_counter++, "B");
        Object *C = create_object(scenario_id_counter++, "C");
        Object *D = create_object(scenario_id_counter++, "D");
        Object *E = create_object(scenario_id_counter++, "E");

        scenario_created[scenario_obj_count++] = A;
        scenario_created[scenario_obj_count++] = B;
        scenario_created[scenario_obj_count++] = C;
        scenario_created[scenario_obj_count++] = D;
        scenario_created[scenario_obj_count++] = E;

        add_reference(A, B);
        add_reference(B, C);
        add_reference(D, E);

        roots[root_count++] = A;

        printf("[Force Leak] Scenario 1 created.\n");
    }

    // ------------------------- SCENARIO 2 -------------------------
    else if (scenario_id == 2) {
        Object *A = create_object(scenario_id_counter++, "A");
        Object *B = create_object(scenario_id_counter++, "B");
        Object *C = create_object(scenario_id_counter++, "C");

        scenario_created[scenario_obj_count++] = A;
        scenario_created[scenario_obj_count++] = B;
        scenario_created[scenario_obj_count++] = C;

        add_reference(A, B);
        add_reference(B, C);
        add_reference(C, A);

        Object *D = create_object(scenario_id_counter++, "D");
        Object *E = create_object(scenario_id_counter++, "E");

        scenario_created[scenario_obj_count++] = D;
        scenario_created[scenario_obj_count++] = E;

        add_reference(D, E);
        add_reference(E, D);

        roots[root_count++] = A;

        printf("[Force Leak] Scenario 2 (cycle) created.\n");
    }

    // ------------------------- SCENARIO 3 -------------------------
    else if (scenario_id == 3) {
        Object *prev = NULL;
        char name[16];

        for (int i = 0; i < 10; ++i) {
            snprintf(name, sizeof(name), "N%d", i);
            Object *node = create_object(scenario_id_counter++, name);

            scenario_created[scenario_obj_count++] = node;

            if (prev) add_reference(prev, node);
            else roots[root_count++] = node;

            prev = node;
        }

        for (int i = 1; i <= 5; ++i) {
            snprintf(name, sizeof(name), "X%d", i);
            Object *x = create_object(scenario_id_counter++, name);
            scenario_created[scenario_obj_count++] = x;
        }

        printf("[Force Leak] Scenario 3 created.\n");
    }

    else {
        printf("Invalid scenario ID.\n");
        return;
    }

    // -------------------------------------------------------------------
    // MARK PHASE
    // -------------------------------------------------------------------
    gc_mark_all(roots, root_count);

    // -------------------------------------------------------------------
    // REPORT UNREACHABLE OBJECTS
    // -------------------------------------------------------------------
    printf("\n--- Unreachable Objects (Garbage) ---\n");
    int unreachable_count = 0;
    size_t garbage_bytes_est = 0;

    for (int i = 0; i < scenario_obj_count; ++i) {
        Object *o = scenario_created[i];

        if (o && !o->marked) {
            printf(" - %s\n", o->name);
            unreachable_count++;

            garbage_bytes_est += sizeof(Object) + strlen(o->name) + 1;
        }
    }

    if (unreachable_count == 0) {
        printf("No unreachable objects! All nodes are alive.\n");
    } else {
        printf("\nTotal unreachable = %d object(s)\n", unreachable_count);
        printf("Garbage memory (will be freed now) = %zu bytes\n", garbage_bytes_est);
    }

    printf("-------------------------------------\n");

    // -------------------------------------------------------------------
    // SWEEP PHASE — free garbage
    // -------------------------------------------------------------------
    if (unreachable_count > 0)
        gc_sweep();
    else {
        Object *cur = heap_head;
        while (cur) {
            cur->marked = 0;
            cur = cur->next;
        }
    }

    // -------------------------------------------------------------------
    // REPORT MEMORY AFTER SCENARIO
    // -------------------------------------------------------------------
    printf("\n--- Memory Status After Scenario ---\n");
    printf("Total objects created: %d\n", total_objects_allocated);
    printf("Total objects freed:   %d\n", total_objects_freed);
    printf("Total memory allocated: %zu bytes\n", total_memory_allocated);
    printf("Total memory freed:     %zu bytes\n", total_memory_freed);
    printf("Current memory in use:  %zu bytes\n",
           total_memory_allocated - total_memory_freed);
    printf("-------------------------------------\n\n");

    // -------------------------------------------------------------------
    // FINAL RESET — so user can continue normally
    // -------------------------------------------------------------------
    printf("[Force Leak] Scenario completed. Resetting simulator...\n");
    reset_simulator_state();
}

// ------------------------------------------------------
// Reset simulator completely
// ------------------------------------------------------
void reset_simulator_state() {
    Object *cur = heap_head;

    while (cur) {
        Object *next = cur->next;

        clear_references(cur);
        free(cur->name);
        free(cur);

        cur = next;
    }

    heap_head = NULL;

    for (int i = 0; i < root_count; i++)
        roots[i] = NULL;

    root_count = 0;

    total_objects_allocated = 0;
    total_objects_freed = 0;
    total_memory_allocated = 0;
    total_memory_freed = 0;
}

// ------------------------------------------------------
// Export Snapshot (.txt)
// ------------------------------------------------------
void export_snapshot(void) {
    static int snapshot_counter = 1;
    char filename[64];

    sprintf(filename, "snapshot_%d.txt", snapshot_counter++);

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Error: Could not create snapshot file.\n");
        return;
    }

    fprintf(f, "===== HEAP SNAPSHOT =====\n");

    time_t now = time(NULL);
    fprintf(f, "Timestamp: %s\n", ctime(&now));

    fprintf(f, "Total objects created: %d\n", total_objects_allocated);
    fprintf(f, "Total objects freed:   %d\n", total_objects_freed);
    fprintf(f, "Total memory allocated: %zu bytes\n", total_memory_allocated);
    fprintf(f, "Total memory freed:     %zu bytes\n", total_memory_freed);
    fprintf(f, "Current memory in use:  %zu bytes\n\n",
            total_memory_allocated - total_memory_freed);

    fprintf(f, "=== Objects in Heap ===\n");

    Object *cur = heap_head;
    while (cur) {
        fprintf(f, "Object: %s (ID: %d)\n", cur->name, cur->id);

        RefNode *r = cur->refs;
        if (!r)
            fprintf(f, "   No references\n");
        else {
            while (r) {
                fprintf(f, "   -> %s\n", r->to->name);
                r = r->next;
            }
        }

        fprintf(f, "\n");
        cur = cur->next;
    }

    fprintf(f, "======= END OF SNAPSHOT =======\n");
    fclose(f);

    printf("Snapshot exported successfully: %s\n", filename);
}
