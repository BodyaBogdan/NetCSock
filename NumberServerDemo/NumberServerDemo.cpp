#include "stdafx.h"
#include <math.h>
#include <thread>

#define DEFAULT_PORT 5557
#define CONNECTION_QUEUE 100
#define M_PI (3.141592653589793)
#define M_2PI (2.*M_PI)

struct Request {
	double a, b, c;
};

struct Response {
	int type;
	double x1, x2;
};

bool parse_cmd(int argc, char* argv[], char* host, short* port);
void handle_connection(SOCKET, sockaddr_in*);
Response square_eq(Request&);
//Response Cubic(Request&);
void error_msg(const char*);
void exit_handler();

SOCKET server_socket;

int main(int argc, char* argv[])
{
	atexit(exit_handler);
	short port = DEFAULT_PORT;
	char host[128] = "";
	bool parse_cmd_result = parse_cmd(argc, argv, host, &port);

	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		error_msg("Error init of WinSock2");
		return -1;
	}

	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket <= 0) {
		error_msg("Can't create socket");
		return -1;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if (parse_cmd(argc, argv, host, &port) && strlen(host) > 0) {
		server_addr.sin_addr.s_addr = inet_addr(host);
	}
	else {
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	//Bind socket to the address on the server
	if (bind(server_socket, (sockaddr*)&server_addr, sizeof(sockaddr))) {
		char err_msg[128] = "";
		sprintf(err_msg, "Can't bind socket to the port %d", port);
		error_msg(err_msg);
		return -1;
	}

	if (listen(server_socket, CONNECTION_QUEUE)) {
		error_msg("Error listening socket");
		return -1;
	}

	printf("Server running at the port %d\n", port);

	while (true)
	{
		sockaddr_in incom_addr;
		memset(&incom_addr, 0, sizeof(incom_addr));
		int len = sizeof(incom_addr);
		SOCKET socket = accept(server_socket, (sockaddr*)&incom_addr, &len);
		if (socket <= 0) {
			error_msg("Can't accept connection");
			return -1;
		}


			std::thread thr(handle_connection, std::ref(socket), &incom_addr);////
			thr.detach();


		handle_connection(socket, &incom_addr);
	}
	closesocket(server_socket);

	return 0;
}

bool parse_cmd(int argc, char* argv[], char* host, short* port)
{
	if (argc < 2) {
		return false;
	}

	char all_args[256];
	memset(all_args, 0, sizeof all_args);

	for (int i = 1; i < argc; ++i) {
		strcat(all_args, argv[i]);
		strcat(all_args, " ");
	}


	const int count_vars = 3;
	const int host_buf_sz = 128;
	int tmp_ports[count_vars] = { -1, -1, -1 };
	char tmp_hosts[count_vars][host_buf_sz];
	for (int i = 0; i < count_vars; ++i) {
		memset(tmp_hosts[i], 0, host_buf_sz);
	}
	char* formats[count_vars] = { "-h %s -p %d", "-p %d -h %s", "-p %d" };

	int results[] = {
		sscanf(all_args, formats[0], tmp_hosts[0], &tmp_ports[0]) - 2,
		sscanf(all_args, formats[1], &tmp_ports[1], tmp_hosts[1]) - 2,
		sscanf(all_args, formats[2], &tmp_ports[2]) - 1
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

void handle_connection(SOCKET socket, sockaddr_in* addr) {
	char* str_in_addr = inet_ntoa(addr->sin_addr);
	printf("[%s]>>%s\n", str_in_addr, "Establish new connection");
	while (true) {
		Request rq;
		int rc = recv(socket, (char*)&rq, sizeof(rq), 0);
		if (rc > 0) {
			printf("[%s]:Received coefs a=%f, b=%f, c=%f\n", str_in_addr, rq.a, rq.b, rq.c);
			Response resp = square_eq(rq);
			//Response resp = Cubic(rq);
			rc = send(socket, (char*)&resp, sizeof(resp), 0);
			if (rc <= 0)
			{
				printf("Can't send response to client %s", str_in_addr);
				break;
			}
		}
		else {
			break;
		}
	}
	closesocket(socket);
	printf("[%s]>>%s", str_in_addr, "Close incomming connection\n");
}

/*
Response Cubic(Request& rq) {
	double q, r, r2, q3;
	Response res;
	q = (rq.a*rq.a - 3.*rq.b) / 9.; r = (rq.a*(2.*rq.a*rq.a - 9.*rq.b) + 27.*rq.c) / 54.;
	r2 = r*r; q3 = q*q*q;
	if (r2<q3) {
		double t = acos(r / sqrt(q3));
		rq.a /= 3.; q = -2.*sqrt(q);
		res.x1 = q*cos(t / 3.) - rq.a;
		res.x2 = q*cos((t + M_2PI) / 3.) - rq.a;
		res.x3 = q*cos((t - M_2PI) / 3.) - rq.a;
		res.type = 3;
		return res;
	}
	else {
		double aa, bb;
		if (r <= 0.) r = -r;
		aa = -pow(r + sqrt(r2 - q3), 1. / 3.);
		if (aa != 0.) bb = q / aa;
		else bb = 0.;
		rq.a /= 3.; q = aa + bb; r = aa - bb;
		res.x1 = q - rq.a;
		res.x2 = (-0.5)*q - rq.a;
		res.x3 = (sqrt(3.)*0.5)*fabs(r);
		if (res.x3 == 0.) { 
			res.type = 2; 
			return res;
		}
		res.type = 1;
		return res;
	}
}
*/

Response square_eq(Request& rq)
{
	Response res;
	double d = rq.b*rq.b - 4 * rq.a*rq.c;
	if (d > 0)
	{
		res.type = 2;
		res.x1 = ((-rq.b) + sqrt(d)) / (2 * rq.a);
		res.x2 = (rq.b + sqrt(d)) / (2 * rq.a);

	}
	else if (d < 0) {
		res.type = 0;
	}
	else {
		res.type = 1;
		res.x1 = -rq.b / rq.a;

	}
	return res;
}

void error_msg(const char* msg) {
	printf("%s\n", msg);
}

void exit_handler()
{
	closesocket(server_socket);
	WSACleanup();
}