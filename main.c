#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>


#define BAUD 9600
#define UBRR_VALLUE ((F_CPU / (16UL * BAUD)) - 1)

#define CMD_BUF_SIZE 32
#define BUZZER_PIN PB1

#define SEG1 0x1
#define SEG2 0x2
#define SEG3 0x3

void ManualMode();
void AutomaticMode();
void TogglePower(int i);
void ManualInit();

int manualMode = 0;

char cmd[CMD_BUF_SIZE];
uint8_t idx = 0;
int powerMode = 0;
char segFlags = 0;

void uart_init(void)
{
	UBRR0H = (uint8_t)(UBRR_VALLUE >> 8);
	UBRR0L = (uint8_t)UBRR_VALLUE;
	
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void uart_putchar(char c)
{
	while (!(UCSR0A &  (1 << UDRE0)));
	UDR0 = c;
}


void uart_puts(const char *s)
{
	while(*s)
	{
		uart_putchar(*s++);
	}
}

char uart_getchar(void)
{
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void adc_init(void) {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0); 
}



uint16_t adc_read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void uart_print_int(int value) {
    char buf[5];
    sprintf(buf, "%d", value);
    uart_puts(buf);
}

void buzz_1()
{
	PORTB |= (1<<BUZZER_PIN);
	_delay_ms(1000);
	PORTB &= ~(1<<BUZZER_PIN);
}

void buzz_half()
{
	PORTB |= (1<<BUZZER_PIN);
	_delay_ms(500);
	PORTB &= ~(1<<BUZZER_PIN);
}

float ReadVoltagePc1()
{
	ADMUX = (1 << REFS0) | (1 << MUX0);
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1<<ADSC));
	uint16_t adc = ADC;
	float Vadc = adc * (5.0 / 1023.0);
	float Vgrid = Vadc * (1);
	return Vgrid;
}

float ReadVoltagePc2()
{
	ADMUX = (1 << REFS0) | (2 & 0x0F);
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1<<ADSC));
	uint16_t adc = ADC;
	float Vadc = adc * (5.0 / 1023.0);
	float Vgrid = Vadc * (1);
	return Vgrid;
}

float ReadVoltagePc3()
{
	ADMUX = (1 << REFS0) | (3 & 0x0F);
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1<<ADSC));
	uint16_t adc = ADC;
	float Vadc = adc * (5.0 / 1023.0);
	float Vgrid = Vadc * (1);
	return Vgrid;
}

void SetPowerSeg1(int pow)
{
	if(pow)
	{
		PORTB |= (1<<PB2);
		segFlags |= (1<<SEG1);
	}
	else
	{
		PORTB &= ~(1<<PB2);
		segFlags &= ~(1<<SEG1);
	}
}

void SetPowerSeg2(int pow)
{
	if(pow)
	{
		PORTB |= (1<<PB3);
		segFlags |= (1<<SEG2);
	}
	else
	{
		PORTB &= ~(1<<PB3);
		segFlags &= ~(1<<SEG2);
	}
}

void SetPowerSeg3(int pow)
{
	if(pow)
	{
		PORTB |= (1<<PB4);
		segFlags |= (1<<SEG3);
	}
	else
	{
		PORTB &= ~(1<<PB4);
		segFlags &= ~(1<<SEG3);
	}
}



void Start()
{
	uart_puts("TYPE AUTOMATIC FOR AUTOMATIC MODE - MANUAL FOR MANUAL MODE\r\n");
	uart_puts("$");
	while(1)
	{
		char c = uart_getchar();
		uart_putchar(c);
		if (c == '\r' || c == '\n')
		{
			cmd[idx] = '\0';
            idx = 0;
            uart_puts("\r\n");
            if (strcmp(cmd, "MANUAL") == 0)
            {
            	manualMode = 1;
            	ManualInit();
            	return;
			}
			else if (strcmp(cmd, "AUTOMATIC") == 0)
			{
				manualMode = 0;
				TogglePower(1);
				return;
			}
			else
			{
				uart_puts("PLEASE ENTER THE CORRECT QUERY\r\n");
			}
			uart_puts("$");
		}
		else
		{
			if (idx < CMD_BUF_SIZE - 1 && c != 8 || c != 127) 
			{
                cmd[idx++] = c;
            }
            if(c == 8 || c == 127 || c == '\b' && cmd[0] != '\b')
            {
            	cmd[idx--] = '\b';
            	cmd[idx--] = '\b';
			}
		}
	}
	
}

