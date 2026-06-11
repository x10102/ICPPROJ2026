#ifndef __GUI_CONNECTOR_H
#define __GUI_CONNECTOR_H

#include <array>
#include <cstdint>
#include <future>
#include <netinet/in.h>
#include <sys/socket.h>

#include "gui/picojson.h"

// 8kb seems fair
static constexpr uint32_t NETWORK_BUFFER_SIZE = 8192;

// Funny constant that we need for setsockopt because sockets in the unix kernel are silly
static constexpr int one = 1;

class GuiConnector {
    public:
        bool receive_command(picojson::value &received);
        bool bind_socket();
        void send_buffer(char *buffer);
        void end_connection();
        GuiConnector(const uint16_t port);
        ~GuiConnector();

    private:
        uint16_t port;
        struct sockaddr_in self_addr;
        struct sockaddr_in editor_addr;
        int gui_socket;
        std::mutex con_mutex;
        std::array<char, NETWORK_BUFFER_SIZE> datagram_buffer;

};

#endif