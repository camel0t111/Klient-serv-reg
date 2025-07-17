#include <iostream>              // для cin, cout
#include <vector>                // зберігати список юзерів
#include <string>                // string — для логіна і пароля
#include <winsock2.h>            // сокети Windows
#include <ws2tcpip.h>            // inet_ntop і тд

#pragma comment(lib, "ws2_32.lib")  // сокетна бібліотека

#define PORT 8080                // порт сервера
#define BUFFER_SIZE 1024         // розмір буфера для прийому

// проста структура користувача
struct User {
    std::string login;
    std::string password;
};

// список усіх юзерів
std::vector<User> database;

// перевірка на існування юзера
bool registerUser(const std::string& login, const std::string& password) {
    for (const auto& user : database) {
        if (user.login == login) return false; // вже є такий
    }
    database.push_back({login, password});
    return true;
}

// логін перевірка
bool loginUser(const std::string& login, const std::string& password) {
    for (const auto& user : database) {
        if (user.login == login && user.password == password)
            return true;
    }
    return false;
}

int main() {
    WSADATA wsaData;              // для старту Winsock
    SOCKET serverSocket, clientSocket; // головний та клієнтський сокет
    sockaddr_in serverAddr, clientAddr;
    int clientLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];     // буфер під вхідні дані

    // ініціалізація сокетів
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSA не стартанув\n";
        return 1;
    }

    // створюємо TCP-сокет
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Не вийшло створити серверний сокет\n";
        WSACleanup();
        return 1;
    }

    // налаштовуємо адресу сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // слухаємо на всіх інтерфейсах
    serverAddr.sin_port = htons(PORT);

    // прив’язка до порта
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Помилка прив'язки до порта\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // слухаємо з'єднання
    listen(serverSocket, SOMAXCONN);
    std::cout << "Сервер працює на порту " << PORT << "\n";

    // нескінченний цикл прийому підключень
    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Клієнт не підключився\n";
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE); // чистимо буфер
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            continue;
        }
        buffer[bytesReceived] = '\0';

        std::string request(buffer);
        std::string response;

        // розділяємо на частини
        size_t p1 = request.find(' ');
        size_t p2 = request.find(' ', p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) {
            response = "Некоректний запит";
        } else {
            std::string command = request.substr(0, p1);
            std::string login = request.substr(p1 + 1, p2 - p1 - 1);
            std::string password = request.substr(p2 + 1);

            if (command == "REGISTER") {
                response = registerUser(login, password) ?
                    "Реєстрація успішна" : "Такий юзер вже є";
            } else if (command == "LOGIN") {
                response = loginUser(login, password) ?
                    "Вхід успішний" : "Невірний логін або пароль";
            } else {
                response = "Команда невідома";
            }
        }

        // надсилаємо відповідь
        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket); // закриваємо клієнта
    }

    // закриваємо сервер
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
