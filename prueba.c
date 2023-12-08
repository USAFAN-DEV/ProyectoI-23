#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define TAMBLOQUE 5
#define TAM_LINEA 16 
#define NUM_FILAS 8

//TODO

typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

char *LeelineaDinamicaFichero(FILE *fd);
char *HexToBin(char* hexdec);
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(long unsigned int addr, int *ETQ, int*palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ,int linea, int bloque);

int globaltime=0;
int numfallos=0;

int main(){
    FILE *fd;
    int contador_caracteres_RAM=0;
    unsigned int linea;
    unsigned int bloque;
    unsigned int palabra;
    unsigned int ETQ;
    T_CACHE_LINE tbl[NUM_FILAS];
    unsigned char Simul_RAM[4096]; 
    char texto[100];
    
    LimpiarCACHE(tbl);

    fd=fopen("CONTENTS_RAM.bin","r");
    if(fd==NULL){
        printf("No existe el fichero para que pueda ser leido\n");
        return -1;
    }
    else{

        char caracter;

        printf("Existe el fichero\n");
        if((caracter = fgetc(fd))==EOF){
            printf("El fichero esta vacio\n");
        }
        else{
            Simul_RAM[contador_caracteres_RAM]=caracter;
            contador_caracteres_RAM++;
            while((caracter = fgetc(fd))!=EOF){
                Simul_RAM[contador_caracteres_RAM]=caracter;
                contador_caracteres_RAM++;
            }
        }
    }
    fclose(fd);

    fd=fopen("accesos_memoria.txt","r");
     if(fd==NULL){
        printf("No existe el fichero para que pueda ser leido\n");
    }
    else{
        char* addrStr;
        long unsigned int addr;

        for(int i=0;i<14;i++){
            addrStr = LeelineaDinamicaFichero(fd);
            addr = strtol(addrStr, NULL, 16);
            ParsearDireccion(addr,&ETQ,&palabra,&linea,&bloque);

            if((tbl[i].ETQ) == ETQ){//TODO
                printf("T: %d, Acierto de CACHE, ADDR %03X Label %X linea %02X palabra %02X DATO %02X\n\n",globaltime,addr,ETQ,linea,tbl[i].Data);
            }
            else{
                numfallos++;
                printf("T: %d, Fallo de CACHE %d, ADDR %03X Label %X linea %02X palabra %02X bloque %02X\n",globaltime,numfallos,addr,ETQ,linea,palabra,bloque);
                globaltime+=20;
                TratarFallo(tbl,Simul_RAM,ETQ,linea,bloque);
                printf("T: %d, Acierto de CACHE, ADDR %03X Label %X linea %02X palabra %02X DATO %02X\n\n",globaltime,addr,tbl[linea].ETQ,linea,palabra,tbl[linea].Data[0]);
                VolcarCACHE(tbl);
                printf("\ntexto: ");
                for(int i = 0; i < TAM_LINEA; i++){

                    texto[i] = tbl[linea].Data[i];
                    printf("%c", texto[i]);

                }
                printf("\n\n");
            }
            sleep(1);
        }
    }
    fclose(fd);
    
    printf("\n\nAccesos totales: %d, Numero de fallos: %d, tiempo medio: %f", 14, numfallos, (float)globaltime/14);

    fd=fopen("CONTENTS_CACHE.bin","w");
    for(int i=0;i<NUM_FILAS;i++){
            fprintf(fd, "%02X\tDatos: ",tbl[i].ETQ);

            for(int j=0;j<TAM_LINEA;j++){
                fprintf(fd, "%02X\t",tbl[i].Data[j]);
            }
            fprintf(fd, "\n");
        }
    fclose(fd);
    
    return 0;
}

//INTERPRETACION DE LA DIRECCION DE MEMORIA
void ParsearDireccion(long unsigned int addr, int *ETQ, int*palabra, int *linea, int *bloque){
    *palabra=addr & 0b1111;
    *bloque=addr>>4;
    *linea=*bloque & 0b111;
    *ETQ=*bloque>>3; 
}

//LEER UNA LINEA DE UN FICHERO
char *LeelineaDinamicaFichero(FILE *fd){
    int contador=0,numbloque=1;
    char caracter,*linea;
    linea=(char*)malloc(TAMBLOQUE);
    if((caracter=fgetc(fd))==EOF){
        printf("El fichero esta vacio\n");
    }
    else{
        linea[contador]=caracter;
        contador++;
        while((caracter=fgetc(fd))!='\n' && caracter!=EOF){
            if(contador>=numbloque*TAMBLOQUE){
                numbloque++;
                linea=(char*)realloc(linea,TAMBLOQUE*numbloque);
            }
            linea[contador]=caracter;
            contador++;
        }
        linea[contador]='\0';
    }
    return linea;
}

