# Multi-Client Chat Server (C++ / Linux)

## **Description**
A high-performance TCP chat server and client built in C++ for Linux.  
Supports multiple concurrent clients using **threads**, **poll**, and can be extended to **epoll** for large-scale applications.  
Designed to demonstrate **system programming, network programming, concurrency**, and **Linux development skills** — perfect for interview discussion and resume projects.  

---

## **Features**
- Multi-client chat using TCP sockets  
- Sequential client IDs for clarity (`Client 1`, `Client 2`…)  
- Concurrent clients handled with **threads** (or scalable with `poll` / `epoll`)  
- Thread-safe client management using **mutexes**  
- Clean, formatted server output  
- Easy to build with **Makefile** or **CMake**  

---

## **Build Instructions**

### **Using Makefile**
```bash
make        # builds both server and client
make server # builds only server
make client # builds only client
make clean  # removes executables
```

### **Using CMake**
1. Create a build directory and navigate into it:
```bash
mkdir build
cd build
```
2. Generate build files with CMake:
```bash
cmake ..
```
3. Compile the project:
```bash
make
```
4. Run the server:
```bash
./server
```
5. Run clients in separate terminals:
```bash
./client
```
