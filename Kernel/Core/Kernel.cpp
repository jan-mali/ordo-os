#include <stdint.h>

extern "C" void kmain()
{
    while (true)
    {
        asm volatile ("hlt");
    }
}
