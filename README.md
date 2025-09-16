# OS-Assignment-2

## Team Member Names
- Akshit K Bansal (2024058)  
- Dhruv Agarwal (2024187)  

**Group Number:** 18  

---

## Contributions
- Approach was discussed and work was equally divided between the 2 group members.  
- All the files with code written were checked and written by both of the members.  

---

## Overview
This project implements a simple Unix shell in C, fulfilling the assignment requirements.  
The shell continuously waits for user input, parses commands, executes them (with or without pipes), and maintains a command history.  

---

## Core Components

1. **`launch()` method**  
   - Creates a child process using `fork()`.  
   - In the child, uses `execvp()` to execute the command along with its arguments.  
   - The parent process waits with `waitpid()` for child termination.  

2. **Input Parsing**  
   - User input is tokenized by spaces to separate command and arguments.  
   - If pipes (`|`) are present, the input is further split into stages.  

3. **Pipes Handling** 
   - For each stage in a pipeline, `pipe()` is used to connect `stdout` of one process to `stdin` of the next.  
   - `dup2()` is used to redirect file descriptors appropriately before `execvp()`.  

4. **History Management**  
   - Each command is stored in an in-memory history list.  
   - Typing `history` prints previously entered commands.  
   - On exit, a detailed history is shown with: PID, start time, end time, and execution duration.  

5. **Basic Structure**  
   - `main()` → infinite loop reading input, checking for built-in commands (e.g., `history`, `exit`), otherwise passing input to parser/launch.  
   - `parse_input()` → handles raw user input string gracefuly.  
   - `launch()` → executes each command.  
   - `display_short_history()` / `display_detailed_history()` → manages short and detailed history.  

---

## Unsupported Commands

The following commands are not supported in this shell, with reasons:  

- **`cd` (change directory)**  
  Reason: `cd` is a shell built-in, not an external binary. Our shell only executes external programs with `execvp()`, so directory changes cannot persist in the parent shell process.  

- **Commands with quotes or backslashes**  
  Reason: Parsing is restricted to space-separated words, as specified in the assignment.  

---

## Github Repo Link
[https://github.com/Dhruv-IIITD-187/OS-Assignment-2](https://github.com/Dhruv-IIITD-187/OS-Assignment-2)
