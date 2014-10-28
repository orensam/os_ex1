#include "osm.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int isNum(char *string);
#define DEFAULT_ITERATIONS 50000

int main(int argc, char *argv[]){
    	if ((argc != 2) || ((argc == 2) && !(isNum(argv[1]))) ){
		printf("Usage: measure <iterations>\n");
		return 1;
	}
    printf("yes");
	unsigned int looptimes = DEFAULT_ITERATIONS;
	if (argc == 2){
		sscanf(argv[1],"%u", &looptimes);
	}
	
	timeMeasurmentStructure hep= measureTimes(looptimes);
	
	// print results
	printf("machine: %s\niterations: %d\ninstruction time: %.02lf\nfunction time: %.02lf\ntrap time: %.02lf\n",
		   hep.machineName,hep.numberOfIterations,hep.instructionTimeNanoSecond,hep.functionTimeNanoSecond,hep.trapTimeNanoSecond);
	printf("ratio function-instruction: %.02f\nratio trap-instruction: %.02f\n",hep.functionInstructionRatio, hep.trapInstructionRatio);
	return 0;
}

/*
 * checks if the null terminated string is a number (contains only digits)
 */
int isNum(char *string){
	while( *(string) ){
		if (!isdigit(*string)){
			return 0;
		}
		++string;
        
	}
	return 1;
}