void TogglePower(int i)
{
	if(i)
	{
		DDRB |= (1<<PB0); 
		PORTB |= (1<<PB0);
		powerMode = 1;
		uart_puts("POWER MODE - ONLINE\r\n");
		buzz_1();
	}
	else
	{
		PORTB &= ~(1<<PB0);
		powerMode = 0;
		uart_puts("POWER MODE - OFFLINE\r\n");
		buzz_half();
	}
}


void AutomaticInit()
{
	uart_puts("--AUTOMATIC MODE--\r\n");
	uart_puts("\r\n");
	uart_puts("STATUS PENDING....\r\n");
	TogglePower(1);

}

void ManualInit()
{
	uart_puts("---MANUAL MODE ACTIVATED---\r\n");
	uart_puts("TYPE 'HELP' FOR HELP\r\n");
	uart_puts("$");
}


	
int main(void) 
{


	uart_init();
	adc_init();
	uart_puts("---SCADA READY---\r\n");

	DDRB |= (1<<BUZZER_PIN);
	DDRB |= (1<<PB2);
	DDRB |= (1<<PB3);
	DDRB |= (1<<PB4);
	
	DDRC &= ~(1<<PC1);
	PORTC &= ~(1<<PC1);
	
	       
	
	ADCSRA =
    	(1 << ADEN)  |
    	(1 << ADPS2) |
    	(1 << ADPS1) |
    	(1 << ADPS0); 
	
	
	Start();
	if(!manualMode)
	{
		SetPowerSeg1(1);
		SetPowerSeg2(1);
		SetPowerSeg3(1);
	}
	while(1)
	{
		if(manualMode)
		{
			ManualMode();
		}
		else
		{
			AutomaticMode();
		}
	}
}

void ManualMode()
{
		char c = uart_getchar();
		
		uart_putchar(c);
		
		if (c == '\r' || c == '\n') 
		{
            cmd[idx] = '\0';
            idx = 0;

            uart_puts("\r\n");
			
            if (strcmp(cmd, "STATUS") == 0) 
			{
                uart_puts("OK: SYSTEM RUNNING\r\n");
                if(powerMode) uart_puts(" - POWER MODE - ONLINE\r\n");
                else uart_puts(" - POWER MODE - OFFLINE\r\n");
                if(segFlags & (1<<SEG1))
                {
                	uart_puts(" - SEGMENT 1 - ONLINE\r\n");
                	float voltage = ReadVoltagePc1();
                	uart_puts(" - 		VOLTAGE: ");
                	#ifdef DEBUG
                		uart_print_int(voltage);
                	#endif
                	if(voltage <= 4)
                	{
                		uart_puts(" NOMINAL\r\n");
					}
					else
					{
						uart_puts("WRNG: OVERVOLTAGE!\r\n");
					}
					
				}
				else
				{
					uart_puts(" - SEGMENT 1 - OFFLINE\r\n");
				}
				if(segFlags & (1<<SEG2))
                {
                	uart_puts(" - SEGMENT 2 - ONLINE\r\n");
                	float voltage = ReadVoltagePc2();
                	uart_puts(" - 		VOLTAGE: ");
                	#ifdef DEBUG
                		uart_print_int(voltage);
                	#endif
                	if(voltage <= 4)
                	{
                		uart_puts(" NOMINAL\r\n");
					}
					else
					{
						uart_puts("WRNG: OVERVOLTAGE!\r\n");
					}
				}
				else
				{
					uart_puts(" - SEGMENT 2 - OFFLINE\r\n");
				}
				if(segFlags & (1<<SEG3))
                {
                	uart_puts(" - SEGMENT 3 - ONLINE\r\n");
                	float voltage = ReadVoltagePc3();
                	uart_puts(" - 		VOLTAGE: ");
                	#ifdef DEBUG
                		uart_print_int(voltage);
                	#endif
                	if(voltage <= 4)
                	{
                		uart_puts(" NOMINAL\r\n");
					}
					else
					{
						uart_puts("WRNG: OVERVOLTAGE!\r\n");
					}
				}
				else
				{
					uart_puts(" - SEGMENT 3 - OFFLINE\r\n");
				}
            }
            else if(strcmp(cmd, "HELP") == 0)
            {
            	uart_puts("- STATUS\r\n");
            	uart_puts("- POWER [0|1]\r\n");
            	uart_puts("- TEMP\r\n");
            	uart_puts("- SEG[1|2|3] [0|1]\r\n");
            	uart_puts("- CREDITS\r\n");
			}
            else if(strcmp(cmd, "POWER 1") == 0)
            {
        		DDRB |= (1<<PB0); 
				PORTB |= (1<<PB0);
				powerMode = 1;
				uart_puts("POWER MODE - ONLINE\r\n");
				buzz_1();
			}
			else if(strcmp(cmd, "POWER 0") == 0)
			{
				PORTB &= ~(1<<PB0);
				powerMode = 0;
				uart_puts("POWER MODE - OFFLINE\r\n");
				buzz_half();
			}
			else if(strcmp(cmd, "SEG1 0") == 0)
			{
				SetPowerSeg1(0);
				uart_puts("SEGMENT 1 - OFFLINE\r\n");
			}
			else if(strcmp(cmd, "SEG1 1") == 0)
			{
				SetPowerSeg1(1);
				uart_puts("SEGMENT 1 - ONLINE\r\n");
			}
			else if(strcmp(cmd, "SEG2 0") == 0)
			{
				SetPowerSeg2(0);
				uart_puts("SEGMENT 2 - OFFLINE\r\n");
			}
			else if(strcmp(cmd, "SEG2 1") == 0)
			{
				SetPowerSeg2(1);
				uart_puts("SEGMENT 2 - ONLINE\r\n");
			}
			else if(strcmp(cmd, "SEG3 0") == 0)
			{
				SetPowerSeg3(0);
				uart_puts("SEGMENT 3 - OFFLINE\r\n");
			}
			else if(strcmp(cmd, "SEG3 1") == 0)
			{
				SetPowerSeg3(1);
				uart_puts("SEGMENT 3 - ONLINE\r\n");
			}
			else if(strcmp(cmd, "TEMP") == 0) 
			{
				uart_puts("TEMP: ");
				uart_print_int(34);
				uart_puts(" C\r\n");
			}
			else if(strcmp(cmd, "CREDITS") == 0)
			{
				uart_puts("PROGRAMMED BY JAN TESKEREDZIC\r\n");
			}
            else 
			{
                uart_puts("ERR: UNKNOWN CMD\r\n");
            }
            uart_puts("$");
        }
        else
		{
            if (idx < CMD_BUF_SIZE - 1 && c != 8 || c != 127) 
			{
                cmd[idx++] = c;
            }
            if(c == 8 || c == 127 || c == '\b' && cmd[0] != '\b')
            {
            	cmd[idx--] = '\b';
            	cmd[idx--] = '\b';
			}
        }
}


