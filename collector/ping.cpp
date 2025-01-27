#include "ping.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <time.h> 
#include <typeinfo>
//#include <fcntl.h> 


#define PACKETSIZE  64
struct packet
{
    struct icmphdr hdr;
    char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;
struct protoent *proto = NULL;
int cnt=1;

/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short checksum(void *b, int len)
{
    unsigned short *buf = (unsigned short *) b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*    return 0 is ping Ok, return 1 is ping not OK.                ---*/
/*--------------------------------------------------------------------*/
long double ping(char *address)
{
    const int val=255;
    int i, sd;
    struct packet pckt;
    struct sockaddr_in r_addr;
    int loop;
    struct hostent *hname;
    struct sockaddr_in addr_ping,*addr;
    long double rtt_msec;

    struct timespec time_start, time_end, tfs, tfe;

    clock_gettime(CLOCK_MONOTONIC, &tfs); 


    pid = getpid();
    proto = getprotobyname("ICMP");
    hname = gethostbyname(address);
	bzero(&addr_ping, sizeof(addr_ping));
    addr_ping.sin_family = hname->h_addrtype;
	addr_ping.sin_port = 0;
	addr_ping.sin_addr.s_addr = *(long*)hname->h_addr;

	
    addr = &addr_ping;

    sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
    if ( sd < 0 )
    {
        perror("socket");
        return 0;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        perror("Set TTL option");
        return 0;
    }
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
    {
        perror("Request nonblocking I/O");
        return 0;
    }

    int len = 0;
    double timeElapsed = 0;
    bzero(&pckt, sizeof(pckt));
    pckt.hdr.type = ICMP_ECHO;
    pckt.hdr.un.echo.id = pid;
        
    for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
        pckt.msg[i] = i+'0';       
       
    pckt.msg[i] = 0;
    pckt.hdr.un.echo.sequence = cnt++;
    pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));


///////////// CHECK LINE 218 /////////////////

    clock_gettime(CLOCK_MONOTONIC, &time_start); 
	
    if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
        perror("sendto");

    len=sizeof(r_addr);

    if ( recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) > 0 )
    {
        printf("Failed recvfrom\n");
     	return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &time_end); 

    timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000.0;
    rtt_msec = (time_end.tv_sec- time_start.tv_sec) + timeElapsed; 
			

    printf("%s: Got time %Lf\n", address, rtt_msec);	

    return rtt_msec;
}




