#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <unistd.h>
#include <cstring>

using namespace std;

class Client {
private:
    string name;
    string host = "127.0.0.1";
    int port;
    int period;
    int sock;

public:
    Client(string n, int p, int pr) : name(n), port(p), period(pr) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            cout<< "Could not create socket\n";
            exit(1);
        }
    }

    string get_time() {
        auto now = chrono::system_clock::now();
        auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = chrono::system_clock::to_time_t(now);

        stringstream ss;
        ss << put_time(localtime(&timer), "%Y-%m-%d %H:%M:%S") << '.' << setfill('0') << setw(3) << milliseconds.count();

        return ss.str();
    }

    void send_message() {
        while (1) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == -1) {
                cout<< "Could not create socket\n";
                exit(1);
            }

            struct sockaddr_in server;
            server.sin_addr.s_addr = inet_addr(host.c_str());
            server.sin_family = AF_INET;
            server.sin_port = htons(port);

            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
                cout<< "Connect failed. Error no is : " << std::strerror(errno) << "\n";
                close(sock);
                exit(1);
            }

            string message = "[" + get_time() + "] " + name;

            // Logging the message
            cout << "Sending message: " << message << "\n";

            if (send(sock, message.c_str(), message.size(), 0) < 0) {
                cout << "Send failed. Error no is : " << std::strerror(errno) << "\n";
                close(sock);
                exit(1);
            }

            close(sock);
            this_thread::sleep_for(chrono::seconds(period));
        }
    }
};

int main(int argc, char* argv[]) {
    if(argc != 4) {
        cerr << "Usage: ./client [NAME] [PORT] [PERIOD]\n";
        return 1;
    }

    string name = argv[1];
    int port = stoi(argv[2]);
    int period = stoi(argv[3]);

    Client client(name, port, period);
    client.send_message();

    return 0;
}
