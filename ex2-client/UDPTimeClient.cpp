#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h> 
#include <string.h>

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
	DOHA,
	PRAGUE,
	NEW_YORK,
	BERLIN
};

const int requestCount = 13;
const int supportedCityCount = 4;

void main()
{

	// Initialize Winsock (Windows Sockets).

	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";
		return;
	}

	// Client side:
	// Create a socket and connect to an internet address.

	SOCKET connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a client to communicate on a network, it must connect to a server.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called server. 
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(TIME_PORT);

	char requestToDescription[][255]{
		"What's the time?",
		"What's the time? (no date)",
		"What's the time since epoch? (in seconds)",
		"What's the delay estimation between the client and the server?",
		"What's the RTT measurement?",
		"What's the time? (no date & no seconds)",
		"What's the year?",
		"What's the month and day?",
		"What's the amount of seconds since the beginning of the month?",
		"What's the week number for the current year?",
		"Is it daylight savings time currently?",
		"What's the time in a specific city?.. (no date)",
		"What's the time-lap measurement? (in seconds)"
	};

	char cityToName[][255]{
		"Doha (Qatar)",
		"Prague (Czechia)",
		"New York (United States)",
		"Berlin (Germany)"
	};

	bool exited = false;
	while (!exited) {

		int i = 1, choice;
		for (i = 1; i <= requestCount; i++) {
			cout << i << ". " << requestToDescription[i - 1] << endl;
		}
		cout << "0. Exit" << endl;
		
		do {
			cout << "Enter selection number: ";
			char input[255];
			cin >> input;
			try {
				choice = atoi(input);
			}
			catch (exception ex) {
				choice = -1;
			}
			if (cin.fail() || choice > i || choice < 0) {
				cout << "Invalid selection!" << endl << "Please try again." << endl;
			}
		}
		while (choice > i || choice < 0);

		if (choice != 0) {
			// Send and receive data.

			int bytesSent = 0;
			int bytesRecv = 0;
			char sendBuff[255];
			sprintf(sendBuff, "%d", choice);
			char recvBuff[255];

			if (choice - 1 == Request::TIME_WITHOUT_DATE_IN_CITY) {
				cout << "Available cities: ";
				int j = 0;
				for (j = 0; j < supportedCityCount - 1; j++) {
					cout << cityToName[j] << ", ";
				}
				cout << cityToName[j] << endl << "Enter city name: ";
				int sendBuffLength = strlen(sendBuff);
				sendBuff[sendBuffLength] = ' ';
				cin >> sendBuff + sendBuffLength + 1;
			}

			// Asks the server what's the currnet time.
			// The send function sends data on a connected socket.
			// The buffer to be sent and its size are needed.
			// The fourth argument is an idicator specifying the way in which the call is made (0 for default).
			// The two last arguments hold the details of the server to communicate with. 
			// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
			bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&server, sizeof(server));
			if (SOCKET_ERROR == bytesSent)
			{
				cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
				closesocket(connSocket);
				WSACleanup();
				return;
			}
			cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

			// Gets the server's answer using simple recieve (no need to hold the server's address).
			bytesRecv = recv(connSocket, recvBuff, 255, 0);
			if (SOCKET_ERROR == bytesRecv)
			{
				cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
				closesocket(connSocket);
				WSACleanup();
				return;
			}

			recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
			cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";

			if (choice - 1 == Request::CLIENT_TO_SERVER_DELAY_ESTIMATION) {
				int sum = 0, prev = atoi(recvBuff), curr;
				for (int i = 0; i < 100; i++) {
					sprintf(sendBuff, "%d", Request::PING + 1);
					bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&server, sizeof(server));
					if (SOCKET_ERROR == bytesSent)
					{
						cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
						closesocket(connSocket);
						WSACleanup();
						return;
					}
					cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
				}
				for (int i = 0; i < 100; i++) {
					bytesRecv = recv(connSocket, recvBuff, 255, 0);
					if (SOCKET_ERROR == bytesRecv)
					{
						cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
						closesocket(connSocket);
						WSACleanup();
						return;
					}

					recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
					cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
					curr = atoi(recvBuff);
					sum += (curr - prev);
					prev = curr;
				}

				cout << "Client to server delay estimation: " << sum / 100 << " milliseconds" << endl;
			}

			else if (choice - 1 == Request::MEASURE_RTT) {
				int sum = 0, prev = atoi(recvBuff), curr;
				for (int i = 0; i < 100; i++) {
					sprintf(sendBuff, "%d", Request::PING + 1);
					bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&server, sizeof(server));
					if (SOCKET_ERROR == bytesSent)
					{
						cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
						closesocket(connSocket);
						WSACleanup();
						return;
					}
					cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

					bytesRecv = recv(connSocket, recvBuff, 255, 0);
					if (SOCKET_ERROR == bytesRecv)
					{
						cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
						closesocket(connSocket);
						WSACleanup();
						return;
					}

					recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
					cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
					curr = atoi(recvBuff);
					sum += (curr - prev);
					prev = curr;
				}

				cout << "RTT measurement: " << sum / 100 << " milliseconds" << endl;
			}
		}

		else {
			exited = true;
		}
	}

	// Closing connections and Winsock.
	cout << "Time Client: Closing Connection.\n";
	closesocket(connSocket);

	system("pause");
}