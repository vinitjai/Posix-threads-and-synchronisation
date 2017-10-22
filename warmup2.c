#include "warmup2.h"

double getDiff(struct timeval Bigger,
	       struct timeval Smaller)
{
	double Time1 = (((double) Bigger.tv_sec)*1000) +
                        (((double) Bigger.tv_usec)/1000);
        double Time2 = (((double) Smaller.tv_sec)*1000) +
                        (((double) Smaller.tv_usec)/1000);
        return (double) Time1-Time2;
}

double PrintTimeDiff()
{
	double startTime = (((double) FDS.start.tv_sec)*1000) +
			(((double) FDS.start.tv_usec)/1000);
	struct timeval now;
	gettimeofday(&now, NULL);
	double currentTime = (((double) now.tv_sec)*1000) +
                        (((double) now.tv_usec)/1000);
	return (double) currentTime-startTime;
}


int ReadAndGetNumberOFPacketsInTraceDrivenMode()
{
	int NOP = 0;
	char buf[1024];
	memset(buf,0,sizeof(buf));
	if(fgets(buf,sizeof(buf),fp) != NULL){
		if(strlen(buf) > 1024){
		    fprintf(stderr,"Error: input file has line length greater than 1024\n");
                    exit(1);
		}
		sscanf(buf, "%d", &NOP);
	}
	if(NOP == 0){
		fprintf(stderr,"Error: input file is not in the right format\n");
		fprintf(stderr,"Usage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");	
		exit(1);
	}
	return  NOP;

}

struct Packetdata ReadAndGetNextDataFromFile()
{
	struct Packetdata pd ;
	memset(&pd,0,sizeof(pd));

	char buf[1024];
	memset(buf,0,sizeof(buf));
	if(fgets(buf,sizeof(buf),fp) != NULL){
	      if(strlen(buf) > 1024){
                    fprintf(stderr,"Error: input file has line length greater than 1024\n");
                  exit(1);
		}

		int i=0;
		char* IAT_b = strtok(buf," \t");
		for(i=0;IAT_b[i] == ' ';i++) { }
                      char* IAT = &IAT_b[i];

		char* NOT_b = strtok(NULL," \t");
		for(i=0;NOT_b[i] == ' ';i++) { }
                      char* NOT = &NOT_b[i];

		char* ST_b = strtok(NULL,"\n");
		for(i=0;ST_b[i] == ' ';i++) { }
                      char* ST = &ST_b[i];


		pd.inter_arrival_time = (strtod(IAT,NULL));
		sscanf(NOT,"%d",&(pd.number_of_tokens));
		pd.service_time = (strtod(ST,NULL));
	}else{
		Completed = 1;
	}

	return pd;
}

double getInterArrivalRate()
{
	return (double) ( ( 1/CLP.packetArrivalLate_lambda) * 1000 );
}

double getInterTokenRate()
{
	return (double)  ( (1/CLP.tokenArrivalLate_r) * 1000 ) ;
}	

double getServiceRate()
{
	return (double) ( (1/CLP.packetServiceRate_mu) * 1000 ) ;
}

int getBucketDepth()
{
	 return CLP.bucketDepth_B;

}

int getNumberofTokens()
{
	return CLP.requiredPacketsForTransmission_P;
}

struct Packetdata getDataInDeterministicMode()
{
	struct Packetdata returnValue;
	memset(&returnValue,0,sizeof(returnValue));

 	returnValue.number_of_tokens = getNumberofTokens();
	returnValue.inter_arrival_time = getInterArrivalRate();
	returnValue.service_time = getServiceRate();

	return returnValue;
}

struct Packetdata getDataInTraceDrivenMode()
{
	 return ReadAndGetNextDataFromFile();

}

struct Packetdata getData()
{
	if(IsTraceDrivenMode) 
		return getDataInTraceDrivenMode();
	return getDataInDeterministicMode();
}


