/*
 * Giridhar Manoharan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>

#define SERVER_PORT 5432
#define ACK_PORT 5442
#define MAX_LINE 256

int main(int argc, char * argv[])
{
	char *fname;
	char *sender;
        char buf[MAX_LINE], ack[7];
	struct hostent *sender_hp;
        struct sockaddr_in sin;
        int len;
        int s, i;
        struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	char seq_num = 1;
	FILE *fp;

        if (argc>=2) {
                fname = argv[1];
        }
        else {
                fprintf(stderr, "usage: ./client_udp filename \n");
        	exit(1);
        }


        /* build address data structure */
        bzero((char *)&sin, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(SERVER_PORT);

        /* setup passive open */
        if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("simplex-talk: socket");
                exit(1);
        }
        if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
                perror("simplex-talk: bind");
                exit(1);
        }

	socklen_t sock_len = sizeof sin;
	srandom(time(NULL));

	fp = fopen(fname, "w");
        if (fp==NULL){
                printf("Can't open file\n");
                exit(1);
        }

	int ack_no=-1, pckt_no;
	while(1){
		len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &sock_len);
		if(len < 0){
        		perror("PError");
		}	
		else if(len == 1){
			if (buf[0] == 0x02){
        	    		printf("SERVER: Transmission Complete\n");
				break;
			}
        	    	else{
				perror("Error: Short packet\n");
			}
		}	
		else if(len > 1){
			char *p = strchr((char *)buf, '_');
			int index = p - (char *)buf;
			char t[3];
			strcpy(t, ((char *)buf)+ index + 1);
			t[2] = '\0';
			pckt_no = atoi(t);
			buf[index] = '\0';
			if(ack_no < pckt_no)
			{
				ack_no = pckt_no;
				if(fputs((char *) buf, fp) < 1){
					printf("SERVER: fputs() error\n");
				}
				strcpy(ack, "ACK_");
				char temp[5];
				snprintf(temp, 5, "%d", ack_no);
				strcat(ack, temp);
				printf("SERVER: packet_%d received\n", pckt_no);
			}
			else
			{
				strcpy(ack, "ACK_");
				char temp[5];
				snprintf(temp, 5, "%d", ack_no);
				strcat(ack, temp);
				printf("SERVER: packet_%d already received\n", pckt_no);
			}
			printf("SERVER: sending %s\n", ack);
			if(sendto(s, ack, strlen(ack), 0, (struct sockaddr *)&sin, sock_len)<0){
				perror("SendTo Error\n");
				exit(1);
			}
		}
        }
	fclose(fp);
	close(s);
}
