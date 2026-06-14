#include <cstdint>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "guiconnector.hpp"
#include "debug.hpp"
#include "gui/picojson.h"

GuiConnector::GuiConnector(const uint16_t port) {
    this->port = port;
    memset(&this->self_addr, 0, sizeof(struct sockaddr_in));
    memset(&this->editor_addr, 0, sizeof(struct sockaddr_in));
    self_addr.sin_addr.s_addr = INADDR_ANY;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(port);
    editor_addr.sin_port = htons(port-1); // Port 6767, pretty bad way to define it but whatever
    this->gui_socket = -1;
}

bool GuiConnector::bind_socket() {
    gui_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(gui_socket < 0) {
        LOG_I("Application error: failed to create socket");
        exit(2);
    }
    socklen_t saddr = sizeof(struct sockaddr_in);
    const int one = 1;
    setsockopt(gui_socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(bind(gui_socket, (struct sockaddr*)&self_addr, saddr) < 0) {
        LOG_I("Application error: failed to bind socket");
        perror("socket");
        exit(2);
    }
    return true;
}


bool GuiConnector::receive_command(picojson::value &received) {
    int recv_len;
    socklen_t saddr = sizeof(struct sockaddr_in);
    recv_len = recvfrom(gui_socket, datagram_buffer.data(), NETWORK_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)&editor_addr, &saddr);
    std::string decode_error = picojson::parse(received, datagram_buffer.data());
    if(!decode_error.empty() || !received.is<picojson::object>()) {
        LOG_I("Application error: received malformed command, ignoring it.");
        return false;
    }
    return true;
}

void GuiConnector::send_buffer(char *buffer) {
    // Gets unlocked before return
    std::lock_guard<std::mutex> lg(this->con_mutex);
    socklen_t saddr = sizeof(struct sockaddr_in);
    editor_addr.sin_port = htons(port-1);
    sendto(gui_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&editor_addr, saddr);
}

void GuiConnector::end_connection() {
    if(gui_socket != -1) {
        close(gui_socket);
    }
}

GuiConnector::~GuiConnector() {
    // Make sure we close the socket when the instance is destroyed
    end_connection();
}