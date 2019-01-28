#include <stdio.h>
#include <stdlib.h>

struct PROCESS {
	char p_name[20];
	int p_arrival;
	int p_burst;
	int leftBurst;
};

void readDatas(struct PROCESS*,int);
int getEnteredProcessNumber(struct PROCESS*,int,int);
int listenProcessQueue(struct PROCESS*,int,int);
int searchMinBurstProcess(struct PROCESS*,int,int);
void sort(struct PROCESS *,int);

int processIndisAtCPU = -1;								// it keeps the indis of process that use cpu
int *waiting;											// this integer arrays keeps the waiting times for each process
int *lastServedTime;									// this integer arrays keeps the last used cpu time that process use

int main(void){
	int processNum;										// it keeps process number
	int terminatedProcNum = 0;							// it keeps the number of processes that terminated
	int enteredProcNum = 0;								// it keeps the number of processes that entered the cpu queue
	int cpuTime = 0;									// it keeps cpu cycle time
	
	printf("Enter number of process : ");
	scanf("%d",&processNum);
	
	struct PROCESS *processList;						// processList keeps process informations
	processList = (struct PROCESS *) malloc (processNum*sizeof(struct PROCESS));
	
	waiting = (int*) malloc (processNum*sizeof(int));
	
	lastServedTime = (int*) malloc (processNum*sizeof(int));
	
	readDatas(processList,processNum);
	
	int i = 0;
	for(; i<processNum ; i++){								// Waiting arrays set to 0 because all process do not wait at the begin of system
		*(waiting + i) = 0;									// lastServedTime array set to process' arrival time because it is the minimum value for that value
		*(lastServedTime + i) = processList[i].p_arrival;
	}
	
	while(terminatedProcNum < processNum){					// This loop continue while all process terminated
		printf("CPU time : %d\n",cpuTime);
		if(enteredProcNum < processNum)						// If all process do not entered the queue, cpu time will increase one by one	
		{								
			int getEnteredProcNum = getEnteredProcessNumber(processList,processNum,cpuTime);
			if(getEnteredProcNum != 0)						// If there is at least new process added to cpu queue
			{						
				// If there is any shortest job or our cpu-served process is done
				if((listenProcessQueue(processList,processNum,cpuTime)) || (processList[processIndisAtCPU].leftBurst == 0))
				{
					if((processIndisAtCPU != -1) && (processList[processIndisAtCPU].leftBurst == 0))	// If our cpu-served process done, update terminatedProcNum
						terminatedProcNum++;
						
					// finding indis of new cpu-served process	
					int newProcIndis = searchMinBurstProcess(processList,processNum,cpuTime);		
		
					// If there is no appropriate process, newProcIndis will be equal to processIndisAtCPU
					// We check this situation here
					if((newProcIndis == processIndisAtCPU) && (processList[processIndisAtCPU].leftBurst == 0))
					{
						printf("%d . cycle : NONE\n",cpuTime);
						processIndisAtCPU = -1;
					}
					else					// Here we do necessary update
					{
						processIndisAtCPU = newProcIndis;
						printf("%d . cycle : (%d.) - %s process got CPU\n",cpuTime,processIndisAtCPU+1,processList[processIndisAtCPU].p_name);
						
						// waiting calculated as waiting = waiting + (cpuTime - lastServedTime)
						*(waiting + processIndisAtCPU) += cpuTime - *(lastServedTime + processIndisAtCPU);
						
						// waiting calculated as waiting = waiting + (cpuTime - lastServedTime)
						*(lastServedTime + processIndisAtCPU) = cpuTime + 1;
					}
				}
				else			// If there are no shorter job on new came processes and our cpu-served process is not done, it continue its job.
				{
					printf("%d . cycle : (%d.) - %s process got CPU\n",cpuTime,processIndisAtCPU+1,processList[processIndisAtCPU].p_name);
					*(waiting + processIndisAtCPU) += cpuTime - *(lastServedTime + processIndisAtCPU);
					*(lastServedTime + processIndisAtCPU) = cpuTime + 1;
				}
				enteredProcNum += getEnteredProcNum;				// enteredProcNum updated
			}
			else					// If there are no came process
			{
				if(processIndisAtCPU == -1)							// If there are no process at CPU
				{
					printf("%d . cycle : NONE\n",cpuTime);		
				}
				else
				{

					if(processList[processIndisAtCPU].leftBurst == 0)					// If cpu-served process is done
					{
						terminatedProcNum++;
						int newProc = searchMinBurstProcess(processList,processNum,cpuTime);		// Finding shortest job process at cpu queue
						if(newProc == processIndisAtCPU)				// There is no appropriate process at queue
						{
							printf("%d . cycle : NONE\n",cpuTime);
							processIndisAtCPU = -1;
						}
						else			// If there is, necessary updates will be do
						{
							processIndisAtCPU = newProc;
							printf("%d . cycle : (%d.) -  %s process got CPU\n",cpuTime,processIndisAtCPU+1,processList[processIndisAtCPU].p_name);
							*(waiting + processIndisAtCPU) += cpuTime - *(lastServedTime + processIndisAtCPU);
							*(lastServedTime + processIndisAtCPU) = cpuTime + 1;
						}
					}
					else				// If cpu-served process is not done
					{
						printf("%d . cycle : (%d.) - %s process got CPU\n",cpuTime,processIndisAtCPU+1,processList[processIndisAtCPU].p_name);
						*(waiting + processIndisAtCPU) += cpuTime - *(lastServedTime + processIndisAtCPU);
						*(lastServedTime + processIndisAtCPU) = cpuTime + 1;
					}	
				}
			}
			cpuTime++;									// cpuTime increse 1 because there are process that will be come later
			if(processIndisAtCPU != -1)								// We update leftBurst value for cpu-served process
				processList[processIndisAtCPU].leftBurst--;
			if((enteredProcNum == processNum) && (processList[processIndisAtCPU].leftBurst == 0))		// If there is termination, we update that value
				terminatedProcNum++;
		}
		else					// All process come to queue, we select shortest one and we serve cpu until it terminated
		{
			if(processList[processIndisAtCPU].leftBurst != 0)				// In our if section, last cpu-served process could not terminated
			{																// Firstly, we end it
				cpuTime += processList[processIndisAtCPU].leftBurst;
				processList[processIndisAtCPU].leftBurst = 0;
			}
			else
			{
				// We calculate shortest job process at cpu queue
				int newProcessIndis = searchMinBurstProcess(processList,processNum,cpuTime);
				processIndisAtCPU = newProcessIndis;
				printf("%d . cycle : (%d.) -  %s process got CPU\n",cpuTime,processIndisAtCPU+1,processList[processIndisAtCPU].p_name);
				*(waiting + processIndisAtCPU) += cpuTime - *(lastServedTime + processIndisAtCPU);
				*(lastServedTime + processIndisAtCPU) = cpuTime + processList[processIndisAtCPU].leftBurst;
				
				// cpuTime increase by cpu-served process' leftBurst value because we do not have to check new came process that it might need served cpu
				cpuTime += processList[processIndisAtCPU].leftBurst;
				processList[processIndisAtCPU].leftBurst = 0; 
			}
			terminatedProcNum++;
			printf("Terminated : %d\n\n",terminatedProcNum);
		}
	}
		
	printf("\n\nAt the end of all process terminated\nCPU time : %d\n\n",cpuTime);
	
	sort(processList,processNum);					// In pdf document, process information listed sequentially. So I sort relevant informations.
	
	i = 0;
	int totalWaitingTime = 0;						// The variable for calculate total waiting time and average waiting time
	printf("ProcessName   ArrivalTime     BurstTime      WaitingTime\n");
	for( ; i<processNum ; i++){
		printf("%s \t\t %d \t\t ",processList[i].p_name,processList[i].p_arrival);
		printf("%d \t\t %d\n",processList[i].p_burst,waiting[i]);
		totalWaitingTime += waiting[i];
	}
	
	printf("\n\nTotal waiting time : %d\n",totalWaitingTime);
	printf("Average waiting time : %lf",((double)totalWaitingTime / (double)processNum));
	
	
	
	free(processList);
	free(waiting);
	free(lastServedTime);
	
	return 0;
}

