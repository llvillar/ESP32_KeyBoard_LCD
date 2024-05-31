#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>


#define DELAY_LCD_MENU     1000 // 1 SECONDS
#define MOSTRAR_DATOS     0// No mostrar datos


uint8_t state;

// define numero de filas
const uint8_t ROWS = 4;
// define numero de columnas
const uint8_t COLS = 4;
// define la distribucion de teclas
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
// pines correspondientes a las filas
uint8_t colPins[COLS] = { 16, 4, 2, 15 };
// pines correspondientes a las columnas
uint8_t rowPins[ROWS] = { 19, 18, 5, 17 };
// crea objeto con los prametros creados previamente
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows


void    bienvenida();
void    menuPrincipal();
void    seleccionarTablas();
void    inicializarTablas(int tablas[], int tamanio);
void    imprimirDatos();
int     generarOperacionAleatoria();
int     mostrarJugada();
void    guardar_record();
void    leer_record();
void    jugar();
void    inicializarPartida();
void    fin_de_partida(int gana);
//void  menuPrincipal();
//void  iniciarTabla();
//int   leerRespuesta();
int   checkOpcion(char key);
int   leerKeyPad(int longitud, char fin);
int longitudNumero(int numero);
char* cadenaResultado(int longitud);
//void getInput();


int opcion;

int puntuacion;
int record;

typedef struct {
    int a;
    int b;
    int c;
} Operacion;

Operacion *datos;
int totalOperaciones;

char datosKeyPad[20];

int tablas[10] = {0,0,0,0,0,0,0,0,0,0};

int eeAdress = 0;

char *cadena_resultado;

void setup(){
  EEPROM.begin(sizeof(int));

  EEPROM.get(eeAdress, record);

  Serial.begin(11600);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  bienvenida();
  menuPrincipal();
}


void loop(){
  inicializarPartida();
  leerKeyPad(1, ' ');
  char key = datosKeyPad[0];
  if (key){
    lcd.setCursor(15,1);
    lcd.print(key);
    opcion = checkOpcion(key);
  }
  if(opcion > 0 && opcion < 5){
    seleccionarTablas();
    inicializarTablas(tablas, 10);
    imprimirDatos();
    jugar();
    menuPrincipal();
  } else {
    menuPrincipal();
  }
}

void bienvenida(){
  lcd.print("Tablas de");
  lcd.setCursor(0,1);
  lcd.print("multiplicar");
  delay(2000);
}

void inicializarPartida(){
  free(datos);
  for (int i = 0; i < 10; i++)
  {
    tablas[i] = 0;
  }
  puntuacion = 0;
}

void menuPrincipal() {
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Elige opcion: ");
    lcd.setCursor(0,0);
    lcd.print("1. a X b = ?");
    delay(DELAY_LCD_MENU);
    lcd.setCursor(0,0);
    lcd.print("2. ? X b = c");
    delay(DELAY_LCD_MENU);
    lcd.setCursor(0,0);
    lcd.print("3. a X ? = c");
    delay(DELAY_LCD_MENU);
    lcd.setCursor(0,0);
    lcd.print("4. ? X ? = ?");    
}

int checkOpcion(char key){
  int opcion = 0;
  switch (key)
  {
  case '1': case '2': case '3': case '4':
      opcion = key - '0';
    break;
  default:
      opcion = 0; 
    break;
  }
  return opcion;
}

void seleccionarTablas(){
    lcd.clear();
    lcd.print("12345678910#");
    lcd.setCursor(0,1);
    lcd.print("(*) Todas");
    delay(1000);
    int longitud = leerKeyPad(20, '#');

    // Verificar si el carácter '*' está presente
    if (strchr(datosKeyPad, '*') != NULL) {
        for (int i = 0; i < 10; i++)
        {
            tablas[i] = i + 1;
        }
    } else {
        for (int i = 0; i < longitud; i++)
        {
            int numero = datosKeyPad[i] - '0';
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
  if(MOSTRAR_DATOS){
    for (int i = 0; i < totalOperaciones; i++) {
      lcd.clear();
      lcd.printf("%d x %d = %d\n", datos[i].a, datos[i].b, datos[i].c);
      delay(500);
    }
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

    lcd.clear();
      
    int longitud_a = longitudNumero(o.a);
    int longitud_b = longitudNumero(o.b);
    int longitud_c = longitudNumero(o.c);
    
    switch (tipoJugada)
    {
        case 1:
                cadena_resultado = cadenaResultado(longitud_c); 
                lcd.printf("%d X %d = %s", o.a, o.b, cadena_resultado);
                leerKeyPad(longitud_c, ' ');
                respuesta = atoi(datosKeyPad);
                acierto = (o.c == respuesta) ? 1 : 0;
            break;
        case 2:
                cadena_resultado = cadenaResultado(longitud_a); 
                lcd.printf("%s X %d = %d", cadena_resultado, o.b, o.c);
                leerKeyPad(longitud_a, ' ');
                respuesta = atoi(datosKeyPad);
                acierto = (o.a == respuesta) ? 1 : 0;
            break;    
        case 3:
                cadena_resultado = cadenaResultado(longitud_b); 
                lcd.printf("%d X %s = %d\n", o.a, cadena_resultado, o.c);
                leerKeyPad(longitud_b, ' ');
                respuesta = atoi(datosKeyPad);               
                acierto = (o.b == respuesta) ? 1 : 0;
            break;
        default:
            break;
    }

    return acierto;
}


int leerKeyPad(int longitud, char fin){

  // Lee continuamente desde el keypad hasta la longitud deseada
  int contador = 0;
  while (true) {
    char caracter = keypad.getKey();
    if (caracter != NO_KEY) {
      
      datosKeyPad[contador] = caracter;
      contador++;
      if(contador >= longitud || caracter == fin){
        break;
      }
    }
  }

  return contador;
}

void jugar(){
  while (1)
    {
        int posicion = generarOperacionAleatoria();
        if (mostrarJugada(posicion))
        {
            eliminarElemento(posicion);
            puntuacion++;
        } else {
          fin_de_partida(0);
          break;
        }
        imprimirDatos();
        if(totalOperaciones <= 0){
          fin_de_partida(1);
          break;
        }
    }
    opcion = 0;
}

void fin_de_partida(int gana){

  lcd.clear();
  lcd.print("Fin de partida!!");
  
  if(puntuacion > record){
    lcd.setCursor(0,1);
    lcd.printf("Nuevo record : %d", puntuacion);
    record = puntuacion;
    EEPROM.write(eeAdress, record);
    EEPROM.commit();
    delay(5000);
  }else{
    lcd.setCursor(0,1);
    lcd.printf("Puntos %d: ", puntuacion);
    delay(5000);
  }
}


int longitudNumero(int numero){
  return floor(log10(abs(numero)) + 1);
}

char* cadenaResultado(int longitud){
  char* cadena = (char*)malloc((longitud + 1) * sizeof(char));
  memset(cadena, '?', longitud);
  cadena[longitud] = '\0';
  return cadena;
}
