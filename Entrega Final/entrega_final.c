// Entrega Final
// Alexandre Augusto Novarino Britto - 14754672
// Eduardo Magaldi Magno - 15448780
// Raphael Parra - 15554782

// Definicoes do LCD no PORTD
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

// Pino da Resistencia
sbit RESISTENCIA at LATD7_bit;
sbit RESISTENCIA_Dir at TRISD7_bit;

// Variaveis de controle
volatile char tempo_restante = 0;
volatile char tick_250ms = 0;
volatile bit flag_atualiza;
volatile char modo = 0; // 0:Aguarda, 1:Longo, 2:Curto, 3:Fim
unsigned int leitura_adc;
unsigned int temp_c; // Temperatura x10 (ex: 255 = 25.5 C)

void interrupt() {
    // Botao Longo (60s) - INT0
    if (INT0IF_bit) {
        INT0IF_bit = 0;
        tempo_restante = 60;
        modo = 1;
        TMR0H = 0xC2; TMR0L = 0xF7; // 1s
        TMR0ON_bit = 1; TMR1ON_bit = 0;
        flag_atualiza = 1;
    }
    // Botao Curto (10s) - INT1
    if (INT1IF_bit) {
        INT1IF_bit = 0;
        tempo_restante = 10;
        tick_250ms = 0;
        modo = 2;
        TMR1H = 0x0B; TMR1L = 0xDC; // 250ms
        TMR1ON_bit = 1; TMR0ON_bit = 0;
        flag_atualiza = 1;
    }
    // Timer 0 - Base 1s
    if (TMR0IF_bit) {
        TMR0IF_bit = 0;
        TMR0H = 0xC2; TMR0L = 0xF7;
        if (modo == 1 && tempo_restante > 0) tempo_restante--;
        if (tempo_restante == 0) { modo = 3; TMR0ON_bit = 0; }
        flag_atualiza = 1;
    }
    // Timer 1 - Base 250ms
    if (TMR1IF_bit) {
        TMR1IF_bit = 0;
        TMR1H = 0x0B; TMR1L = 0xDC;
        if (modo == 2) {
            tick_250ms++;
            if (tick_250ms >= 4) {
                tick_250ms = 0;
                if (tempo_restante > 0) tempo_restante--;
                if (tempo_restante == 0) { modo = 3; TMR1ON_bit = 0; }
                flag_atualiza = 1;
            }
        }
    }
}

void main() {
    // Inicializacao ADC e Correcao de Bug do MikroC
    ADC_Init(); 
    // ADCON1: Vref+ em AN3, Vref- em AN2, AN0 como analogico (0b00111011)
    ADCON1 = 0x3B; 
    CMCON = 0x07;

    TRISB0_bit = 1; TRISB1_bit = 1; // Botoes
    RESISTENCIA_Dir = 0; RESISTENCIA = 0; // Saida Resistencia

    // Configuracao Timers
    T0CON = 0b00000110; // 16 bits, 1:128
    T1CON = 0b10110000; // 16 bits, 1:8

    // Habilitacao Interrupcoes
    INT0IE_bit = 1; INT1IE_bit = 1;
    TMR0IE_bit = 1; TMR1IE_bit = 1;
    INTEDG0_bit = 1; INTEDG1_bit = 1; // Borda de subida
    GIE_bit = 1; PEIE_bit = 1;

    Lcd_Init();
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Lcd_Out(1, 1, " Aguardando... ");

    while(1) {
        // Leitura continua da temperatura (0-100.0 C)
        leitura_adc = ADC_Get_Sample(0); 
        // Com Vref=1V, 1023 representa 102.3 C (escala 10mV/C)
        temp_c = (leitura_adc * 100) / 102; 

        // Logica da Resistencia (Histerese)
        if (temp_c < 600) RESISTENCIA = 1;      // Liga abaixo de 60.0 C
        else if (temp_c > 800) RESISTENCIA = 0; // Desliga acima de 80.0 C

        if (flag_atualiza || (modo != 0)) {
            flag_atualiza = 0;
            
            // Exibicao do Tempo
            if (modo == 1) Lcd_Out(1, 1, "Timer: LONGO   ");
            else if (modo == 2) Lcd_Out(1, 1, "Timer: CURTO   ");
            else if (modo == 3) Lcd_Out(1, 1, "Finalizado!    ");
            else Lcd_Out(1, 1, " Aguardando... ");

            // Exibicao formatada: "Restam: XXs XX.XC"
            Lcd_Chr(2, 1, (tempo_restante/10) + '0');
            Lcd_Chr(2, 2, (tempo_restante%10) + '0');
            Lcd_Out(2, 3, "s ");
            
            Lcd_Chr(2, 6, (temp_c/100) + '0');
            Lcd_Chr(2, 7, ((temp_c/10)%10) + '0');
            Lcd_Out(2, 8, ".");
            Lcd_Chr(2, 9, (temp_c%10) + '0');
            Lcd_Out(2, 10, " C  ");
            
            Delay_ms(100); // Estabilidade visual
        }
    }
}