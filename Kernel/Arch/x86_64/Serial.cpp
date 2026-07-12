#include <Serial.hpp>
#include <IO.hpp>

#define SERIAL_PORT 0x03F8

#define SERIAL_DATA_PORT(base)             (base + 0)
#define SERIAL_INTERRUPT_ENABLE_PORT(base) (base + 1)
#define SERIAL_FIFO_CONTROL_PORT(base)     (base + 2)
#define SERIAL_LINE_CONTROL_PORT(base)     (base + 3)
#define SERIAL_MODEM_CONTROL_PORT(base)    (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)      (base + 5)

#define SERIAL_LINE_ENABLE_DLAB 0x80
#define SERIAL_LINE_READY       0x20

#define SERIAL_BAUD_DIVISOR(baud) (115200 / (baud))

Serial serial;

void Serial::init() {
    const unsigned short port = SERIAL_PORT;

    IO::outb(SERIAL_INTERRUPT_ENABLE_PORT(port), 0x00);                // Disable interrupts
    IO::outb(SERIAL_LINE_CONTROL_PORT(port), SERIAL_LINE_ENABLE_DLAB); // Enable DLAB
    IO::outb(SERIAL_DATA_PORT(port), 0x03);                            // Baud rate divisor low byte (115200 / 38400 = 3)
    IO::outb(SERIAL_INTERRUPT_ENABLE_PORT(port), 0x00);                // High byte
    IO::outb(SERIAL_LINE_CONTROL_PORT(port), 0x03);                    // 8 bits, no parity, one stop bit
    IO::outb(SERIAL_FIFO_CONTROL_PORT(port), 0xC7);                    // Enable FIFO, clear, 14-byte threshold
    IO::outb(SERIAL_MODEM_CONTROL_PORT(port), 0x0B);                   // IRQs enabled, RTS/DSR set

    // Loopback test
    IO::outb(SERIAL_MODEM_CONTROL_PORT(port), 0x1E); // Set loopback mode
    IO::outb(SERIAL_DATA_PORT(port), 0xAE);          // Test byte
    if (IO::inb(SERIAL_DATA_PORT(port)) != 0xAE) {
        return; // Hardware not responding
    }

    // Exit loopback, normal operation
    IO::outb(SERIAL_MODEM_CONTROL_PORT(port), 0x0F);
    initialized = true;
}

// TODO: Use a buffer & IRQs
static int serial_is_transmit_ready(void) {
    return IO::inb(SERIAL_LINE_STATUS_PORT(SERIAL_PORT)) & SERIAL_LINE_READY;
}

void Serial::putc(char c) {
    if (!initialized)
        return;

    while (!serial_is_transmit_ready()) {
        // wait until ready
    }

    IO::outb(SERIAL_DATA_PORT(SERIAL_PORT), c);
}

void Serial::write(const char* str) {
    if(!str)
        return;

    while (*str) {
        putc(*str++);
    }
}
