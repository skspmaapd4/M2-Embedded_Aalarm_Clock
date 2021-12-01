#define F_CPU 16000000L

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//--------------------------------------------------------------------------------------------------------------------------------------------------
//variable definitions
volatile unsigned long milliseconds;
volatile unsigned long previousTimeX = 0;
volatile unsigned long previousTimeY = 0;
volatile unsigned long previousInterrupt = 0;
volatile unsigned long previousBlink = 0;
volatile unsigned long previousSound = 0;

uint8_t table_binary[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b001101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

uint8_t sec = 0;
uint8_t minute = 0;
uint8_t hour = 12;
uint8_t day = 1;
uint8_t month = 1;
int year = 2010;
int8_t temp = 0;

uint8_t alarmHour = 12;
uint8_t alarmMinute = 0;

uint8_t s_t, s_o, m_t, m_o, h_t, h_o, d_t, d_o, mon_t, mon_o, y_th, y_h, y_t, y_o, am_t, am_o, ah_t, ah_o, t_t, t_o;

uint8_t leapYear = 0;
uint8_t longMonth = 0;
uint8_t buttonFlag = 0;
uint8_t settingMode = 0;
uint8_t alarmOn = 0;
uint8_t alarmTriggered = 0;
uint8_t blinkOn = 0;
uint8_t left = 0;
uint8_t right = 0;
uint8_t up = 0;
uint8_t down = 0;

uint8_t currentScenario = 0;
uint8_t currentADC = 0;
uint8_t currentSetting = 0;

unsigned int ADC_value = 0;
unsigned int analogX = 0;
unsigned int analogY = 0;
unsigned int analogTemp = 0;


//--------------------------------------------------------------------------------------------------------------------------------------------------
//functions declarations

//return milliseconds
unsigned long millis(){
	return milliseconds;
}

//convert ten base to individual digits
void initAlamHour(){
	ah_t = (alarmHour/10)%10;
	ah_o = alarmHour%10;
}

void initAlarmMinute(){
	am_t = (alarmMinute/10)%10;
	am_o = alarmMinute%10;
}

void initHour(){
	h_t = (hour/10)%10;
	h_o = hour%10;	
}

void initMinute(){
	m_t = (minute/10)%10;
	m_o = minute%10;	
}

void initSecond(){
	s_t = (sec/10)%10;
	s_o = sec%10;	
}

void initYear(){
	y_th = (year/1000)%10;
	y_h = (year/100)%10;
	y_t = (year/10)%10;
	y_o = year%10;	
}

void initMonth(){
	mon_t = (month/10)%10;
	mon_o = month%10;	
}

void initDay(){
	d_t = (day/10)%10;
	d_o = day%10;
}

void initTemp(){
	if (temp >= 0){
		t_t = (temp/10)%10;
		t_o = temp%10;
	}
	else 
	{
		t_t = (((-1)*temp)/10)%10;
		t_o = ((-1)*temp)%10;
	}
}

void initAll(){
	initAlamHour();
	initAlarmMinute();
	initHour();
	initMinute();
	initSecond();
	initYear();
	initMonth();
	initDay();
	initTemp();
}

void checkAxis(){	
	if ((analogX) > 800 && (millis() - previousTimeX > 400)){
		right = 1;
		previousTimeX = millis();
	}
	else right = 0;
			
	if ((analogX < 400) && millis() - previousTimeX > 400){
		left = 1;
		previousTimeX = millis();
	}
	else left = 0;
	
	if ((analogY < 400) && (millis() - previousTimeY > 300)){
		up = 1;
		previousTimeY = millis();
	}
	else up = 0;
						
	if ((analogY > 800) &&  (millis() - previousTimeY > 300)){
		down = 1;
		previousTimeY = millis();
	}
	else down = 0;		
}

//check if month is long (31 days)
void checkMonth(int m){
	if (m == 2 || m == 4 || m == 6 || m == 9 || m == 11){
		longMonth = 0;
	}
	else{
		longMonth = 1;
	}
}

//check if year is a leap year
void checkYear(int y){
	if ((y%4 == 0 && y%100 !=0) || y%400 == 0){
		leapYear = 1;
	}
	else{
		leapYear = 0;
	}
}

//start ADC conversion
void startConversion(){
	ADCSRA |= (1 << ADSC); //restart conversion
}

//setup ADC
void setupADC(){
	ADMUX = (1 << REFS0) | (0 << REFS1) | (1 << MUX0); //use AVcc as reference, set ADC1 0001, ADC2 0010
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); //enable ADC, enable interrupt on completed conversion, set prescaler to 128
	DIDR0 = (1 << ADC1D) | (1 << ADC2D); //disable digital input buffer PC1 and PC2
	
	startConversion();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

int main(void){
	//initialize 1ms time base (8bit timer)
	TCCR0A = (1 << WGM01); //set CTC bit
	OCR0A = 250; //output compare register
	TIMSK0 = (1 << OCIE0A); //enable output compare match interrupt
	TCCR0B = (1 << CS01 | 1 << CS00); //prescaler clk/64
	
	//initialize 1s time base (16bit timer)
	TCCR1B = (1 << WGM12); //set CTC bit
	OCR1A = 15625; //output compare register
	TIMSK1 = ( 1 << OCIE1A); //enable output compare match interrupt
	TCCR1B |= (1 << CS12) | (1 << CS10); //prescaler clk/1024
	
	//configure PCINT for PC0
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT8);
	
	sei(); //set event interrupt
	
	DDRD = 0xff; //port D all out
	DDRB = 0xff; //port B all out
	DDRC = 0x08; //PC3 out, others in
	PORTC = 0x01; //attach pull-up resistor to PC0
	
	checkMonth(month);
	checkYear(year);
	initAll();
	setupADC();
    
    while (1){
		
		checkAxis();
		
		if (buttonFlag){
			settingMode = !settingMode;
			currentSetting = 0;
			currentScenario = 0;
			buttonFlag = 0;
		}
		
		//time and alarm setting routine
		if (settingMode){
			if (right){
				if (currentSetting < 8) currentSetting++;
				else currentSetting = 0;
				previousTimeX = millis();
			}
			
			if (left){
				if (currentSetting > 0) currentSetting--;
				else currentSetting = 8;
				previousTimeX = millis();
			}		
				
			switch(currentSetting){
				case 0:
					if (up){
						if (hour < 23) hour++;
						else hour = 0;
						initHour();
						previousTimeY = millis();
					}
					
					if (down){
						if (hour > 1) hour--;
						else hour = 23;
						initHour();
						previousTimeY = millis();
					}
					currentScenario = 0;				
				break;
				
				case 1:
					if (up){
						if (minute < 59) minute++;
						else minute = 0;
						initMinute();
						previousTimeY = millis();
					}
					
					if (down){
						if (minute > 1) minute--;
						else minute = 59;
						initMinute();
						previousTimeY = millis();
					}	
					currentScenario = 0;				
				break;
				
				case 2:
					if (up || down){
						sec = 0;
						initSecond();
						previousTimeY = millis();
					}
					currentScenario = 1;		
				break;
				
				case 3:
				if (up){
					if ((day < 31 && longMonth) || (day < 30 && !longMonth && month !=2) || (day < 29 && month == 2 && leapYear) || (day < 28 && month == 2 && !leapYear)) day++;
					else day = 1;
					initDay();
					previousTimeY = millis();
					
				}
				
				if (down){
					if (day > 1) day--;
					else{
						if (longMonth) day = 31;
						else{
							if(month == 2){
								if (leapYear) day = 29;
								else day = 28;
							}
							else day = 30;
						}
					}
					initDay();
					previousTimeY = millis();
				}
				currentScenario = 2;
				break;

				case 4:
				if (up){
					if (month < 12) month++;
					else month = 1;
					initMonth();
					checkMonth(month);
					previousTimeY = millis();
				}
				
				if (down){
					if (month > 1) month--;
					else month = 12;
					initMonth();
					checkMonth(month);
					previousTimeY = millis();
				}
				currentScenario = 2;
				break;	
	
				case 5:
					if (up){
						if (year < 9999) year++;
						else year = 1;
						initYear();
						checkYear(year);
						previousTimeY = millis();
					}
					
					if (down){
						if (year > 1) year--;
						else year = 9999;
						initYear();
						checkYear(year);
						previousTimeY = millis();
					}	
					currentScenario = 3;		
				break;
				
				case 6:
					if (up){
						if (alarmHour < 23) alarmHour++;
						else alarmHour = 1;
						initAlamHour();
						previousTimeY = millis();
					}
					
					if (down){
						if (alarmHour > 1) alarmHour--;
						else alarmHour = 23;
						initAlamHour();
						previousTimeY = millis();
					}	
					currentScenario = 4;			
				break;
				
				case 7:
					if (up){
						if(alarmMinute < 59) alarmMinute++;
						else alarmMinute = 1;
						initAlarmMinute();
						previousTimeY = millis();
					}
					
					if (down){
						if(alarmMinute > 1) alarmMinute--;
						else alarmMinute = 59;
						initAlarmMinute();
						previousTimeY = millis();
					}	
					currentScenario = 4;		
				break;
				
				case 8:
					if (up){
						alarmOn = 1;
						previousTimeY = millis();
					}
					
					if (down){
						alarmOn = 0;
						previousTimeY = millis();
					}	
					currentScenario = 5;			
				break;				
			}			
		}
		
		//check alarm
		if (hour == alarmHour && minute == alarmMinute && alarmOn){
			alarmTriggered = 1;
		}
		
		//sound alarm
		if (alarmTriggered){
			if (down) {
				alarmTriggered = 0;
				PORTC &= 0b11110111;
			}
			if (millis() - previousSound > 100){
				PORTC ^= 0x08;
				previousSound = millis();
			}
			alarmOn = 0;
		}
		
		//navigate menu
		if (!settingMode){
			if (right){
				if (currentScenario < 6) currentScenario++;
				else currentScenario = 0;
				previousTimeX = millis();
			}

			if (left){
				if (currentScenario > 0) currentScenario--;
				else currentScenario = 6;
				previousTimeX = millis();
			}			
		}
		
		//setting display blinking
		if (settingMode){
			if((!blinkOn && (millis() - previousBlink > 100)) || (blinkOn && (millis() - previousBlink > 800))){
			blinkOn = !blinkOn;
			previousBlink = millis();
			}
		}
		else blinkOn = 1;
		
		//7seg display routine
		switch(currentScenario){
			case 0:
				PORTB = ~(1 << PORTB0);
				if(!blinkOn && currentSetting == 1) PORTD = 0xff;
				else PORTD = ~table_binary[m_o];
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				if(!blinkOn && currentSetting == 1) PORTD = 0xff;
				else PORTD = ~table_binary[m_t];
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				if(!blinkOn && currentSetting == 0) PORTD = 0b01111111;
				else PORTD = ~table_binary[h_o] & 0b01111111;
				_delay_ms(1);
				PORTB = ~(1 << PORTB3);
				if(!blinkOn && currentSetting == 0) PORTD = 0xff;
				else PORTD = ~table_binary[h_t];
				_delay_ms(1);
			break;
			
			case 1:
				PORTB = ~(1 << PORTB0);
				if(!blinkOn && currentSetting == 2) PORTD = 0xff;
				else PORTD = ~table_binary[s_o];
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				if(!blinkOn && currentSetting == 2) PORTD = 0xff;
				else PORTD = ~table_binary[s_t];
				_delay_ms(1);
			break;
			
			case 2:
				PORTB = ~(1 << PORTB0);
				if(!blinkOn && currentSetting == 4) PORTD = 0xff;
				else PORTD = ~table_binary[mon_o];
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				if(!blinkOn && currentSetting == 4) PORTD = 0xff;
				else PORTD = ~table_binary[mon_t];
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				if(!blinkOn && currentSetting == 3) PORTD = 0b01111111;
				else PORTD = ~table_binary[d_o] & 0b01111111;
				_delay_ms(1);
				PORTB = ~(1 << PORTB3);
				if(!blinkOn && currentSetting == 3) PORTD = 0xff;
				else PORTD = ~table_binary[d_t];
				_delay_ms(1);			
			break;
				
			case 3:
				PORTB = ~(1 << PORTB0);
				if(!blinkOn && currentSetting == 5) PORTD = 0xff;
				else PORTD = ~table_binary[y_o];
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				if(!blinkOn && currentSetting == 5) PORTD = 0xff;
				else PORTD = ~table_binary[y_t];
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				if(!blinkOn && currentSetting == 5) PORTD = 0xff;
				else PORTD = ~table_binary[y_h];
				_delay_ms(1);
				PORTB = ~(1 << PORTB3);
				if(!blinkOn && currentSetting == 5) PORTD = 0xff;
				else PORTD = ~table_binary[y_th];
				_delay_ms(1);			
			break;
			
			case 4:
				PORTB = ~(1 << PORTB0);
				if(!blinkOn && currentSetting == 7) PORTD = 0xff;
				else PORTD = ~table_binary[am_o];
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				if(!blinkOn && currentSetting == 7) PORTD = 0xff;
				else PORTD = ~table_binary[am_t];
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				if(!blinkOn && currentSetting == 6) PORTD = 0b01111111;
				else PORTD = ~table_binary[ah_o] & 0b01111111;
				_delay_ms(1);
				PORTB = ~(1 << PORTB3);
				if(!blinkOn && currentSetting == 6) PORTD = 0xff;
				else PORTD = ~table_binary[ah_t];
				_delay_ms(1);			
			break;
			
			case 5:
				PORTB = ~(1 << PORTB3);
				PORTD = ~0b01110111;
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				PORTD = ~0b00111000;
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				PORTD = ~0b00000000;
				_delay_ms(1);
				PORTB = ~(1 << PORTB0);
				if(!blinkOn && currentSetting == 8) PORTD = 0xff;
				else PORTD = ~table_binary[alarmOn];
				_delay_ms(1);
			break;
			
			case 6:
				PORTB = ~(1 << PORTB3);
				if (temp < 0) PORTD = 0xbf;
				else PORTD = 0xff;
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				PORTD = ~table_binary[t_t];
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				PORTD = ~table_binary[t_o];
				_delay_ms(1);
				PORTB = ~(1 << PORTB0);
				PORTD = ~0b01100011;
				_delay_ms(1);
			break;
			
			default:
				PORTB = ~(1 << PORTB3);
				PORTD = ~0b01111001;
				_delay_ms(1);
				PORTB = ~(1 << PORTB2);
				PORTD = ~0b01010000;
				_delay_ms(1);
				PORTB = ~(1 << PORTB1);
				PORTD = ~0b01010000;
				_delay_ms(1);
		}
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------

//interrupt for 8bit time compare, ms increase subroutine
ISR(TIMER0_COMPA_vect){
	milliseconds++;
}

//interrupt for 16bit timer compare, clock increase subroutine
ISR(TIMER1_COMPA_vect){
	sec++;
	temp = (analogTemp - 324.31)/1.1 - 4;
	initTemp();
	if (sec == 60){
		sec = 0;
		minute++;
		if (minute == 60){
			minute = 0;
			hour ++;
			if (hour == 24){
				hour = 0;
				day++;
				if ((day == 31 && longMonth) || (day == 30 && !longMonth && month !=2) || (day == 29 && month == 2 && leapYear) || (day == 28 && month == 2 && !leapYear)){
					day = 0;
					month++;
					checkMonth(month);
					if (month == 13){
						month = 1;
						year++;
						checkYear(year);
						initYear();
					}
					initMonth();
				}	
				initDay();
			}
			initHour();
		}
		initMinute();
	}
	initSecond();
}

//interrupt for ADC conversion completed
ISR(ADC_vect){
	switch(currentADC){
		case 0:
			analogX = ADC;
			ADMUX = (1 << REFS0) | (0 << REFS1) | (1 << MUX1);
			startConversion();
			currentADC = 1;			
		break;
		
		case 1:
			analogY = ADC;
			ADMUX = (1 << REFS0) | (1 << REFS1) | (1 << MUX3);
			startConversion();
			currentADC = 2;		
		break;
		
		case 2:
			analogTemp = ADC;
			ADMUX = (1 << REFS0) | (0 << REFS1) | (1 << MUX0);
			startConversion();
			currentADC = 0;
		break;
	}	
}

//interrupt for button pin change
ISR(PCINT1_vect) {
	if (millis() - previousInterrupt > 400){	
		if ( !(PINC & (1 << PINC0))) buttonFlag = 1;
		previousInterrupt = millis();
	}
}