
#pragma once

#include "Message.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

/// Clase Socket que crea un objeto socket con una estructura que almacena
/// la informacion y además envia y recibe mensajes.
class Socket
{
 public:

    Socket(const sockaddr_in& address);
    ~Socket();
    void send_to(const  uint8_t& data, const sockaddr_in& address);
    void send_to(const Message& message, const sockaddr_in& address);
    void receive_from(uint8_t& data, sockaddr_in& address);
    void receive_from(Message& message, sockaddr_in& address);

 private:
    /// Descriptor del socket que se usa en varios metodos.
    int fd_;
};

