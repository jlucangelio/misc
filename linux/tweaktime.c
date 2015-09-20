/* tweaktime.c - Small program to synchronize the computer's clock
			with an online SNTP server. */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char ** argv) {
	struct hostent * host;		/* SNTP server */
	struct in_addr address;		/* Address of SNTP server */
	struct sockaddr_in local, rem;	/* Local and remote addresses for socket  */
	struct tm;			/* Time representations of origin of time
                                		in timestamp format -1/1/1900- and
                                		Unix epoch */
                                 
	struct timeval *tv;		/* timeval and timezone for get/settimeofday() */ 
	struct timezone *tz;  

	int sock;                  
	int res;                   
	unsigned int msg_s[15], msg_r[15]; 
	unsigned int seconds;    
	unsigned int tdiff;  
	socklen_t len;                   
	time_t t;

	if (argc != 2) {
		fprintf(stderr, "Only a single argument is expected, the NTP host.\n");
		return 1;
	}

	/* If the argument looks like an IP address, assume it was one. */
	if (inet_aton(argv[1], &address))
    		host = gethostbyaddr((char *) &address, sizeof(address), AF_INET);
	else {
		host = gethostbyname(argv[1]);
		address = **(struct in_addr **) host->h_addr_list;
	}

	if (!host) {
		herror("Error looking up host!");
		return 1;
	}

	/* Create socket, initialize remote address and bind them */  

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
	memset(&rem, 0, sizeof(rem));
  
	rem.sin_family = AF_INET;
	rem.sin_addr = address;
	rem.sin_port = htons(123);
  
	len = sizeof(rem);  

	local.sin_family = AF_INET;
	memset(&(local.sin_addr), 0, sizeof(local.sin_addr));
	memset(&(local.sin_port), 0, sizeof(local.sin_port));
  
	res = bind(sock, (struct sockaddr * )&local, sizeof(local));

	if (res) {
		perror("Error binding socket!");
		return 1;
	}

	/* Create SNTP message and send it */

	msg_s[0] = htonl(0x0B000000);
	memset(msg_s+1, 0, 56);
 
	res = sendto(sock, msg_s, sizeof(msg_s), 0,
			(struct sockaddr *)&rem, sizeof(rem));

	if (res)
		perror("Sending message to remote host");

	/* Get reply and extract the time */

	res = recvfrom(sock, msg_r, sizeof(msg_r), 0,
			(struct sockaddr *)&rem, &len);

	if (res)
		perror("Receiving answer from remote host");

	seconds = ntohl(msg_r[10]);

	/* Converts between timestamp and Unix epoch.
		There are 86400 seconds in a day, and there are 17 leap years
		between 1900 and 1970 */ 

	tdiff = (86400 * 365) * 53;
	tdiff += (86400 * 366) * 17;

	tv = (struct timeval*)malloc(sizeof(struct timeval));
	tz = (struct timezone*)malloc(sizeof(struct timezone));

	/* Set time and display results */

	gettimeofday(tv, tz); 

	time(&t);

	printf("\nOriginal time: %s", ctime(&t));

	tv->tv_sec = seconds - tdiff;
	tv->tv_usec = 0;

	settimeofday(tv, tz);

	time(&t);

	printf("Corrected time: %s\n", ctime(&t));

	free(tv);
	free(tz);

	return 0;
}
