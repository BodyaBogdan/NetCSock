#include "stdafx.h"

struct Request {
	double a, b, c;
};

struct Response {
	int type;
	double x1, x2;
};

bool parse_cmd(int argc, char* argv[], char* host, short* port);
void error_msg(const char*);
void exit_handler();

SOCKET client_socket;
int main(int argc, char* argv[])
{
	atexit(exit_handler);
	short port;
	char host[128] = "";
	bool parse_cmd_result = parse_cmd(argc, argv, host, &port);

	if (!parse_cmd_result || !host || !strlen(host))
	{
		printf("Invalid host or port. Usage %s -h host -p port\n", argv[0]);
		return -1;
	}

	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		error_msg("Error init of WinSock2");
		return -1;
	}

	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket <= 0) {
		error_msg("Can't create socket");
		return -1;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(host);

	//Connect to the server
	if (connect(client_socket, (sockaddr*)&server_addr, sizeof(sockaddr))) {
		char err_msg[128] = "";
		sprintf(err_msg, "Can't connect to the server %s:%d", host, port);
		error_msg(err_msg);
		return -1;
	}

	printf("Connection to the server %s:%d success\n", host, port);



	Request req;
	Response res;

	printf("%s", "Enter coefs of square eq:");
	scanf("%lf %lf %lf", &(req.a), &(req.b), &(req.c));
	printf("a = %0.2f; b = %0.2f; c = %0.2f; \n", req.a, req.b, req.c);

	int sc = send(client_socket, (char*)&req, sizeof(req), 0);
	if (sc <= 0) {
		char err_msg[128] = "";
		sprintf(err_msg, "Can't send data to the server %s:%d", host, port);
		error_msg(err_msg);
		return -1;
	}

	{
		sc = recv(client_socket, (char*)&res, sizeof(res), 0);
		if (sc <= 0)
		{
			printf("Server is not able\n");
			return -1;
		}
		switch (res.type)
		{
		case 0: printf("no root\n");
			break;
		case 1: printf("one root %f", res.x1);
			break;
		case 2: printf("two roots %f %f", res.x1, res.x2);
			break;
		default:
			printf("err response");
			break;
		}
	}

	closesocket(client_socket);
	Sleep(3000);
	return 0;
}

bool parse_cmd(int argc, char* argv[], char* host, short* port)
{
	if (argc < 2) {
		return false;
	}

	char all_args[256] = "";

	for (int i = 1; i < argc; ++i) {
		strcat(all_args, argv[i]);
		strcat(all_args, " ");
	}

	const int count_vars = 2;
	const int host_buf_sz = 128;
	int tmp_ports[count_vars] = { -1, -1 };
	char tmp_hosts[count_vars][host_buf_sz];
	for (int i = 0; i < count_vars; ++i) {
		memset(tmp_hosts[i], 0, host_buf_sz);
	}
	char* formats[count_vars] = { "-h %s -p %d", "-p %d -h %s" };

	int results[] = {
		sscanf(all_args, formats[0], tmp_hosts[0], &tmp_ports[0]) - 2,
		sscanf(all_args, formats[1], &tmp_ports[1], tmp_hosts[1]) - 2
	};

	for (int i = 0; i < sizeof(results) / sizeof(int); ++i) {
		if (!results[i]) {
			if (strlen(tmp_hosts[i]) > 0) {
				strcpy(host, tmp_hosts[i]);
			}
			if (tmp_ports[i] > 0) {
				*port = (short)tmp_ports[i];
				return true;
			}
		}
	}

	return false;

}

void error_msg(const char* msg) {
	printf("%s\n", msg);
}

void exit_handler()
{
	closesocket(client_socket);
	WSACleanup();
}
