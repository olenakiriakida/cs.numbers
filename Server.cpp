#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        return 1;
    }

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << "\n";
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "socket failed with error: " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "bind failed with error: " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cout << "listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    SOCKET ClientSocket;
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "accept failed with error: " << WSAGetLastError() << "\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    while (true)
    {
        float receivedNumber;
        int iResult = recv(ClientSocket, (char*)&receivedNumber, sizeof(receivedNumber), 0);
        if (iResult > 0) {
            cout << "Received: " << receivedNumber << "\n";
            float response = receivedNumber + 1;
            iResult = send(ClientSocket, (char*)&response, sizeof(response), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "send failed with error: " << WSAGetLastError() << "\n";
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (iResult == 0)
            cout << "Connection with the client is closed.\n";
        else {
            cout << "recv failed with error: " << WSAGetLastError() << "\n";
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    }

    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}