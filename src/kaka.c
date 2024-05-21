#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARCHIVO_OPERACIONES "operaciones.txt"

int     menuPrincipal();
void    seleccionarTablas();
void    inicializarTablas();
void    imprimirDatos();
int     generarOperacionAleatoria();
int     mostrarJugada();
void    escribir_operaciones();

int opcion;
int tablas[10] = {0,0,0,0,0,0,0,0,0,0};

typedef struct {
    int a;
    int b;
    int c;
} Operacion;

Operacion *datos;
int totalOperaciones;

int puntuacion = 0;



int main()
{
    opcion = menuPrincipal();
    seleccionarTablas();
    inicializarTablas(tablas, 10);
    imprimirDatos();

    while (1)
    {
        int posicion = generarOperacionAleatoria();
        if (mostrarJugada(posicion))
        {
            eliminarElemento(posicion);
            puntuacion++;
        } else {
            escribir_operaciones();
        }

        imprimirDatos();
    }

    return 0;
}

int menuPrincipal() {
    int opcion;
    printf("Tablas de multiplicar\n");
    printf("1. a X b = ?\n");
    printf("2. ? X b = c\n");
    printf("3. a X ? = c\n");
    printf("4. ? X ? = ?\n");
    printf("Ingrese su opcion: ");
    scanf("%d", &opcion);
    return opcion;
}

void seleccionarTablas(){

    char buffer[12] = {};

    printf("Tablas a repasar 1 al 9 (* para todas): ");
    scanf("%s[^\n]", buffer); 

    // Verificar si el carácter '*' está presente
    char posicion = strchr(buffer, '*');

    if (posicion != NULL) {
        for (int i = 0; i < 10; i++)
        {
            tablas[i] = i + 1;
        }
    } else {
        for (int i = 0; i < sizeof(buffer); i++)
        {
            int numero = buffer[i] - '0';
            if (numero >= 1 && numero <= 10) {
                tablas[i] = numero;
            }
        }
    }
}

void inicializarTablas(int tablas[], int tamanio){

    int contador = 0;
    for (int i = 0; i < tamanio; i++)
    {
        if (tablas[i] != 0)
        {
            contador++;
        }else{
            break;
        }
    }

    totalOperaciones = 10 * contador;
    datos = (Operacion *) malloc(totalOperaciones * sizeof(Operacion));
    
    for (int i = 0; i < contador; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            datos[j+(i*10)].a = tablas[i];
            datos[j+(i*10)].b = j + 1;
            datos[j+(i*10)].c = tablas[i] * (j + 1);
        }     
    }

}

void imprimirDatos(){
    printf("Operaciones generadas:\n");
    for (int i = 0; i < totalOperaciones; i++) {
        printf("%d x %d = %d\n", datos[i].a, datos[i].b, datos[i].c);
    }
}

int generarOperacionAleatoria(){

    srand(time(NULL));

    int random = (rand() % totalOperaciones);

    printf("aleatorio %d\n", random);

    return random;
}


void eliminarElemento(int posicion) {
    // Verificar si la posición es válida
    if (posicion < 0 || posicion >= totalOperaciones) {
        printf("Posición inválida\n");
        return;
    }

    // Mover los elementos posteriores a la posición una posición hacia la izquierda
    for (int i = posicion; i < totalOperaciones - 1; i++) {
        datos[i] = datos[i + 1];
    }

    // Reducir el tamaño del array
    totalOperaciones--;
}


int mostrarJugada(int posicion){

    int respuesta;
    Operacion o = datos[posicion];

    srand(time(NULL));

    int tipoJugada = (opcion != 4) ? opcion : (rand()%3 +1);

    printf("Tipo jugada %d:\n", tipoJugada);
    int acierto = 0;

    switch (tipoJugada)
    {
        case 1:
                printf("%d X %d = ?\n", o.a, o.b);
                scanf("%d", &respuesta);
                acierto = (o.c == respuesta) ? 1 : 0;
            break;
        case 2:
                printf("? X %d = %d\n", o.b, o.c);
                scanf("%d", &respuesta);
                acierto = (o.a == respuesta) ? 1 : 0;
            break;    
        case 3:
                printf("%d X ? = %d\n", o.a, o.c);
                scanf("%d", &respuesta);
                acierto = (o.b == respuesta) ? 1 : 0;
            break;
        default:
            break;
    }

    return acierto;
}



// Función para escribir las operaciones en un archivo
void escribir_operaciones() {
    FILE *archivo = fopen(ARCHIVO_OPERACIONES, "w");
    if (archivo == NULL) {
        printf("Error al abrir el archivo para escribir.\n");
        return;
    }

    for (int i = 0; i < totalOperaciones; i++) {
        fprintf(archivo, "%d %d %d\n", datos[i].a, datos[i].b, datos[i].c);
    }

    fclose(archivo);
}

// Función para leer las operaciones desde un archivo
void leer_operaciones(Operacion *operaciones) {
    FILE *archivo = fopen(ARCHIVO_OPERACIONES, "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo para leer, generando nuevas operaciones...\n");
        return;
    }

    totalOperaciones = 0;
    while (fscanf(archivo, "%d %d %d", datos[totalOperaciones].a, datos[totalOperaciones].b, datos[totalOperaciones].c) == 3) {
        totalOperaciones++;
        //if (*tamaño >= TAMAÑO_MAX_OPERACIONES) {
        //    printf("Se alcanzó el límite máximo de operaciones.\n");
        //    break;
        //}
    }

    fclose(archivo);
}