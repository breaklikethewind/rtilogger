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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>

#include "transport.h"

/* This is unique per application instance and RTI driver instance */
#define RTI_UDP_PORT 32004
#define MAX_LOGMSG_BYTES 256
#define FILE_NAME_TXT "RTI_Logfile.txt"
#define MSG_QUEUE_KEY 1

struct sockaddr_in servaddr;

int sockfd;
int rtiUdpPort;

typedef struct
{
	int file_idx_txt;
	int file_txt_desc;
	char filename_txt[256];
} status_t;

<<<<<<< HEAD
struct msg_queue_st {
    long int msg_type;
    char msg[MAX_LOGMSG_BYTES];
}; 

int msgqid;

=======
struct queue_entry_t
{
	char buf[256];
	struct queue_entry_t* next;
};

struct queue_entry_t* queue_tail;
struct queue_entry_t* queue_head;
>>>>>>> 80295ea511924c7d4dcd6479b65fcc00a124d4c6
status_t status;
int exitflag = 0;
pthread_mutex_t lock; // sync between UDP thread and main
commandlist_t command_list;

void write_queue_to_file(void);
void *thread_write_file( void *ptr );

typedef int (*cmdfunc)(char* request, char* response);

int opentxtfile(char* request, char* response);
int closetxtfile(char* request, char* response);
int logtxt(char* request, char* response);
int app_exit(char* request, char* response);

pushlist_t pushlist[] = { 
{ "LOGTXTIDX",    TYPE_INTEGER, &status.file_idx_txt},
{ "",             TYPE_NULL,    NULL} 
};

commandlist_t device_commandlist[] = { 
{ "LOGTXT",          "LOGTXTIDX",    &logtxt,       TYPE_INTEGER, &status.file_idx_txt},
{ "GETTXTIDX",       "LOGTXTIDX",    NULL,          TYPE_INTEGER, &status.file_idx_txt},
{ "EXIT",            "EXIT",         &app_exit,     TYPE_INTEGER, &exitflag},
{ "",                "",             NULL,          TYPE_NULL,    NULL}
};

<<<<<<< HEAD
=======
int opentxtfile(char* request, char* response)
{
	strncpy(status.filename_txt, request, 255);
	status.file_txt_desc = open(status.filename_txt, O_CREAT | O_WRONLY, S_IWUSR);
	
	sprintf(response, "%u", (status.file_txt_desc) ? 1:0);
	
	return 0;
}

int closetxtfile(char* request, char* response)
{
	write_queue_to_file(); // flush the queue
	close(status.file_txt_desc);
	status.file_txt_desc = 0;
	sprintf(response, "%u", (status.file_txt_desc) ? 1:0); // I know, this will always be 0
	
	return 0;
}

>>>>>>> 80295ea511924c7d4dcd6479b65fcc00a124d4c6
int logtxt(char* request, char* response)
{
	time_t t;
	struct tm tm;
<<<<<<< HEAD
	struct msg_queue_st msgq;
	int error = 0;
=======
>>>>>>> 80295ea511924c7d4dcd6479b65fcc00a124d4c6
	char time_s[25];
	
	t = time(NULL);
	tm = *localtime(&t);

	sprintf(time_s, "%d-%d-%d %d:%d:%d\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

	if (status.file_txt_desc) 
	{		
		sprintf(msgq.msg, "%s: %s", time_s, request);
		if (msgsnd(msgqid, (void *)&msgq, strlen(msgq.msg) + 1 , 0) == -1) 
		{
<<<<<<< HEAD
		    fprintf(stderr, "msgsnd failed\n");
		    error = -1;
		}		
	
		pthread_mutex_lock(&lock);
		status.file_idx_txt++;
=======
			queue_tail->next = malloc(sizeof(struct queue_entry_t));
			queue_tail = queue_tail->next;
		}
		else
		{
			queue_tail = malloc(sizeof(struct queue_entry_t));
			queue_head = queue_tail;
		}
		queue_tail->next = NULL;
		sprintf(queue_tail->buf, "%s: %s", time_s, request);
>>>>>>> 80295ea511924c7d4dcd6479b65fcc00a124d4c6
		pthread_mutex_unlock(&lock);
	}
	sprintf(response, "%u", (status.file_txt_desc) ? 1:0); 
	
	return error;
}

int app_exit(char* request, char* response)
{
	char* junk;

	exitflag = strtol(request, &junk, 0);
	sprintf(response, "%u", exitflag);
	
	return 0;
}

void write_queue_to_file(void)
{
	struct queue_entry_t* queue_stale;

	// Fetch file queue data
	while (queue_head)
	{
		pthread_mutex_lock(&lock);
		write(status.file_txt_desc, queue_head->buf, strlen(queue_head->buf));
		queue_stale = queue_head;
		queue_head = queue_head->next;
		free(queue_stale);
		if (queue_head == NULL)
			queue_tail = NULL;
		pthread_mutex_unlock(&lock);	
		sleep(0);
	}
}

void *thread_write_file( void *ptr ) 
{
<<<<<<< HEAD
	struct msg_queue_st msgq;
	long int msg_to_receive = 0;
	
	while (!exitflag)
	{
		// Fetch file queue data (blocking)
		if (msgrcv(msgqid, (void *)&msgq, MAX_LOGMSG_BYTES, msg_to_receive, 0) != -1) 
			write(status.file_txt_desc, msgq.msg, strlen(msgq.msg));
		else
		{
			fprintf(stderr, "msgrcv failed with error: %s\n", 
			    strerror(errno));
			exit(EXIT_FAILURE);
		}
=======
	
	while (!exitflag)
	{
		write_queue_to_file();
		sleep(WRITEOUT_PERIOD);
>>>>>>> 80295ea511924c7d4dcd6479b65fcc00a124d4c6
	}
	
	return NULL;
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
	pthread_t write_file_thread_handle;
	int msgqkey = MSG_QUEUE_KEY;

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
	strcpy(status.filename_txt, FILE_NAME_TXT);
	status.file_txt_desc = open(status.filename_txt, O_CREAT | O_WRONLY, S_IWUSR);
	status.file_idx_txt = 0;
	
	// Initialize message queue
	msgqid = msgget(msgqkey, 0660 | IPC_CREAT);

	iret1 = pthread_mutex_init(&lock, NULL); 
	if(iret1)
	{
		printf("Error - mutex init failed, return code: %d\n",iret1);
		return -1;
	}

	/* Initialize the threads */
	iret1 = pthread_create( &write_file_thread_handle, NULL, thread_write_file, NULL);
	if(iret1)
	{
		printf("Error - pthread_create() return code: %d\n",iret1);
 		return -2;
	}
	else
		printf("Launching thread sensor_sample\r\n");

	tp_handle_requests(device_commandlist, &lock);
	
	tp_handle_data_push(pushlist, &lock);

	while (!exitflag) sleep(0);
	
	printf("RTILogger Exit Set...\r\n");
	
	// Exit	
	tp_stop_handlers();
	pthread_join(write_file_thread_handle, NULL);
	close(status.file_txt_desc);
	pthread_mutex_destroy(&lock);

	return 0;
}

