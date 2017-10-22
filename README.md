Warmup Assignment #2


Multi-threading - Token Bucket Emulation in C

The token bucket has a capacity (bucket depth) of Btokens. Tokens arrive into the token bucket according to an unusual arrival process where the inter-arrival time between two consecutive tokens is 1/r. We will call r the token arrival rate (although technically speaking, it's not exactly the token arrival rate; please understand that this is quite different from saying that the tokens arrive at a constant rate of r). Extra tokens (overflow) would simply disappear if the token bucket is full. A token bucket, together with its control mechanism, is referred to as a token bucket filter.
Packets arrive at the token bucket filter according to an unusual arrival process where the inter-arrival time between two consecutive packets is 1/lambda. We will call lambda the packet arrival rate (although technically speaking, it's not exactly the packet arrival rate; please understand that this is quite different from saying that the packets arrive at a constant rate of lambda). Each packet requires P tokens in order for it to be eligiable for transmission. (Packets that are eligiable for transmission are queued at the Q2 facility.) When a packet arrives, if Q1 is not empty, it will just get queued onto the Q1 facility. Otherwise, it will check if the token bucket has P or more tokens in it. If the token bucket has P or more tokens in it, P tokens will be removed from the token bucket and the packet will join the Q2 facility (technically speaking, you are required to first add the packet to Q1 and timestamp the packet, remove the P tokents from the token bucket and the packet from Q1 and timestamp the packet, before moving the packet into Q2), and wake up the servers in case they are sleeping. If the token bucket does not have enough tokens, the packet gets queued into the Q1 facility. (Please note that, in this case, you do not have to check if there is enough tokens in the bucket so you can move the packet at the head of Q1 into Q2 and you need to understand why you do not need to perform such a check.) Finally, if the number of tokens required by a packet is larget than the bucket depth, the packet must be dropped (otherwise, it will block all other packets that follow it).

The transmission facility (denoted as S1 and S2 in the above figure and they are referred to as the "servers") serves packets in Q2 in the first-come-first-served order and at a service rate of mu per second. When a server becomes available, it will dequeue the first packet from Q2 and start transmitting the packet. When a packet has received 1/museconds of service, it leaves the system. You are required to keep the servers as busy as possible.

When a token arrives at the token bucket, it will add a token into the token bucket. If the bucket is already full, the token will be lost. It will then check to see if Q1 is empty. If Q1 is not empty, it will see if there is enough tokens to make the packet at the head of Q1 be eligiable for transmission (packets in Q1 in also served in the first-come-first-served order). If it does, it will remove the corresponding number of tokens from the token bucket, remove that packet from Q1 and move it into Q2, and wake up the servers in case they are sleeping. It will then check the packet that is now at the head of Q1 to see if it's also eligiable for transmission, and so on.

Technically speaking, the "servers" are not part of the "token bucket filter". Nevertheless, it's part of this assignment to emulation the severs because the servers are considered part of the "system" to be emulated.

Our system can run in only one of two modes.

Deterministic	 : 	In this mode, all inter-arrival times are equal to 1/lambda seconds, all packets require exactly P tokens, and all service times are equal to 1/mu seconds (rounded to the nearest millisecond). If 1/lambda is greater than 10 seconds, please use an inter-arrival time of 10 seconds. If 1/mu is greater than 10 seconds, please use an service time of 10 seconds.
 
Trace-driven	 : 	In this mode, we will drive the emulation using a trace specification file. Each line in the trace file specifies the inter-arrival time of a packet, the number of tokens it need in order for it to be eligiable for transmission, and its service time. (Please note that in this mode, it's perfectly fine if an inter-arrival time or a service time is greater than 10 seconds.)
Your job is to emulate the packet and token arrivals, the operation of the token bucket filter, the first-come-first-served queues Q1 and Q2, and servers S1 and S2. You also must produce a trace of your emulation for every important event occurred in your emulation. Please see more details below for the requirements.

You must use:

one thread for packet arrival
one thread for token arrival
one thread for each server
You must not use one thread for each packet.
In addition, you must use at least one mutex to protect Q1, Q2, and the token bucket. (It is recommended that you use exactly one mutex to protect Q1, Q2, and the token bucket.)

Finally, Q1 and Q2 must have infinite capacity (i.e., you should use My420List from warmup assignment #1 to implement them and not use arrays).

We will not go over the slides for this assignment in class. Although it's important that you are familiar with it. Please read it over. If you have questions, please e-mail the instructor.

Compiling : Please use a Makefile so that when the grader simply enters:
make warmup2
An executable named warmup2 is created (minor variation is permitted if you document it). Please make sure that your submission conforms to other general compilation requirements and README requirements. CommandlineThe command line syntax (also known as "usage information") for warmup2 is as follows:
warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]
Square bracketed items are optional. You must follow the UNIX convention that commandline options can come in any order. (Note: a commandline option is a commandline argument that begins with a - character in a commandline syntax specification.) Unless otherwise specified, output of your program must go to stdout and error messages must go to stderr.
The lambda, mu, r, B, and P parameters all have obvious meanings (according to the description above). The -n option specifies the total number of packets to arrive. If the -t option is specified, tsfile is a trace specification file that you should use to drive your emulation. In this case, you should ignore the -lambda, -mu, -P, and -n commandline options and run your emulation in the trace-driven mode. You may assume that tsfile conforms to the tracefile format specification. (This means that if you detect an error in this file, you may simply print an error message and call exit(). There is no need to perform error recovery.) If the -t option is not used, you should run your emulation in the deterministic mode.

The default value (i.e., if it's not specified in a commandline option) for lambda is 1 (packets per second), the default value for mu is 0.35 (packets per second), the default value for r is 1.5 (tokens per second), the default value for B is 10 (tokens), the default value for P is 3 (tokens), and the default value for num is 20 (packets). B, P, and num must be positive integers with a maximum value of 2147483647 (0x7fffffff). lambda, mu, and r must be positive real numbers.

If 1/r is greater than 10 seconds, please use an inter-token-arrival time of 10 seconds.

Running Your Code and Program OutputThe emulation should go as follows. At emulation time 0, all 4 threads (arrival thread, token depositing thread, and servers S1 and S2 threads) got started. The arrival thread would sleep so that it can wake up at a time such that the inter-arrival time of the first packet would match the specification (either according to lambda or the first record in a tracefile). At the same time, the token depositing thread would sleep so that it can wake up every 1/r seconds and would try to deposit one token into the token bucket. The actual arrival time of the first packet p1 is denoted as time T1, the actual arrival time of the 2nd packet p2 is denoted as time T2, and so on.
As a packet or a token arrives, or as a server becomes free, you need to follow the operational rules of the token bucket filter. Since we have four threads accessing shared data structures, you must use the tricks you learned from Chapter 2 related lectures. Please also check out the slides for this assignment for the skeleton code for these threads.

You are required to produce a detailed trace for every important event occurred during the emulation. Each line in the trace must correspond to one of the following situations:

If a packet is served by a server (server S1 is assumed below for illustration), there must be exactly 7 output lines that correspond to this packet. They are:
p1 arrives, needs 3 tokens, inter-arrival time = 503.112ms p1 enters Q1 p1 leaves Q1, time in Q1 = 247.810ms, token bucket now has 0 token p1 enters Q2 p1 leaves Q2, time in Q2 = 0.216ms p1 begins service at S1, requesting 2850ms of service p1 departs from S1, service time = 2859.911ms, time in system = 3109.731ms
Please note the following:
The value printed for "inter-arrival time" must equal to the timestamp of the "p1 arrives" event minus the timestamp of the "arrives" event for the previous packet.
The value printed for "time in Q1" must equal to the timestamp of the "p1 leaves Q1" event minus the timestamp of the "p1 enters Q1" event.
The value printed for "time in Q2" must equal to the timestamp of the "p1 leaves Q2" event minus the timestamp of the "p1 enters Q2" event.
The value printed for "requesting ???ms of service" must be the requested service time (which must be an integer) of the corresponding packet.
The value printed for "service time" must equal to the timestamp of the "p1 departs from S1" event minus the timestamp of the "p1 begins service at S1" event (and it should be larger than the requested service time printed for the "begin service" event).
The value printed for "time in system" must equal to the timestamp of the "p1 departs from S1" event minus the timestamp of the "p1 arrives" event;
If a packet is dropped, you must print:
p1 arrives, needs 3 tokens, inter-arrival time = 503.112ms, dropped
Please note that the value printed for "inter-arrival time" must equal to the timestamp of the "p1 arrives" event minus the timestamp of the "arrives" event for the previous packet.
If <Cntrl+C> is pressed by the user, you must print the following (and print a '\n' before it to make sure that it lines up with all the other trace printouts):
SIGINT caught, no new packets or tokens will be allowed
Please understand that in order for the above to get printed correctly in a trace printout, using a signal handler to catch signals may not work. You are strongly advised to use a separate SIGINT-catching thread and uses sigwait().
If a packet is removed when it's in Q# (Q1 or Q2) because <Cntrl+C> is pressed by the user, you must print:
p1 removed from Q#
If a token is accepted, you must print:
token t1 arrives, token bucket now has 1 token
If a token is dropped, you must print:
token t1 arrives, dropped
When you are ready to start your emulation, you must print:
emulation begins
When you are ready to end your emulation, you must print:
emulation ends
All the numeric values above are made up. You must replace them with the actual packet number, actual number of tokens required, actual server number, measured inter-arrival time, measured time spent in Q1, actual number of tokens left behind when a packet is moved into Q2, measured time spent in Q2, measured service time, and measured time in the system.
The output format of your program must satisfy the following requirements.

You must first print all the emulation paramters. Please see the sample printout for what the output must look like.
Whenever a token arrives, you must assign a number to it, and add it to the token bucket. You must then print its arrival time, the fact that it has arrived, and the number of tokens in the the token bucket. Please see the sample printout for what the output must look like.
Whenever a packet arrives, you must assign a number to it. You must then print its arrival time, the fact that it has arrived, the number of tokens it needs for transmission, and the time between its arrival time and the arrival time of the previous packet. Please see the sample printout for what the output must look like.
You then must append this packet onto Q1. Afterwards, you must then print the time this packet entered Q1 and the fact that it has entered Q1. Please see the sample printout for what the output must look like.

Later on, when this packet leaves Q1, it removes the correct number of tokens from the token bucket. You must then print the time this packet leaves Q1, the fact that it has left Q1, the amount of time it spent in Q1, and the number of tokens in the the token bucket. Please see the sample printout for what the output must look like.

You must then append this packet onto Q2. Afterwards, you must then print the time this packet entered Q2 and the fact that it has entered Q2. Please see the sample printout for what the output must look like.

Later on, when this packet leaves Q2 and enters the server, you must then print which server the packet entered, the time the packet begin service, the fact that it has begun service, and the amount of time it spent in Q2. Please see the sample printout for what the output must look like.

When emulation ends, you must print all the necessary statistics.