void startupRoutineForPacket()
{
     for (;;) {

	struct Packetdata data;;
 	memset(&data,0,sizeof(data));
	data = getData();
	if(IsTraceDrivenMode){
		if(NumberOFPacketsInTraceDrivenMode == PacketID){
		 Completed = 1;
		}	
	}else{
		if( CLP.numberOfPacktes_n == PacketID ){
		Completed = 1;
	     }
	}
	
	if( Completed == 1 || wasCntrlCPressed == 1) {
		 NumberOfthreadsDied++;
		 pthread_mutex_unlock(&mutex);
		 pthread_exit(0); 
	}

	double ActualTimeToSleep = (double) 0;
	if(PacketID == 0){
		gettimeofday(&LastarrivalTimeOfPacket,NULL);
		ActualTimeToSleep = data.inter_arrival_time;
	}else{
	
		gettimeofday(&CurrentarrivalTimeOfPacket,NULL);	
		double DiffTime = getDiff(CurrentarrivalTimeOfPacket,
					  LastarrivalTimeOfPacket);
		if( DiffTime > data.inter_arrival_time){
			ActualTimeToSleep = data.inter_arrival_time;
		}else{
			ActualTimeToSleep = DiffTime;
		}
		LastarrivalTimeOfPacket = CurrentarrivalTimeOfPacket;
	}

	FDS.TotalInterArrivalTime += ActualTimeToSleep;
	double sleepInMicroSec = ActualTimeToSleep*1000;
	usleep(sleepInMicroSec);

	struct Packet* packet = malloc(sizeof(struct Packet));
	memset(packet,0,sizeof(struct Packet));

        gettimeofday(&packet->ArrivesTS, NULL);

	pthread_mutex_lock(&mutex);
	fprintf(stdout,"%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n",PrintTimeDiff(),PacketID+1,data.number_of_tokens,(double)ActualTimeToSleep);
	pthread_mutex_unlock(&mutex);

	packet->serviceTime  = data.service_time;
	packet->numberOfTokens =  data.number_of_tokens;
	packet->InterArrivalTime = data.inter_arrival_time;
	PacketID = PacketID+1;
	packet->id = PacketID;
	
	pthread_mutex_lock(&mutex);
	pthread_cleanup_push(pthread_mutex_unlock,&mutex);

	gettimeofday(&packet->EntersQ1TS,NULL);
	(void)My402ListAppend(&my402list_Q1,packet);
	fprintf(stdout,"%012.3fms: p%d enters Q1\n",PrintTimeDiff(),PacketID);

	 My402ListElem* elem = My402ListFirst(&my402list_Q1);
	
	if(elem && (My402ListLength(&my402list_Q1) == 1)) {
	if( CurrentBucketSize >= ((struct Packet*)(elem->obj))->numberOfTokens ){

                  CurrentBucketSize = CurrentBucketSize - ((struct Packet*)(elem->obj))->numberOfTokens;

		  gettimeofday(&((struct Packet*)(elem->obj))->LeavesQ1TS,NULL);
		  My402ListUnlink(&my402list_Q1,elem);
		  fprintf(stdout,"%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n",PrintTimeDiff(),
			((struct Packet*)(elem->obj))->id,
			getDiff (((struct Packet*)(elem->obj))->LeavesQ1TS, ((struct Packet*)(elem->obj))->EntersQ1TS),
			CurrentBucketSize);


	          My402ListAppend(&my402list_Q2,elem->obj);
		  gettimeofday(&((struct Packet*)(elem->obj))->EntersQ2TS,NULL);
		  fprintf(stdout,"%012.3fms: p%d enters Q2\n",PrintTimeDiff(),((struct Packet*)(elem->obj))->id);

	      	  pthread_cond_broadcast(&cv);

	     }
	}
	pthread_cleanup_pop(0);	 
	pthread_mutex_unlock(&mutex);
	
	}
}