void AutomaticMode()
{
	float segVolt1 = ReadVoltagePc1();
	float segVolt2 = ReadVoltagePc2();
	float segVolt3 = ReadVoltagePc3();
	#ifdef DEBUG
		uart_puts("VOLTAGE: \r\n");
		uart_print_int(segVolt1);
	#endif
	if(segVolt1 > 4)
	{
		#ifdef DEBUG
		uart_puts("VOLTAGE: \r\n");
		uart_print_int(segVolt1);
		#endif	
		uart_puts("SEGMENT 1 - OVERVOLTAGE .. DISABLING\r\n");
		SetPowerSeg1(0);
		uart_puts("SEGMENT 1 - OFFLINE\r\n");
		uart_puts("PLEASE CONTACT ZONE ADMINISTRATOR\r\n");
		uart_puts("STATUS PENDING....\r\n");
	}
	if(segVolt2 > 4)
	{
		#ifdef DEBUG
		uart_puts("VOLTAGE: \r\n");
		uart_print_int(segVolt2);
		#endif
		uart_puts("SEGMENT 2 - OVERVOLTAGE .. DISABLING\r\n");
		SetPowerSeg2(0);
		uart_puts("SEGMENT 2 - OFFLINE\r\n");
		uart_puts("PLEASE CONTACT ZONE ADMINISTRATOR\r\n");
		uart_puts("STATUS PENDING....\r\n");
	}
	if(segVolt3 > 4)
	{
		#ifdef DEBUG
		uart_puts("VOLTAGE: \r\n");
		uart_print_int(segVolt3);
		#endif
		uart_puts("SEGMENT 3 - OVERVOLTAGE .. DISABLING\r\n");
		SetPowerSeg3(0);
		uart_puts("SEGMENT 3 - OFFLINE\r\n");
		uart_puts("PLEASE CONTACT ZONE ADMINISTRATOR\r\n");
		uart_puts("STATUS PENDING....\r\n");
	}
	
	_delay_ms(1000);
}