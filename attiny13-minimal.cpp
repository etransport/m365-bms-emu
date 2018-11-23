/* Following sketch is minimalistic approach. No battery percentage is transmitted, no power usage logging */
/* Scooter moves on its own and does not show any errors                                                   */
/* External active quartz is necesarry, internal RC is too unstable for soft 115200 baudrate               */
/* Courtesy of https://electro.club/forum/zamena_yacheek_akb_na_Xiaomi_Mijia_Scooter                       */
/* Proof of concept only!                                                                                  */

#define F_CPU 9600000 
/*
9.6 mHz internal RC-oscillator is used. This stuff is extremely unstable and not to be recommended/
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
const char magic[]PROGMEM ={0x55,0xAA,0x0E,0x25,0x01,0x30,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE1,0x00,0x28,0x28,0x91,0xFE};

/*
55AA0E2501300100000000000000E100282891FE works well with 1.3.0-1.3.4, untested with higher firmwares.
Mihome battery info stays blank, though kickscooter does not beep and achieves full speed.
No overvoltage protection is hardcoded!
*/

#define SOFT_TX_PIN (1<<PB1) // PB1 is concidered as TX output
#define SOFT_TX_PORT PORTB
#define SOFT_TX_DDR DDRB
void uart_tx_init ()
{
  TCCR0A = 1 << WGM01;    // Compare mode
  TCCR0B = 1 << CS00;   // Prescaler 1
  SOFT_TX_PORT |= SOFT_TX_PIN;
  SOFT_TX_DDR |= SOFT_TX_PIN;
  OCR0A = 75;     //115200 baudrate at prescaler 1, ~83-84us baud period in real world, according to logic analyzer
}

//Bitbang UART
void uart_send_byte (unsigned char data)
{
  unsigned char i;
  TCCR0B = 0;
  TCNT0 = 0;
  TIFR0 |= 1 << OCF0A;
  TCCR0B |= (1 << CS00);
  TIFR0 |= 1 << OCF0A;
  SOFT_TX_PORT &= ~SOFT_TX_PIN;
  while (!(TIFR0 & (1 << OCF0A)));
  TIFR0 |= 1 << OCF0A;
  for (i = 0; i < 8; i++)
  {
    if (data & 1)
      SOFT_TX_PORT |= SOFT_TX_PIN;
    else
      SOFT_TX_PORT &= ~SOFT_TX_PIN;
    data >>= 1;
    while (!(TIFR0 & (1 << OCF0A)));
    TIFR0 |= 1 << OCF0A;
  }
  SOFT_TX_PORT |= SOFT_TX_PIN;
  while (!(TIFR0 & (1 << OCF0A)));
  TIFR0 |= 1 << OCF0A;
}

void setup()
{
    uart_tx_init (); 
}

void loop()
{
    for(byte char i=0;i<20;i++)
      uart_send_byte(str[i++]);
    _delay_ms(1000);
     
}