void startupRoutineForToken()
{
	for(;;){
	      pthread_mutex_lock(&mutex);
	      pthread_cleanup_push(pthread_mutex_unlock,&mutex);

	      if( (Completed == 1 && My402ListLength(&my402list_Q1) == 0) ||  (wasCntrlCPressed == 1)) {

			pthread_cond_broadcast(&cv);
			NumberOfthreadsDied++;
			pthread_mutex_unlock(&mutex);
			pthread_exit(0);
		}

		pthread_cleanup_pop(0);
		pthread_mutex_unlock(&mutex);
		double SleepInMicro = getInterTokenRate()*1000;
		usleep(SleepInMicro);

		pthread_mutex_lock(&mutex);
		pthread_cleanup_push(pthread_mutex_unlock,&mutex);
		GlobalTokens++;

		if(CurrentBucketSize > getBucketDepth()) {

			TokenDropped++;
			fprintf(stdout,"%012.3fms: token t%d arrives, dropped\n",PrintTimeDiff(),
				 GlobalTokens);

		}else{
			CurrentBucketSize++;
			fprintf(stdout,"%012.3fms: token t%d arrives, token bucket now has %d token\n",
			PrintTimeDiff(),
			GlobalTokens,CurrentBucketSize);

		}
		My402ListElem* elem = My402ListFirst(&my402list_Q1);

		if(elem){
                if( CurrentBucketSize >= ((struct Packet*)(elem->obj))->numberOfTokens ){

	           CurrentBucketSize = CurrentBucketSize - ((struct Packet*)(elem->obj))->numberOfTokens;

		   gettimeofday(&((struct Packet*)(elem->obj))->LeavesQ1TS,NULL);	
                   My402ListUnlink(&my402list_Q1,elem);
		   fprintf(stdout,"%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n",PrintTimeDiff(),
			((struct Packet*)(elem->obj))->id,
		         getDiff (((struct Packet*)(elem->obj))->LeavesQ1TS , ((struct Packet*)(elem->obj))->EntersQ1TS ),
			 CurrentBucketSize);

                   My402ListAppend(&my402list_Q2,elem->obj);
		   gettimeofday(&((struct Packet*)(elem->obj))->EntersQ2TS,NULL);
                   fprintf(stdout,"%012.3fms: p%d enters Q2\n",PrintTimeDiff(),((struct Packet*)(elem->obj))->id);
		
		   pthread_cond_broadcast(&cv);

	        }else if( getBucketDepth() <  (((struct Packet*)(elem->obj))->numberOfTokens) ){

		PacketDropped++;	
    fprintf(stdout,"%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms, dropped\n",
		PrintTimeDiff(),
                ((struct Packet*)(elem->obj))->id,
		 ((struct Packet*)(elem->obj))->numberOfTokens,
			 ((struct Packet*)(elem->obj))->InterArrivalTime);			

			My402ListUnlink(&my402list_Q1,elem);
			free(elem->obj);	
		}
	   }
		pthread_cleanup_pop(0);
		pthread_mutex_unlock(&mutex);
	}

}

