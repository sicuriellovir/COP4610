Project 1    |     Introduction to Operating Systems   |   COP 4610

Team members:  Alec Walther (Github: apwal1), Virginia Sicuriello Level (Github: sicuriellovir), Sean (Yixuan) Liu (Github: Designexplained)

Division of Labor:
Alec Walther (Github: apwal1):
Contributed to Parsing, Prompt, Piping, External command execution, Tilde Expansion, Makefile, tested Built-in functions.

Virginia Sicuriello Level (Github: sicuriellovir):
Contributed to Built-in functions, I/O redirections, tested External command execution.

Sean (Yixuan) Liu (Github: Designexplained)
Contributed to documentation and Built-in functions. 

Contents of the Tar Archive: 

functions.h       This file contains our built in functions such as exit, cd PATH, echo ARGS, and jobs.
parser.c        	This file contains our parser, prompt. external command execution, I/O Redirection, as well as piping, 
pathSearch.c     	This file contains our pathSearch.
tildeExpansion.c	This file contains our tildeExpansion.

Compiling our Executable with Makefile:
We found that our executable must be compiled using gcc with the -std=c99 flag or else Linprog will return a lot of errors.

Bugs Not Fixed Upon Submission:
Some of the programs, such as tail and grep, do not execute properly when piped together with at least two other programs. This Bug only occurs with some programs, but seems to be working with others (such as head, wc, and cat). We have tried fixing our piping function, but this bug is really weird since it has been popping up on an inconsistent basis with some programs but not others. 

Fixed Bugs:
We came across numerous bugs during testing. At first there was the a bug relating with cd that did not display the present working directory correctly. However, that was corrected by fixing our built in function. Another Bug of note we fixed was the wrong count for the number of commands executed on exit. At first we were using an integer called commandSize to count the number of commands executed so that it will be displayed once the User exits. Since that was not working, we added an integer to main in parser.c that got incremented at the end of each loop. Upon exit, that integer was passed into the built in functions and printed before exiting. 
