#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
    int clientSocket;
    int port = 5555;
    std::string serverAddress = "127.0.0.1";

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    struct sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert server address from string to network address structure
    if (inet_pton(AF_INET, serverAddress.c_str(), &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Invalid server address" << std::endl;
        return -1;
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    std::cout << "Connected to the server" << std::endl;

    // Receive welcome message from the server
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(clientSocket, buffer, BUFFER_SIZE, 0) <= 0) {
        std::cerr << "Error receiving welcome message" << std::endl;
        close(clientSocket);
        return -1;
    }

    std::cout << buffer << std::endl;

    // Send client's name to the server
    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    if (send(clientSocket, name.c_str(), name.length(), 0) <= 0) {
        std::cerr << "Error sending name" << std::endl;
        close(clientSocket);
        return -1;
    }

    // Game loop
    while (true) {
        // Receive question from the server
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) <= 0) {
            std::cerr << "Error receiving question" << std::endl;
            break;
        }

        else if (strcmp("GAME_OVER", buffer) == 0){
            std::cout << buffer << std::endl;
            break;
        }

        std::cout << "Question: " << buffer << std::endl;

        // Send answer to the server
        std::string answer;
        std::cout << "Enter your answer: ";
        std::getline(std::cin, answer);
        if (send(clientSocket, answer.c_str(), answer.length(), 0) <= 0) {
            std::cerr << "Error sending answer" << std::endl;
            break;
        }
    }

    // Receive client's score from the server
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(clientSocket, buffer, BUFFER_SIZE, 0) <= 0) {
        std::cerr << "Error receiving score" << std::endl;
    } else {
        std::cout << buffer << std::endl;
    }

    // Close the client socket
    close(clientSocket);

    return 0;
}
