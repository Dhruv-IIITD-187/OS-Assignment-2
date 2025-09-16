# OS-Assignment-2

## Team Members
- Akshit K Bansal (2024058)
- Dhruv Agarwal (2024187)

**Group Number:** 18

---

## Contributions
We discussed the overall approach and divided the work equally between us.  
All the code files were checked and written by both members.

---

## Overview
This project is a simple Unix shell written in C.  
The shell keeps running in a loop, takes user commands, parses them, executes (with or without pipes), and keeps a history of the commands.

---

## Core Components

1. **launch() method**  
   - Uses `fork()` to create a child process.  
   - Child runs the command using `execvp()`.  
   - Parent waits for the child with `waitpid()`.  

2. **Input Parsing**  
   - Commands are split by spaces into command + arguments.  
   - If a pipe `|` is found, the input is broken into separate stages.  

3. **Pipes**  
   - `pipe()` is used to connect output of one process to input of the next.  
   - `dup2()` is used to set up the correct file descriptors.  

4. **History**  
   - Each entered command is stored in memory.  
   - Typing `history` shows all past commands.  
   - On exit, a detailed history is shown with PID, start time, end time and duration.  

5. **Structure**  
   - `main()` → infinite loop reading input.  
   - `parse()` → breaks input into tokens.  
   - `launch()` → executes the commands.  
   - `add_to_history()` / `show_history()` → manage history.  

---

## Unsupported Commands

- **cd (change directory)**  
  `cd` is a built-in shell command, not a binary. Since our shell uses `execvp()` to run external programs, changing directory this way would not affect the parent shell.  

- **Commands with quotes or backslashes**  
  Our parser only supports simple space-separated input, so things like `echo "hello world"` won’t work.  

---

## Github Repo Link
https://github.com/Dhruv-IIITD-187/OS-Assignment-2
