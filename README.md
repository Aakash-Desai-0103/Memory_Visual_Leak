# Memory Leak Visual Demo – Garbage Collection Simulation (C)

A mini-project that **simulates a Mark-and-Sweep Garbage Collector** to visualize how **unreachable objects accumulate and cause memory pressure** in managed runtimes.
Built entirely in **C**, this project models heap memory, object references, and garbage collection using classic data structures.

---

## 📌 Problem Statement

To visualize how unreachable objects accumulate in memory and cause leaks, and how a **Garbage Collector (Mark-and-Sweep)** identifies and frees them using reachability analysis.

---

## 🧠 Key Concepts Demonstrated

* Heap memory simulation
* Object reference graphs
* Root-based reachability
* Mark-and-Sweep garbage collection
* Memory leak scenarios and cleanup
* Visualization using Graphviz

---

## ⚙️ Features

* Create dynamic heap objects
* Create directed references between objects
* Mark objects as GC roots
* Run **Mark-and-Sweep GC**
* Visualize heap as a graph (reachable vs unreachable)
* Force predefined memory leak scenarios
* Export heap snapshots to text files
* Accurate memory allocation & deallocation tracking

---

## 🏗️ Data Structures Used

### 1. Directed Graph (Object Reference Graph)

* Represents heap objects as nodes
* References as directed edges
* Supports cycles (important for GC simulation)
* Implemented using adjacency lists

### 2. Singly Linked List

* Maintains the heap object list
* Dynamic allocation using `malloc()` / `free()`
* O(1) insertion at head

### 3. Stack (Implicit – Call Stack)

* Used during recursive marking
* Enables **Depth-First Search (DFS)** for reachability

---

## 🗑️ Garbage Collection Algorithm

**Mark-and-Sweep**

1. **Mark Phase**

   * Traverse from root objects
   * Mark all reachable objects (DFS)
2. **Sweep Phase**

   * Traverse heap list
   * Free all unmarked (unreachable) objects
   * Update memory statistics

---

## 📂 Project Structure

```
Memory_Leak_Visual_Demo/
│
├── Source Files
│   ├── main.c
│   ├── gc_functions.c
│   └── gc_simulator.h
│
├── Executable
│   └── memleak_visual_demo.exe
│
├── GraphViz Output (Generated)
│   ├── heap_visual.dot
│   └── heap_visual.png
│
├── Snapshots (Generated)
│   └── snapshot_1.txt
│
├── Documentation
│   ├── Memory_Leak_Visual_Demo.pptx
│   └── Memory_Leak_Visual_Demo_Report.pdf
```

---

## ▶️ How to Run

### Compile

```bash
gcc main.c gc_functions.c -o memleak_visual_demo
```

### Run

```bash
./memleak_visual_demo
```

---

## 📊 Visualization Legend

* 🟦 **Root Object** – Light blue double circle
* 🟩 **Reachable Object** – Green
* 🟥 **Unreachable Object** – Red (collected by GC)

Graph visualization is generated using **Graphviz (.dot → .png)**.

---

## 🧪 Sample Scenario Demonstrated

* Objects created: `A, B, C, D, E`
* References:

  * `A → B → C` (reachable)
  * `D → E` (unreachable island)
* Root: `A`

**Result after GC**

* `D` and `E` detected as unreachable
* Memory freed correctly
* `A, B, C` preserved

---

## 📈 Memory Tracking

* Total objects created
* Total objects freed
* Bytes allocated & freed
* Current memory usage

All statistics are computed using **real allocation sizes**, including object metadata and references.

---

## 🧹 Clean Exit

On program termination, all remaining objects are freed using `final_cleanup()` to ensure **zero memory leaks**.

---

## 📚 Learning Outcomes

* Practical understanding of garbage collection
* Why reference counting fails with cycles
* How reachability-based GC works internally
* Use of graphs to model runtime memory
* Safer dynamic memory management in C

---

## 👨‍💻 Team

* **Aakash Desai** (PES1UG24CS006)
* **Aarush Muralidhara** (PES1UG24CS010)
* **Abhay Balakrishna** (PES1UG24CS012)

---

## 🏁 Conclusion

This project successfully demonstrates how **unreachable objects cause memory leaks** and how a **Mark-and-Sweep Garbage Collector** identifies and reclaims memory.
The visualization and detailed memory tracking make abstract GC concepts concrete and easy to understand.

---

⭐ If you found this helpful or educational, feel free to star the repository!
