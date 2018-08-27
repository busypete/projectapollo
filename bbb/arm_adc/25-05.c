#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define ADC_TSC         0x44E0D000
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

/*********************************************************************************/
// Função:    memory
// Descrição: Acesso (leitura e escrita) a registros do AM335x utilizando o driver
//            /dev/mem. Os argumentos 'content' e 'logic' não possuem nenhum efeito
//            caso acess = 'r'.
// Entrada:   unsigned int  ADDR    - endereço de memória de interesse
//            char acess            - 'r': leitura, 'w': escrita
//            unsigned int  content - conteúdo a ser escrito no endereço ADDR
//            char logic            - 'k': *ADDR = content, 'o': *ADDR |= content
//                                  - 'a': *ADDR = *ADDR & content
// Saída:     unsigned int          - resultado da leitura
/*********************************************************************************/
unsigned int memory(unsigned int ADDR, char acess, unsigned int content, char logic)
{
int fd;
void *map_base, *virt_addr;
unsigned int read_result;
off_t target;

target = ADDR;

if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
fflush(stdout);

map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
if(map_base == (void *) -1) FATAL;
fflush(stdout);

virt_addr = map_base + (target & MAP_MASK);
read_result = *((unsigned long *) virt_addr);
//printf("Antes:  0x%X",read_result);

if(acess == 'r') read_result = *((unsigned long *) virt_addr);
else if(acess == 'w')
    {
    if(logic == 'k') *((unsigned long *) virt_addr) = content;
    else if(logic == 'o') *((unsigned long *) virt_addr) |= content;
    else if(logic == 'a') *((unsigned long *) virt_addr) &= content;
    read_result = *((unsigned long *) virt_addr);
    }

//printf("\nDepois: 0x%X\n",read_result);

fflush(stdout);
if(munmap(map_base, MAP_SIZE) == -1) FATAL;
close(fd);

return(read_result);
}
/********************************************/

/*********************************************************************************/
// Função:    bit_clear
// Descrição: Limpa um bit usando a função "memory"
// Entrada:   unsigned int  ADDR - endereço de memória de interesse
//            unsigned int  x    - bit de interesse
// Saída:     -
/*********************************************************************************/
void bit_clear(unsigned int ADDR, unsigned int x)
{
memory(ADDR,'w',(~(1<<x)),'a');
}
/********************************************/

/*********************************************************************************/
// Função:    bit_set
// Descrição: Seta um bit usando a função "memory"
// Entrada:   unsigned int  ADDR - endereço de memória de interesse
//            unsigned int  x    - bit de interesse
// Saída:     -
/*********************************************************************************/
void bit_set(unsigned int ADDR, unsigned int x)
{
memory(ADDR,'w',(1<<x),'o');
}
/********************************************/

