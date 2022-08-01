#include <array>

struct Message 
{
    std::array<char, 1024> text;   
    uint8_t flag;
    /// Limpia el mensaje por si se envian menos de 1024byte
    void Clean() {
        for(unsigned desplazamiento = 0; desplazamiento <= sizeof(text); desplazamiento ++) 
          text[desplazamiento]='\0';
    }
};