void startupRoutineForserver(int i)
{
	int sid = 0;
	if( i == 2) {
		sid = 1;
	}else{
		sid = 2;
	}
	double service_time = 0;
	while(1){
		 pthread_mutex_lock(&mutex);

		 if( Completed == 1 && 
		     My402ListEmpty(&my402list_Q1) &&
		     My402ListEmpty(&my402list_Q2) ){

			 pthread_cond_signal(&cv);
			 pthread_mutex_unlock(&mutex);
			 pthread_exit(0);
		}

		while (My402ListLength(&my402list_Q2) == 0) {		 

		 if( Completed == 1 && 
		     My402ListEmpty(&my402list_Q1) &&
		     My402ListEmpty(&my402list_Q2) ){

			 pthread_cond_signal(&cv);
			 pthread_mutex_unlock(&mutex);
			 pthread_exit(0);
		    }

		    if( wasCntrlCPressed == 1 ){
			 pthread_cond_signal(&cv);
			 pthread_mutex_unlock(&mutex);
			 pthread_exit(0);
		   }
	           pthread_cond_wait(&cv, &mutex);
		}


		My402ListElem* elem = NULL;
		elem = My402ListFirst(&my402list_Q2);


		My402ListUnlink(&my402list_Q2,elem); 
	
		  gettimeofday(&((struct Packet*)(elem->obj))->LeavesQ2TS,NULL);

		 fprintf(stdout,"%012.3fms: p%d leaves Q2, time in Q2 = %.3fms\n",
			PrintTimeDiff(),
                        ((struct Packet*)(elem->obj))->id,
                        getDiff (((struct Packet*)(elem->obj))->LeavesQ2TS , ((struct Packet*)(elem->obj))->EntersQ2TS ));
		

		service_time = (((struct Packet*)(elem->obj))->serviceTime)*1000;	

		fprintf(stdout,"%012.3fms: p%d begins service at S%d, requesting %.3fms of service\n",
			PrintTimeDiff(),
                        ((struct Packet*)(elem->obj))->id,
			 i-1,
			 ((struct Packet*)(elem->obj))->serviceTime);


		pthread_mutex_unlock(&mutex);

		gettimeofday(&((struct Packet*)(elem->obj))->ServiceBeginTS,NULL);
		usleep(service_time);
		gettimeofday(&((struct Packet*)(elem->obj))->ServiceEndTS,NULL);

		pthread_mutex_lock(&mutex);
		ActualPacketsServed++; 
		fprintf(stdout,"%012.3fms: p%d departs from S%d, service time = %.3fms, time in system = %.3fms\n",PrintTimeDiff(),
                        ((struct Packet*)(elem->obj))->id,
			i-1,
                        getDiff (((struct Packet*)(elem->obj))->ServiceEndTS , ((struct Packet*)(elem->obj))->ServiceBeginTS ),
                      getDiff (((struct Packet*)(elem->obj))->ServiceEndTS , ((struct Packet*)(elem->obj))->ArrivesTS));
		 pthread_mutex_unlock(&mutex);

		FDS.timeSpentAtQ2 += getDiff (((struct Packet*)(elem->obj))->LeavesQ2TS , ((struct Packet*)(elem->obj))->EntersQ2TS );
		FDS.timeSpentAtQ1 += getDiff (((struct Packet*)(elem->obj))->LeavesQ1TS , ((struct Packet*)(elem->    obj))->EntersQ1TS );
		FDS.timeSpentinSystem += getDiff (((struct Packet*)(elem->obj))->ServiceEndTS , ((struct Packet*)(elem->obj))->ArrivesTS );
		double TimeSpentInSystem = getDiff (((struct Packet*)(elem->obj))->ServiceEndTS, ((struct Packet*)(elem->obj))->ArrivesTS );
		SqofTimeSpentSystem += (double) (TimeSpentInSystem*TimeSpentInSystem);
		if((i-1) == 1){
			FDS.timeSpentAtS1 += getDiff (((struct Packet*)(elem->obj))->ServiceEndTS , ((struct Packet*)(elem->obj))->ServiceBeginTS);
		}

		if((i-1) == 2){

			FDS.timeSpentAtS2 += getDiff (((struct Packet*)(elem->obj))->ServiceEndTS , ((struct Packet*)(elem->obj))->ServiceBeginTS);	

		}
		free(elem->obj);
		free(elem);
		elem = NULL;
	}
}

void* startupRoutine(void* i) 
{
		int threadType = (int) i;
		switch(threadType){
			//Thread for Packet Arrival
			case 0:{
				       startupRoutineForPacket();	
				       break;
			       }
			//Thread for Token Arrival
			case 1:{
				       startupRoutineForToken();
				       break;
			       }
			//Thread for servers
			case 2:{
				       startupRoutineForserver(threadType);
				       break;
			       }
			case 3:{
				       startupRoutineForserver(threadType);
				       break;
			       }
			default:{
					//should never happen
					break;
				}
		}
		return 0;
}

