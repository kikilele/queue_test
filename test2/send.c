
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
#include <arpa/inet.h>

#include "errexit.c"

#define	BUFSIZE	1024

int errexit(const char *format,...);
unsigned int mstime(unsigned int *);

struct header
{	
	unsigned int sip;	 	/* source IP */
	unsigned int dip; 	 	/* destination IP */
	unsigned int sport;		/* source port */
	unsigned int dport;		/* destination port */
	unsigned int weight;		/* weight */
	unsigned int fid;  		/* flow ID */
	unsigned int sendtime;  	/* set the send time of packets */ 
};



int
main(int argc, char *argv[])
{
	struct sockaddr_in sendadd, recvadd; /* send address and receive address */
	int    ssock, rsock;        		   /* send and receive socket description */	
	int    packnum;					   /* want to send packets numbers */
	struct header head;	                /* packets header */
	
	/* set header of packet */ 
	switch (argc) {
	case	1:  break;
	case    8:  head.sip = inet_addr(argv[1]);
			    head.sport = htonl(atoi(argv[2]));	
				head.dip = inet_addr(argv[3]);
				head.dport = htonl(atoi(argv[4]));
	    		head.weight = htonl(atoi(argv[5]));
	    		head.fid = htonl(atoi(argv[6]));
				packnum = atoi(argv[7]);
				break;
	//
	default  :	errexit("usage:<src IP><dest IP><src port><dest port><weight><flow ID><pack nums>\n");
	}
	
	/* printf packet header */
	printf("send information of packets:\n");
	printf("sour IP   :%s\n", argv[1]);
	printf("sour sport:%u\n", ntohl(head.sport));
	printf("dest IP   :%s\n", argv[3]);
	printf("dest sport:%u\n", ntohl(head.dport));
	printf("weight    :%u\n", ntohl(head.weight));
	printf("flow ID   :%u\n", ntohl(head.fid));
	printf("pack sum  :%u\n", packnum);
	printf("---------------------------\n");
	sleep(1);	
	
		
	memset(&sendadd, 0, sizeof(sendadd));
	sendadd.sin_family = AF_INET;
	sendadd.sin_port = htons(ntohl(head.sport));
	sendadd.sin_addr.s_addr = head.sip;
	
	
	ssock = socket(PF_INET, SOCK_DGRAM, 0);
	if(ssock < 0){
		printf("canot create send socket");
		exit(1);
	}
	if(bind(ssock, (struct sockaddr *)&sendadd, sizeof(sendadd)) < 0)
	{
		printf("cannot bind send socket");
		exit(1);
	}
			
	
	memset(&recvadd, 0, sizeof(recvadd));
	recvadd.sin_family = AF_INET;
	recvadd.sin_port = htons(ntohl(head.sport)+1);						
	recvadd.sin_addr.s_addr = head.sip;   	

	
	rsock = socket(PF_INET, SOCK_DGRAM, 0);
	if(rsock < 0){
		printf("can't create receive soket");
		exit(1);
	}
	if(bind(rsock, (struct sockaddr *)&recvadd, sizeof(recvadd)) < 0)
	{
		printf("cannot bind receive port");
		exit(1);
	}
		
	/* create two processes to send and receive packets */
	switch(fork()){
		case  0:  	 /* child--send packets */
				(void) close(rsock);
				exit(packsend(ssock, &head, packnum));
		default:		/* parent--receive packets */
				(void) close(ssock);
				(void) packrecv(rsock);
		case -1:
			    errexit("fork error: %s\n", strerror(errno));
	}
}
/*---------------------------
 * function to send packets
 *---------------------------
 */
int packsend(int ssock, struct header *ptr, int packnum)
{
	struct sockaddr_in toadd;	/* to router address */
	char    outbuf[BUFSIZE];    /* send buf */
	int 	i,j=1;
	unsigned int now;			/* current time */
	
		
	memset(&toadd, 0, sizeof(toadd));
	toadd.sin_family = AF_INET;
	toadd.sin_port = htons(20000);						
	toadd.sin_addr.s_addr = inet_addr("10.20.181.200");   	

	printf("begin send packets\n");
	for(i = packnum; i>0; i--){
		
		ptr->sendtime = mstime(&now);
		memset(outbuf, 0, BUFSIZE);
	    memcpy(outbuf, ptr, 28);
		
		sendto(ssock, outbuf, 1024, 0, (struct sockaddr *)&toadd, sizeof(toadd));
		printf("send %d pakets\n",j++);
	}
	return 1;
}
	
/*-----------------------------
 * function to receive packets
 *----------------------------
 */
int packrecv(int rsock )
{
	char    inbuf[BUFSIZE];     /* receive buf */ 
	int     i=1;
	unsigned int now;			/* current time */
	struct header head;
		
	while(1){
		if(recvfrom(rsock, inbuf, sizeof(inbuf), 0, NULL, NULL) < 0)
			errexit("recvfrom: %s\n", strerror(errno));
	
		memset(&head, 0, 28);
		memcpy(&head, inbuf, 28);

		/* printf packet header */
		printf("get %i packet:",i++);
		//printf("sur IP %s ", head.sip);
		printf("sur sport %u ", ntohl(head.sport));
		//printf("des IP %s ", head.dip);
		printf("des sport %u ", ntohl(head.dport));
		printf("weight %u", ntohl(head.weight));
		printf("flow ID %u ", ntohl(head.fid));
		printf("send time %u ", head.sendtime);
		printf("delay time %u us\n", mstime(&now)-head.sendtime);
	}
}

/*------------------------------------------------------------------------
 * mstime - report the number of milliseconds elapsed
 *------------------------------------------------------------------------
 */
unsigned int
mstime(unsigned int *pms)
{
	struct timeval		now;

	if (gettimeofday(&now, (struct timezone *)0))
		errexit("gettimeofday: %s\n", strerror(errno));
	*pms = now.tv_sec * 1000000 + now.tv_usec;
	return *pms;
}
