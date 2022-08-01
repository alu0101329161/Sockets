

#include "Socket.h"

#include <atomic>
#include <csignal>
#include <exception>
#include <thread>


/// Introduzco en la estructura los datos necesarios
sockaddr_in 
make_ip_address(int port_, const std::string& ip_address_ =std::string())
{ 
    struct sockaddr_in socket{};  

    socket.sin_family = AF_INET;  

    /// Si el no se le pasa ip entonces se asigna una automaticamente.

    if(ip_address_.size() == 0)
      socket.sin_addr.s_addr=INADDR_ANY;
    else {
      char const* address = ip_address_.c_str();  
      int error = inet_aton(address, &socket.sin_addr); 
    }
    /// El puerto tiene que estar en entre estos valores.

    if (port_ >= 0  && port_ <= 65525)
      socket.sin_port = htons(port_);
    else
    throw std::invalid_argument("El puerto introducido no esta entre los valores"
    "posibles");

    return socket;
}

/// Divido los datos introducidos por teclado
void
SplitString(std::string& lectura, std::string& dato) {
  bool espacio = false;
  std::string aux, aux1;
  for (unsigned recorrido = 0; recorrido < lectura.size(); recorrido++) {
    if (lectura[recorrido] != ' ')
      aux.push_back(lectura[recorrido]);
      else
      espacio = true;
    if (espacio == true) {
      aux1 = aux;
      aux.clear();
      espacio = false;
    }
  }
  if (aux1.size() != 0 && aux.size() != 0) {
    dato = aux;
    lectura = aux1;
  }
  aux.clear();
  aux1.clear();
}


/// Acaba con proceso 1
void
killSignal(sigset_t& sigwaitset, std::atomic_bool& close, std::thread& process1) {
  int signum;
  sigwait(&sigwaitset, &signum);
  close = true;
  pthread_kill(process1.native_handle(), SIGUSR1);
}
