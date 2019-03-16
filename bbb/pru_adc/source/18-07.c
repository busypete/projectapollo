// Portabilidade da função memory e suas derivadas para o contexto da PRU
// Acessando normalmente o mapa de memória global do ARM
// ADC devidamente configurado e funcionando
// Próximo: trigar a interrupção do ADC a partir da interrupção do timer IEP

#include <stdint.h>
#include <stdlib.h>
#include <pru_cfg.h>
#include "rsc_table_pru1.h"

volatile register uint32_t __R30;               // Registro de saída
volatile register uint32_t __R31;               // Registro de entrada

#define LED             (1<<6)                  // P8.39 = R30_6
#define BOTAO           (1<<2)                  // P8.43 = R31_2

#define PRU_ICSS        0x4A300000
#define PRU_BUFFER      PRU_ICSS + 0x00010000   // Frame de leitura do ponto de vista da PRU
#define ARM_BUFFER      PRU_ICSS + 0x00011000   // Frame de escrita do ponto de vista da PRU
#define CONTROL_BUFFER  PRU_ICSS + 0x00012000   // Frame de controle
#define PRU_READ        0xAA                    // Palavra de controle que habilita a leitura do PRU_BUFFER, por parte da PRU
#define ARM_READ        0xCC                    // Palavra de controle que habilita a leitura do ARM_BUFFER, por parte do ARM
#define LOCK            0xDD

#define ADC_TSC         0x44E0D000
#define SYSCONFIG       (ADC_TSC + 0x10)
#define IRQSTATUS_RAW   (ADC_TSC + 0x24)
#define IRQSTATUS       (ADC_TSC + 0x28)
#define IRQENABLE_CLR   (ADC_TSC + 0x30)
#define IRQWAKEUP       (ADC_TSC + 0x34)
#define DMAENABLE_CLR   (ADC_TSC + 0x3C)
#define ADC_CTRL        (ADC_TSC + 0x40)
#define ADCSTAT         (ADC_TSC + 0x44)
#define STEPENABLE      (ADC_TSC + 0x54)
#define IDLECONFIG      (ADC_TSC + 0x58)
#define STEPCONFIG1     (ADC_TSC + 0x64)
#define STEPDELAY1      (ADC_TSC + 0x68)
#define STEPCONFIG3     (ADC_TSC + 0x74)
#define STEPDELAY3      (ADC_TSC + 0x78)
#define FIFO0COUNT      (ADC_TSC + 0xE4)
#define FIFO1COUNT      (ADC_TSC + 0xF0)
#define FIFO0DATA       (ADC_TSC + 0x100)
#define FIFO1DATA       (ADC_TSC + 0x200)

#define CH1             36                                  // Número de amostras a serem feitas do canal 1
#define CH3             2                                   // Número de amostras a serem feitas do canal 3
#define PACK            4                                   // Número de pacotes


// Função:    button_state
// Descrição: Lê o estado do botão caracterizado pelo #define do programa
// Entrada:   -
// Saída:     unsigned int          - 0: botão não pressionado
//                                  - 1: botão pressionado
/*********************************************************************************/
int button_state(void)
{
if((__R31&BOTAO)==0) return(1);                 // Botão apertado
else return(0);                                 // Botão não apertado
}
/********************************************/

// Função:    debounce
// Descrição: Realiza debounce do botão caracterizado pelo #define do programa
// Entrada:   -
// Saída:     -
/*********************************************************************************/
void debounce(void)
{
while(button_state());
__delay_cycles(10000000);
}
/********************************************/

// Função:    led_read
// Descrição: Lê o estado lógico do LED caracterizado pelo #define do programa
// Entrada:
// Saída:     unsigned int          - 0: led apagado
//                                  - 1: led aceso
/*********************************************************************************/
int led_read(void)
{
if((__R30 &= LED)==0) return(0);                // LED apagado
else return (1);                                // LED aceso
}
/********************************************/

// Função:    led_write
// Descrição: Aplica um estado lógico ao LED caracterizado pelo #define do programa
// Entrada:   int i                 - 0: led apagado
//                                  - 1: led aceso
// Saída:     -
/*********************************************************************************/
void led_write(int i)
{
if(i==1) __R30 |= LED;                          // R30 = R30 | 0x00008000
else __R30 &= ~(LED);                           // R30 = R30 & 0xFFFF7FFF
}
/********************************************/