void PrintErrorMessage(int argc,char* argv[])
{
        fprintf(stderr, "Usage: %s [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n",argv[0]);
        exit(1);
}


void checkforFileErrors(char* fname,int argc,char* argv[])
{
       
	FILE * fOpen = fopen (fname, "rw+");
	if (fOpen == NULL) {
                        fprintf(stderr, "%s\n", strerror(errno));
			PrintErrorMessage(argc,argv);		
	}else{
			fclose(fOpen);
	}

        if( access( fname, F_OK ) != -1 ) {
                // file exists
          } else {
                fprintf(stderr,"input file %s cannot be accessed\n",fname);
		PrintErrorMessage(argc,argv);
                exit(-1);
         }
       
       struct stat path_stat;
       stat(fname, &path_stat);
       if(S_ISREG(path_stat.st_mode)){
                //file is actually a file
       }else{   
                fprintf(stderr,"input file %s is a directory\n",fname);
		PrintErrorMessage(argc,argv);
                exit(-1);
       }
}

int Validate(int argc,char* argv[])
{
    argc--, argv++;
    int ret = 1;
    if (argc%2 != 0) {
	ret = 0;
        fprintf(stderr, "malformed command");
        for(; argc > 0; argc-=2, argv+=2){
           if(strchr(*argv,'-')==NULL){
             fprintf(stderr, ": Not a valid argument %s\n",*argv);break;
           }else if(argc>=2){
             if(strchr(*(argv+1),'-')!=NULL && strcmp(*argv,"-t")!=0){
             fprintf(stderr, ": Missing valid value  for %s \n",*argv);break;
             }   
           }else{
                 fprintf(stderr, ": Missing value  for %s \n",*argv);break;
                }   
            }                   
     }
     return ret;

}


void ParseAndStoreCommandLineArgs(int argc,char* argv[])
{
    if(!Validate(argc,argv)){
	PrintErrorMessage(argc,argv);
	exit(1);
     }
    double dval=(double)0;
    int ival=0;
    int i=0;
    for(i=1;i<argc;i=i+2){
       if(argv[i+1]){
	if(strcmp(argv[i],"-lambda") == 0){

		if (sscanf(argv[i+1], "%lf", &dval) != 1) {
        		/* cannot parse argv[i+1] to get a double value */
			PrintErrorMessage(argc,argv);
    		} else {
        		/* success */
			CLP.packetArrivalLate_lambda = dval;
    		}
		
	}else if(strcmp(argv[i],"-mu") == 0){

		if (sscanf(argv[i+1], "%lf", &dval) != 1) {
                        /* cannot parse argv[i+1] to get a double value */
			PrintErrorMessage(argc,argv);
                } else {
                        /* success */
                        CLP.packetServiceRate_mu = dval;
                }

	
	}else if(strcmp(argv[i],"-r") == 0){
	
		 if (sscanf(argv[i+1], "%lf", &dval) != 1) {
                        /* cannot parse argv[i+1] to get a double value */
			PrintErrorMessage(argc,argv);
                } else {
                        /* success */
                        CLP.tokenArrivalLate_r = dval;
                }


	}else if(strcmp(argv[i],"-B") == 0){

		 if (sscanf(argv[i+1], "%d", &ival) != 1) {
                        /* cannot parse argv[i+1] to get a double value */
			PrintErrorMessage(argc,argv);
                } else {
                        /* success */
                        CLP.bucketDepth_B = ival;
                }


	}else if(strcmp(argv[i],"-P") == 0){

		 if (sscanf(argv[i+1], "%d", &ival) != 1) {
                        /* cannot parse argv[i+1] to get a double value */
			PrintErrorMessage(argc,argv);
                } else {
                        /* success */
                        CLP.requiredPacketsForTransmission_P = ival;
                }


	}else if(strcmp(argv[i],"-n") == 0){

		 if (sscanf(argv[i+1], "%d", &ival) != 1) {
                        /* cannot parse argv[i+1] to get a double value */
			PrintErrorMessage(argc,argv);
                } else {
                        /* success */
                        CLP.numberOfPacktes_n = ival;
                }


	}else if(strcmp(argv[i],"-t") == 0){

                        strcpy(CLP.traceSpecificationFile_tsfile,argv[i+1]);
			checkforFileErrors(argv[i+1],argc,argv);
			IsTraceDrivenMode = 1;
         }else{
		PrintErrorMessageWith(argc,argv);
	}
      }else{
	   PrintErrorMessageWith(argc,argv);
     }
   }
}

