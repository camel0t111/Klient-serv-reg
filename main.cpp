#include <iostream>              // для cin, cout
#include <winsock2.h>            // сокети Windows
#include <ws2tcpip.h>            // inet_pton
#include <string>                // string, щоб зчитати логін/пароль

#pragma comment(lib, "ws2_32.lib")  // підключаємо бібліотеку сокетів

#define SERVER_IP "127.0.0.1"    // адреса сервера (локальна машина)
#define PORT 8080                // порт, має бути такий самий як у сервера
#define BUFFER_SIZE 1024         // буфер для прийому/відправки

int main() {
    WSADATA wsaData;             // структура для ініціалізації Winsock
    SOCKET clientSocket;         // клієнтський сокет
    sockaddr_in serverAddr;      // структура для з'єднання з сервером
    char buffer[BUFFER_SIZE];    // буфер для отримання відповіді від сервера

    // запускаємо Winsock (обов'язково на Windows)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Не вдалося запустити WSA\n";
        return 1;
    }

    // створюємо TCP сокет
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Не вдалося створити сокет\n";
        WSACleanup();
        return 1;
    }

    // задаємо IP і порт сервера
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);   // строку IP -> формат для сокетів
    serverAddr.sin_port = htons(PORT);                     // порт в мережевому порядку байтів

    // намагаємося підключитись
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Підключення не вдалося\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Підключено до сервера!\n";

    std::string command, login, password;

    std::cout << "Введи команду (REGISTER або LOGIN): ";
    std::cin >> command;

    std::cout << "Логін: ";
    std::cin >> login;

    std::cout << "Пароль: ";
    std::cin >> password;

    // склеюємо в одне повідомлення
    std::string request = command + " " + login + " " + password;

    // шлемо на сервер
    send(clientSocket, request.c_str(), request.length(), 0);

    // приймаємо відповідь
    memset(buffer, 0, BUFFER_SIZE); // чистимо буфер
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // кінець строки
        std::cout << "Сервер відповів: " << buffer << "\n";
    } else {
        std::cout << "Немає відповіді від сервера або помилка\n";
    }

    closesocket(clientSocket); // закриваємо клієнтський сокет
    WSACleanup();              // прибираємо за собою
    return 0;
}
