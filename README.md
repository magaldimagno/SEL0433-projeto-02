# Projeto 1: Sistema de Dosagem Rotativa

**Disciplina:** SEL0433 - Aplicação de Microprocessadores  
**Autores:** 
* Alexandre Augusto Novarino Britto - 14754672
* Eduardo Magaldi Magno - 15448780
* Raphael Parra - 15554782

## Objetivo do Projeto
Este projeto implementa um sistema de monitoramento de temperatura e tempo para fornos industriais utilizando o microcontrolador PIC18F4550 (simulado via SimulIDE e baseado no Kit EasyPIC v7).

O dispositivo realiza a leitura contínua da temperatura interna através de um sensor LM35 (emulado por potenciômetro), gerencia cronômetros regressivos de curta (10s) e longa (60s) duração via interrupções externas e controla uma resistência de aquecimento (LED) com lógica de histerese.

## Mapeamento de Hardware (I/O)

* **Porta A**
    * **Pino RA0:** Entrada analógica (AN0) conectada ao sensor de temperatura LM35 (emulado por potenciômetro).
    * **Pino RA2:** Entrada de referência de tensão negativa (VREF-) conectada ao GND para calibração do ADC.
    * **Pino RA3:** Entrada de referência de tensão positiva (VREF+) configurada para 1.0V, garantindo a precisão na leitura da escala de 0 a 100 °C.

* **Porta B**
    * **Pino RB0:** Botão de interrupção externa (INT0) utilizado para selecionar e iniciar o Timer Longo (60s).
    * **Pino RB1:** Botão de interrupção externa (INT1) utilizado para selecionar e iniciar o Timer Curto (10s).

* **Porta P3**
    * **Pinos RD0 a RD3:** Barramento de dados (D4 a D7) do LCD 16x2 operando em modo de comunicação de 4 bits.
    * **Pino RD4:** Pino de controle RS (Register Select) do display LCD.
    * **Pino RD5:** Pino de controle EN (Enable) do display LCD.
    * **Pino RD7:** Saída digital para controle do LED indicador da Resistência, operando com lógica de histerese (liga < 60 °C, desliga > 80 °C).

## Arquitetura e Lógica de Implementação

### 1. Tratamento de Dados e Conversão

Para otimizar o uso da memória de dados (RAM) e cumprir os requisitos do manual, a leitura da temperatura foi implementada utilizando apenas lógica de inteiros.

* **Cálculo com Escala:** O valor lido pelo ADC (0 a 1023) é processado pela fórmula `temp_c = ((unsigned long)leitura_adc * 100) / 102`, onde o resultado representa a temperatura com uma casa decimal implícita (ex: 255 equivale a 25.5 °C).
* **Prevenção de Overflow:** Foi utilizado o type casting para `unsigned long` (32 bits) durante a multiplicação. Isso evita o erro de estouro de variável que ocorria em cálculos de 16 bits quando a temperatura atingia 64 °C, fazendo o valor resetar incorretamente para zero.

### 2. Configuração do Conversor ADC e Bug do Compilador

A precisão decimal do sensor LM35 (10mV/°C) exige uma referência de tensão de 1.0V para que a escala de 0-100 °C utilize toda a resolução do ADC

* **Ordem de Inicialização:** Devido a um bug no compilador MikroC, a função `ADC_Init()` reseta as configurações de referência externa do registrador `ADCON1`. A solução aplicada foi configurar `ADCON1 = 0x3B;` (habilitando VREF+ no pino A3 e VREF- no A2) obrigatoriamente após a chamada de inicialização da biblioteca.
* **Leitura Estável:** Para garantir a estabilidade das referências externas, utilizou-se a função `ADC_Get_Sample(0)` em vez de `ADC_Read()`, conforme recomendado para evitar resets indesejados no hardware durante a simulação.

### 3. Temporização e Interrupções

O sistema utiliza dois temporizadores internos para gerenciar os cronômetros regressivos e a taxa de atualização da interface:

* **Timer 0 (Base de 1s):** Configurado com prescaler de 1:128 e carga inicial de 49911 para gerar interrupções precisas a cada 1 segundo, utilizado no Timer Longo (60s).
* **Timer 1 (Base de 250ms):** Configurado com prescaler de 1:8 e carga inicial de 3036 para gerar interrupções a cada 0,25 segundos, utilizado no Timer Curto (10s).
* **Seleção de Modos:** O início dos cronômetros é acionado por interrupções externas (INT0 e INT1) nos botões, garantindo resposta imediata ao comando do usuário.

### 4. Interface LCD e Estabilidade Visual

A comunicação com o display 16x2 ocorre no modo de 4 bits para economia de pinos do microcontrolador.

* **Técnica de Máscara Estática:** Para evitar erros de sincronismo e o lixo visual no SimulIDE, o código não utiliza o comando de limpeza total (`Lcd_Cmd(_LCD_CLEAR)`) no loop principal. Em vez disso, uma string base, como uma "máscara", é enviada e apenas os caracteres numéricos que sofrem alteração são sobrescritos em posições fixas via `Lcd_Chr`.
* **Conversão ASCII Manual:** Como o uso de `float` e bibliotecas de conversão complexas foi evitado, os dígitos são extraídos individualmente via divisões sucessivas e restos (`/` e `%`), somando-se ao valor 48 (caractere '0') para exibição direta no LCD.

### 5. Controle da Resistência

O acionamento da resistência (simulada pelo LED no pino RB2) segue uma lógica de histerese para evitar oscilações rápidas e danos ao atuador:

* **Liga:** Quando a temperatura lida é menor que 60.0 °C.
* **Desliga:** Quando a temperatura ultrapassa 80.0 °C.

Essa lógica permanece ativa durante todos os modos de operação, garantindo a segurança térmica do forno industrial

## Evidência de Funcionamento

[Vídeo de demonstração do projeto](https://youtu.be/BnYTmTPhuEs)