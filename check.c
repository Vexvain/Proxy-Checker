// Proxy Checker
// Format: IP PORT
// Usage: cat proxies.txt | ./checker 2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>

int Timeout;
#define ERROR "[\x1b[31m?\x1b[37m]"
#define FAILED "[\x1b[31m-\x1b[37m]"
#define SUCCESS "[\x1b[32m+\x1b[37m]"

void Trim(char *str)
{
    int i;
    int begin = 0;
    int end = strlen(str) - 1;
    while (isspace(str[begin])) begin++;
    while ((end >= begin) && isspace(str[end])) end--;
    for (i = begin; i <= end; i++) str[i - begin] = str[i];
    str[i - begin] = '\0';
}

void proxy(char *host, int port, char *output_file)
{
	int fd = -1;
	struct timeval timeout;
	struct sockaddr_in fds;
	timeout.tv_sec = Timeout;
	timeout.tv_usec = 0;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
	fds.sin_family = AF_INET;
	fds.sin_port = htons(port);
	fds.sin_addr.s_addr = inet_addr(host);
	if(connect(fd, (struct sockaddr *)&fds, sizeof(fds)) == -1)
	{
		// failed 2 connect
		printf(FAILED" %s:%d Timeout...\n", host, port);
		goto end;
	}
	else
	{
		// successful connection
		printf(SUCCESS" Connected to proxy [%s:%d]\n", host, port);
		char cmd[45];
		snprintf(cmd, sizeof(cmd), "echo '%s %d' >> %s", host, port, output_file);
		system(cmd);
		goto end;
	}
	end:
	close(fd);
	return;
}

int main(int argc, char **argv)
{
	char buffer[513];
	if(argc < 3 || argc > 3)
	{
		printf(ERROR" Usage: cat proxies.txt | %s <timeout> <outfile>\n", argv[0]);
		exit(0);
	}
	Timeout = atoi(argv[1]);
	while(fgets(buffer, sizeof(buffer) - 1, stdin))
	{
		if(strlen(buffer) < 3 || buffer == NULL)
			break;
		Trim(buffer);
		char *Host = strtok(buffer, " ");
		int port = atoi(Host+strlen(Host)+1);
		//printf("Host: %s\nPort: %d\n", Host, port);
		if(!(fork()))
		{
			proxy(Host, port, argv[2]);
			exit(0);
		}
	}
	return 0;
}
