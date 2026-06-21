// projeto 2 - checkpoint 2: temporizadores e interrupcoes pic18f4550
// Nomes:
// Alexandre Augusto Novarino Britto - 14754672
// Eduardo Magaldi Magno - 15448780
// Raphael Parra - 15554782
// frequencia do oscilador: 8.000 mhz

// definicoes dos pinos do lcd no portd (usando latd para evitar falha de rmw)
sbit LCD_RS at LATD4_bit;
sbit LCD_EN at LATD5_bit;
sbit LCD_D4 at LATD0_bit;
sbit LCD_D5 at LATD1_bit;
sbit LCD_D6 at LATD2_bit;
sbit LCD_D7 at LATD3_bit;

sbit LCD_RS_Direction at TRISD4_bit;
sbit LCD_EN_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD0_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D7_Direction at TRISD3_bit;

// variaveis globais e maquina de estados
volatile char tempo_longo = 0;
volatile char tempo_curto = 0;
volatile char tick_250ms = 0;
volatile bit flag_atualiza_lcd;
volatile char modo_timer = 0; // 0=Aguardando, 1=Longo, 2=Curto, 3=Finalizado

// rotina de tratamento de interrupcoes
void interrupt() {

    // verificando o apertar do botao 1 (contagem longa de 60s)
    if (INT0IF_bit == 1) {
        INT0IF_bit = 0;

        tempo_longo = 60;
        modo_timer = 1;         // define estado para longo

        TMR0H = 0xC2;           // recarrega fisicamente para o 1º segundo ser exato
        TMR0L = 0xF7;
        TMR0ON_bit = 1;
        TMR1ON_bit = 0;
        flag_atualiza_lcd = 1;
    }

    // verificando o apertar do botao 2 (contagem curta de 10s)
    if (INTCON3.INT1IF == 1) {
        INTCON3.INT1IF = 0;

        tempo_curto = 10;
        tick_250ms = 0;
        modo_timer = 2;         // define estado para curto

        TMR1H = 0x0B;           // recarrega fisicamente para o 1º ciclo ser exato
        TMR1L = 0xDC;
        TMR1ON_bit = 1;
        TMR0ON_bit = 0;
        flag_atualiza_lcd = 1;
    }

    // estouro do timer 0 (base de tempo de 1s)
    if (TMR0IF_bit == 1) {
        TMR0IF_bit = 0;
        TMR0H = 0xC2;
        TMR0L = 0xF7;

        if (modo_timer == 1) {
            if (tempo_longo > 0) {
                tempo_longo--;
                flag_atualiza_lcd = 1;
            } else {
                modo_timer = 3;     // tempo acabou, transita para finalizado
                TMR0ON_bit = 0;     // desliga o timer de forma limpa
                flag_atualiza_lcd = 1;
            }
        }
    }

    // estouro do timer 1 (base de tempo de 250ms)
    if (TMR1IF_bit == 1) {
        TMR1IF_bit = 0;
        TMR1H = 0x0B;
        TMR1L = 0xDC;

        if (modo_timer == 2) {
            tick_250ms++;

            if (tick_250ms >= 4) {
                tick_250ms = 0;

                if (tempo_curto > 0) {
                    tempo_curto--;
                    flag_atualiza_lcd = 1;
                } else {
                    modo_timer = 3; // tempo acabou, transita para finalizado
                    TMR1ON_bit = 0; // desliga o timer de forma limpa
                    flag_atualiza_lcd = 1;
                }
            }
        }
    }
}

void main() {
    // variaveis locais do laco principal
    char tempo_exibir;
    char txt_dezena;
    char txt_unidade;

    // configuracoes dos pinos digitais
    ADCON1 = 0x0F;
    CMCON  = 0x07;

    TRISB0_bit = 1;
    TRISB1_bit = 1;

    // configuracao do timer 0 e timer 1
    T0CON = 0b00000110;
    T1CON = 0b10110000;

    // configuracao das interrupcoes externas
    INTCON2.INTEDG0 = 1;
    INTCON2.INTEDG1 = 1;

    INT0IF_bit = 0;
    INT0IE_bit = 1;

    INTCON3.INT1IF = 0;
    INTCON3.INT1IE = 1;

    // configuracao das interrupcoes de timers
    TMR0IF_bit = 0;
    TMR0IE_bit = 1;

    TMR1IF_bit = 0;
    TMR1IE_bit = 1;

    PEIE_bit = 1;
    GIE_bit = 1;

    // inicializacao do lcd
    Delay_ms(200);
    Lcd_Init();
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Delay_ms(200);
    Lcd_Out(1, 1, " Aguardando... ");

    // loop de atualizacao do display
    while(1) {

        if (flag_atualiza_lcd == 1) {
            flag_atualiza_lcd = 0;
            Delay_ms(10);
            // decide qual tempo processar via maquina de estados
            tempo_exibir = 0;
            if (modo_timer == 1) tempo_exibir = tempo_longo;
            else if (modo_timer == 2) tempo_exibir = tempo_curto;

            // conversao de valor para caractere
            txt_dezena = (tempo_exibir / 10) + '0';
            txt_unidade = (tempo_exibir % 10) + '0';

            // maquina de estados para os textos do cabecalho
            if (modo_timer == 1) {
                Lcd_Out(1, 1, "Timer: LONGO   ");
                Lcd_Out(2, 1, "Restam: 00s    ");
                Lcd_Chr(2, 9, txt_dezena);
                Lcd_Chr(2, 10, txt_unidade);
            }
            else if (modo_timer == 2) {
                Lcd_Out(1, 1, "Timer: CURTO   ");
                Lcd_Out(2, 1, "Restam: 00s    ");
                Lcd_Chr(2, 9, txt_dezena);
                Lcd_Chr(2, 10, txt_unidade);
            }
            else if (modo_timer == 3) {
                Lcd_Out(1, 1, "Finalizado!    ");
                Lcd_Out(2, 1, "               ");
            }
            else {
                Lcd_Out(1, 1, " Aguardando... ");
                Lcd_Out(2, 1, "               ");
            }
        }
    }
}