void* signalHandler()
{
	int sig;
		sigwait(&set,&sig);
		pthread_mutex_lock(&mutex);
		wasCntrlCPressed = 1;
		pthread_cancel(thread[0]);
		pthread_cancel(thread[1]);
		fprintf(stdout,"\n");
		fprintf(stdout,"SIGINT caught, no new packets or tokens will be allowed\n");
		pthread_cond_broadcast(&cv);
		pthread_mutex_unlock(&mutex);
		pthread_exit(0);

	return 0;

}


void RemoveDataFromQueues()
{
	My402ListElem *elem=NULL;
	int len = My402ListLength(&my402list_Q1);
	for (elem=My402ListFirst(&my402list_Q1); elem != NULL && len>0; elem=My402ListNext(&my402list_Q1, elem),len--) {
		if(elem){
		fprintf(stdout,"%012.3fms: p%d removed from Q1\n",PrintTimeDiff(),((struct Packet*)(elem->obj))->id);
		free(elem->obj);
                elem->obj = NULL;
                My402ListUnlink(&my402list_Q1, elem);
		}
	}	

	len = My402ListLength(&my402list_Q2);
        for (elem=My402ListFirst(&my402list_Q2); elem != NULL && len>0; elem=My402ListNext(&my402list_Q2, elem),len--) {      
		if(elem){
                fprintf(stdout,"%012.3fms: p%d removed from Q2\n",PrintTimeDiff(),((struct Packet*)(elem->obj))->id);
                free(elem->obj);
                elem->obj = NULL;
                My402ListUnlink(&my402list_Q2, elem);
		}
        }

}

void CreateAndJoinThread()
{
		pthread_t sigthread;

		sigemptyset(&set);
		sigaddset(&set,SIGINT);
		sigprocmask(SIG_BLOCK,&set,0);

		pthread_create(&sigthread,0,signalHandler,0);
		pthread_detach(sigthread);

		int i=0;
		int pthreadCreateReturnValue = 0;
		for(i=0;i<4;i++)
		{
			pthreadCreateReturnValue = pthread_create(&thread[i],0,startupRoutine,(void*)i);

		}

		gettimeofday(&FDS.start, NULL);
		fprintf(stdout,"00000000.000ms: emulation begins\n");

		for(i=0;i<4;i++)
		{       
			pthread_join(thread[i],0);

		}

		RemoveDataFromQueues();
		gettimeofday(&FDS.end, NULL);
		fprintf(stdout,"%012.3fms: emulation ends\n",PrintTimeDiff());

}

