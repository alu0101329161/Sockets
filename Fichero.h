#include "Socket.h"

#include <fcntl.h>
class Fichero {

    public:
    Fichero (const std::string& filename, int flags , int length = 0, std::string dir = "");
    Fichero (const std::string& filename, int flags , mode_t mode, int length = 0, std::string dir = "");
    ~Fichero();

    void SendMessage (std::array<char, 1024>& text);
    void ReceiveMessage (std::array<char, 1024>& text);

    std::string GetLengthstring();
    char* GetMemoryMapped();
    int GetLength();
    bool GetEnd();


    private:
    bool end_ ;
    int posicion_actual_ ;
    int fd;
    int length_;
    char* memory_mapped;
}; 