#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#include "my402list.h"


sigset_t set;
struct CommandLineParams CLP;
int IsTraceDrivenMode;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
My402List my402list_Q2;
My402List my402list_Q1;
FILE* fp;
int NumberOFPacketsInTraceDrivenMode;
int Completed;
int CurrentBucketSize;
int PacketID;
struct FinalStatisticsData FDS;
int NumberOfthreadsDied;
int GlobalTokens;
int PacketDropped;
int wasCntrlCPressed;
pthread_t thread[4];
struct timeval LastarrivalTimeOfPacket;
struct timeval CurrentarrivalTimeOfPacket;
int ActualPacketsServed;
int TokenDropped;
double SqofTimeSpentSystem;


struct FinalStatisticsData{
	struct timeval start;
	struct timeval end;
	double timeSpentAtQ1;
	double timeSpentAtQ2;
	double timeSpentAtS1;
	double timeSpentAtS2;
	double timeSpentinSystem;
	double TotalInterArrivalTime;
};


struct Packet{
	int id;
	double serviceTime;
	int numberOfTokens;
	double InterArrivalTime;
	struct timeval ArrivesTS;
	struct timeval EntersQ1TS;
	struct timeval LeavesQ1TS;
	struct timeval EntersQ2TS;
	struct timeval LeavesQ2TS;
	struct timeval ServiceBeginTS;
	struct timeval ServiceEndTS;
		
};

struct Packetdata{
	double inter_arrival_time;
	int number_of_tokens;
	double service_time;
};

struct CommandLineParams{

	double tokenArrivalLate_r;
	double packetArrivalLate_lambda;
	int bucketDepth_B;
	int requiredPacketsForTransmission_P;
	double packetServiceRate_mu;
	int numberOfPacktes_n;
	char traceSpecificationFile_tsfile[1024];
};