void checkAndSetCommandLineParams()
{

	if(CLP.packetArrivalLate_lambda == 0){
		CLP.packetArrivalLate_lambda = 1;
	}

	if(CLP.packetServiceRate_mu == 0){
		CLP.packetServiceRate_mu = 0.35;
	}

	if(CLP.tokenArrivalLate_r == 0){
		CLP.tokenArrivalLate_r = 1.5;
	}

	if(CLP.bucketDepth_B == 0){
		CLP.bucketDepth_B = 10;
	}

	if(CLP.requiredPacketsForTransmission_P == 0){
		CLP.requiredPacketsForTransmission_P = 3;
	}

	if(CLP.numberOfPacktes_n == 0){
		CLP.numberOfPacktes_n = 20;
	}

	if( CLP.bucketDepth_B > 2147483647 || CLP.requiredPacketsForTransmission_P > 2147483647 || CLP.numberOfPacktes_n > 2147483647){
	       fprintf(stderr,"Error: Value given greater than expected value\n");
               exit(1);	
	}

	if( CLP.bucketDepth_B < 0 || CLP.requiredPacketsForTransmission_P < 0 || CLP.numberOfPacktes_n < 0){
		 fprintf(stderr,"Error: Value given is negative\n");
		 exit(1);
	}

	if( 1/CLP.tokenArrivalLate_r > 10){
		CLP.tokenArrivalLate_r = 1/10;
	}


}

void printEmulationParameters()
{

	fprintf(stdout,"\nEmulation Parameters:\n");

     if(IsTraceDrivenMode)  fprintf(stdout,"\tnumber to arrive = %.6g\n",(double)NumberOFPacketsInTraceDrivenMode);
     else fprintf(stdout,"\tnumber to arrive = %d\n",CLP.numberOfPacktes_n);

	if(!IsTraceDrivenMode) fprintf(stdout,"\tlambda = %.6g\n",(double)CLP.packetArrivalLate_lambda);
	if(!IsTraceDrivenMode) fprintf(stdout,"\tmu = %.6g\n",(double)CLP.packetServiceRate_mu);

	fprintf(stdout,"\tr = %.6g\n",(double)CLP.tokenArrivalLate_r);
	fprintf(stdout,"\tB = %.6g\n",(double)CLP.bucketDepth_B);
	
	if(!IsTraceDrivenMode) fprintf(stdout,"\tP = %.6g\n",(double)CLP.requiredPacketsForTransmission_P);

	if(IsTraceDrivenMode) fprintf(stdout,"\ttsfile = %s\n",CLP.traceSpecificationFile_tsfile);

	fprintf(stdout,"\n");
	

}

double CalculateSD(double tsins)
{
	int NOP = 0;
	if (IsTraceDrivenMode) NOP = NumberOFPacketsInTraceDrivenMode;
        else NOP = CLP.numberOfPacktes_n;

	double AVofsqOfX = (double) (SqofTimeSpentSystem/ActualPacketsServed);
	double AvofX = (double) (tsins/ActualPacketsServed);
	double SqOfAvgOfx = AvofX*AvofX;

	double V = AVofsqOfX - SqOfAvgOfx;
	double Variance = (double)sqrt(V);
	
	Variance = Variance>=0?Variance:(-1)*Variance;

	return (Variance/1000.0);

}


