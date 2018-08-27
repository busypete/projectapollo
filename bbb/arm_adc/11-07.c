// Conversão AD sendo escrita no frame da PRU
// PRU manipulando esses dados e escrevendo de volta para o ARM
// Próximo: sincronizar essas ações

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

#define PRU_ICSS        0x4A300000
#define PRU_BUFFER      PRU_ICSS + 0x00010000               // Frame de escrita do ponto de vista do ARM
#define ARM_BUFFER      PRU_ICSS + 0x00012000               // Frame de leitura do ponto de vista do ARM
#define CH1             8                                   // Número de amostras a serem feitas do canal 1
#define CH3             2                                   // Número de amostras a serem feitas do canal 3

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
bit_write_interval(STEPDELAY1,17,0,0x0);      // Máximo open delay
bit_write_interval(STEPDELAY1,31,24,0xFF);    // Máximo sample delay

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
bit_write_interval(STEPDELAY3,31,24,0xFF);    // Mínimo sample delay

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
unsigned int i = 0, stepID = 0, pointer = 0, old = 0, new = 0, m = 2*n, ok = 0;
int* result = (int*)(malloc((n+1)*sizeof(int)));                   // Aloca vetor de resultado

// Executa 'n' conversões single-shot do step desejado
for(i=0;i<m;i++)
    {
    while((bit_read(ADCSTAT,5))!=0);                              // Aguarda estado IDLE
    old = bit_read_interval(FIFO1COUNT,6,0);                      // Aguarda uma nova palavra ser efetivamente escrita no FIFO
    bit_set(STEPENABLE,step);                                     // Reabilitacanal
    new = old;
    do new = bit_read_interval(FIFO1COUNT,6,0);
    while(new == old);
    }

// Percorre o FIFO capturando uma metade do espelho
for(i=0;i<m;i++)
    {
    if(i<(m/2)) pointer = FIFO1DATA + 0xFC - 4*(n/2) - 4*i;       // Atualiza o ponteiro do FIFO
    else break;
    stepID = bit_read_interval(pointer,19,16)+1;                  // Captura stepID
    if(stepID != step)                                            // Validação da medida
        {
        result[n] = ';';                                          // StepID de falha
        break;
        }
    else ok++;
    result[i] = bit_read_interval(pointer,11,0);                  // Captura o resultado da conversão
    }

if(ok==n) result[n] = stepID;                                     // StepID de sucesso
return(result);
}
/********************************************/

/*********************************************************************************/
// Função:    frame_write
// Descrição: Escreve um vetor de inteiros na memória. A primeira posição do frame
//            recebe o tamanho do frame
// Entrada:   int*       vector     - vetor a ser impresso
//            unsigned int length   - tamanho do vetor a ser impresso
//            unsigned int ADDR     - endereço a partir do qual o vetor será impresso
// Saída:     -
/*********************************************************************************/
void frame_write(int* vector, unsigned int length, unsigned int ADDR)
{
unsigned int i;                                                 // Índice de iteração
memory(ADDR,'w',length,'k');                                    // Envia o tamanho do vetor na primeira posição do frame
for(i=1;i<(length+1);i++) memory(ADDR+sizeof(int)*i,'w',vector[i-1],'k');     // Escreve cada posição do vetor no frame buffer da PRU
}
/********************************************/

// Função:    frame_read
// Descrição: Escreve um vetor de inteiros na memória
// Entrada:   unsigned int ADDR     - endereço a partir do qual o vetor será lido
// Saída:     int*                  - vetor contendo a leitura do frame
/*********************************************************************************/
int* frame_read(unsigned int ADDR)
{
int i, length;                                           // Índice de iteração e tamanho do frame
length = memory(ADDR,'r',0x00,'o');                      // Captura o tamanho do frame
int* readback = (int*)(malloc(length*sizeof(int)));      // Aloca o vetor com o resultado da leitura
for(i=1;i<(length+1);i++) readback[i-1] = memory(ADDR+sizeof(int)*i,'r',0x00,'o'); // Preenche o vetor com a leitura do frame
return(readback);
}
/********************************************/

