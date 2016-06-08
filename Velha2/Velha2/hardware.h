#ifndef HARDWARE_H
#define HARDWARE_H

#define F_CPU 8000000
#include <util/delay.h>
#include <avr/cpufunc.h>

#define ANALOG	PORTD3
#define CLOCK	PORTD2
#define STROBE	PORTD1
#define DATA	PORTD0

#define LED_X 1
#define LED_O 2
uint8_t leds[3][3];
uint8_t *leds_l = leds[0];
#define PRE 150
int16_t valores[9], bastioes[9], thresholds[9]={PRE,100,PRE,PRE,100,PRE,PRE,PRE,PRE};
uint8_t estados[9];

inline void apagar(){
	for(int i=0;i<9;i++)leds_l[i]=0;
}
void atualizar_display(){
	uint8_t i;
	uint32_t dado = 0, shift;
	if(leds[0][0]&LED_X)dado |= 1UL<<19;
	if(leds[0][0]&LED_O)dado |= 1UL<<18;
	if(leds[0][1]&LED_X)dado |= 1UL<<11;
	if(leds[0][1]&LED_O)dado |= 1UL<<10;
	if(leds[0][2]&LED_X)dado |= 1UL<<3;
	if(leds[0][2]&LED_O)dado |= 1UL<<2;
	if(leds[1][0]&LED_X)dado |= 1UL<<17;
	if(leds[1][0]&LED_O)dado |= 1UL<<21;
	if(leds[1][1]&LED_X)dado |= 1UL<<9;
	if(leds[1][1]&LED_O)dado |= 1UL<<13;
	if(leds[1][2]&LED_X)dado |= 1UL<<1;
	if(leds[1][2]&LED_O)dado |= 1UL<<5;
	if(leds[2][0]&LED_X)dado |= 1UL<<16;
	if(leds[2][0]&LED_O)dado |= 1UL<<20;
	if(leds[2][1]&LED_X)dado |= 1UL<<8;
	if(leds[2][1]&LED_O)dado |= 1UL<<12;
	if(leds[2][2]&LED_X)dado |= 1UL<<0;
	if(leds[2][2]&LED_O)dado |= 1UL<<4;
	PORTD &= ~((1<<CLOCK)|(1<<STROBE)|(1<<DATA));
	for(shift=1,i=0 ; i<24 ; i++,shift<<=1){
		if((dado&shift)!=0)PORTD |= (1<<DATA);
		else               PORTD &=~(1<<DATA);
		_NOP();
		PORTD |= (1<<CLOCK);
		_NOP();
		PORTD &=~(1<<CLOCK);
	}
	PORTD |= (1<<STROBE);
	_NOP();
	_NOP();
	PORTD &=~(1<<STROBE);
}
uint16_t adc(uint8_t porta){
	ADMUX = (1<<REFS0)|(porta&0x0F);
	_NOP();
	ADCSRA |= (1<<ADSC);
	while(ADCSRA&(1<<ADSC));
	return ADC;
}
void determinar_estados(){
	uint8_t i;
	for(i=0;i<9;i++)valores[i] = 0;
	for(i=0;i<8;i++){
		PORTD &=~(1<<ANALOG);
		valores[2] += adc(0);
		valores[3] += adc(3);
		valores[4] += adc(4);
		valores[6] += adc(5);
		PORTD |= (1<<ANALOG);
		valores[5] += adc(1);
		valores[8] += adc(2);
		valores[0] += adc(3);
		valores[1] += adc(4);
		valores[7] += adc(5);
	}
	for(i=0;i<9;i++){
		valores[i]/=8;
		
		/*
		if(valores[i]>bastioes[i])estados[i]=0;
		else                      estados[i]=1;
		*/
		if(estados[i]){
			if(valores[i]>bastioes[i]){
				estados[i]=0;
				bastioes[i] = valores[i]-thresholds[i];
			}
		}else{
			if(valores[i]<bastioes[i]){
				estados[i]=1;
				bastioes[i] = valores[i]+thresholds[i];
			}
		}
		if(valores[i]>(bastioes[i]+thresholds[i]))bastioes[i]=valores[i]-thresholds[i];
		if(valores[i]<(bastioes[i]-thresholds[i]))bastioes[i]=valores[i]+thresholds[i];
	}
}
void init_hardware(){
	DDRD |= (1<<ANALOG)|(1<<CLOCK)|(1<<STROBE)|(1<<DATA);
	PORTD &= ~((1<<ANALOG)|(1<<CLOCK)|(1<<STROBE)|(1<<DATA));
	apagar();
	atualizar_display();
	DDRC = 0;
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(0b00000110);
	ADCSRB = 0;
	DIDR0 = 0b00111111;
	
	PORTD &=~(1<<ANALOG);
	valores[2] = adc(0);
	valores[3] = adc(3);
	valores[4] = adc(4);
	valores[6] = adc(5);
	PORTD |= (1<<ANALOG);
	valores[5] = adc(1);
	valores[8] = adc(2);
	valores[0] = adc(3);
	valores[1] = adc(4);
	valores[7] = adc(5);
	for(uint8_t i=0;i<9;i++)bastioes[i]=valores[i]-thresholds[i];
}
void setup_and_test(){
	apagar();
	atualizar_display();
	for(uint8_t i=0;i<9;i++){
		leds_l[i] = LED_X;
		atualizar_display();
		_delay_ms(50);
		determinar_estados();
		apagar();
		atualizar_display();
		_delay_ms(50);
		determinar_estados();
	}
	for(uint8_t i=0;i<9;i++){
		leds_l[i] = LED_O;
		atualizar_display();
		_delay_ms(50);
		determinar_estados();
		apagar();
		atualizar_display();
		_delay_ms(50);
		determinar_estados();
	}
}
void fazerJogada(uint8_t posicao, uint8_t jogada){
	uint8_t ultima = leds_l[posicao];
	for(int i=0;i<3;i++){
		leds_l[posicao] = jogada;
		atualizar_display();
		_delay_ms(100);
		determinar_estados();
		leds_l[posicao] = ultima;
		atualizar_display();
		_delay_ms(100);
		determinar_estados();
	}
	leds_l[posicao] = jogada;
	atualizar_display();
	determinar_estados();
}
uint8_t esperarJogada(int32_t timeout){
	uint8_t ultimos_estados[9], i;
	determinar_estados();
	for(i=0;i<9;i++)ultimos_estados[i]=estados[i];
	while(timeout>0){
		determinar_estados();
		for(i=0;i<9;i++){
			if(estados[i]&&!ultimos_estados[i])return i;
		}
		for(uint8_t i=0;i<9;i++)ultimos_estados[i]=estados[i];
		_delay_ms(50);
		timeout -= 55;
	}
	return 10;
}



#endif