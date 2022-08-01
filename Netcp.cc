/** Univerdidad: Universidad de La Laguna.
* Grado:        Grado de ingeniería informática.
*  Asignatura:  Asignatura de sistemas operativos.
*  Autor:       Joseph Gabino Rodríguez.
*  Correo:      alu0101329161@ull.edu.es
*  Práctica nº: Sockets
*  Comentario:  Segundo prototipo de programa Netcp en que dos programas
*               se van a comunicar por medio de Sockets y se van a enviar 
*               mensajes
*  Compilar:    g++ -g -pthread -o Netcp Netcp.cc Socket.cc Fichero.cc
*               ./Netcp (192.168.1.37)ip (5555)puerto
*/

#include "Socket.h"
#include "Fichero.h"
#include "Funciones.h"

#include <atomic>
#include <csignal>
#include <exception>
#include <thread>
#include <vector>


/// Tarea 3
void
NetcpReceive(int port, std::string& ip_address,
std::atomic_bool& abort_receive, std::atomic_bool& fallo, std::atomic_bool& wait, std::string& dir) {

  try {
  /// Declaro la estructura donde se guarda la información necesaria para
  /// realizar la conexión.
  struct sockaddr_in cliente, servidor;
  /// Guardo los datos pasados por parámetros y pongo una ip de linux.
  cliente = make_ip_address(port,"127.0.0.1");
  servidor = make_ip_address(0, ip_address);

  /// Creo objeto Socket localy le asigno una estructura.
  Socket cliente_local(cliente);
  struct Message message;
  std::string lenght_string;
  std::string name;
  int lenght;

  std::cout<<"Esperando mensaje"<<std::endl;
  int recorrido = 0;

  std::this_thread::sleep_for(std::chrono::seconds(8));
  if(abort_receive)
   throw true; 

  //Recibir nombre y tamaño , los guardo.
  wait = true;
  cliente_local.receive_from(message, servidor);
  wait = false;

  std::cout<<"Recibiendo Mensaje con datos"<<std::endl;
  while (message.text[recorrido] != '\0' ) {
   name.push_back(message.text[recorrido]);
   recorrido ++;
  }
  recorrido = 0;

  wait = true;
  cliente_local.receive_from(message, servidor);
  wait = false;

  while (message.text[recorrido] != '\0' ) {
   lenght_string.push_back(message.text[recorrido]);
   recorrido ++;
  }
  lenght = stoi(lenght_string);
  name = dir + '/' + name;

  std::this_thread::sleep_for(std::chrono::seconds(8));
  if(abort_receive)
   throw true; 

  std::cout<<"Creando fichero nuevo"<<std::endl;
  /// He recibido el nombre del fichero y el tamaño
  Fichero fichero(name, O_RDWR | O_CREAT | O_TRUNC,
  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,lenght, dir);

 std::cout<<"Recibiendo Mensaje de fichero"<<std::endl;

  do { 
  cliente_local.receive_from(message, servidor);
  fichero.ReceiveMessage(message.text);
  } while (fichero.GetEnd() == false);
  std::cout<<"Mensaje de fichero Recibido"<<std::endl;
  }
  catch(std::system_error& e) {
    fallo = true;
   std::cerr << "mitalk" << ": " << e.what() << '\n';
   if (e.code().value() != EINTR)
      std::cerr << e.what() << '\n';
  }
  catch(...) {
    fallo = true;
    abort_receive = false;
    std::cout << "Se aborto el recibo\n";
  }
}

