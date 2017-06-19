

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>

#include "errexit.c"

#define	BUFSIZE		1024*1000	/* buffer 1000 packets */
#define SIZE         1024      	/* packets size */


struct header
{	
	unsigned int sip;	 	/* source IP */
	unsigned int dip; 	 	/* destination IP */
	unsigned int sport;		/* source port */
	unsigned int dport;		/* destination port */
	unsigned int weight;	    /* weight */
	unsigned int fid;  		/* flow ID */
	unsigned int sendtime;   /* set the send time of packets */
};

char buffer[1000][1024];

pthread_mutex_t mutex;

int	errexit(const char *format, ...);
int packrecv(int fd);
int packsend(int fd);


int
main(int argc, char *argv[])
{
	pthread_t	th;
	pthread_attr_t	ta;

	struct	sockaddr_in fadd;			/* the address of a source */
	struct  sockaddr_in tadd;			    /* the address of a destination */
	unsigned int	alen;			     	/* length of source address */
	int	rsock, ssock;			  		/* recieve and send socket	 */
	
	memset(buffer, 0, sizeof(buffer)); 	/* set buffer zero before receive packets */
	
	memset(&fadd, 0, sizeof(fadd));
	memset(&tadd, 0, sizeof(fadd));
	
	fadd.sin_family = AF_INET;
	tadd.sin_family = AF_INET;

	/* arguments is: router IP | receive port | send port */ 
	switch (argc) {
	case	1:	break;
	case 	4:	fadd.sin_addr.s_addr = tadd.sin_addr.s_addr = inet_addr(argv[1]);
	          	fadd.sin_port = htons(atoi(argv[2]));
				tadd.sin_port = htons(atoi(argv[3]));
				break;		
	default :	printf("usage: <dest IP> <dest port>\n");
				exit(1);
	}
		
	/* allocate a receive socket and bind it to endpoint address---fadd */
	rsock = socket(PF_INET, SOCK_DGRAM, 0);
	if(rsock < 0){
		printf("canot create recieve socket");
		exit(1);
	}
	if(bind(rsock, (struct sockaddr *)&fadd, sizeof(fadd)) < 0)
	{
		printf("cannot bind recieve socket");
		exit(1);
	}
			
	/* allocate a send socket and bind it to endpoint address---tadd */
	ssock = socket(PF_INET, SOCK_DGRAM, 0);
	if(ssock < 0){
		printf("canot create send socket");
		exit(1);
	}
	if(bind(ssock, (struct sockaddr *)&tadd, sizeof(tadd)) < 0)
	{
		printf("cannot bind send socket");
		exit(1);
	}
	
	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
	(void) pthread_mutex_init(&mutex, 0);
	
	if (pthread_create(&th, &ta, (void * (*)(void *))packsend,(void *)ssock) < 0)
			errexit("pthread_create: %s\n", strerror(errno));

	(void) packrecv(rsock);
}

/*---------------------------------------
 *packrecv - receive packets from source
 *---------------------------------------
 */
int
packrecv(int fd)
{	
	int    i=0;
	struct header head;
	
	while(1){
		
		memset(&head, 0, 28);

		(void) pthread_mutex_lock(&mutex);
		if(recvfrom(fd, buffer+i, 1024, 0, NULL, NULL) < 0)
			errexit("recvfrom: %s\n", strerror(errno));
		memcpy(&head, buffer+i, 28);
        (void) pthread_mutex_unlock(&mutex);
		
		printf("get %d packet\n",i+1);
		
		/* printf packet header */
		//printf("sour IP    %s\n", inet_ntoa(head.sip));
		printf("sour sport %u\n", ntohl(head.sport));
		//printf("dest IP    %s\n", inet_ntoa(head.dip));
		printf("dest sport %u\n", ntohl(head.dport));
		printf("weight     %u\n", ntohl(head.weight));
		printf("flow ID    %u\n", ntohl(head.fid));
        i++;
		if(i==1000) i=0;
		sleep(1);		
	}
}

/*---------------------------------------
 *packsend - send packets to destination
 *---------------------------------------
 */
int
packsend(int fd)
{
	struct sockaddr_in sendadd; /* receive endpoint */
	struct header head;
	int    alen, i=0, j=1;
	
	while(1){
		
		
		memset(&head, 0, 28);
		
		(void) pthread_mutex_lock(&mutex);
		memcpy(&head, buffer+i, 28);
        (void) pthread_mutex_unlock(&mutex);

		if( head.fid == 0) continue;	/* if no packets come then continue while */
		
		/* allocate sendto address */	
		memset(&sendadd, 0, sizeof(sendadd));
		sendadd.sin_family = AF_INET;
		sendadd.sin_port = htons(ntohl(head.dport));
		sendadd.sin_addr.s_addr = head.dip;
		
	 			
		if(sendto(fd, buffer+i, 1024, 0, (struct sockaddr *)&sendadd,
				sizeof(sendadd)) < 0)
			errexit("send error: %s\n", strerror(errno));
memset(buffer+i, 0, sizeof(buffer+i));	/* set buffer zero after send packets */
				
		printf("send %d packets\n", j++);
			
		
		i++;
		if(i==1000) i=0;
		sleep(1);
	}		
}
