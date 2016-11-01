#include <avr/io.h>
#include <util/delay.h>

#define LTHRES 500
#define RTHRES 500

unsigned char write_data, recv_data;

void TWI_init_slave(void) // Function to initilaize slave
{
    TWAR=0x20; // Fill slave address to TWAR
}

void TWI_write_slave(void) // Function to write data
{
    TWDR= write_data;           // Fill TWDR register whith the data to be sent
    TWCR= (1<<TWEN)|(1<<TWINT);   // Enable TWI, Clear TWI interrupt flag
    while((TWSR & 0xF8) != 0xC0); // Wait for the acknowledgement
}

void TWI_match_write_slave(void) //Function to match the slave address and slave dirction bit(write)
{
    while((TWSR & 0xF8)!= 0xA8) // Loop till correct acknoledgement have been received
    {
        // Get acknowlegement, Enable TWI, Clear TWI interrupt flag
        TWCR=(1<<TWEA)|(1<<TWEN)|(1<<TWINT);
        while (!(TWCR & (1<<TWINT)));  // Wait for TWINT flag
    }
}

void TWI_read_slave(void)
{
    // Clear TWI interrupt flag,Get acknowlegement, Enable TWI
    TWCR= (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT))); // Wait for TWINT flag
    while((TWSR & 0xF8)!=0x80); // Wait for acknowledgement
    recv_data=TWDR; // Get value from TWDR
    PORTB=recv_data; // send the receive value on PORTB
}

void TWI_match_read_slave(void) //Function to match the slave address and slave dirction bit(read)
{
    while((TWSR & 0xF8)!= 0x60)  // Loop till correct acknoledgement have been received
    {
        // Get acknowlegement, Enable TWI, Clear TWI interrupt flag
        TWCR=(1<<TWEA)|(1<<TWEN)|(1<<TWINT);
        while (!(TWCR & (1<<TWINT)));  // Wait for TWINT flag
    }
}



void initIO(void) {
    
    //portB
    DDRB |= 1 << PINB0;//init as out
    DDRB |= 1 << PINB2;
    DDRB |= 1 << PINB4;
}

// initialize adc
void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0);
    
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
    
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
    
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));
    
    return (ADC);
}

void TWRoutine()
{
    
    TWI_match_read_slave(); //Function to match the slave address and slave dirction bit(read)
    TWI_read_slave(); // Function to read data
    
    write_data = adc_read(0); // Togglem the receive data
    
    TWI_match_write_slave(); //Function to match the slave address and slave dirction bit(write)
    TWI_write_slave();       // Function to write data
}

int main()
{
    uint16_t adc_result0, adc_result1;
    char int_buffer[10];
    DDRC = 0x01;           // to connect led to PC0
    
    
    DDRB=0xff;
    TWI_init_slave(); // Function to initilaize slave
    adc_init(); // initialize adc
    initIO();
    
    _delay_ms(50);
    
    while(1)
    {
        TWRoutine();
        adc_result0 = adc_read(0);      // read adc value at PA0
        adc_result1 = adc_read(1);      // read adc value at PA1
        
        // condition for led to glow
        if (adc_result0 < LTHRES && adc_result1 < RTHRES)
            PORTC = 0x01;
        else
            PORTC = 0x00;
        
        itoa(adc_result0, int_buffer, 10);
        
        
        itoa(adc_result1, int_buffer, 10);
        
        //3 c> 2
        if (adc_result0 < 512) {
            PORTB &= ~(1 << PINB2); //pin B2 goes low
        }else{
            PORTB |= (1 << PINB2); // Pin B2 goes high
        }
        //5 c> 4
        if (adc_result1 < 256) {
            PORTB &= ~(1 << PINB4); //pin B4 goes low
        }else{
            PORTB |= (1 << PINB4); // Pin B4 goes high
        }
        
        //        _delay_ms(50);
    }
}


