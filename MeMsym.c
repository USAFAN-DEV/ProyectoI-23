#include <stdio.h>
#include <stdlib.h>
#define TAMBLOQUE 5
#define TAM_LINEA 16 
#define NUM_FILAS 8

typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

char *LeelineaDinamicaFichero(FILE *fd);
char *HexToBin(char* hexdec);
char *BinToHex(int bin);
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int*palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ,int linea, int bloque);

int globaltime=0;
int numfallos=0;

int main(){
    FILE *fd;
    int contador_caracteres_RAM=0;
    int linea;
    int bloque;
    int palabra;
    int ETQ;
    T_CACHE_LINE *tbl;
    unsigned char Simul_RAM[4096]; 
    
    fd=fopen("CONTENTS_RAM.bin","r");
    if(fd==NULL){
        printf("No existe el fichero para que pueda ser leido\n");
    }
    else{
        printf("Existe el fichero\n");
        if(fgetc(fd)==EOF){
            printf("El fichero esta vacio\n");
        }
        else{
            Simul_RAM[contador_caracteres_RAM]=fgetc(fd);
            contador_caracteres_RAM++;
            while(fgetc(fd)!=EOF){
                Simul_RAM[contador_caracteres_RAM]=fgetc(fd);
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
        for(int i=0;i<NUM_FILAS;i++){
            ParsearDireccion(atoi(HexToBin(LeelineaDinamicaFichero(fd))),&ETQ,&palabra,&linea,&bloque);
            if((tbl[i].ETQ) == ETQ){
                printf("T: %d, Acierto de CACHE, ADDR %04X Label%X linea %02X palabra %02X DATO %02X",globaltime,LeelineaDinamicaFichero(fd),ETQ,linea,tbl[i].Data);
            }
            else{
                numfallos++;
                printf("T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02Xpalabra %02X bloque %02X",globaltime,numfallos,LeelineaDinamicaFichero(fd),ETQ,linea,palabra,bloque);
                globaltime+=20;
                TratarFallo(tbl,Simul_RAM,ETQ,linea,bloque);
                printf("T: %d, Acierto de CACHE, ADDR %04X Label%X linea %02X palabra %02X DATO %02X",globaltime,LeelineaDinamicaFichero(fd),tbl[linea].ETQ,linea,palabra,tbl[linea].Data[0]);
            }
        }
    }
    
    return 0;
}

//LIMPIAR LA CACHE
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
    for(int i=0;i<NUM_FILAS;i++){
        tbl[i].ETQ=' ';
        for(int j=0;j<TAM_LINEA;j++){
            tbl[i].Data[j]='#';
        }
    }
}

//IMPRIMIR LA CACHE
void VolcarCACHE(T_CACHE_LINE *tbl){
    for(int i=0;i<NUM_FILAS;i++){
        printf("%c",tbl[i].ETQ);
        for(int j=0;j<TAM_LINEA;j++){
            printf("Direcciones:%c",tbl[i].Data[j]);
        }
    }
}

//INTERPRETACION DE LA DIRECCION DE MEMORIA
void ParsearDireccion(unsigned int addr, int *ETQ, int*palabra, int *linea, int *bloque){
    *palabra=addr & 0b1111;
    *bloque=addr>>4;
    *linea=*bloque & 0b111;
    *ETQ=*linea>>3; 
}

//TRATAR FALLO DE LA CACHE
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ,int linea, int bloque){
    printf("Cargando el bloque %02X en la linea %02X",bloque,linea);
    for(int i=4096-linea*16;i>4096-(linea+1)*16;i--){
        tbl[linea].Data[i]=MRAM[i];
    }
    
    tbl[linea].ETQ=ETQ;
}

//PASAR DE BINARIO A HEXADECIMAL
char *BinToHex(int bin){
    int resto;
    int hexadecimalval=0;
    int i=1;
    while (bin != 0){
        resto = bin % 10;
        hexadecimalval = hexadecimalval + resto * i;
        i = i * 2;
        bin = bin / 10;
    }
    return itoa(hexadecimalval);
}

//PASAR DE HEXADECIMAL A BINARIO
char * HexToBin(char* hexdec){ 
    int i=0;
    int contador=2;
    char addr[14];
    addr[0]='0';
    addr[1]='b';
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