#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>

#define TIME_PORT	27015

enum Request {
	TIME,
	TIME_WITHOUT_DATE,
	TIME_SINCE_EPOCH,
	CLIENT_TO_SERVER_DELAY_ESTIMATION,
	MEASURE_RTT,
	TIME_WITHOUT_DATE_OR_SECONDS,
	YEAR,
	MONTH_AND_DAY,
	SECONDS_SINCE_BEGINNING_OF_MONTH,
	WEEK_OF_YEAR,
	DAYLIGHT_SAVINGS,
	TIME_WITHOUT_DATE_IN_CITY,
	MEASURE_TIME_LAP,
	PING
};

enum City {
	NONE,
	DOHA,
	PRAGUE,
	NEW_YORK,
	BERLIN
};

time_t* timer_start = NULL;

char* processRequest(SOCKET m_socket, const Request& request, sockaddr* client_addr, int* client_addr_len, const City& city = City::NONE);

void main()
{

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called m_socket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							datagram sockets (SOCK_DGRAM), 
	//							and the UDP/IP protocol (IPPROTO_UDP).
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The "if" statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == m_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must first bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigdned long (4 bytes) data type).
	// INADDR_ANY means to listen on all interfaces.
	// inet_addr (Internet address) is used to convert a string (char *) into unsigned int.
	// inet_ntoa (Internet address) is the reverse function (converts unsigned int to char *)
	// The IP address 127.0.0.1 is the host itself, it's actually a loop-back.
	serverService.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR *)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		return;
	}

	// Waits for incoming requests from clients.

	// Send and receive data.
	sockaddr client_addr;
	int client_addr_len = sizeof(client_addr);
	int bytesSent = 0;
	int bytesRecv = 0;
	char recvBuff[255];

	// Get client's requests and answer them.
	// The recvfrom function receives a datagram and stores the source address.
	// The buffer for data to be received and its available size are 
	// returned by recvfrom. The fourth argument is an idicator 
	// specifying the way in which the call is made (0 for default).
	// The two last arguments are optional and will hold the details of the client for further communication. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	cout << "Time Server: Wait for clients' requests.\n";

	while (true)
	{
		bytesRecv = recvfrom(m_socket, recvBuff, 255, 0, &client_addr, &client_addr_len);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recvfrom(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";

		char* choiceStr = strtok(recvBuff, " ");
		char* cityStr = NULL;
		int choice = atoi(choiceStr);
		City chosenCity = City::NONE;

		if (choice == Request::TIME_WITHOUT_DATE_IN_CITY + 1) {
			cityStr = strtok(NULL, " ");
			for (int i = 0; cityStr[i]; i++) {
				cityStr[i] = tolower(cityStr[i]);
			}

			if (strcmp(cityStr, "doha") == 0) {
				chosenCity = DOHA;
			}
			else if (strcmp(cityStr, "prague") == 0) {
				chosenCity = PRAGUE;
			}
			else if (strcmp(cityStr, "new york") == 0) {
				chosenCity = NEW_YORK;
			}
			else if (strcmp(cityStr, "berlin") == 0) {
				chosenCity = BERLIN;
			}
		}

		char* response;
		response = processRequest(m_socket, Request(choice - 1), &client_addr, &client_addr_len, chosenCity);
		if (response != NULL) {
			response[strlen(response)] = '\0'; //to remove the new-line from the created string

			// Sends the answer to the client, using the client address gathered
			// by recvfrom. 
			bytesSent = sendto(m_socket, response, (int)strlen(response), 0, (const sockaddr*)&client_addr, client_addr_len);

			if (SOCKET_ERROR == bytesSent)
			{
				cout << "Time Server: Error at sendto(): " << WSAGetLastError() << endl;
				delete response;
				closesocket(m_socket);
				WSACleanup();
				return;
			}

			cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(response) << " bytes of \"" << response << "\" message.\n";
			delete response;
		}
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(m_socket);
	WSACleanup();
}

char* getTimeWithoutDateInCity(const time_t& now, const City& city = City::NONE) {
	char* response = new char[255];
	tm* t = gmtime(&now);
	t->tm_isdst = -1;

	switch (city) {
	case City::DOHA:
		t->tm_hour += 3;
		break;
	case City::PRAGUE:
		t->tm_hour += 1;
		break;
	case City::NEW_YORK:
		t->tm_hour -= 5;
		break;
	case City::BERLIN:
		t->tm_hour += 1;
		break;
	}

	mktime(t);
	strftime(response, 255, "%H:%M:%S", t);
	return response;
}

char* processRequest(SOCKET m_socket, const Request& request, sockaddr* client_addr, int* client_addr_len, const City& city) {
	char* response = new char[255];
	time_t now;
	time(&now);

	switch (request) {
		case Request::TIME: {
			strcpy(response, ctime(&now));
			response[strlen(response) - 1] = '\0';
			break;
		}
		case Request::TIME_WITHOUT_DATE: {
			strftime(response, 255, "%H:%M:%S", localtime(&now));
			break;
		}
		case Request::TIME_SINCE_EPOCH: {
			sprintf(response, "%I64u", time(NULL));
			break;
		}
		case Request::CLIENT_TO_SERVER_DELAY_ESTIMATION: {
			sprintf(response, "%I64u", GetTickCount64());
			break;
		}
		case Request::MEASURE_RTT: {
			sprintf(response, "%I64u", GetTickCount64());
			break;
		}
		case Request::TIME_WITHOUT_DATE_OR_SECONDS: {
			strftime(response, 255, "%H:%M", localtime(&now));
			break;
		}
		case Request::YEAR: {
			strftime(response, 255, "%Y", localtime(&now));
			break;
		}
		case Request::MONTH_AND_DAY: {
			strftime(response, 255, "%d/%m", localtime(&now));
			break;
		}
		case Request::SECONDS_SINCE_BEGINNING_OF_MONTH: {
			tm time_maker = *localtime(&now);
			time_maker.tm_mday = 1;
			time_maker.tm_hour = 0;
			time_maker.tm_min = 0;
			time_maker.tm_sec = 0;
			time_t time_of_month_start = mktime(&time_maker);
			sprintf(response, "%.0f", floor(difftime(now, time_of_month_start)));
			break;
		}
		case Request::WEEK_OF_YEAR: {
			tm time_maker = *localtime(&now);
			time_maker.tm_mon = 0;
			time_maker.tm_mday = 1;
			time_maker.tm_hour = 0;
			time_maker.tm_min = 0;
			time_maker.tm_sec = 0;
			time_t time_of_year_start = mktime(&time_maker);
			sprintf(response, "%.0f", floor(difftime(now, time_of_year_start) / 60 / 60 / 24 / 7));
			break;
		}
		case Request::DAYLIGHT_SAVINGS: {
			sprintf(response, "%d", localtime(&now)->tm_isdst);
			break;
		}
		case Request::TIME_WITHOUT_DATE_IN_CITY: {
			char* timeWithoutDateInCity = getTimeWithoutDateInCity(now, city);
			strcpy(response, timeWithoutDateInCity);
			delete[] timeWithoutDateInCity;
			break;
		}
		case Request::MEASURE_TIME_LAP: {
			if (timer_start == NULL || difftime(now, *timer_start) / 60 > 3) {
				delete timer_start;
				timer_start = new time_t(now);
				strcpy(response, "Timer started! - Send another request to stop.");
			}
			else {
				sprintf(response, "%.0f", difftime(now, *timer_start));
				delete timer_start;
				timer_start = NULL;
			}
			break;
		}
		case Request::PING: {
			sprintf(response, "%I64u", GetTickCount64());
			break;
		}
	}

	return response;
}