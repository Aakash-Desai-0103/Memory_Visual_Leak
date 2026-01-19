#include "gc_simulator.h"

int main() {
    int choice, id_counter = 1;
    char name1[100], name2[100]; // use temporary local buffers for user input
    int c; // for clearing buffer

    while (1) {
        printf("\n===== MEMORY LEAK VISUAL DEMO =====\n");
        printf("1. Create Object\n");
        printf("2. Create Reference\n");
        printf("3. Mark Object as Root\n");
        printf("4. Run Garbage Collector\n");
        printf("5. Visualize Heap (DOT)\n");
        printf("6. Show Memory Status\n");
        printf("7. Exit\n");
        printf("8. Force Leak Scenario\n");
        printf("9. Export Snapshot\n");

        // Input safely (handles non-numeric & out-of-range)
        choice = get_menu_choice();

        switch (choice) {
            // ------------------------------------------------------
            // Option 1: Create Object
            // ------------------------------------------------------
            case 1:
                printf("Enter object name: ");
                if (scanf("%99s", name1) == 1) {
                    create_object(id_counter++, name1); // dynamic allocation inside
                    while ((c = getchar()) != '\n' && c != EOF) {} // clear buffer
                }
                break;

            // ------------------------------------------------------
            // Option 2: Create Reference
            // ------------------------------------------------------
            case 2:
                printf("Enter source object name: ");
                if (scanf("%99s", name1) != 1) break;
                printf("Enter target object name: ");
                if (scanf("%99s", name2) != 1) break;
                {
                    Object *from = find_object_by_name(name1);
                    Object *to = find_object_by_name(name2);

                    if (!from && !to)
                        printf("Error: Both source and target objects do not exist.\n");
                    else if (!from)
                        printf("Error: Source object '%s' does not exist.\n", name1);
                    else if (!to)
                        printf("Error: Target object '%s' does not exist.\n", name2);
                    else
                        add_reference(from, to);
                }
                while ((c = getchar()) != '\n' && c != EOF) {} // clear buffer
                break;

            // ------------------------------------------------------
            // Option 3: Mark Object as Root
            // ------------------------------------------------------
            case 3:
                printf("Enter object name to mark as root: ");
                if (scanf("%99s", name1) == 1) {
                    Object *root_obj = find_object_by_name(name1);
                    if (!root_obj) {
                        printf("Error: Object '%s' not found. Create it first.\n", name1);
                        break;
                    }

                    // Prevent duplicate root marking
                    int already_root = 0;
                    for (int i = 0; i < root_count; i++) {
                        if (roots[i] == root_obj) {
                            already_root = 1;
                            break;
                        }
                    }

                    if (already_root)
                        printf("Object '%s' is already a root.\n", name1);
                    else {
                        roots[root_count++] = root_obj;
                        printf("Object '%s' marked as root.\n", name1);
                    }
                }
                while ((c = getchar()) != '\n' && c != EOF) {} // clean input buffer
                break;

            // ------------------------------------------------------
            // Option 4: Run Garbage Collector
            // ------------------------------------------------------
            case 4:
                run_gc(roots, root_count);
                break;

            // ------------------------------------------------------
            // Option 5: Visualize Heap (DOT)
            // ------------------------------------------------------
            case 5:
                printf("Marking heap for visualization...\n");
                gc_mark_all(roots, root_count);
                write_dot("heap_visual.dot", roots, root_count);
                break;

            // ------------------------------------------------------
            // Option 6: Show Memory Status
            // ------------------------------------------------------
            case 6:
                printf("\n--- Memory Status ---\n");
                printf("Total objects created: %d\n", total_objects_allocated);
                printf("Total objects freed:   %d\n", total_objects_freed);
                printf("Total memory allocated: %zu bytes\n", total_memory_allocated);
                printf("Total memory freed:     %zu bytes\n", total_memory_freed);
                printf("Current memory in use:  %zu bytes\n",
                       total_memory_allocated - total_memory_freed);
                break;

            // ------------------------------------------------------
            // Option 7: Exit
            // ------------------------------------------------------
            case 7:
                final_cleanup();
                printf("\nProgram terminated successfully.\n");
                return 0;
            case 8:
                printf("\n--- Force Leak Scenarios ---\n");
                printf("1. Simple chain leak (A -> B -> C   and   D -> E)\n");
                printf("2. Cyclic leak (A -> B -> C -> A   and   D <-> E)\n");
                printf("3. Long chain + garbage nodes\n");

                printf("Select scenario: ");

                int s;
                if (scanf("%d", &s) == 1) {
                force_leak_scenario(s);
                } else {
                    printf("Invalid scenario number.\n");
                }

                while ((c = getchar()) != '\n' && c != EOF) {}
                break;

            case 9:
                export_snapshot();
                break;

            // ------------------------------------------------------
            // Invalid Option (fallback)
            // ------------------------------------------------------
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
}
