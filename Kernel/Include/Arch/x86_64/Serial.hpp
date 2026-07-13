#pragma once

class Serial {
public:
    void init();
    void putc(char c);
    void write(const char* str);

private:
    bool initialized = false;

};

extern Serial serial;