/// Tarea 2
void
NetcpSend(int port, std::string& ip_address, std::atomic_bool& pause,
std::atomic_bool& abort, std::atomic_bool& fallo, std::string& archivo) {

  try{
  /// Declaro la estructura donde se guarda la información necesaria para
  /// realizar la conexión.
  struct sockaddr_in cliente, servidor;
  /// Guardo los datos pasados por parámetros y pongo una ip de linux.
  cliente = make_ip_address(port,"127.0.0.1");
  servidor = make_ip_address(0,ip_address);
  /// Creo objeto Socket remoto y le asigno una estructura.
  Socket servidor_remoto(servidor);
  struct Message message;

  std::string name1 = "prueba1.txt";
  int lenght;
  std::string lenght_string;
  std::cout<<"Abriendo fichero existente"<<std::endl;
  Fichero fichero(archivo, O_RDONLY);
  std::cout<<"Enviando Mensaje con datos"<<std::endl;

  lenght_string = fichero.GetLengthstring();
  for(unsigned desplazamiento = 0; desplazamiento <= name1.size(); desplazamiento ++){
    message.text[desplazamiento]= name1[desplazamiento];
  }

  std::this_thread::sleep_for(std::chrono::seconds(8));
  if(abort)
    throw true; 
  if(pause)
   while(pause) {}

  servidor_remoto.send_to(message, cliente);

  for(unsigned recorrido =0;recorrido <= lenght_string.size(); recorrido ++){
    message.text[recorrido]= lenght_string[recorrido];
  }
  servidor_remoto.send_to(message, cliente);
  message.Clean();

  std::this_thread::sleep_for(std::chrono::seconds(8));
  if(abort)
    throw true; 
  if(pause)
  while(pause) {}

  std::cout<<"Enviando Mensaje del fichero"<<std::endl;
  do {
  if(fichero.GetEnd() == false) {
   message.Clean();
   fichero.SendMessage(message.text);
   servidor_remoto.send_to(message, cliente);
  }
  }while(fichero.GetEnd() == false);
  }
  catch(std::system_error& e) {
   std::cerr << "mitalk" << ": " << e.what() << '\n';
  }
  catch(...) {
    fallo = true;
    abort = false;
    std::cout << "Aborto el Send\n";
  }

}


/// Tarea 1 que empieza en el main
void
Read (int port, std::exception_ptr eptr, std::atomic_bool& acabar, std::string& ip_address) {
try{
  std::cout<<"Comienza el programa"<<std::endl;

  std::string lectura, dato, archivo, directorio;
  std::thread process2, process3;
  std::vector<std::string> separador;
  size_t found;
  // Para el envio y lo reanuda
  std::atomic_bool pause {false};
  // Hace que ya no pueda recibir
  std::atomic_bool abort_receive {false};
  // aborta el envio
  std::atomic_bool abort {false};
  // detiene esperas indefinidas
  std::atomic_bool wait {false};
  // Errores del send
  std::atomic_bool fallo {false};
  // Errores del receive
  std::atomic_bool fallo2 {false};


  while (lectura != "Quit" && acabar == false) {
    getline(std::cin, lectura);

    found = lectura.find("[");
    separador.push_back(lectura.substr(0,found));
    separador.push_back(lectura.substr(found+1,lectura.size()));

    SplitString(lectura, dato);

    /// En caso de fallo el programa acaba su ejecución de manera ordenada.

    if(fallo) {
      process2.join();
      fallo = false;
      pause=false;
    }
    if(fallo2) {
      process3.join();
      fallo = false;
      pause=false;
    }

    if(lectura == "Send" && dato.size() != 0) {
      std::cout<<" -> thread2"<<std::endl;
      archivo = dato;
      process2 = std::thread (&NetcpSend, port, std::ref(ip_address), 
      std::ref(pause), std::ref(abort), std::ref(fallo), std::ref(archivo));
    }
    if(lectura == "Send" && dato.size() == 0)
      std::cout << "Introduzca el nombre del archivo" << std::endl;

    if(lectura == "Receive" && dato.size() != 0) {
      std::cout<<" -> thread3"<<std::endl;
      directorio = dato;
      process3 = std::thread (&NetcpReceive, port, std::ref(ip_address), 
      std::ref(abort_receive), std::ref(fallo2),std::ref(wait), std::ref(directorio));
    }
    if(lectura == "Receive" && dato.size() == 0)
      std::cout << "Introduzca el directorio para guardar archivo" << std::endl;
  
    if(lectura == "Pause" && dato.size() == 0) { 
      pause = true;
      std::cout<<"---MENSAJE PAUSADO---"<<std::endl;
    }
    if(lectura == "Resume" && dato.size() == 0) { 
      pause = false;
      std::cout<<"---MENSAJE CONTINUA---"<<std::endl;
    }
    if(lectura == "Abort" && dato.size() == 0) { 
      abort = true;
      std::cout<<"---ABORTO---"<<std::endl;
    }
    if(lectura == "Abort" && dato == "Receive") { 
      abort_receive = true;
    /// En caso de espera indefinida se manda una señal para acabar con el hilo.
    if(wait == true) {
      if (pthread_kill(process3.native_handle(), SIGUSR1) != 0)
        throw std::invalid_argument("Se aborto Espera Indefinida");
    }
    std::cout<<"---SE ABORTO EL RECIBO---"<<std::endl;
    }

    if(lectura != "Quit")
      lectura.clear();

    dato.clear();
  }
    process2.join();
    process3.join();
    std::cout<<"Programa ha finalizado"<<std::endl;
} 
catch (...) {
  eptr = std::current_exception();
}

}

