//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2º de grado de Ingeniería Informática
//                       
//              This class processes an FTP transaction.
// 
//****************************************************************************


#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <netdb.h>

#include <array>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h> 
#include <iostream>
#include <dirent.h>

#include "common.h"

#include "ClientConnection.h"




ClientConnection::ClientConnection(int s, unsigned long ip) {

    int sock = (int)(s);
    char buffer[MAX_BUFF];
    control_socket = s;
    //Use to passive mode(127.0.0.1)
    server_address = ip; 

    // Function associates a stream with the existing file descriptor, fd
    fd = fdopen(s, "a+");
    if (fd == NULL){
	    std::cout << "Connection closed" << std::endl;
	    fclose(fd);
	    close(control_socket);
	    ok = false;
	    return ;
    }
    
    ok = true;
    data_socket = -1;
    parar = false;
}


ClientConnection::~ClientConnection() {

 	fclose(fd);
	close(control_socket); 
  
}


int connect_TCP( uint32_t address,  uint16_t  port) {

    struct sockaddr_in sin;
    int s;

    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0)
        errexit("No se ha podido crear el socket: %s\n", strerror(errno));

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = address;
    sin.sin_port = htons(port);

    if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        errexit("No se ha podido conectar con %s: %s\n", address, strerror(errno));

    return s;

}



void ClientConnection::stop() {
    close(data_socket);
    close(control_socket);
    parar = true;
  
}



#define COMMAND(cmd) strcmp(command, cmd)==0

// This method processes the requests.
// Here you should implement the actions related to the FTP commands.
// See the example for the USER command.
// If you think that you have to add other commands feel free to do so. You 
// are allowed to add auxiliary methods if necessary.

