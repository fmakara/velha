/*
 * Velha2.c
 *
 * Created: 29/04/2016 17:28:19
 * Author : Felipe Douglas Makara
 * Ajudante do projeto original que vale a pena ser mencionado mais uma vez:
 *                        Diogo Rikio Miyazaki
 *
 *
 * Este programa foi feito para o jogo da velha do mural do grupo PET Elétrica UFPR
 * Nesta versão há dois modos de jogo: PvP ou PvM. Como o jogo da velha é completamente mapeado, 
 * no modo PvM o microcontrolador nunca perde.
 * As entradas são feitas através dos 6 pinos analógicos, multiplexadas com um 4053 (mux analogico)
 * São usados, portanto, conjuntos emissor-receptor IR para que seja lido os "botoes".
 * As saídas são feitas através de 3 shift registers 4094 buferizadas por transistores.
 * A lógica da leitura analógica segue o modelo do esquema de botões do mural, onde o limiar nunca 
 * tem o modulo da diferença maior que um determinado valor, formando uma lógica de schimidt-trigger digital
 */
#include <avr/io.h>

#include "hardware.h"
#include "jogadas.h"

const uint8_t vencedoras[9][3] = {{0,0,0},{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
uint8_t checa_vencedor(){
	for(uint8_t i=1;i<9;i++)
		if(leds_l[vencedoras[i][0]]==leds_l[vencedoras[i][1]] && leds_l[vencedoras[i][0]]==leds_l[vencedoras[i][2]] && leds_l[vencedoras[i][0]]!=0)return i;
	return 0;
}
void pisca_vencedor(){
	uint8_t original[9],sobreposta[9], i, p, j;
	for(i=0;i<9;i++){
		original[i]=leds_l[i];
		sobreposta[i]=leds_l[i];
	}
	p = checa_vencedor();
	if(p==0)for(i=0;i<9;i++)sobreposta[i]=0;
	sobreposta[vencedoras[p][0]]=0;
	sobreposta[vencedoras[p][1]]=0;
	sobreposta[vencedoras[p][2]]=0;
	for(j=0;j<5;j++){
		for(i=0;i<9;i++)leds_l[i]=sobreposta[i];
		atualizar_display();
		_delay_ms(200);
		for(i=0;i<9;i++)leds_l[i]=original[i];
		atualizar_display();
		_delay_ms(200);
	}
}
void jogar2p(){
	uint8_t jogador, jogada;
	apagar();
	atualizar_display();
	for(jogador=0;jogador<9;jogador++){
		jogada = esperarJogada(120000);
		if(jogada>9){
			pisca_vencedor();
			return;
		}
		if(leds_l[jogada]){
			jogador--;
		}else{
			if(jogador&1)fazerJogada(jogada,LED_O);
			else         fazerJogada(jogada,LED_X);
			if(checa_vencedor()){
				pisca_vencedor();
				return;
			}
		}
	}
	pisca_vencedor();
	return;
}

//Okay, para que eu não me esqueça e/ou alguem leia essa loucura no futuro, vou explicar passo a passo como funcionará
//esse algoritmo. Em primeiro lugar, precisamos de todas as jogadas feitas pelo jogador. As jogadas precisam estar
//ordenadas, assim não é necessário saber as jogadas já feitas pelo uC. Porém, a ultima jogada não nos importa
//uma vez que não é possivel fazer a 10ª jogada, logo, somente é necessário o conhecimento de apenas 4 jogadas
//#include "jogadas.h"
//com isso, para jogar, será feito o seguinte: o jogador joga, é checado (apezar de em vão) a vitória (ou o empate)
//então com as jogadas anteriores, o uC joga, é testado para ver se venceu (ou empatou)

void jogar1p(){
	uint8_t jogadas[4], jogada;
	apagar();
	atualizar_display();
	do{   ////////////////////////////////////////////////1
		jogada = esperarJogada(120000);
		if(jogada>9){
			pisca_vencedor();
			return;
		}
	}while(leds_l[jogada]);
	fazerJogada(jogada,LED_X);
	jogadas[0] = jogada;  ////////////////////////////////2
	jogada = pgm_read_byte( &JOG1[jogadas[0]] ) - 1;
	fazerJogada(jogada,LED_O);
	
	do{   ////////////////////////////////////////////////3
		jogada = esperarJogada(120000);
		if(jogada>9){
			pisca_vencedor();
			return;
		}
	}while(leds_l[jogada]);
	fazerJogada(jogada,LED_X);
	jogadas[1] = jogada;  ////////////////////////////////4
	jogada = pgm_read_byte( &JOG2[jogadas[0]][jogadas[1]] ) - 1;
	fazerJogada(jogada,LED_O);
	
	do{   ////////////////////////////////////////////////5
		jogada = esperarJogada(120000);
		if(jogada>9){
			pisca_vencedor();
			return;
		}
	}while(leds_l[jogada]);
	fazerJogada(jogada,LED_X);
	jogadas[2] = jogada;  ////////////////////////////////6
	jogada = pgm_read_byte( &JOG3[jogadas[0]][jogadas[1]][jogadas[2]] ) - 1;
	fazerJogada(jogada,LED_O);
	if(checa_vencedor()){
		pisca_vencedor();
		return;
	}
	
	do{   ////////////////////////////////////////////////7
		jogada = esperarJogada(120000);
		if(jogada>9){
			pisca_vencedor();
			return;
		}
	}while(leds_l[jogada]);
	fazerJogada(jogada,LED_X);
	jogadas[3] = jogada;  ////////////////////////////////8
	jogada = pgm_read_byte( &JOG4[jogadas[0]][jogadas[1]][jogadas[2]][jogadas[3]] ) - 1;
	fazerJogada(jogada,LED_O);
	if(checa_vencedor()){
		pisca_vencedor();
		return;
	}
	
	do{   ////////////////////////////////////////////////9
		jogada = esperarJogada(120000);
		if(jogada>9){
			pisca_vencedor();
			return;
		}
	}while(leds_l[jogada]);
	fazerJogada(jogada,LED_X);
	pisca_vencedor();
}

int main(void){
	init_hardware();
	setup_and_test();
	while (1){
		apagar();
		leds[1][0] = LED_O|LED_X;
		leds[1][2] = LED_X;
		atualizar_display();
		uint8_t jogada = esperarJogada(10000);
		if(jogada==3){
			fazerJogada(3,0);
			jogar2p();
		}else if(jogada==5){
			fazerJogada(5,0);	
			jogar1p();
		}
	}
}

