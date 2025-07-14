#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

struct User {
    std::string login;
    std::string password;
};

std::vector<User> database;

bool registerUser(const std::string& login, const std::string& password) {
    for (const auto& user : database) {
        if (user.login == login)
            return false;
    }
    database.push_back({login, password});
    return true;
}

bool loginUser(const std::string& login, const std::string& password) {
    for (const auto& user : database) {
        if (user.login == login && user.password == password)
            return true;
    }
    return false;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientLen = sizeof(clientAddr);
    char buffer[1024] = {};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock\n";
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета\n";
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, SOMAXCONN);
    std::cout << "Сервер запущен на порту 8080\n";

    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Ошибка при подключении клиента\n";
            continue;
        }

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            continue;
        }
        buffer[bytesReceived] = '\0';

        std::string input(buffer);
        std::string response;

        size_t pos1 = input.find(' ');
        size_t pos2 = input.find(' ', pos1 + 1);
        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            response = "Некорректный формат запроса";
        } else {
            std::string command = input.substr(0, pos1);
            std::string login = input.substr(pos1 + 1, pos2 - pos1 - 1);
            std::string password = input.substr(pos2 + 1);

            if (command == "REGISTER") {
                if (registerUser(login, password))
                    response = "Регистрация успешна";
                else
                    response = "Пользователь уже существует";
            } else if (command == "LOGIN") {
                if (loginUser(login, password))
                    response = "Вход выполнен";
                else
                    response = "Неверные данные";
            } else {
                response = "Неизвестная команда";
            }
        }

        send(clientSocket, response.c_str(), response.size(), 0);
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