void CalcualteAndPrintStatistics()
{
	fprintf(stdout,"\nStatistics:\n\n");


	if(isnan((FDS.TotalInterArrivalTime/PacketID)/1000.0)){
		fprintf(stdout,"\taverage packet inter-arrival time = N/A\n");
	}else{
	fprintf(stdout,"\taverage packet inter-arrival time = %.6g\n", 
		(FDS.TotalInterArrivalTime/PacketID)/1000.0);
	}

	if(isnan((FDS.timeSpentAtS1 + FDS.timeSpentAtS2)/ActualPacketsServed)/1000.0){
		fprintf(stdout,"\taverage packet service time = N/A, No Packets served\n");
	}else{
	fprintf(stdout,"\taverage packet service time = %.6g\n\n",
		((FDS.timeSpentAtS1 + FDS.timeSpentAtS2)/ActualPacketsServed)/1000.0);
	}

	double tts = getDiff (FDS.end , FDS.start);
	double aq1 = ((double)(FDS.timeSpentAtQ1)/tts);
	double aq2 = ((double)(FDS.timeSpentAtQ2)/tts);
	double as1 = ((double)(FDS.timeSpentAtS1)/tts);
	double as2 = ((double)(FDS.timeSpentAtS2)/tts);
	if(isnan(aq1/1000.0)){
		fprintf(stdout,"\taverage number of packets in Q1 = N/A, No packets arrived at this facility\n");
	}else{
	fprintf(stdout,"\taverage number of packets in Q1 = %.6g\n",(double) (aq1/1000.0));
	}

	if(isnan(aq2/1000.0)){
                fprintf(stdout,"\taverage number of packets in Q2 = N/A, No packets arrived at this facility\n");
        }else{
	fprintf(stdout,"\taverage number of packets in Q2 = %.6g\n", (double)(aq2/1000.0) );
	}

	if(isnan(as1/1000.0)){
                fprintf(stdout,"\taverage number of packets in S1 = N/A, No packets arrived at this facility\n");
        }else{
	fprintf(stdout,"\taverage number of packets in S1 = %.6g\n", (double)(as1/1000.0) );
	}

	if(isnan(as2/1000.0)){
                fprintf(stdout,"\taverage number of packets in S2 = N/A, No packets arrived at this facility\n");
        }else{
	fprintf(stdout,"\taverage number of packets in S2 = %.6g\n\n", (double)(as2/1000.0) );
	}


	
	double atsins = (double)0;
	if (IsTraceDrivenMode) atsins = ((FDS.timeSpentinSystem)/ActualPacketsServed);
	else atsins = (FDS.timeSpentinSystem)/ActualPacketsServed;


	if(isnan(atsins/1000.0)){
		fprintf(stdout,"\taverage time a packet spent in system = NA, No Packet is Processed\n");
	}else{	
	fprintf(stdout,"\taverage time a packet spent in system = %.6g\n",atsins/1000.0);
	}

	if(isnan(CalculateSD(FDS.timeSpentinSystem))){
		fprintf(stdout,"\tstandard deviation for time spent in system = NA, No Packet is Processed\n");
	}else{
	fprintf(stdout,"\tstandard deviation for time spent in system = %.6g\n\n", CalculateSD(FDS.timeSpentinSystem));
	}


	double tnotd = (double)  TokenDropped/GlobalTokens;
	if(isnan(tnotd)){
                fprintf(stdout,"\ttoken drop probability = N/A, No tokens arrived at this facility\n");
        }else{	
	fprintf(stdout,"\ttoken drop probability = %.6g\n",tnotd);
	}


	double tPacPob = (double)0;
        if (IsTraceDrivenMode) tPacPob = (double) PacketDropped/NumberOFPacketsInTraceDrivenMode;
        else tPacPob = (double) PacketDropped/CLP.numberOfPacktes_n;
        
	 if(isnan(tPacPob)){
                fprintf(stdout,"\tpacket drop probability = N/A, No packets arrived at this facility\n");
        }else{
        fprintf(stdout,"\tpacket drop probability = %.6g\n",tPacPob);
	}      

	fprintf(stdout,"\n");

}

int main(int argc,char* argv[])
{
		ParseAndStoreCommandLineArgs(argc,argv);
			checkAndSetCommandLineParams();

		if(IsTraceDrivenMode){
			fp = fopen(CLP.traceSpecificationFile_tsfile,"r");
			if (fp == NULL) {
				fprintf(stderr, "Input file %s cannot be opened.\n", CLP.traceSpecificationFile_tsfile);
				exit(-1);
			}
			NumberOFPacketsInTraceDrivenMode = ReadAndGetNumberOFPacketsInTraceDrivenMode();
		}

		printEmulationParameters();
		CreateAndJoinThread();
		CalcualteAndPrintStatistics();
		if(IsTraceDrivenMode){
			fclose(fp);
		}
		return 0;
}