// Função:    compare_vector
// Descrição: Compara dois vetores. Necessariamente, eles devem possuir a mesma dimensão
// Entrada:   int* vector1          - operando 1
//            int* vector1          - operando 2
//            unsigned int length   - tamanho dos vetores
// Saída:     0                     - vetores diferentes
// Saída:     1                     - vetores iguais
/*********************************************************************************/
int compare_vector(int* vector1, int* vector2, unsigned int length)
{
int i,match = 0;                                // Índice de iteração e número de matches
for(i=0;i<length;i++)                           // Aplica a restrição de igual a cada posição dos vetores
    {
    if(vector1[i]==vector2[i]) match++;         // Incrementa o acumulador de matches
    else break;                                 // Quebra o laço quando encontrar qualquer diferença
    }
if(match==length) return(1);                    // Vetores idênticos
else return(0);
}
/********************************************/

/*********************************************************************************/
// Função:    print_vector
// Descrição: Imprime um vetor de inteiros
// Entrada:   int*       vector     - vetor a ser impresso
//            unsigned int length   - tamanho do vetor a ser impresso
// Saída:     -
/*********************************************************************************/
void print_vector(int* vector, unsigned int length)
{
unsigned int i;
for(i=0;i<length;i++) printf("\nPosição %d: %d",i,vector[i]);
printf("\n\n");
}
/********************************************/

/*********************************************************************************/
// Função:    print_channel
// Descrição: Imprime as tensões e o stepID associados a um vetor de amostras
//            Avalia se o vetor é válido, checando a sua última posição
// Entrada:   int*       vector     - vetor de amostras a ser impresso
//            unsigned int length   - tamanho do vetor de amostras a ser impresso
// Saída:     -
/*********************************************************************************/
void print_channel(int* vector, unsigned int length)
{
unsigned int i;
float tensao;
for(i=0;i<length;i++)
    {
    tensao = (float)(((vector[i]*1.0)/(4096.0))*(1.8));
    printf("\nStepID: %d ; Leitura: %d ; Tensão: %.2f V",vector[length],vector[i],tensao);
    }
if(vector[length]==';') printf("\nAmostra inválida\n");
else printf("\nAmostra válida\n");
printf("\n\n");
}
/********************************************/

int main(void)
{

config_ADC();                                                       // Configura o módulo ADC
int* chan1 = capture(1,CH1);                                        // Amostra canal 1
int* chan3 = capture(3,CH3);                                        // Amostra canal 3
bit_clear(ADC_CTRL,0);                                              // Desabilita o módulo ADC

printf("\nMedição:\n");
print_vector(chan1,CH1);                                            // Imprime amostras do canal 1
print_vector(chan3,CH3);                                            // Imprime amostras do canal 3

int* tx1 = (int*)(malloc((CH1+1)*sizeof(int)));                     // Aloca vetor para o resultado do readback do canal 1
frame_write(chan1,CH1+1,PRU_BUFFER);                                // Escreve os resultados do canal 1 no frame de escrita
tx1 = frame_read(PRU_BUFFER);                                       // Readback do frame de escrita, relativo ao canal 1

int* tx3 = (int*)(malloc((CH3+1)*sizeof(int)));                     // Aloca vetor para o resultado do readback do canal 3
frame_write(chan3,CH3+1,PRU_BUFFER);                                // Escreve os resultados do canal 3 no frame de escrita
tx3 = frame_read(PRU_BUFFER);                                       // Readback do frame de escrita, relativo ao canal 3

printf("\nEnviado para a PRU:\n");
print_vector(tx1,CH1);                                              // Imprime readback do canal 1
print_vector(tx3,CH3);                                              // Imprime readback do canal 3

if(compare_vector(tx1,chan1,CH1+1)) printf("\nCanal 1 válido");     // Validação do frame de escrita relativo ao canal 1
else printf("\nCanal 1 inválido\n");
if(compare_vector(tx3,chan3,CH3+1)) printf("\nCanal 3 válido\n");   // Validação do frame de escrita relativo ao canal 3
else printf("\nCanal 3 inválido\n");

printf("\nRecebido da PRU:\n");
int* rx3 = (int*)(malloc((CH3+1)*sizeof(int)));                     // Aloca vetor para o resultado do readback do canal 3
rx3 = frame_read(ARM_BUFFER);                                       // Readback do frame de leitura, relativo ao canal 3
print_vector(rx3,CH3);                                              // Imprime readback do canal 3

if(compare_vector(rx3,chan3,CH3+1)) printf("\nCanal 3 válido\n");   // Validação do frame de leitura, relativo ao canal 3
else printf("\nCanal 3 inválido\n");

return 0;
}