void ClientConnection::WaitForRequests() {
    if (!ok) {
	 return;
    }
    bool good_login;
    
    fprintf(fd, "220 Service ready\n");
  
    while(!parar) {

      fscanf(fd, "%s", command);

      // Command USER inplemented
      if (COMMAND("USER")) {
	    fscanf(fd, "%s", arg);
	    fprintf(fd, "331 User name ok, need password\n");
      }
      else if (COMMAND("PWD")) {
	   
      }

      // Command PASS inplemented
      else if (COMMAND("PASS")) {
        fscanf(fd, "%s", arg);
        if(strcmp(arg,"1234") == 0){
            fprintf(fd, "230 User logged in\n");
        }
        else{
            fprintf(fd, "530 Not logged in.\n");
            parar = true;
        }
        good_login = true;
      }
 
      // Command PORT inplemented 
      else if (COMMAND("PORT")) {
        // parar = false;
        unsigned int port[2];
        unsigned int ip[4];

        fscanf(fd, "%d,%d,%d,%d,%d,%d,", &ip[0],&ip[1],&ip[2],&ip[3], &port[0], &port[1]);
        // We save the ip and the port(>0 and <255)
        uint32_t ip_addr = ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0];
        uint16_t god_port = port[0] << 8 | port[1];
        data_socket = connect_TCP(ip_addr, god_port);

        fprintf(fd, "200 Okey\n");
      }

      // Command PASV inplemented
      else if (COMMAND("PASV")) {
        passive = true;
        printf("(PASSIVE):%d\n", passive); 

        struct sockaddr_in sin, sa;
        socklen_t sa_len = sizeof(sa);
        int s;

        s = socket(AF_INET, SOCK_STREAM, 0);

        if (s<0)
            errexit("Error al crear al socket: %s\n", strerror(errno));

        // We use class attribute to save the ip
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = server_address;//127.0.0.1
        sin.sin_port = 0;

        if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
            errexit("No se ha podido hacer bind con el puerto: %s\n", strerror(errno));

        // Listen for any request
        if (listen(s,5) < 0)
            errexit("Fallo en listen: %s\n", strerror(errno));

        getsockname(s, (struct sockaddr *)&sa, &sa_len);

        fprintf(fd, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",
                (unsigned)(server_address & 0xff),(unsigned)((server_address >> 8) & 0xff),
                (unsigned)((server_address >> 16) & 0xff),(unsigned)((server_address >> 24) & 0xff),
                (unsigned)(sa.sin_port & 0xff),(unsigned)(sa.sin_port >> 8));

        data_socket = s;
      }

      // Command STORE inplemented
      else if (COMMAND("STOR") ) {

        fscanf(fd, "%s", arg);
        printf("(STORE IN SERVER):%s\n", arg);

        // Open the file
        FILE* file = fopen(arg,"wb");

        //if not it opens correctly
        if (!file) {
            fprintf(fd, "450 Requested file action not taken. File unavaible.\n");
            close(data_socket);
        } else {

            fprintf(fd, "150 File status okay; about to open data connection.\n");
            fflush(fd);

            struct sockaddr_in sa;
            socklen_t sa_len = sizeof(sa);
            char buffer[MAX_BUFF];
            int n;
 
            if (passive)
                data_socket = accept(data_socket,(struct sockaddr *)&sa, &sa_len);

            do {
                n = recv(data_socket, buffer, MAX_BUFF, 0); 
                fwrite(buffer, sizeof(char), n, file);

            } while (n == MAX_BUFF);
                          
            fprintf(fd,"226 Closing data connection. Requested file action successful.\n");
            fclose(file);
            close(data_socket);
        }
      }

    // Command RETR inplemented
      else if (COMMAND("RETR")) {

        // Print in console
        fscanf(fd, "%s", arg);
        printf("COPY FROM THE SERVER\n");

        // Open the file
        FILE* file = fopen(arg,"rb");

        // If dont open correct
        if (!file) {
            fprintf(fd, "450 Requested file action not taken. File unavaible.\n");
            close(data_socket);
        } else {

            fprintf(fd, "150 File status okay; about to open data connection.\n");

            struct sockaddr_in sa;
            socklen_t sa_len = sizeof(sa);
            // Store the text
            char buffer[MAX_BUFF];
            int n;
 
            if (passive)
                data_socket = accept(data_socket,(struct sockaddr *)&sa, &sa_len);

            do {
                n = fread(buffer, sizeof(char), MAX_BUFF, file); 
                send(data_socket, buffer, n, 0);

            } while (n == MAX_BUFF); //Limit of 10000
                          
            fprintf(fd,"226 Closing data connection. Requested file action successful.\n");
            fclose(file);
            close(data_socket);
        }
      }

    // Command LIST inplemented
      else if (COMMAND("LIST")) {
            // Print in console
            printf("LS EJECUTED IN SERVER\n");
            fprintf(fd, "125 Data connection already open; transfer starting\n");

            struct sockaddr_in sa;
            socklen_t sa_len = sizeof(sa);
            // Use other struct to warnings
            std::array<char, MAX_BUFF> buffer;
            // Store text
            std::string mostrar;
            // String to use in popen
            std::string god_command = "ls ";

            // Use popen to run ls
            FILE* pipe = popen(god_command.c_str(), "r");
 
            // If dont open correct
            if (!pipe){
                fprintf(fd, "450 Requested file action not taken. File unavaible.\n");
                close(data_socket);
            }

            else {
                // Passive mode active
                if (passive)
                    data_socket = accept(data_socket,(struct sockaddr *)&sa, &sa_len);

                // Store in struct
                while(fgets(buffer.data(), MAX_BUFF, pipe) != NULL) 
                    mostrar += buffer.data();

                // Send mostrar
                send(data_socket, mostrar.c_str(), mostrar.size(), 0);

                fprintf(fd, "250 Closing data connection. Requested file action successful.\n");
                // Close all
                pclose(pipe);
                close(data_socket);
            }
      }
      // Command SYST inplemented
      else if (COMMAND("SYST")) {
           fprintf(fd, "215 UNIX Type: L8.\n");   
      }

      // Command TYPE inplemented
      else if (COMMAND("TYPE")) {
	  fscanf(fd, "%s", arg);
	  fprintf(fd, "200 OK\n");   
      }
     
     // Command QUIT inplemented
      else if (COMMAND("QUIT")) {
        fprintf(fd, "221 Service closing control connection. Logged out if appropriate.\n");
        close(data_socket);	
        parar=true;
        break;
      }

      // Case default inplemented
      else  {
	    fprintf(fd, "502 Command not implemented.\n"); fflush(fd);
	    printf("Comando : %s %s\n", command, arg);
	    printf("Error interno del servidor\n");
	
      }
      
    }
    
    fclose(fd);
    return;
  
};