// Função:    led_toggle
// Descrição: Inverte o estado do LED caracterizado pelo #define do programa
// Entrada:   -
// Saída:     -
/*********************************************************************************/
void led_toggle(void)
{
__R30 ^= LED;                                   // Inverte LED
}
/********************************************/

/*********************************************************************************/
// Função:    memory
// Descrição: Leitura e escrita em posições de memória.
//            Os argumentos 'content' e 'logic' não possuem nenhum efeito
//            caso acess = 'r'.
// Entrada:   int  ADDR             - endereço de memória de interesse
//            char acess            - 'r': leitura
//                                  - 'w': escrita
//            int  content          - conteúdo a ser escrito no endereço ADDR
//            char logic            - 'k': *ADDR = content         (overwrite)
//                                  - 'o': *ADDR |= content        (lógica OR)
//                                  - 'a': *ADDR = *ADDR & content (lógica AND)
// Saída:     int                   - resultado da leitura
/*********************************************************************************/
int memory(int ADDR, char acess, int content, char logic)
{
int read_result = 0;                                                    // Variável para resultado da leitura
int* pointer = (int*)(ADDR);                                            // Cria ponteiro para a posição de memória de interesse

if(acess == 'r') read_result = *pointer;                                // Leitura da posição de memória
else if(acess == 'w')                                                   // Escrita na posição de memória
    {
    if(logic == 'k') *pointer = content;                                // Overwrite
    else if(logic == 'o') *pointer |= content;                          // Ĺógica OR
    else if(logic == 'a') *pointer &= content;                          // Lógica AND
    read_result = *pointer;                                             // Leitura da posição de memória após a escrita
    }

return(read_result);                                                    // Retorna resultado da leitura
}
/********************************************/

/*********************************************************************************/
// Função:    bit_clear
// Descrição: Limpa um bit usando a função "memory"
// Entrada:   int ADDR              - endereço de memória de interesse
//            int x                 - bit de interesse
// Saída:     -
/*********************************************************************************/
void bit_clear(int ADDR, int x)
{
memory(ADDR,'w',(~(1<<x)),'a');
}
/********************************************/

/*********************************************************************************/
// Função:    bit_set
// Descrição: Seta um bit usando a função "memory"
// Entrada:   int  ADDR             - endereço de memória de interesse
//            int  x                - bit de interesse
// Saída:     -
/*********************************************************************************/
void bit_set(int ADDR, int x)
{
memory(ADDR,'w',(1<<x),'o');
}
/********************************************/

/*********************************************************************************/
// Função:    bit_write_interval
// Descrição: Escreve num intervalo de bits usando a função "memory"
// Entrada:   int  ADDR             - endereço de memória de interesse
//            int  MSB              - bit mais significativo do intervalo
//            int  LSB              - bit menos significativo do intervalo
//            int  content          - conteúdo a ser escrito no intervalo
// Saída:     -
/*********************************************************************************/
void bit_write_interval(int ADDR, int MSB, int LSB, int content)
{
int i=0,j=0,before=0,after=0,and=0,or=0;

j = 1<<31;
for(i=31;i>MSB;i--)                             // Preenche com '1' à esquerda do intervalo de interesse
    {
    before = before | j;
    j = j/2;
    }

j = 1<<0;
for(i=0;i<LSB;i++)                              // Preenche com '1' à direita do intervalo de interesse
    {
    after = after | j;
    j = 2*j;
    }

and = (0xFFFFFFFF)&((content<<LSB)|after|before);  // Palavra a ser limpada
or = content<<LSB;                              // Palavra a ser setada

memory(ADDR,'w',and,'a');                       // Limpa os bits definidos como '0' em 'content'
memory(ADDR,'w',or,'o');                        // Seta os bits definidos como '1' em 'content'
}
/********************************************/

/*********************************************************************************/
// Função:    bit_read
// Descrição: Lê um bit usando a função "memory"
// Entrada:   int  ADDR             - endereço de memória de interesse
//            int  x                - bit de interesse
// Saída:     int                   - nivel lógico do bit de interesse                                 -
/*********************************************************************************/
int bit_read(int ADDR, int x)
{
return(((memory(ADDR,'r',0x0,'o'))&(1<<x))>>x);
}
/*********************************************************************************/

