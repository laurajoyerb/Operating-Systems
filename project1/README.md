# Project 1
## EC440: Intro to Operating Systems
##### Laura Joy Erb

### Meta Characters
The shell can interpret '<', '>', '|', and '&' meta characters for file input/output redirection as well as piping of commands together.

The input can be redirected as long as there is at least one space between the commands. For example, both "cat< nums.txt" and "cat <nums.txt" are valid, but the shell will not be able to process "cat<nums.txt". The output redirection follows the same restrictions. Only the first argument can have its input redirected, and only the last argument can have its output redirected.

The ampersand allows the shell to run processes in the background and immediately re-prompts the user without waiting for the process to finish.

Piping can be executed for up to four separate arguments. If the shell reads more than 3 pipe characters, it will return that is unable to execute the command.

### References

#### General
https://www.geeksforgeeks.org/making-linux-shell-c/

#### Piping
http://www.sarathlakshman.com/2012/09/24/implementation-overview-of-redirection-and-pipe-operators-in-shell
https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
https://stackoverflow.com/questions/20187734/c-pipe-to-stdin-of-another-program
https://gist.github.com/mplewis/5279108

#### Background Processing
https://stackoverflow.com/questions/7171722/how-can-i-handle-sigchld

#### Redirection
https://stackoverflow.com/questions/9084099/re-opening-stdout-and-stdin-file-descriptors-after-closing-them
