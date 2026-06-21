// Checkpoint 1
// Alexandre Augusto Novarino Britto - 14754672
// Eduardo Magaldi Magno - 15448780
// Raphael Parra - 15554782
// Definicoes dos pinos do LCD (Padrao EasyPIC v7)
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;

// Pino do botao
sbit BOTAO_INCREMENTO at RD0_bit;

void main() {
    unsigned short valor_contador = 0;    // Armazenando a contagem numerica de 0 a 9
    char caractere_exibicao = '0'; // Armazenando o 0 para poder aplicar no LCD
    unsigned short flag_botao_pressionado = 0; // Flag auxiliar

    // Configuracoes dos pinos digitais
    ADCON1 = 0x0F; // Configurando os pinos AD como digitas
    CMCON  = 0x07; // Desativando os comparadores internos
    TRISD0_bit = 1; // Configura o pino RD0 como entrada

    // Inicializacao do LCD
    Lcd_Init();      // Inicializa o LCD no modo de 4 bits
    Lcd_Cmd(_LCD_CLEAR); // Limpando a tela
    Lcd_Cmd(_LCD_CURSOR_OFF); // Desligando o cursor que pisca

    Lcd_Out(1, 1, "HelloWrld");   // Escrita do texto na coordenada linha1/coluna1

    // Loop de contagem
    while(1) {
        // Tratamento do debounce e borda de subida
        // Verificando o apertar do botao e se o estado da flag esta zerado
        if (BOTAO_INCREMENTO == 1 && flag_botao_pressionado == 0) {
            Delay_ms(20);  // Delay pra corrigir o debounce

            // Confirmando se o botao continua apertado apos o delay
            if (BOTAO_INCREMENTO == 1) {
                flag_botao_pressionado = 1;    // Ativando a flag para evitar incrementos incorretos
                valor_contador++;              // Incrementa a contagem

                if (valor_contador > 9) {
                    valor_contador = 0;
                }

                // Conversao de valor para caractere
                switch(valor_contador) {
                    case 0: caractere_exibicao = '0'; break;
                    case 1: caractere_exibicao = '1'; break;
                    case 2: caractere_exibicao = '2'; break;
                    case 3: caractere_exibicao = '3'; break;
                    case 4: caractere_exibicao = '4'; break;
                    case 5: caractere_exibicao = '5'; break;
                    case 6: caractere_exibicao = '6'; break;
                    case 7: caractere_exibicao = '7'; break;
                    case 8: caractere_exibicao = '8'; break;
                    case 9: caractere_exibicao = '9'; break;
                    default: caractere_exibicao = '0';
                }
            }
        }

        // Liberacao do botao
        if (BOTAO_INCREMENTO == 0) {
            flag_botao_pressionado = 0;   // Liberando a flag
        }

        // Atualizacao do display com o caractere correspondente ao contador da coordenada Linha2/Coluna1
        Lcd_Chr(2, 1, caractere_exibicao);
    }
}