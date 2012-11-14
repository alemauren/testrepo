#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_STR_LENGTH 256
#define S_EMPTY 0x00
#define S_FULL 0xFF
#define DAY_SHIFT 5
#define MONTH_SHIFT 4
/*El sistema devuelve el año como la diferencia entre el a\~no calendario y 1900*/
#define ANOS_90 1999-1900+100
#define SYSTEM_YEAR_COMPARE 2000-1900
/*Los datos est\'an guardados en formato Big Endian en el achivo, este hack
permite reacomodar los bytes de un dato de 2 bytes (ingresado como un string)
a un dato de 2 bytes (usado conjunto Ustring) expresado en formato Little Endian
NOTA: para mejorar la portabilidad, deber\'ia configurarse seg\'un el entorno de ejecuci\'on:
un macro para maquinas LE y uno (vacio) para maquinas BE.*/
#define BE_to_LE(input) (((char*)input)[0] << 8) + ((char*)input)[1]

/*Renombre conveniente*/
typedef unsigned short Ushort;

/*Forward declarations*/
Ushort local_time_to_date(struct tm*);
int print_Cstring(char*, FILE*);

int main(int argc, char *argv[]){

	FILE *file;
	char working_dir[MAX_STR_LENGTH], default_string[MAX_STR_LENGTH], *string_larga;
	Ushort cant_campos, cant_registros, campos_completos, campo, largo_str, date;
	time_t rawtime;
	struct tm *timeinfo;

	/*Si se pasaron m\'as argumentos de los que se piden, abortar.*/
	if (argc != 2) {
		printf("Ingrese exactamente el path de un (1) archivo como argumento.\n");
		exit(1);
	}
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	getcwd(working_dir, MAX_STR_LENGTH);

	/*Si se produce un error al abrir el archivo, abortar.*/
	if(!(file = fopen(argv[1], "r+"))){
		perror("Error al abrir el archivo");
		exit(1);
	}

	printf("---[CONTENIDO DEL ARCHIVO]------------------------\n");

	fgets(default_string, 3, file);
	printf("Nro. de Serie: %d\n", BE_to_LE(default_string));
	/*NOTA: el filepath provisto en el archivo no se usa ni se updatea*/
	printf("Full Filename: %s/%s\n", working_dir, argv[1]);


	/*Avanzo el fileptr hasta el final del Cstring que representa el filepath*/
	fgets(default_string, 2, file);
	if (default_string[0] == S_EMPTY) ;
	else if (default_string[0] == S_FULL){
		fgets(default_string, 3, file);
		largo_str = BE_to_LE(default_string);
		string_larga = malloc(largo_str + 1);
		fgets(string_larga, largo_str + 1, file);
		free(string_larga);
	}else{
		largo_str = (Ushort) *default_string;
		fgets(default_string, largo_str + 1, file);
	}

	/*Update de la fecha con el tiempo del sistema*/
	date = local_time_to_date(timeinfo);	
	unsigned char *blah = (unsigned char*) &date;
	fputc((unsigned int) blah[1], file);
	fputc((unsigned int) blah[0], file);
	printf("Fecha de Modificacion: %4d/%02d/%02d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon, timeinfo->tm_mday);

	fgets(default_string, 3, file);
	cant_campos = BE_to_LE(default_string);
	printf("Cantidad de Campos Customizados: %d\n", cant_campos);


	/*Las descripciones de los campos se guardan en un char**.
	NOTA: si los codigos no fueren contiguos se generaria mal funcionamiento,
	como FIX se puede implementar un diccionario sobre arreglos ordenados (pendiente)*/

	char **descripciones = malloc( sizeof(char*) * cant_campos);

	for (int i = 0; i < cant_campos; i++){
		fgets(default_string, 3, file);
		campo = BE_to_LE(default_string);
		printf("Campo [codigo: %d, ", campo);
		fgets(default_string, 2, file);
		largo_str = (Ushort) *default_string;
		descripciones[i] = malloc(largo_str + 1);
		fgets(descripciones[i], largo_str + 1, file);
		printf("descripcion: %s]\n", descripciones[i]);
	}

	printf("--------------------------\n");

	fgets(default_string, 3, file);
	cant_registros = BE_to_LE(default_string);
	for(int j = 0; j < cant_registros; j++){
		fgets(default_string, 3, file);
		campos_completos = BE_to_LE(default_string);
			for(int k = 0; k < campos_completos; k++){
				fgets(default_string, 3, file);
				printf("%s: ", descripciones[BE_to_LE(default_string) - 1]);
				print_Cstring(default_string, file);
			}
		printf("--------------------------\n");
	}

	/*Se libera la memoria pedida para el arreglo de descripciones*/
	for(int l = 0; l < cant_campos; l++){
		free(descripciones[l]);
	}
	free(descripciones);
	
	/*Si se incurre en un error al cerrar el archivo, se aborta*/
	if (fclose(file) == EOF){
		perror("Error al cerrar el archivo");
		exit(1);
	}

	return 0;
}	

Ushort local_time_to_date(struct tm *timeinfo){
	Ushort date;
	if (timeinfo->tm_year >= SYSTEM_YEAR_COMPARE) date = timeinfo->tm_year - SYSTEM_YEAR_COMPARE;
	else date = ANOS_90 - timeinfo->tm_year; 
	date <<= MONTH_SHIFT;
	date += timeinfo->tm_mon;
	date <<= DAY_SHIFT;
	date += timeinfo->tm_mday;
	return date;
}

int print_Cstring(char *input, FILE *stream){

	Ushort length;

	fgets(input, 2, stream);
	if (input[0] == S_EMPTY){
		printf("\n");
	}else if (input[0] == S_FULL){
		fgets(input, 3, stream);
		length = BE_to_LE(input);
		char *long_string = malloc(length + 1);
		fgets(long_string, length + 1, stream);
		printf("%s\n", long_string);
		free(long_string);
	}else{
		length = (Ushort) *input;
		fgets(input, length + 1, stream);
		printf("%s\n", input);
	}
	return 0;
}
