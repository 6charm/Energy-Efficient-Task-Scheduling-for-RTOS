Developed on a Debian VM running Linux 6.1
1. Build the shell program via `make`. Then run via `./aSh`.
2. `./aSh` runs the custom shell process as the foreground process group and controller of the default terminal.
3. A REPL waits for user input via terminal and initiates string tokenisation.
4. Parser generates a tree-representation of command-line strings, similar to compiler ASTs.
5. Tree-traversal is performed to execute the command-lines via standard Linux IPC mechanisms. 

Note: 
- **aSh** borrows a implementation ideas (i.e. REPL setup, foreground pgid) from **[Eddie Kohler's sh61](https://github.com/cs61/cs61-f22-psets/blob/main/pset5/sh61.cc)**.
-  **aSh** does not detect invalid user commands. In such cases, eventually an assertion fails or a segfault occurs.
