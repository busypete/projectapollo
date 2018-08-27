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
// Saída:     -
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

// Configura o ADC
/********************************************/
void config_ADC(void)
{
printf("\nADC_CTRL\n");
memory(ADC_CTRL,'w',0xFFFFFDFF,'a');    // Eventos ativados por software são prioritários aos eventos ativados por hardware
memory(ADC_CTRL,'w',0xFFFFFEFF,'a');    // Mapeia evento de hardware para o PEN touch irq
memory(ADC_CTRL,'w',0xFFFFFF7F,'a');    // Desabilita touchscreen
memory(ADC_CTRL,'w',0xFFFFFF9F,'a');    // Desabilita os modos 4-wire e 5-wire
memory(ADC_CTRL,'w',0xFFFFFFF7,'a');    // Seleciona bias interno para AFE (analog front-end: parte do ADC que efetivamente converte)
memory(ADC_CTRL,'w',0x4,'o');           // Habilita a escrita nos registradores STEPCONFIGx
memory(ADC_CTRL,'w',0x2,'o');           // Habilita a escrita da ID do canal sampleado nos registradores FIFOx

printf("\nSTEPCONFIG1\n");
memory(STEPCONFIG1,'w',0xF7FFFFFF,'a'); // Desabilita out-of-range check (compara o valor em FIFOx com os limites definidos em ADCRANGE)
memory(STEPCONFIG1,'w',0x4000000,'o');  // Define FIFO1 como local do resultado da conversão
memory(STEPCONFIG1,'w',0xFDFFFFFF,'a'); // Define como single-ended
memory(STEPCONFIG1,'w',0xFE7FFFFF,'a'); // Define referência negativa como sendo VSSA
memory(STEPCONFIG1,'w',0xFFB7FFFF,'a'); // Associona o STEP1 com  o canal 7  (AIN6)
memory(STEPCONFIG1,'w',0x300000,'o');
memory(STEPCONFIG1,'w',0xFFFF8FFF,'a'); // Define referência positiva como sendo VDDA_ADC
memory(STEPCONFIG1,'w',0x10,'o');       // Define número de médias igual a 16
memory(STEPCONFIG1,'w',0xFFFFFFF3,'a');
memory(STEPCONFIG1,'w',0x1,'o');        // Conversão contínua e habilitada por software

printf("\nSTEPDELAY1\n");
//memory(STEPDELAY1,'w',0xFF03FFFF,'o');  // Define o máximo open delay e sample delay (para debugar FIFO1COUNT e perceber a escrita de cada conversão)
memory(STEPDELAY1,'w',0x00FC0000,'a');  // Define o sample delay mínimo (com open delay nulo) que a estratégia de polling do FIFO1COUNT ainda funciona
memory(STEPDELAY1,'w',0xDF000000,'o');

printf("\nSTEPCONFIG3\n");
memory(STEPCONFIG3,'w',0xF7FFFFFF,'a'); // Desabilita out-of-range check (compara o valor em FIFOx com os limites definidos em ADCRANGE)
memory(STEPCONFIG3,'w',0x4000000,'o');  // Define FIFO1 como local do resultado da conversão
memory(STEPCONFIG3,'w',0xFDFFFFFF,'a'); // Define como single-ended
memory(STEPCONFIG3,'w',0xFE7FFFFF,'a'); // Define referência negativa como sendo VSSA
memory(STEPCONFIG3,'w',0xFF97FFFF,'a'); // Associona o STEP3 com  o canal 3 (AIN2)
memory(STEPCONFIG3,'w',0x100000,'o');
memory(STEPCONFIG3,'w',0xFFFF8FFF,'a'); // Define referência positiva como sendo VDDA_ADC
memory(STEPCONFIG3,'w',0x10,'o');       // Define número de médias igual a 16
memory(STEPCONFIG3,'w',0xFFFFFFF3,'a');
memory(STEPCONFIG3,'w',0x1,'o');        // Conversão contínua e habilitada por software

printf("\nSTEPDELAY3\n");
//memory(STEPDELAY3,'w',0xFF03FFFF,'o');  // Define o máximo open delay e sample delay (para debugar FIFO1COUNT e perceber a escrita de cada conversão)
memory(STEPDELAY3,'w',0x00FC0000,'a');  // Define o sample delay mínimo (com open delay nulo) que a estratégia de polling do FIFO1COUNT ainda funciona
memory(STEPDELAY3,'w',0xDF000000,'o');

printf("\nDMAENABLE_CLR\n");
memory(DMAENABLE_CLR,'w',0x3,'o');      // Desabilita recurso de DMA dos registros FIFO0 e FIFO1

printf("\nIRQWAKEUP\n");
memory(IRQWAKEUP,'w',0xFFFFFFFE,'a');   // Desabilita wakeup generation of hw pen event

printf("\nIRQSTATUS\n");
memory(IRQSTATUS,'w',0x7FF,'o');        // Limpa os flags pendentes de todas as interrupções do módulo ADC

printf("\nIRQENABLE_CLR\n");
memory(IRQENABLE_CLR,'w',0x7FF,'o');    // Desabilita todas as interrupções do módulo ADC

printf("\nSTEPENABLE\n");
memory(STEPENABLE,'w',0xFFFE0000,'a');  // Desabilita todos os STEPS

printf("\nADC_CTRL\n");
memory(ADC_CTRL,'w',0xFFFFFFEF,'a');    // Garante que o AFE está energizado
memory(ADC_CTRL,'w',0x1,'o');           // Habilita o módulo ADC

printf("\nSTEPENABLE\n");
memory(STEPENABLE,'w',0xA,'o');         // Habilita o STEP1 e STEP3
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
unsigned int i = 0, stepID[2], resultado[] = {5000,5000};
float tensao[] = {5,5};
for(i=0;i<3;i++)
    {
    while(((memory(ADCSTAT,'r',0x0,'o'))&(0x20))==0) printf("\nIDLE...");   // Aguarda a FSM sair do estado IDLE

    // Aguarda uma medida válida ser capturada para o canal AIN6
    do stepID[0] = (((memory(FIFO1DATA,'r',0x0,'o'))&(0xF0000))>>16)+1;
    while(stepID[0] != 1);
    resultado[0] = (memory(FIFO1DATA,'r',0x0,'o'))&(0xFFF);


    // Aguarda uma medida válida ser capturada para o canal AIN3
    do stepID[1] = (((memory(FIFO1DATA,'r',0x0,'o'))&(0xF0000))>>16)+1;
    while(stepID[1] != 3);
    resultado[1] = (memory(FIFO1DATA,'r',0x0,'o'))&(0xFFF);


    // Impressão de resultados
    tensao[0] = (((float)(resultado[0]))/(4096))*1.8;                       // Regra de três para retornar ao domínio da tensão
    printf("\nStep ID: %d \nCanal AIN6:  %.3f V\n",stepID[0], tensao[0]);
    tensao[1] = (((float)(resultado[1]))/(4096))*1.8;                       // Regra de três para retornar ao domínio da tensão
    printf("\nStep ID: %d \nCanal AIN2:  %.3f V\n",stepID[1], tensao[1]);

    }
printf("\n");
}
/********************************************/

int main(void)
{
config_ADC();
//read_ADC();
imprime();
return 0;
}