/*********************************************************************************/
// Função:    bit_read_interval
// Descrição: Lê um intervalo de bits usando a função "memory"
// Entrada:   int  ADDR             - endereço de memória de interesse
//            int  MSB              - bit mais significativo
//            int  LSB              - bit menos significativo
//            int  content          - conteúdo a ser escrito
// Saída:     int                   - resultado da leitura                                   -
/*********************************************************************************/
int bit_read_interval(int ADDR, int MSB, int LSB)
{
int i=0,j=0,before=0,after=0;

j = 1<<31;
for(i=31;i>MSB;i--)                                        // Preenche com '0' à esquerda do intervalo de interesse
    {
    before = before | j;
    j = j/2;
    }
before = ~before;

j = 1<<0;
for(i=0;i<LSB;i++)                                         // Preenche com '0' à direita do intervalo de interesse
    {
    after = after | j;
    j = 2*j;
    }
after = ~after;

return(((memory(ADDR,'r',0x0,'o'))&after&before)>>LSB);    // Elimina o conteúdo alheio ao intervalo de interesse
}
/*********************************************************************************/

// Função:    config_ADC
// Descrição: Configura o módulo ADC
// Entrada:   -
// Saída:     -
/*********************************************************************************/
void config_ADC(void)
{
// ADC_CTRL
bit_clear(ADC_CTRL,0);                        // Desabilita o módulo ADC
bit_clear(ADC_CTRL,9);                        // Eventos ativados por software são prioritários aos eventos ativados por hardware
bit_clear(ADC_CTRL,8);                        // Mapeia evento de hardware para pen_event
bit_clear(ADC_CTRL,7);                        // Desabilita touchscreen
bit_write_interval(ADC_CTRL,6,5,0x0);         // Desabilita os modos 4-wire e 5-wire
bit_clear(ADC_CTRL,3);                        // Seleciona bias interno para AFE (analog front-end: parte do ADC que efetivamente converte)
bit_set(ADC_CTRL,2);                          // Habilita a escrita nos registradores STEPCONFIGx
bit_set(ADC_CTRL,1);                          // Habilita a escrita da ID do canal sampleado nos registradores FIFOx

// STEPCONFIG1
bit_clear(STEPCONFIG1,27);                    // Desabilita out-of-range check (compara o valor em FIFOx com os limites definidos em ADCRANGE)
bit_set(STEPCONFIG1,26);                      // Define FIFO1 como local do resultado da conversão
bit_clear(STEPCONFIG1,25);                    // Define como single-ended
bit_write_interval(STEPCONFIG1,24,23,0b00);   // Define referência negativa como sendo VSSA
bit_write_interval(STEPCONFIG1,22,19,0b0110); // Associona o STEP1 com  o canal 7  (AIN6)
bit_write_interval(STEPCONFIG1,14,12,0b000);  // Define referência positiva como sendo VDDA_ADC
bit_write_interval(STEPCONFIG1,4,2,0b100);    // Define número de médias igual a 16
bit_write_interval(STEPCONFIG1,1,0,0b00);     // Conversão singleshot habilitada por software

// STEPDELAY1
bit_write_interval(STEPDELAY1,17,0,0x0);      // Mínimo open delay
bit_write_interval(STEPDELAY1,31,24,0x0);     // Mínimo sample delay

// STEPCONFIG3
bit_clear(STEPCONFIG3,27);                    // Desabilita out-of-range check (compara o valor em FIFOx com os limites definidos em ADCRANGE)
bit_set(STEPCONFIG3,26);                      // Define FIFO1 como local do resultado da conversão
bit_clear(STEPCONFIG3,25);                    // Define como single-ended
bit_write_interval(STEPCONFIG3,24,23,0b00);   // Define referência negativa como sendo VSSA
bit_write_interval(STEPCONFIG3,22,19,0b0010); // Associona o STEP3 com  o canal 3 (AIN2)
bit_write_interval(STEPCONFIG3,14,12,0b000);  // Define referência positiva como sendo VDDA_ADC
bit_write_interval(STEPCONFIG3,4,2,0b100);    // Define número de médias igual a 16
bit_write_interval(STEPCONFIG3,1,0,0b00);     // Conversão singleshot habilitada por software

// STEPDELAY3
bit_write_interval(STEPDELAY3,17,0,0x0);      // Mímimo open delay
bit_write_interval(STEPDELAY3,31,24,0x0);     // Mínimo sample delay

// DMAENABLE_CLR
bit_clear(DMAENABLE_CLR,1);                   // Desabilita recurso de DMA do registro FIFO1
bit_clear(DMAENABLE_CLR,1);                   // Desabilita recurso de DMA do registro FIFO0

// IRQWAKEUP
bit_clear(IRQWAKEUP,0);                       // Desabilita wakeup generation of hw pen event

// IRQSTATUS_RAW
bit_write_interval(IRQSTATUS,10,0,0x7FF);     // Limpa os flags pendentes de todas as interrupções do módulo ADC (mesmo aquelas mascaradas)

// IRQSTATUS
bit_write_interval(IRQSTATUS,10,0,0x7FF);     // Limpa os flags pendentes de todas as interrupções do módulo ADC

// IRQENABLE_CLR
bit_write_interval(IRQENABLE_CLR,10,0,0x7FF); // Desabilita todas as interrupções do módulo ADC

// STEPENABLE
bit_write_interval(STEPENABLE,16,0,0x0);      // Desabilita todos os STEPS

// ADC_CTRL
bit_clear(ADC_CTRL,4);                        // Garante que o AFE está energizado
while((bit_read(ADCSTAT,5))!=0);              // Aguarda estado IDLE
bit_set(ADC_CTRL,0);                          // Habilita o módulo ADC
}
/********************************************/