/*
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netinet/ip_icmp.h> 
#include <time.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <time.h> 

// Define the Packet Constants 
// ping packet size 
#define PING_PKT_S 64 

// Automatic port number 
#define PORT_NO 0 

// Automatic port number 
#define PING_SLEEP_RATE 1000000 x 

// Gives the timeout delay for receiving packets 
// in seconds 
#define RECV_TIMEOUT 1 

// Define the Ping Loop 
int pingloop=1; 








// make a ping request 
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_dom, char *ping_ip, char *rev_host) 
{ 
	int ttl_val=64, msg_count=0, i, addr_len, flag=1, msg_received_count=0; 
	
	struct ping_pkt pckt; 
	struct sockaddr_in r_addr; 
	
	
//////// THIS MIGHT BE WHAT WE ARE LOOKING FOR //////////
	struct timespec time_start, time_end, tfs, tfe; 
/////////////////////////////////////////////////////////
	
	
	long double rtt_msec=0, total_msec=0; 
	struct timeval tv_out; 
	tv_out.tv_sec = RECV_TIMEOUT; 
	tv_out.tv_usec = 0; 

	clock_gettime(CLOCK_MONOTONIC, &tfs); 

	
	// set socket options at ip to TTL and value to 64, 
	// change to what you want by setting ttl_val 
	if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) 
	{ 
		printf("\nSetting socket options to TTL failed!\n"); 
		return; 
	} else { 
		printf("\nSocket set to TTL..\n"); 
	} 

	// setting timeout of recv setting 
	setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out); 

	// send icmp packet in an infinite loop 
	while(pingloop) 
	{ 
		// flag is whether packet was sent or not 
		flag=1; 
	
		//filling packet 
		bzero(&pckt, sizeof(pckt)); 
		
		pckt.hdr.type = ICMP_ECHO; 
		pckt.hdr.un.echo.id = getpid(); 
		
		for ( i = 0; i < sizeof(pckt.msg)-1; i++ ) 
			pckt.msg[i] = i+'0'; 
		
		pckt.msg[i] = 0; 
		pckt.hdr.un.echo.sequence = msg_count++; 
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt)); 


		usleep(PING_SLEEP_RATE); 

		//send packet 
		clock_gettime(CLOCK_MONOTONIC, &time_start); 
		if ( sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*) ping_addr, sizeof(*ping_addr)) <= 0) { 
			printf("\nPacket Sending Failed!\n"); 
			flag=0; 
		} 

		//receive packet 
		addr_len=sizeof(r_addr); 

		if ( recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0 && msg_count>1) { 
			printf("\nPacket receive failed!\n"); 
		} else { 
			clock_gettime(CLOCK_MONOTONIC, &time_end); 
			
			double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0 
			rtt_msec = (time_end.tv_sec- time_start.tv_sec) * 1000.0 + timeElapsed; 
			
			// if packet was not sent, don't receive 
			if(flag) 
			{ 
				if(!(pckt.hdr.type ==69 && pckt.hdr.code==0)) 
				{ 
					printf("Error..Packet received with ICMP 
						type %d code %d\n", 
						pckt.hdr.type, pckt.hdr.code); 
				} 
				else
				{ 
					printf("%d bytes from %s (h: %s) 
						(%s) msg_seq=%d ttl=%d 
						rtt = %Lf ms.\n", 
						PING_PKT_S, ping_dom, rev_host, 
						ping_ip, msg_count, 
						ttl_val, rtt_msec); 

					msg_received_count++; 
				} 
			} 
		}	 
	} 



//////////////// This is all total statistics //////////////////////
	clock_gettime(CLOCK_MONOTONIC, &tfe); 
	double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0; 
	
	total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+ timeElapsed 
					
	printf("\n===%s ping statistics===\n", ping_ip); 
	printf("\n%d packets sent, %d packets received, %f percent 
		packet loss. Total time: %Lf ms.\n\n", 
		msg_count, msg_received_count, 
		((msg_count - msg_received_count)/msg_count) * 100.0, 
		total_msec); 
} 

// Driver Code 
int main(int argc, char *argv[]) 
{ 
	int sockfd; 
	char *ip_addr, *reverse_hostname; 
	struct sockaddr_in addr_con; 
	int addrlen = sizeof(addr_con); 
	char net_buf[NI_MAXHOST]; 

	if(argc!=2) 
	{ 
		printf("\nFormat %s <address>\n", argv[0]); 
		return 0; 
	} 

	ip_addr = dns_lookup(argv[1], &addr_con); 
	if(ip_addr==NULL) 
	{ 
		printf("\nDNS lookup failed! Could 
				not resolve hostname!\n"); 
		return 0; 
	} 

	reverse_hostname = reverse_dns_lookup(ip_addr); 
	printf("\nTrying to connect to '%s' IP: %s\n", 
									argv[1], ip_addr); 
	printf("\nReverse Lookup domain: %s", 
						reverse_hostname); 

	//socket() 
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
	if(sockfd<0) 
	{ 
		printf("\nSocket file descriptor not received!!\n"); 
		return 0; 
	} 
	else
		printf("\nSocket file descriptor %d received\n", sockfd); 

	signal(SIGINT, intHandler);//catching interrupt 

	//send pings continuously 
	send_ping(sockfd, &addr_con, reverse_hostname, 
								ip_addr, argv[1]); 
	
	return 0; 
} 


 */
