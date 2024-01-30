#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TAMBLOQUE 5 //Constante utilizada en la funcion LeelineaDinamicaFichero.
#define TAM_LINEA 16 //Constante. Tamaño de las líneas de la cache / bloques de la memoria RAM
#define NUM_FILAS 8 //Constante. Número de líneas de la caché
#define NUM_DIRECCIONES 14 //Constante. Número de direcciones de memoria

typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE; //Estructura que representa una línea de la cache


//Prototipos de las funciones a desarrollar
char *LeelineaDinamicaFichero(FILE *fd);
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(long unsigned int addr, int *ETQ, int*palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ,int linea, int bloque);

int globaltime=0; //Tiempo que tarda el programa en ejecutarse al completo
int numfallos=0; //Número de fallos al acceder a la caché

int main(){

    FILE *fd; //puntero a un fichero

    //Contadores
    int contador_caracteres_RAM=0;
    int contador_texto_cache = 0;

    //Variables usadas para guardar los valores de los distintos campos de una dirección de memoria
    unsigned int linea;
    unsigned int bloque;
    unsigned int palabra;
    unsigned int ETQ;
    
    T_CACHE_LINE tbl[NUM_FILAS]; //Array de lineas de la cache
    unsigned char Simul_RAM[4096];  //Array de los datos de la RAM
    char texto[100]; //Array del texto leído por la cacher
    
    LimpiarCACHE(tbl);

    fd=fopen("CONTENTS_RAM.bin","r"); //Accedemos al fichero CONTENTS_RAM.bin, que contiene los datos de la RAM

    if(fd==NULL){
        //Si no existe, imprimimos por pantalla un mensaje al usuario y devolvemos un -1. Finaliza el programa
        printf("No existe el fichero para que pueda ser leido\n");
        return -1;
    }
    else{
        //Si existe el archivo: 
        char caracter;
        printf("Existe el fichero\n");

        if((caracter = fgetc(fd))==EOF){
            printf("El fichero esta vacio\n");
        }
        else{
            //Si el fichero tiene carácteres para leer
            Simul_RAM[contador_caracteres_RAM]=caracter; //Metemos el caracter obtenido en el primer fgetc en el array Simul_RAM
            contador_caracteres_RAM++; //incrementamos el contador

            while((caracter = fgetc(fd))!=EOF){ //Hasta que leamos todos los carácteres de la RAM
                Simul_RAM[contador_caracteres_RAM]=caracter; //Metemos el carácter en el array
                contador_caracteres_RAM++; //incrementamos el contador
            }
        }
    }
    fclose(fd); //Cerramos el fichero que estabamos leyendo

    fd=fopen("accesos_memoria.txt","r"); //Abrimos el fichero con las direcciones de memoria
     if(fd==NULL){
         //Si no existe, imprimimos por pantalla un mensaje al usuario y devolvemos un -1. Finaliza el programa
        printf("No existe el fichero para que pueda ser leido\n");
        return -1;
    }
    else{
        //Si existe el archivo:

        char* addrStr; //Variable donde guardaremos la dirección de memoria en formato str
        long unsigned int addr; //Variable donde guardaremos la dirección de memoria en formato long unsigned int. Es necesario que sea long.

        for(int i=0;i<NUM_DIRECCIONES;i++){

            addrStr = LeelineaDinamicaFichero(fd); //Leemos la dirección de memoria de la primera línea
            addr = strtol(addrStr, NULL, 16); //Convertimos la dirección de formato string a long int en base hexadecimal a partir de la función strtol
            ParsearDireccion(addr,&ETQ,&palabra,&linea,&bloque); //Obtenemos los distintos campos de la dirección de memoria

            if((tbl[linea].ETQ) == ETQ){
                //Si la etiqueta de la caché en el número de línea X coincide con la etiqueta de la dirección de memoria, habrá un Acierto de caché
                printf("\nT: %d, Acierto de CACHE, ADDR %03X Label %X linea %02X palabra %02X DATO %02X\n\n",globaltime,addr,ETQ,linea,palabra,tbl[linea].Data[palabra]);
                texto[contador_texto_cache] = tbl[linea].Data[palabra]; //Guardamos el dato leído en la caché en nuestro array texto
                contador_texto_cache++; //Incrementamos el contador de este array

            }
            else{
                //Si la etiqueta de la caché en el número de línea X no coincide con la etiqueta de la dirección de memoria, habrá un Fallo de caché
                numfallos++; //Incrementamos el número de fallos
                printf("\nT: %d, Fallo de CACHE %d, ADDR %03X Label %X linea %02X palabra %02X bloque %02X\n",globaltime,numfallos,addr,ETQ,linea,palabra,bloque);
                globaltime+=20; //Aumentamos el tiempo global en 20
                TratarFallo(tbl,Simul_RAM,ETQ,linea,bloque); //Tratamos el fallo
                printf("\nT: %d, Acierto de CACHE, ADDR %03X Label %X linea %02X palabra %02X DATO %02X\n\n",globaltime,addr,tbl[linea].ETQ,linea,palabra,tbl[linea].Data[palabra]);
                texto[contador_texto_cache] = tbl[linea].Data[palabra]; //Guardamos el dato leído en la caché en nuestro array texto
                contador_texto_cache++; //Incrementamos el contador de este array
                VolcarCACHE(tbl); //Volcamos la cache

            }
            sleep(1); //Sleep de 1 segundo entre cada acceso a la memoria caché
            globaltime+=1; //Aumentamos el tiempo global en 1
            
        }
    }
    fclose(fd); //Cerramos el fichero
    
    printf("\n\nAccesos totales: %d, Numero de fallos: %d, tiempo medio: %.2f", 14, numfallos, (float)globaltime/14);
    printf("\nTexto: %s", texto);

    fd=fopen("CONTENTS_CACHE.bin","w"); //Creamos (si no existe) y nos permitimos escribir en el fichero CONTENTS_CACHE.bin
    for(int i=0;i<NUM_FILAS;i++){ //Escribimos el contenido de la caché en el fichero
            fprintf(fd, "%02X\tDatos: ",tbl[i].ETQ);

            for(int j=TAM_LINEA-1;j>=0;j--){
                fprintf(fd, "%02X\t",tbl[i].Data[j]);
            }
            fprintf(fd, "\n");
        }
    fclose(fd); //Cerramos el fichero
    
    return 0;
}