void readDatas(struct PROCESS *p, int n){						// Process information read from console with this function
	int i;
	for(i=0; i<n; i++){
		printf("For (%d). process;\n",i+1);
		printf("Enter process name : ");
		scanf("%s",p[i].p_name);
		printf("Enter arrival time of process: ");
		scanf("%d",&p[i].p_arrival);
		printf("Enter burst time of process : ");
		scanf("%d",&p[i].p_burst);
		p[i].leftBurst = p[i].p_burst;
		printf("\n\n");
	}
}

int getEnteredProcessNumber(struct PROCESS *p, int n, int c){			// With this function, we calculate the number of process that entered the cpu queue
	int number = 0;
	int i = 0;
	for(; i<n ; i++){
		if(p[i].p_arrival == c){
			number++;
		}
	}
	return number;
}

int listenProcessQueue(struct PROCESS *p, int n, int c){		// This function returns 1, if there is shorter process according to cpu-served process
	int i = 0;
	if(processIndisAtCPU == -1){			// If cpu's empty
		while(i < n){
			if(p[i].p_arrival <= c)			// returns 1 for finding any process that contain less arrival time from cpuTime
				return 1;
			i++;
		}
	}else{									//If cpu is not empty
		while(i < n){
			// Returns 1 only at the process have less arrival time then cpuTime AND it is not terminated AND also it's left burst time must be
			// smaller than cpu-served process'
			
			if((p[i].p_arrival <= c) && (p[i].leftBurst != 0) && (p[i].leftBurst < p[processIndisAtCPU].leftBurst))
				return 1;
			i++;
		}
	}
	return 0;
}

int searchMinBurstProcess(struct PROCESS *p, int n, int c){		// This function returns the indis of process that have to have cpu
	int minIndis = processIndisAtCPU, i = 0;
	if((processIndisAtCPU == -1) || (p[processIndisAtCPU].leftBurst == 0)){		// If cpu is empty OR the cpu-served process is terminated
		while((i < n) && (minIndis == processIndisAtCPU)){				// minIndis should have different value from cpu-served processIndis
			if((p[i].p_arrival <= c) && (p[i].leftBurst != 0))
				minIndis = i;
			i++;
		}
	}
	for( ; i<n ; i++){
		if((p[i].p_arrival <= c) && (p[i].leftBurst != 0) && (p[i].leftBurst < p[minIndis].leftBurst)){
			minIndis = i;
		}
	}
	return minIndis;
}

void sort(struct PROCESS *processList, int processNum){		// INSERTION SORT
	int minArrival;						// the indis of variable that has minimum arrival time 					
	int temp;							// variable for exchanging positions 
	struct PROCESS tempProc;			// variable for exchanging positions 
	int i,j;							// variables for loops
	for(i=0; i<(processNum - 1); i++){
		minArrival = i;
		for( j= i + 1 ; j<processNum ; j++ ){									
			if(processList[j].p_arrival < processList[minArrival].p_arrival)
				minArrival = j;
		}
		tempProc = processList[i];
		processList[i] = processList[minArrival];
		processList[minArrival] = tempProc;
		
		temp = waiting[i];
		waiting[i] = waiting[minArrival];
		waiting[minArrival] = temp;
	}
}
