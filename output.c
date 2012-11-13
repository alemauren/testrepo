#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_STR_LENGTH 256
#define S_EMPTY 0x00
#define S_FULL 0xFF
#define YEAR_SHIFT 9
#define MONTH_SHIFT 5
#define MONTH_MASK 0x000F
#define DAY_MASK 0x001F
#define BE_to_LE(input) (((char*)input)[0] << 8) + ((char*)input)[1]

typedef unsigned short Ushort;

Ushort local_time_to_date(struct tm*);
int print_Cstring(char*, FILE*);

int main(int argc, char *argv[]){

	FILE *file;
	char wd[MAX_STR_LENGTH], default_string[MAX_STR_LENGTH], *long_string;
	Ushort cant_campos, cant_registros, campos_completos, campo, largo_str, date;
	time_t rawtime;
	struct tm *timeinfo;
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	getcwd(wd, MAX_STR_LENGTH);

	printf("---[CONTENIDO DEL ARCHIVO]------------------------\n");

	file = fopen(argv[1], "r+");

	fgets(default_string, 3, file);
	printf("Nro. de Serie: %d\n", BE_to_LE(default_string));
	printf("Full Filename: %s/%s\n", wd, argv[1]);
	fgets(default_string, 2, file);
	if (default_string[0] == S_EMPTY) ;
	else if (default_string[0] == S_FULL){
		fgets(default_string, 3, file);
		largo_str = BE_to_LE(default_string);
		long_string = malloc(largo_str + 1);
		fgets(long_string, largo_str + 1, file);
		free(long_string);
	}else{
		largo_str = (Ushort) *default_string;
		fgets(default_string, largo_str + 1, file);
	}

date = local_time_to_date(timeinfo);	
	char *blah = (char *) &date;
	fputc((unsigned int) blah[1], file);
	fputc((unsigned int) blah[0], file);
	printf("Fecha de Modificacion: %4d/%02d/%02d\n", timeinfo->tm_year + 1900, timeinfo->tm_mon, timeinfo->tm_mday);

	fgets(default_string, 3, file);
	cant_campos = BE_to_LE(default_string);
	printf("Cantidad de Campos Customizados: %d\n", cant_campos);

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

	for(int l = 0; l < cant_campos; l++){
		free(descripciones[l]);
	}
	free(descripciones);
	
	fclose(file);

	return 0;
}	

Ushort local_time_to_date(struct tm *timeinfo){
	Ushort date;
	if (timeinfo->tm_year >= 100) date = timeinfo->tm_year - 100;
	else date = 199 - timeinfo->tm_year; 
	date <<= 4;
	date += timeinfo->tm_mon;
	date <<= 5;
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