/*********************************************************************************/
// Função:    bit_write_interval
// Descrição: Escreve num intervalo de bits usando a função "memory"
// Entrada:   unsigned int  ADDR    - endereço de memória de interesse
//            unsigned int  MSB     - bit mais significativo
//            unsigned int  LSB     - bit menos significativo
//            unsigned int  content - conteúdo a ser escrito
// Saída:     -
/*********************************************************************************/
void bit_write_interval(unsigned int ADDR, unsigned int MSB, unsigned int LSB, unsigned int content)
{
unsigned int i=0,j=0,before=0,after=0,and=0,or=0;

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
// Entrada:   unsigned int  ADDR    - endereço de memória de interesse
//            unsigned int  x       - bit de interesse
// Saída:     unsgined int          - nivel lógico do bit                                    -
/*********************************************************************************/
unsigned int bit_read(unsigned int ADDR, unsigned int x)
{
return(((memory(ADDR,'r',0x0,'o'))&(1<<x))>>x);
}
/*********************************************************************************/

/*********************************************************************************/
// Função:    bit_read_interval
// Descrição: Lê um intervalo de bits usando a função "memory"
// Entrada:   unsigned int  ADDR    - endereço de memória de interesse
//            unsigned int  MSB     - bit mais significativo
//            unsigned int  LSB     - bit menos significativo
//            unsigned int  content - conteúdo a ser escrito
// Saída:     unsgined int          - resultado da leitura                                   -
/*********************************************************************************/
unsigned int bit_read_interval(unsigned int ADDR, unsigned int MSB, unsigned int LSB)
{
unsigned int i=0,j=0,before=0,after=0;

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

// Configura o ADC
/********************************************/
void config_ADC(void)
{
// ADC_CTRL
bit_clear(ADC_CTRL,0);                        // Desabilita o módulo ADC
bit_clear(ADC_CTRL,9);                        // Eventos ativados por software são prioritários aos eventos ativados por hardware
bit_clear(ADC_CTRL,8);                        // Mapeia evento de hardware para o PEN touch irq
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
bit_write_interval(STEPDELAY1,17,0,0x3FFFF);  // Máximo open delay
bit_write_interval(STEPDELAY1,31,24,0x7);     // Pequeno sample delay

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
bit_write_interval(STEPDELAY3,17,0,0x3FFFF);  // Máximo open delay
bit_write_interval(STEPDELAY3,31,24,0x7);     // Pequeno sample delay

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
bit_set(ADC_CTRL,0);                          // Habilita o módulo ADC
}
/********************************************/

// Lê o ADC
/********************************************/
void read_ADC(void)
{
printf("\nSTEPCONFIG3\n");
memory(STEPCONFIG3,'r',0xF7FFFFFF,'a'); // Desabilita out-of-range check (compara o valor em FIFOx com os limites definidos em ADCRANGE)

printf("\nSTEPDELAY3\n");
memory(STEPDELAY3,'r',0x10000000,'o');  // Define sample delay como 10 ciclos de clock do ADC

printf("\nDMAENABLE_CLR\n");
memory(DMAENABLE_CLR,'r',0x3,'o');      // Desabilita recurso de DMA dos registros FIFO0 e FIFO1

printf("\nIRQWAKEUP\n");
memory(IRQWAKEUP,'r',0xFFFFFFFE,'a');   // Desabilita wakeup generation of hw pen event

printf("\nIRQSTATUS\n");
memory(IRQSTATUS,'r',0x7FF,'o');        // Limpa os flags pendentes de todas as interrupções do módulo ADC

printf("\nIRQENABLE_CLR\n");
memory(IRQENABLE_CLR,'r',0x7FF,'o');    // Desabilita todas as interrupções do módulo ADC

printf("\nADC_CTRL\n");
memory(ADC_CTRL,'r',0xFFFFFFEF,'a');    // Garante que o AFE está energizado

printf("\nSTEPENABLE\n");
memory(STEPENABLE,'r',0x2,'o');         // Habilita o STEP1

printf("\nFIFO0DATA\n");
memory(FIFO0DATA,'r',0x0,'o');          // Leitura da conversão AD

printf("\nFIFO1DATA\n");
memory(FIFO1DATA,'r',0x0,'o');          // Leitura da conversão AD

printf("\nFIFO1COUNT\n");
memory(FIFO1COUNT,'r',0x0,'o');          // Leitura da conversão AD
}
/********************************************/

// Impressão de resultados da conversão AD
/********************************************/
void imprime(void)
{
unsigned int i = 0, stepID[]={0,0},resultado[] = {5000,5000};
float tensao[] = {5,5};
for(i=0;i<25;i++)
    {
    // STEP1
    bit_write_interval(STEPENABLE,16,0,0x0);                      // Desabilita todos os STEPS
    bit_set(ADC_CTRL,0);                                          // Habilita o módulo ADC
    if(bit_read(STEPENABLE,1)==0) bit_set(STEPENABLE,1);          // Habilita STEP1
    while(bit_read(ADCSTAT,5)==0);                                // Aguarda início da conversão
    while(bit_read(IRQSTATUS_RAW,1)==0);                          // Aguarda a FSM terminar a sequência
    bit_write_interval(STEPENABLE,16,0,0x0);                      // Desabilita todos os STEPS
    resultado[0] = bit_read_interval(FIFO1DATA,11,0);             // Captura o resultado da conversão
    stepID[0] = bit_read_interval(FIFO1DATA,19,16)+1;             // Captura stepID
    bit_clear(ADC_CTRL,0);                                        // Desabilita o módulo ADC

    // Impressão dos resultados
    tensao[0] = (((float)(resultado[0]))/(4096))*1.8;
    printf("\nStep ID: %d \nCanal AIN6:  %.3f V\n",stepID[0],tensao[0]);

    }
bit_clear(ADC_CTRL,0);                        // Desabilita o módulo ADC
printf("\n");
}
/********************************************/

int main(void)
{
config_ADC();
//read_ADC();
imprime();
//unsigned int resultado = bit_read_interval(FIFO1DATA+19,11,0);
//float tensao = (float)(((resultado*1.0)/(4096))*(1.8));
//printf("\nLeitura: %.3f\n",tensao);
return 0;
}

