#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1

SOCKET ConnectSocket = INVALID_SOCKET;

DWORD WINAPI Sender(void* param)
{
    while (true)
    {
        float number;
        cout << "Enter a number: ";
        cin >> number;

        int iResult = send(ConnectSocket, (char*)&number, sizeof(number), 0);
        if (iResult == SOCKET_ERROR) {
            cout << "send failed with error: " << WSAGetLastError() << "\n";
            closesocket(ConnectSocket);
            WSACleanup();
            return 15;
        }
    }
    return 0;
}

DWORD WINAPI Receiver(void* param)
{
    while (true)
    {
        float receivedNumber;

        int iResult = recv(ConnectSocket, (char*)&receivedNumber, sizeof(receivedNumber), 0);
        if (iResult > 0) {
            cout << "Received: " << receivedNumber << "\n";
        }
        else if (iResult == 0)
            cout << "Connection with the server is closed.\n";
        else
            cout << "recv failed with error: " << WSAGetLastError() << "\n";
    }
    return 0;
}

int main()
{
    setlocale(0, "");
    system("title CLIENT SIDE");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        return 11;
    }

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const char* ip = "localhost";

    addrinfo* result = NULL;
    iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << "\n";
        WSACleanup();
        return 12;
    }

    for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 13;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Unable to connect to the server!\n";
        WSACleanup();
        return 14;
    }

    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
}