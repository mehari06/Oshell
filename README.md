# OSHELL - Custom C Shell Project

**Group Number:** G-1

## Group Members
Please fill in your details below:
| Name | ID |
|------|----|
| [Member 1 Name] | [Member 1 ID] |
| [Member 2 Name] | [Member 2 ID] |
| [Member 3 Name] | [Member 3 ID] |

---

## Project Description
`oshell` is a feature-rich, custom shell implementation written in C. It mimics valid `bash` behavior for a subset of commands and features. It is designed to be a lightweight, robust command-line interpreter that supports both interactive user sessions and batch script processing. The project emphasizes modular design, error handling, and standard shell functionality.

Key design principles:
*   **Modular Architecture:** Code is split into logical modules (Parser, Executor, Input, Errors) for maintainability.
*   **Robustness:** Handles errors gracefully without crashing (e.g., incorrect syntax, missing files).
*   **Compliance:** Compiles with `gcc -Wall -Wextra -Werror` and standards.

---

## Features Implemented

### 1. Built-in Commands
The shell includes custom implementations for the following built-ins:
*   **`exit [code]`**: Terminates the shell with an optional status code.
*   **`cd [dir]`**: Changes the current directory.
    *   Supports `cd` (no args) to go to HOME.
    *   Supports `cd -` to toggle between current and previous directory.
*   **`path [dir1] [dir2] ...`**: Manages the search path for executables.
    *   `path` (no args): Displays the current search path.
    *   `path /bin /usr/bin`: Sets search path.
*   **`alias [name=value]`**: Manages aliases.
    *   `alias`: Lists all aliases.
    *   `alias name=value`: Sets an alias.
    *   Recursive alias expansion is supported.
*   **Environment Variables**:
    *   `env`: Lists all environment variables.
    *   `setenv [key] [value]`: Sets an environment variable.
    *   `unsetenv [key]`: Removes an environment variable.

### 2. Command Execution
*   Executes external programs found in the standard paths (e.g., `/bin`, `/usr/bin`) or via absolute/relative paths.
*   **Variable Expansion**: Supports `$VAR` expansion, `$$` (PID), and `$?` (Last Exit Code).

### 3. Redirection and Piping
*   **Output Redirection (`>`)**: Redirects standard output to a file.
    *   Example: `ls > file.txt`
*   **Piping (`|`)**: Connects the output of one command to the input of another.
    *   Example: `ls | grep .c`

### 4. Control Flow & Logic
*   **Sequential Execution (`;`)**: Run multiple commands in sequence.
    *   Example: `echo hello; echo world`
*   **Logical AND (`&&`)**: Run second command only if first succeeds.
    *   Example: `true && echo success`
*   **Logical OR (`||`)**: Run second command only if first fails.
    *   Example: `false || echo failure_recovered`

### 5. Background Execution
*   **Background Processing (`&`)**: functionality to parse commands terminating with `&` (Note: Full background process management is simulated or implementation dependent).

### 6. Modes of Operation
*   **Interactive Mode**: Displays a prompt (`$ `) and accepts user input.
*   **Batch Mode**: Reads commands from a file provided as an argument.
    *   Example: `./oshell script.txt`

---

## compilation Instructions

### Prerequisites
*   Linux Environment (Ubuntu/Debian recommended)
*   GCC Compiler
*   Make

### How to Compile
1.  Open a terminal in the project directory.
2.  Run the `make` command:
    ```bash
    make
    ```
    This will compile all source files in `src/` and link them into an executable named `oshell`.
    *Flags used:* `-Wall -Wextra -Werror -D_GNU_SOURCE`

### How to Clean
To remove compiled object files and the executable:
```bash
make clean
```

---

## Execution Instructions

### Interactive Mode
Start the shell by running:
```bash
./oshell
```
You will see the `$` prompt. Type commands as you would in a normal shell.

### Batch Mode
To execute commands from a file:
```bash
./oshell cmd.txt
```
The shell will execute each line in `cmd.txt` and exit.

### Testing
Use the provided `cmd.txt` or create your own test scripts.
```bash
echo "ls -l" > test.sh
./oshell test.sh
```

---

## File Structure
*   `Makefile`: Build script.
*   `src/`: Source code files (`main.c`, `parser.c`, `execute.c`, `input.c`, `errors.c`).
*   `include/`: Header files.
*   `README.md`: This documentation.

---

