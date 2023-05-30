#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>



#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

std::mutex mutex;

struct Client {
    int socket;
    std::string name;
    int score;
};

std::vector<Client> clients;


// Sample questions
const std::vector<std::string> questions = {
    "What is the capital of France?",
    "Who wrote the novel 'Pride and Prejudice'?",
    "What is the chemical symbol for the element Gold?",
    "Which planet is known as the Red Planet?",
    "What is the largest ocean on Earth?"
};

// Sample answers for the questions
const std::vector<std::string> answers = {
    "Paris",
    "Jane Austen",
    "Au",
    "Mars",
    "Pacific Ocean"
};

void handleClient(int clientSocket) {
    // int byte_sent, byte_received;
    int questionIndex = 0;
    if (send(clientSocket, "Welcome Client", sizeof("Welcome Client"), 0) <= 0) {
        std::cerr << "Error receiving welcome message" << std::endl;
        close(clientSocket);
        return;
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive client's name
    if (recv(clientSocket, buffer, BUFFER_SIZE, 0) <= 0) {
        std::cout << "Error receiving client name" << std::endl;
        close(clientSocket);
        return;
    }

    // Add the client to the list
    mutex.lock();
    Client client;
    client.socket = clientSocket;
    client.name = buffer;
    client.score = 0;
    clients.push_back(client);
    mutex.unlock();

    std::cout << "Client connected: " << client.name << std::endl;

    // Game loop
    while (true) {
        // Send question to the client
        std::string question = questions[questionIndex];
        if (send(clientSocket, question.c_str(), question.length(), 0) <= 0) {
            std::cout << "Error sending question" << std::endl;
            break;
        }

        // Receive client's answer
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) <= 0) {
            std::cout << "Error receiving answer" << std::endl;
            break;
        }

        // Check client's answer
        std::string answer = answers[questionIndex];
        std::string clientAnswer = buffer;
        if (clientAnswer == answer) {
            // Correct answer
            mutex.lock();
            client.score++;
            mutex.unlock();

            std::cout << "Client " << client.name << " answered correctly" << std::endl;
        }
        else{
            if (send(clientSocket, "GAME_OVER", sizeof("GAME_OVER"), 0) <= 0) {
                std::cout << "Error sending end game signal" << std::endl;
                break;
            }
            break;
        }

        // Move to the next question
        questionIndex++;

        // If all questions have been asked, end the game
        if (questionIndex >= questions.size()) {
            break;
        }
    }

    // Send client's score
    std::string scoreMessage = "Your score: " + std::to_string(client.score) + "\n";
    
    std::cout<<scoreMessage<<std::endl;
    if (send(clientSocket, scoreMessage.c_str(), scoreMessage.length(), 0) <= 0) {
        std::cout << "Error sending score" << std::endl;
    }

    std::cout << "Client " << client.name << " disconnected" << std::endl;

    // Wait for a short duration before closing the socket
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    close(clientSocket);

    // Remove the client from the list
    mutex.lock();
    clients.erase(std::remove_if(clients.begin(), clients.end(),[clientSocket](const Client& c) { return c.socket == clientSocket; }), clients.end());
    mutex.unlock();
}

int main() {
    int serverSocket;
    int port = 5555;
    struct sockaddr_in serverAddr {};

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified IP and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding failed" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Listening failed" << std::endl;
        return -1;
    }

    std::cout << "Server started. Listening on port " << port << std::endl;

    // Accept client connections and handle them in separate threads
    while (true) {
        struct sockaddr_in clientAddr {};
        socklen_t clientAddrSize = sizeof(clientAddr);

        // Accept a client connection
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        // Handle the client in a separate thread
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}