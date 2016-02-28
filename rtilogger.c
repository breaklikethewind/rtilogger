/*
 * boathouse.c:
 *      This app manages the sump pump
 *
 *
 * Copyright (c) 2014 Eric Nelson
 ***********************************************************************
 * This file uses wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/
 

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <arpa/inet.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "transport.h"

#define DEFAULT_SENSOR_PERIOD 60 // Seconds

struct sockaddr_in servaddr;

int sockfd;
int rtiUdpPort;
/* This is unique per application instance and RTI driver instance */
#define RTI_UDP_PORT 32004

typedef struct
{
	int file_open_txt;
	int file_idx_txt;
	int file_txt_desc;
	char filename_txt[255];
} status_t;

status_t status;
int sensor_period = DEFAULT_SENSOR_PERIOD;
int exitflag = 0;
int firstsampleflag = 0;
pthread_mutex_t lock; // sync between UDP thread and main
commandlist_t command_list;
void *thread_sensor_sample( void *ptr );
void measure(void);

typedef int (*cmdfunc)(char* request, char* response);

int app_exit(char* request, char* response);

pushlist_t pushlist[] = { 
{ "TXTFILEOPEN",  TYPE_FLOAT,   &status.file_open_txt}, 
{ "LOGTXT",       TYPE_FLOAT,   &status.file_idx_txt},
{ "",         TYPE_NULL,    NULL} 
};

commandlist_t device_commandlist[] = { 
{ "OPENTXTFILE",     "TXTFILEOPEN",  &opentxtfile,  TYPE_FLOAT,   NULL}, 
{ "CLOSETXTFILE",    "TXTFILEOPEN",  &closetxtfile, TYPE_FLOAT,   NULL}, 
{ "LOGTXT",          "LOGTXT",       &logtxt,       TYPE_FLOAT,   &status.file_idx_txt},
{ "GETIDXTXT",       "IDXTXT",       NULL,          TYPE_INTEGER, &status.file_idx_txt},
{ "SETSENSORPERIOD", "SENSORPERIOD", NULL,          TYPE_INTEGER, &sensor_period},
{ "EXIT",            "EXIT",         &app_exit,     TYPE_INTEGER, &exitflag},
{ "",                "",             NULL,          TYPE_NULL,    NULL}
};

int opentxtfile(char* request, char* response)
{
	strncpy(status.filename_txt, request, 255);
	status.file_txt_desc = open(status.filename_txt, O_CREAT | O_WRONLY, S_IWUSR);
	
	sprintf(response, "%u", (status.file_txt_desc) ? 1:0);
	
	return 0;
}

int closetxtfile(char* request, char* response)
{
}

int logtxt(char* request, char* response)
{
}

int app_exit(char* request, char* response)
{
	char* junk;

	exitflag = strtol(request, &junk, 0);
	sprintf(response, "%u", exitflag);
	
	return 0;
}

void *thread_sensor_sample( void *ptr ) 
{
	
	while (!exitflag)
	{
		measure();
		sleep(sensor_period);
	}
	
	return NULL;
}

void measure( void )
{
	// Fetch sensor data
	pthread_mutex_lock(&lock);
	
	firstsampleflag = 1;
	pthread_mutex_unlock(&lock);
}

/*
 *********************************************************************************
 * main
 *********************************************************************************
 */

int  main(void)
{
	int  iret1;
	int broadcast;
	pthread_t sensor_sample;

	printf("RTILogger Launch...\r\n");

	/* Set up the socket */
	rtiUdpPort = RTI_UDP_PORT;
	broadcast = 1;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(rtiUdpPort);
	bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	// Initialize sensors
	status.file_open_txt = 0;
	status.file_idx_txt = 0;

	iret1 = pthread_mutex_init(&lock, NULL); 
	if(iret1)
	{
		BeepMorse(5, "Mutex Fail");
		printf("Error - mutex init failed, return code: %d\n",iret1);
		return -1;
	}

	/* Initialize the threads */
	iret1 = pthread_create( &sensor_sample, NULL, thread_sensor_sample, NULL);
	if(iret1)
	{
		printf("Error - pthread_create() return code: %d\n",iret1);
		BeepMorse(5, "thread_sensor_sample Thread Create Fail");
		return -2;
	}
	else
		printf("Launching thread sensor_sample\r\n");

	// wait until we have our first sample
	while(!firstsampleflag);
	
	tp_handle_requests(device_commandlist, &lock);
	
	tp_handle_data_push(pushlist, &lock);

	while (!exitflag) sleep(0);
	
	printf("RTILogger Exit Set...\r\n");
	
	// Exit	
	tp_stop_handlers();
	pthread_join(sensor_sample, NULL);
	pthread_mutex_destroy(&lock);

	return 0;
}