//LIMPIAR LA CACHE. Inicializa a xFF el campo ETQ y a x23 los campos de datos de la caché
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
    for(int i=0;i<NUM_FILAS;i++){
        tbl[i].ETQ=0xFF;
        for(int j=0;j<TAM_LINEA;j++){
            tbl[i].Data[j]=0x23;
        }
    }
}

//LEER UNA LINEA DE UN FICHERO. Utilizamos una función desarrollada el año pasado para leer las líneas de un fichero
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

//INTERPRETACION DE LA DIRECCION DE MEMORIA
void ParsearDireccion(long unsigned int addr, int *ETQ, int*palabra, int *linea, int *bloque){
    *palabra=addr & 0b1111; //La palabra serán los primeros cuatro bits de la dirección de memoria
    *bloque=addr>>4; //El resto de bits será el bloque = etiqueta + línea
    *linea=*bloque & 0b111; //La línea serán los 3 primeros bits del bloque
    *ETQ=*bloque>>3; //El resto de bits serán la etiqueta
}

//TRATAR FALLO DE LA CACHE
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ,int linea, int bloque){
    int cont = 0;
    printf("Cargando el bloque %02X en la linea %02X\n",bloque,linea); //Mensaje al usuario para indicarle que estamos cargando el bloque en la línea
    for(int i=bloque*TAM_LINEA;i<(bloque+1)*TAM_LINEA;i++){ //for para cargar los 16 bytes del bloque de la RAM en las líneas de la cache
        tbl[linea].Data[cont]=MRAM[i]; //Introducimos en la línea x, columna y el unsigned character i guardado en la RAM
        cont++; //Incrementamos el contador
    }
    
    tbl[linea].ETQ= (unsigned int) ETQ; //Cambiamos la etiqueta de la línea por la etiqueta de la dirección de memoria
}

//IMPRIMIR LA CACHE
void VolcarCACHE(T_CACHE_LINE *tbl){
    for(int i=0;i<NUM_FILAS;i++){
        printf("%02X\tDatos: ",tbl[i].ETQ);

        for(int j=TAM_LINEA-1;j>=0;j--){
            printf("%02X\t",tbl[i].Data[j]);
        }
        printf("\n");
    }
}





