
#include "Socket.h"

/// Metodo que crea un Socket y Vincular dicho socket a la dirección de 
/// internet especificada mediante el argumento.
Socket::Socket(const sockaddr_in& address) 
{
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
     if (fd_ < 0) {
     throw std::system_error(errno, std::system_category(),
     "no se pudo crear el socket");
    }

    int result = bind(fd_, reinterpret_cast<const sockaddr*>(&address),sizeof(address));
     if (result < 0) {
     throw std::system_error(errno, std::system_category(),
     "fallo en el bind");
    }
}

/// Liberar recursos
Socket::~Socket()
{
    close(fd_);
}

/// Send_to para decir cuando acaba el mensaje.

void Socket::send_to(const  uint8_t  & data, const sockaddr_in& address)
{
    int result = sendto(fd_, &data, sizeof(data), 0, reinterpret_cast<const sockaddr*>(&address),sizeof(address));
    if (result < 0) {
     throw std::system_error(errno, std::system_category(),
     "fallo en el sendto2");
    }
}

/// Send_to para enviar el mensaje leido desde un fichero.

void
Socket::send_to(const Message& message, const sockaddr_in& address)
{ 
    int result = sendto(fd_, &message, sizeof(message), 0, reinterpret_cast<const sockaddr*>(&address),sizeof(address));
     if (result < 0) {
     throw std::system_error(errno, std::system_category(),
     "fallo en el sendto");
    }
    std::cout <<"Se envio el mensaje correctamente"<< std::endl;
}

/// Receive que recibe por argumentos un short int para indicar la finalizacion
/// del programa.

void
Socket::receive_from(uint8_t& data, sockaddr_in& address) {
    socklen_t src_len;
    src_len = sizeof(address);
    int result = recvfrom(fd_, &data, sizeof(data), 0, reinterpret_cast<sockaddr*>(&address), &src_len);
     if (result < 0) {
     throw std::system_error(errno, std::system_category(),
     "fallo en el recvfrom2");
    }
}

/// Recibe el mensaje del servidor y dice si se envio o no correctamente

void
Socket::receive_from(Message& message, sockaddr_in& address)
{
    socklen_t src_len;
    src_len = sizeof(address);
    int result = recvfrom(fd_, &message, sizeof(message), 0, reinterpret_cast<sockaddr*>(&address), &src_len);
     if (result < 0) {
     throw std::system_error(errno, std::system_category(),
     "fallo en el recvfrom");
    }
}