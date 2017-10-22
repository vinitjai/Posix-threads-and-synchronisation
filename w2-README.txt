
cumentation for Warmup Assignment 2
=====================================

+-------+
| BUILD |
+-------+

Comments: 1) make warmup2

+---------+
| GRADING |
+---------+

Basic running of the code : 100 out of 100 pts

Missing required section(s) in README file : Updated all the required sections.
Cannot compile : All the files compiles properly.
Compiler warnings : -Wall flag is set in Makefile. No Compiler Warnings.
"make clean" : clean label is defined in Makefile. So "make clean" removes all the  object files along with the final executable.
Segmentation faults : No Segmentation faults should be seen.
Separate compilation : Yes. Please see the BUILD field for compilation.
Using busy-wait : busy-wait should not happen.
Handling of commandline arguments:
    1) -n : Handled as mentioned in spec.
    2) -lambda : Handled as mentioned in spec.
    3) -mu : Handled as mentioned in spec.
    4) -r : Handled as mentioned in spec.
    5) -B : Handled as mentioned in spec.
    6) -P : Handled as mentioned in spec.
Trace output :
    1) regular packets: Handled as mentioned in spec.
    2) dropped packets: Handled as mentioned in spec.
    3) removed packets: Handled as mentioned in spec.
    4) token arrival (dropped or not dropped): (Comments?)
Statistics output :
    1) inter-arrival time : Handled as mentioned in spec.
    2) service time : Handled as mentioned in spec.
    3) number of customers in Q1 : Handled as mentioned in spec.
    4) number of customers in Q2 : Handled as mentioned in spec.
    5) number of customers at a server : Handled as mentioned in spec.
    6) time in system : Handled as mentioned in spec.
    7) standard deviation for time in system : Handled as mentioned in spec.
    8) drop probability : Handled as mentioned in spec.
Output bad format : Handled as mentioned in grading guildelines and spec.
Output wrong precision for statistics (should be 6-8 significant digits) : Handled as mentioned in FAQ.
Large service time test : Handled as mentioned in spec.
Large inter-arrival time test : Handled as mentioned in spec.
Tiny inter-arrival time test : Handled as mentioned in spec.
Tiny service time test : Handled as mentioned in spec.
Large total number of customers test : Handled as mentioned in spec.
Large total number of customers with high arrival rate test : Handled as mentioned in spec.
Dropped tokens test : Handled as mentioned in spec.
Cannot handle <Cntrl+C> at all (ignored or no statistics) : Handled as mentioned in spec.
Can handle <Cntrl+C> but statistics way off : Handled as mentioned in spec.
Not using condition variables and do some kind of busy-wait : Using condition variables.
Synchronization check : Handled as mentioned in spec.
Deadlocks : Should not happen. 

+----------------------+
| BUGS / TESTS TO SKIP |
+----------------------+

Is there are any tests in the standard test suite that you know that it's not
working and you don't want the grader to run it at all so you won't get extra
deductions, please list them here.  (Of course, if the grader won't run these
tests, you will not get plus points for them.)

Comments: N/A

+------------------+
| OTHER (Optional) |
+------------------+

Comments on design decisions: 1) I have used struct to store objects in the list
			      2) Proper formatting/indentation
			      3) I have made small functions so that code is readable.
			      4) I have written comments for all the functions in warmup.c.
Comments on deviation from spec: No deviation from spec.
