

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define	BUFSIZE		1024

struct header
{	
	unsigned int sip;	 	/* source IP */
	unsigned int dip; 	 	/* destination IP */
	unsigned int sport;		/* source port */
	unsigned int dport;		/* destination port */
	unsigned int weight;		/* weight */
	unsigned int fid;  		/* flow ID */
	unsigned int sendtime; 	/* set the sent of time of packets */
};


int
main(int argc, char *argv[])
{
	struct sockaddr_in sendadd, recvadd; 	/* send and receive endpoint */
	int    rsock;		    	   /* send and receive socket description */
	char   buf[BUFSIZE];	  	   /* buf to receive and send packets */
	int    alen, i=1, j=1;
	struct header head;
	
	
	memset(&recvadd, 0, sizeof(recvadd));
	recvadd.sin_family = AF_INET;

	
	recvadd.sin_addr.s_addr = inet_addr(argv[1]);
	recvadd.sin_port = htons(atoi(argv[2]));
		
	
	 
	rsock = socket(PF_INET, SOCK_DGRAM, 0);
	if(bind(rsock, (struct sockaddr *)&recvadd, sizeof(recvadd)) < 0)
		printf("cannot bind");
	
	while(1){
		alen = sizeof(sendadd);
		
		if(recvfrom(rsock, buf, 1024, 0, NULL, NULL) < 0)
			printf("receive error");
		printf("receive %d packets\n", i++);
		
		memset(&head, 0, 24);
		memcpy(&head, buf, 24);
		/* printf packet header */
		//printf("sour IP    %s\n", inet_ntoa(head.sip));
		printf("sour sport %u\n", ntohl(head.sport));
		//printf("dest IP    %s\n", inet_ntoa(head.dip));
		printf("dest sport %u\n", ntohl(head.dport));
		printf("weight     %u\n", ntohl(head.weight));
		printf("flow ID    %u\n", ntohl(head.fid)); 
		
		
		memset(&sendadd, 0, sizeof(sendadd));
		sendadd.sin_family = AF_INET;
		sendadd.sin_port = htons(ntohl(head.sport)+1);
		sendadd.sin_addr.s_addr = head.sip;
				
	    if(sendto(rsock, buf, 1024, 0, (struct sockaddr *)&sendadd,
			sizeof(sendadd)) < 0)
				printf("send error");
		printf("send %d packets\n", j++);							
	
	}
}