/*********************************************************************************/
// Função:    capture
// Descrição: Captura single-shot de um certo canal
// Entrada:   unsigned int  step    - step do canal de interesse
//            unsigned int  n       - número de medidas
// Saída:     unsgined int* result  - vetor contendo os 'n' resultados das conversões
//                                  - a ultima posição do vetor contém o step ID
//                                  - se a ultima posição for igual a ';', a medida
//                                  - é inválida
/*********************************************************************************/
int* capture(int step, int n)
{
unsigned int i = 0, stepID = 0, ok = 0, new = 0, old = 0;
int* result = (int*)(malloc((n+1)*sizeof(int)));              // Aloca vetor de resultado

// Executa 'n' conversões single-shot do step desejado
for(i=0;i<n;i++)
    {
    while((bit_read(ADCSTAT,5))!=0);                          // Aguarda estado IDLE
    old = bit_read_interval(FIFO1COUNT,6,0);                  // Captura número antigo de palavras armazenadas no FIFO
    bit_set(STEPENABLE,step);                                 // Reabilita canal
    new = old;                                                // Captura número novo de palavras armazenadas no FIFO
    do new = bit_read_interval(FIFO1COUNT,6,0);               // Aguarda uma nova palavra ser efetivamente escrita no FIFO
    while(new == old);
    result[i] = bit_read_interval(FIFO1DATA,11,0);            // Captura o resultado da conversão
    stepID = bit_read_interval(FIFO1DATA,19,16)+1;            // Captura stepID
    if(stepID != step)                                        // Validação da medida
        {
        result[n] = ';';                                      // Tag de falha
        break;
        }
    else ok++;
    }
if(ok==n) result[n] = stepID;                                 // Tag de sucesso
return(result);
}
/********************************************/

// Função:    config
// Descrição: Configura periféricos
// Entrada:   -
// Saída:     -
/*********************************************************************************/
void config(void)
{
CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;             // Habilita a master port OCP (On Chip Peripheral)
config_ADC();                                   // Configura o módulo ADC
}
/********************************************/

void main(void)
{
config();                                           // Configura periféricos
int* chan1 = (int*)(malloc((CH1+1)*sizeof(int)));   // Aloca vetor para receber as amostras do canal 1
led_write(1);                                       // Garante LED vermelho ligado inicialmente
memory(CONTROL_BUFFER,'w',LOCK,'k');
memory(ARM_BUFFER,'w',0xEE,'k');
while(1)
    {
    if(button_state())
        {
        debounce();
        led_toggle();
        chan1 = capture(1,CH1);
        memory(CONTROL_BUFFER,'w',ARM_READ,'k');
        memory(ARM_BUFFER,'w',chan1[2],'k');
        }
    }
}
