# Project 1
## EC440: Intro to Operating Systems
##### Laura Joy Erb

### Meta Characters
The shell can interpret '<', '>', '|', and '&' meta characters for file input/output redirection as well as piping of commands together.

The input can be redirected using the meta character <. For example, both "cat< nums.txt", "cat <nums.txt", and "cat<nums.txt" are valid. The output redirection follows the same restrictions. In the case of both input and output redirection with no spaces, the shell will not be able to execute the command (ie, cat<nums.txt>out.txt). Only the first argument can have its input redirected, and only the last argument can have its output redirected.

The ampersand allows the shell to run processes in the background and immediately re-prompts the user without waiting for the process to finish.

Piping can be executed for up to seven separate arguments. If the shell reads more than 6 pipe characters, it will return that is unable to execute the command.

### Challenges
Piping was a big challenge for this assignment. Trying to execute an "infinite" number of pipes was very difficult to do because of the structure of the arrays or arrays of arrays. I had a very hard time trying to parse and organize the arguments properly to be fed into a looping structure to handle the pipes. Indexing of the arguments often caused seg faults, and the complexity of the array structures made handling them very difficult.

Spacing was also difficult. Looking back, I probably would have structured my parsing function differently to accept more than just spaces as delimiters. It made checking for meta characters in every strtok complex and tangled.

Getting the correct sequence of events for forking, piping, and executing also proved difficult. If not done correctly, I would close the wrong files, or open ones where I didn't need them, or I would execvp() in my parent process and quit the shell program.

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
