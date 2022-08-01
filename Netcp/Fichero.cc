#include "Fichero.h"

/// Constructor que abre el filename sin modo.
Fichero::Fichero (const std::string& filename, int flags, int length ,std::string dir) 
{
    char name[filename.size()];
    strcpy(name, filename.c_str());

    if (dir != "") {
    char dir_[dir.size()];
    strcpy(dir_, dir.c_str());

    if (mkdir(dir_, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
      throw std::system_error(errno, std::system_category(), "No se ha podido"
      " crear el directorio");
    }

    fd = open (name, flags);

    if (fd < 0) {
      throw std::system_error(errno, std::system_category(),
     "fallo al abrir el fichero");
    }
    if(length == 0) {
     length_ = lseek( fd, 0, SEEK_END );
     /// Reservo en la region de memoria el espacio equivalente a el tamaño del filename di da error desmapeamos y liberamos recursos.
     memory_mapped = (char*)mmap(NULL,length_,PROT_READ,MAP_SHARED,fd,0);
     //std::cout << memory_mapped;
     if(memory_mapped == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(),
      "El fichero no se mapeo correctamente");
      }
    } else {
      length_ = length;
      /// Modifico el tamaño del fichero.
      if(-1 == ftruncate (fd, length_)) {
      throw std::system_error(errno, std::system_category(),
      "No se pudo establecer el tamaño");
     }
     memory_mapped = (char*)mmap(NULL,length_,PROT_WRITE,MAP_SHARED,fd,0);
      if(memory_mapped == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(),
      "El fichero no se mapeo correctamente");
      }
    }
    lockf(fd, F_LOCK, 0);
    end_ = false;
    posicion_actual_ = 0;
}

/// Constructor que abre el filename con el modo indicado.
Fichero::Fichero (const std::string& filename, int flags, mode_t mode, int length, std::string dir) 
{
    char name[filename.size()];
    strcpy(name, filename.c_str());

    if (dir != "") {
    char dir_[dir.size()];
    strcpy(dir_, dir.c_str());
    // Imprimir la ruta
    std::cout <<"La ruta es ->"<< filename <<std::endl;

    if (mkdir(dir_, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
      throw std::system_error(errno, std::system_category(), "No se ha podido"
      " crear el directorio");
    }

    fd = open (name, flags, mode);

    if (fd < 0) {
      throw std::system_error(errno, std::system_category(),
     "fallo al abrir el fichero");
    }
    if(length == 0) {
     length_ = lseek( fd, 0, SEEK_END );
     /// Reservo en la region de memoria el espacio equivalente a el tamaño del filename di da error desmapeamos y liberamos recursos.
     memory_mapped = (char*)mmap(NULL,length_,PROT_READ,MAP_SHARED,fd,0);
     //std::cout << memory_mapped;
     if(memory_mapped == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(),
      "El fichero no se mapeo correctamente");
      }
    } else {
      length_ = length;
        /// Modifico el tamaño del fichero.
      if(-1 == ftruncate (fd, length_)){
      throw std::system_error(errno, std::system_category(),
      "No se pudo establecer el tamaño");
     }
     memory_mapped = (char*)mmap(NULL,length_,PROT_WRITE,MAP_SHARED,fd,0);
      if(memory_mapped == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(),
      "El fichero no se mapeo correctamente");
      }
    }
    lockf(fd, F_LOCK, 0);
    end_ = false;
    posicion_actual_ = 0;
}

// Pasar de map a vector

void
Fichero::SendMessage(std::array<char, 1024>& Message) {
    for(unsigned desplazamiento = 0; desplazamiento < 1023; desplazamiento ++) {
        if(posicion_actual_ < length_) {
          Message[desplazamiento] = memory_mapped [posicion_actual_];
          posicion_actual_++;
        }
    }
    if(posicion_actual_ == length_)
    end_= true;
}

// Pasar de vector a map

void
Fichero::ReceiveMessage(std::array<char, 1024>& Message) {
    for(unsigned desplazamiento = 0; desplazamiento < 1024; desplazamiento ++) {
        if(Message[desplazamiento] != '\0') {
          memory_mapped[posicion_actual_] = Message[desplazamiento];
          posicion_actual_++;
        }
    }
    if(posicion_actual_ == length_)
    end_ = true;
}

Fichero::~Fichero()
{
    munmap(memory_mapped,length_);
    lockf(fd, F_ULOCK, 0);
    close(fd); 
}

std::string 
Fichero::GetLengthstring(){
  std::string length_string;
  length_string = std::to_string(length_);
  return length_string;
}


char*
Fichero::GetMemoryMapped() {
  return memory_mapped;
}

int 
Fichero::GetLength() {
  return length_;
}

bool
Fichero::GetEnd() {
  return end_;
}