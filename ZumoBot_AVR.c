/*******************************************************************
  Developer -- Mandy Wells
  Course ----- EECE 3624
  Project ---- ZumoBot Final Project

  Reads the ReadyAVR joystick and sends movement commands to the
  ZumoBot through the HC-05 Bluetooth module.
*******************************************************************/
#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>

// Initialize USART0 for Bluetooth communication
void bluetooth_init(){
    // USART0 baud rate = 9600
    // F_CPU = 7.3728 MHz
    UBRR0H = 0;
    UBRR0L = 47;

    // Enable USART0 transmitter
    UCSR0B = (1 << TXEN0);

    // 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // Set TXD0 as output
    DDRE |= (1 << DDE1);
}


// Send one character through Bluetooth
void bluetooth_send(char command){
    // Wait until transmit buffer is empty
    while(!(UCSR0A & (1 << UDRE0)));

    UDR0 = command;
}


// Read the joystick and determine the command
char readJoystick(){
    unsigned char joystick = PINB;

    // Up
    if(!(joystick & (1 << PB3))){
        return 'F';
    }

    // Down
    if(!(joystick & (1 << PB1))){
        return 'B';
    }

    // Left
    if(!(joystick & (1 << PB2))){
        return 'L';
    }

    // Right
    if(!(joystick & (1 << PB0))){
        return 'R';
    }

    // Center button - change speed
    if(!(joystick & (1 << PB4))){
        return 'V';
    }

    // No direction pressed
    return 'S';
}


int main(void){
    char command;
    char lastCommand = '\0';

    // Configure joystick pins as inputs
    DDRB = 0x00;

    // Enable internal pull-up resistors
    PORTB = 0x1F;

    // Initialize Bluetooth
    bluetooth_init();

    while (1){
        // Read joystick position
        command = readJoystick();

        // Debounce joystick input
        _delay_ms(20);

        if(command == readJoystick()){
            // Only transmit when the command changes
            if(command != lastCommand){
                bluetooth_send(command);
                lastCommand = command;
            }
        }
    }
}