//PASAR DE HEXADECIMAL A BINARIO
char * HexToBin(char* hexdec){ 
    int i=0;
    int contador=2;
    static char addr[14];
    addr[0] = '0';
    addr[1] = 'b';
    while (hexdec[i]!='\0') {
        switch (hexdec[i]) {
        case '0':
            addr[contador]='0';
            addr[contador+1]='0';
            addr[contador+2]='0';
            addr[contador+3]='0';
            contador+=4;
            break;
        case '1':
            addr[contador]='0';
            addr[contador+1]='0';
            addr[contador+2]='0';
            addr[contador+3]='1';
            contador+=4;
            break;
        case '2':
            addr[contador]='0';
            addr[contador+1]='0';
            addr[contador+2]='1';
            addr[contador+3]='0';
            contador+=4;
            break;
        case '3':
             addr[contador]='0';
            addr[contador+1]='0';
            addr[contador+2]='1';
            addr[contador+3]='1';
            contador+=4;
            break;
        case '4':
             addr[contador]='0';
            addr[contador+1]='1';
            addr[contador+2]='0';
            addr[contador+3]='0';
            contador+=4;
            break;
        case '5':
             addr[contador]='0';
            addr[contador+1]='1';
            addr[contador+2]='0';
            addr[contador+3]='1';
            contador+=4;
            break;
        case '6':
             addr[contador]='0';
            addr[contador+1]='1';
            addr[contador+2]='1';
            addr[contador+3]='0';
            contador+=4;
            break;
        case '7':
             addr[contador]='0';
            addr[contador+1]='1';
            addr[contador+2]='1';
            addr[contador+3]='1';
            contador+=4;
            break;
        case '8':
             addr[contador]='1';
            addr[contador+1]='0';
            addr[contador+2]='0';
            addr[contador+3]='0';
            contador+=4;
            break;
        case '9':
             addr[contador]='1';
            addr[contador+1]='0';
            addr[contador+2]='0';
            addr[contador+3]='1';
            contador+=4;
            break;
        case 'A':
             addr[contador]='1';
            addr[contador+1]='0';
            addr[contador+2]='1';
            addr[contador+3]='0';
            contador+=4;
            break;
        case 'B':
             addr[contador]='1';
            addr[contador+1]='0';
            addr[contador+2]='1';
            addr[contador+3]='1';
            contador+=4;
            break;
        case 'C':
             addr[contador]='1';
            addr[contador+1]='1';
            addr[contador+2]='0';
            addr[contador+3]='0';
            contador+=4;
            break;
        case 'D':
             addr[contador]='1';
            addr[contador+1]='1';
            addr[contador+2]='0';
            addr[contador+3]='1';
            contador+=4;
            break;
        case 'E':
             addr[contador]='1';
            addr[contador+1]='1';
            addr[contador+2]='1';
            addr[contador+3]='0';
            contador+=4;
            break;
        case 'F':
             addr[contador]='1';
            addr[contador+1]='1';
            addr[contador+2]='1';
            addr[contador+3]='1';
            contador+=4;
            break;
        }
        i++;
    }
    return addr;
}

//LIMPIAR LA CACHE
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
    for(int i=0;i<NUM_FILAS;i++){
        tbl[i].ETQ=0xFF;
        for(int j=0;j<TAM_LINEA;j++){
            tbl[i].Data[j]=0x23;
        }
    }
}


//TRATAR FALLO DE LA CACHE
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ,int linea, int bloque){
    int cont = 0;
    printf("Cargando el bloque %02X en la linea %02X\n",bloque,linea);
    for(int i=4095-linea*16;i>4095-(linea+1)*16;i--){
        tbl[linea].Data[cont]=MRAM[i];

        cont++;
    }
    
    tbl[linea].ETQ= (unsigned int) ETQ;
}

//IMPRIMIR LA CACHE
void VolcarCACHE(T_CACHE_LINE *tbl){
    for(int i=0;i<NUM_FILAS;i++){
        printf("%02X\tDatos: ",tbl[i].ETQ);

        for(int j=0;j<TAM_LINEA;j++){
            printf("%02X\t",tbl[i].Data[j]);
        }
        printf("\n");
    }
}