int protected_main(int argc, char* argv[])
{
  std::string ayuda = "--help";
  /// Si el usuario escribe --help.
    if (argc == 2 && argv[1] == ayuda) {
    throw std::bad_alloc();
  }
  /// Se tienen que introducir los parametos correctamente.
  if (argc != 3) {
    throw std::invalid_argument("Modo de empleo: ./Netcp <ip> <puerto>"
    " Pruebe ./Netcp --help’ para más información.");
  }
  std::exception_ptr eptr {};
  std::atomic_bool acabar {false};
  int port = atoi(argv[2]);
  std::string ip_address = argv[1];

  struct sigaction signal;
  sigaction (SIGUSR1, &signal, NULL);

  sigset_t sigwaitset;
  sigemptyset( &sigwaitset );
  sigaddset ( &sigwaitset, SIGHUP);
  sigaddset ( &sigwaitset, SIGTERM);
  sigaddset ( &sigwaitset, SIGINT);
  sigprocmask( SIG_BLOCK, &sigwaitset, NULL );


  std::cout<<" -> thread1"<<std::endl;
  std::thread process1(&Read, port, std::ref(eptr),std::ref(acabar), std::ref(ip_address));
  std::thread block_signal(&killSignal, std::ref(sigwaitset), std::ref(acabar), std::ref(process1));
  block_signal.detach();

  process1.join();

  if (eptr) 
		std::rethrow_exception(eptr);

}

// Crea el hilo de la tarea 1
int main(int argc, char* argv[]) {

  try {
   return protected_main(argc, argv);
	}	
	catch(std::bad_alloc& e) {
	std::cerr << "mytalk" << ":" << "Este programa es un prototipo multihilo que usa varios "
    "procesos y además usa archivos mapeados en memoria. La ip introducida"
    "debe ser la ipv4 y hay que comprobar que ese puerto no este siendo ocupado ya" << '\n';
  std::cerr << "mytalk" << ":" << "Los comandos admitidos son Receive directorio,Send prueba.txt "
  "Pause(Pausa el programa), Resume(reanuda el programa), Abort(aborta envio), Abort Receive(aborta recibo)";
  return 1;
	}
  catch(std::system_error& e) {
   std::cerr << "mitalk" << ": " << e.what() << '\n';
	return 2;
  }
   catch(std::invalid_argument& e) {
   std::cerr << "mitalk" << ": " << e.what() << '\n';
	return 3;
  }
  catch (...) {
	std::cout << "Error desconocido\n";
  }
  return 99;
}

