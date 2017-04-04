/*
 * Giridhar Manoharan
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define SERVER_PORT 5432
#define ACK_PORT 5442
#define MAX_LINE 80

int main(int argc, char * argv[])
{
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char *fname;
	char buf[MAX_LINE], ack[7];
	int s;
	int slen;
        struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if (argc>=3) {
		host = argv[1];
		fname= argv[2];
	}
	else {
		fprintf(stderr, "Usage: ./a.out host filename\n");
		exit(1);
	}
	/* translate host name into peer’s IP address */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "Unknown host: %s\n", host);
		exit(1);
	}

	fp = fopen(fname, "r");
	if (fp==NULL){
		fprintf(stderr, "Can't open file: %s\n", fname);
		exit(1);
	}

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);

	/* active open */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Socket");
		exit(1);
	}

	srandom(time(NULL));
	socklen_t sock_len= sizeof sin;

	/* setting timeout for recvfrom */
	if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) { 
		perror("PError");
	}

	/* main loop: get and send lines of text */
	int line = 0;
	while(fgets(buf, 80, fp) != NULL){
		slen = strlen(buf);
		strcat(buf, "_");
		char temp[5];
		snprintf(temp, 5, "%d", line);
		strcat(buf, temp);
		printf("CLIENT: sending packet_%d\n", line);
		//printf("%s\n",buf);
        	if(sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&sin, sock_len)<0){
			perror("SendTo Error\n");
			exit(1);
		}
		while(1){
			printf("CLIENT: waiting for ACK...\n");
			int length = recvfrom(s, ack, 10, 0, (struct sockaddr *)&sin, &sock_len);
			if(length < 0) {
				perror("RecvFrom Error\n");
				printf("CLIENT: waiting for ACK - timed out... resending packet_%d\n", line);
				if(sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&sin, sock_len)<0){
					perror("SendTo Error\n");
					exit(1);
				}
			}
			else {
				int ack_no = atoi(ack+4);
				printf("CLIENT: packet_%d %s Received\n", line, ack);
				if(ack_no < line)
				{
					continue;
				}
				line++;
				break;
			}
		}
	}
	*buf = 0x02;
        if(sendto(s, buf, 1, 0, (struct sockaddr *)&sin, sock_len)<0){
		perror("SendTo Error\n");
		exit(1);
	}
	fclose(fp);
}
