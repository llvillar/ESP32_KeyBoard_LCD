#include <Arduino.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <pitches.h>

#define DELAY_LCD_MENU    800 // milisegundos
#define MOSTRAR_DATOS     0   // Mostrar/No mostrar datos por si se han generado bien
#define SPEAKER_PIN 23

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
uint8_t rowPins[ROWS] = { 19, 18, 5, 17 };
// pines correspondientes a las columnas
uint8_t colPins[COLS] = { 16, 4, 2, 15 };

// crea objeto con los prametros creados previamente
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

/*
* score - puntuación actual
* record
* opcion - opcion del modo de juego 1,2,3,4
*/
int score, record, opcion; 

/*
* Estructura con los datos de la operación de la tabla de multiplicar
* donde: a X b = c
*/
typedef struct {
    int a;
    int b;
    int c;
} Operacion;

//Puntero a las operaciones que van a realizarse durante el juego
Operacion *datos;

// total de operaciones a realizar, es un contador 
int totalOperaciones;

//Array para almacenar temporalmente las datos del teclado
char datosKeyPad[20];

//Array para almacenar las tablas a practicar
int tablas[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

//Direccion de memoria para la EEPROM
int eeAdress = 0;

//Puntero para almacenar los datos de la solución 
char *cadena_resultado;

//longitud de la solucion, para poder mostrarla por la pantalla LCD en la posicón deseada
int longitud_total = 0;

String inputPassword = ""; // Almacena la entrada del usuario
String correctPassword = "ABBA"; // La clave correcta para resetear record

void resetRecord(){
    record = 0;
    EEPROM.writeInt(eeAdress, record);
    EEPROM.commit();
}

//Obtenemos el record almacenado en la EEPROM
void initRecord(){
  EEPROM.begin(sizeof(int));
  EEPROM.begin(1024);
  record = EEPROM.readInt(eeAdress);
}

//Iniciar la pantalla LCD
void initLCD(){
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.noCursor();
}

//Iniocializar el buzzer
void initSpeaker(){
  pinMode(SPEAKER_PIN, OUTPUT);
}

//Mensaje de bienvenida
void wellcome(){
  lcd.print("Tablas de");
  lcd.setCursor(0,1);
  lcd.print("multiplicar");
  delay(1500);
}

/*
* metodo para leer desde el teclado 
* longitud - indica la cantidad de caracteres que se desean leer
* fin - char para indicar unncaracter de fin de lectura 
* rowLCD, colLCD - posición del cursor del LCD para escribir mientras se introducen datos
*
*/
int readKeyPadFromMenu(int longitud, char fin, int rowLCD, int colLCD){
  String clave = "ABBA";
  // Lee continuamente desde el keypad hasta la longitud deseada o caracter de fin
  int contador = 0;
  lcd.setCursor(colLCD, rowLCD);
  lcd.cursor();
  while (true) {
    char caracter = keypad.getKey();

    if (caracter != NO_KEY && (caracter == '1' || caracter == '2' || caracter == '3' 
    || caracter == '4' || caracter == '5' || caracter == '6' || caracter == '7' || caracter == '8' 
    || caracter == '9' || caracter == '0' || caracter == fin )) {
      
      lcd.print(caracter);

      datosKeyPad[contador] = caracter;
      contador++;
      if(contador >= longitud || caracter == fin){
        /*if(caracter == fin){
          datosKeyPad[contador] = caracter;
          contador++;
        }*/
        datosKeyPad[contador] = '\0';
        break;
      }
    }
    
    if (caracter != NO_KEY && (caracter == 'A' || caracter == 'B' || caracter == 'C' || caracter == 'D' )) {
        inputPassword += caracter;
        if(inputPassword.length() >= 4){
          if (inputPassword == correctPassword) {
            resetRecord();
            tone(SPEAKER_PIN, 2000, 200);
          } else{
            tone(SPEAKER_PIN, 500, 200);
          }
          inputPassword = "";
        }
    }
  }
  lcd.noCursor();

  return contador;
}

/*
* Menu principal donde se elige la opcion de juego
*/
void mainMenu() {
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Elige opcion: ");
    //lcd.setCursor(0,0);
    //lcd.print("0.Reset record");
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
    readKeyPadFromMenu(1,' ', 1, 13);
    delay(500);
}

/*
* Metodo de inicialización de las variables del juego, conjunto de datos, tablas y puntuación.
*/
void initGame(){
  if(datos) {
    free(datos);
    datos = NULL;
  }

  for (int i = 0; i < 9; i++)
  {
    tablas[i] = 0;
  }

  for (int i = 0; i < 20; i++)
  {
    datosKeyPad[i] = '\0';
  }

  score = 0;
}

/*
* Metodo que comprueba que la opción de juego es correcta la 1,2,3,4
*/
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

/*
* Metodo que imprime la solución en caso de que el jugador haya fallado la operación
*/
void imprimirSolucion(int solucion)
{
    lcd.setCursor(0,1);
    lcd.printf("Solucion : %d", solucion);
    delay(1000);
}

/*
* Metodo que ejecuta una melodia si se ha alcanzado un nuevo record
*/
void playRecordMelody() {

int melody[] = {
  NOTE_E5, NOTE_E5, 0, NOTE_E5,
  0, NOTE_C5, NOTE_E5, 0,
  NOTE_G5, 0, 0, 0,
  NOTE_G4, 0, 0, 0,

  NOTE_C5, 0, 0, NOTE_G4,
  0, 0, NOTE_E4, 0,
  0, NOTE_A4, 0, NOTE_B4,
  0, NOTE_AS4, NOTE_A4, 0,

  NOTE_G4, NOTE_E5, 0, NOTE_G5,
  NOTE_A5, 0, NOTE_F5, NOTE_G5,
  0, NOTE_E5, 0, NOTE_C5,
  NOTE_D5, NOTE_B4, 0, 0,

  NOTE_C5, 0, 0, NOTE_G4,
  0, 0, NOTE_E4, 0,
  0, NOTE_A4, 0, NOTE_B4,
  0, NOTE_AS4, NOTE_A4, 0,

  NOTE_G4, NOTE_E5, 0, NOTE_G5,
  NOTE_A5, 0, NOTE_F5, NOTE_G5,
  0, NOTE_E5, 0, NOTE_C5,
  NOTE_D5, NOTE_B4, 0, 0
};

// Duraciones de cada nota (4 significa una negra, 8 significa una corchea, etc.)
int noteDurations[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12
};


  for (int thisNote = 0; thisNote < sizeof(melody)/sizeof(melody[0]); thisNote++) {
    // Calcular la duración de la nota
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(SPEAKER_PIN, melody[thisNote], noteDuration);

    // Pausa entre las notas para distinguirlas claramente
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    // Apagar el buzzer entre las notas
    noTone(SPEAKER_PIN);
  }
}

/*
* Método que muestra la puntuación obtenida tras la partida
* si hay un nuevo record lo guarda en la EEPROM
*/
void displayScore() {
  if(score > record){
    lcd.setCursor(0,1);
    lcd.printf("Record: %d", score);
    record = score;
    EEPROM.writeInt(eeAdress, record);
    EEPROM.commit();
    playRecordMelody();
    //delay(4000);
  }else{
    lcd.setCursor(0,1);
    lcd.printf("Puntos: %d ", score);
    delay(3000);
    lcd.setCursor(0,1);
    lcd.printf("Record: %d", score);
    delay(2000);
  }
}

/*
* Metodo que muestra las tablas de multiplicar 1 al 9 por pantalla e indica que se pulse * para confirmar
* Si se pulsa un numero la tabla es eliminada/mostrada en función si está o no seleccionada.
*/
void seleccionarTablas(){
  while(1){
    int tablasSeleccionadas = 0;
    lcd.clear();
    for (int i = 0; i < 9; i++)
    {
      if(tablas[i] != 0){
        lcd.print(tablas[i]);
        tablasSeleccionadas  = 1;
      }
    }
    if(!tablasSeleccionadas){
      lcd.print("Marque tablas");      
    }
    lcd.setCursor(0,1);
    lcd.print("(*) Confirmar");
    //delay(100);

    int longitud = readKeyPadFromMenu(1, '*', 0, 0);

    // Verificar si el carácter '*' está presente
    if (datosKeyPad[0] == '1'
          || datosKeyPad[0] == '2'
          || datosKeyPad[0] == '3'
          || datosKeyPad[0] == '4'
          || datosKeyPad[0] == '5'
          || datosKeyPad[0] == '6'
          || datosKeyPad[0] == '7'
          || datosKeyPad[0] == '8'
          || datosKeyPad[0] == '9') {

        
        int indice = datosKeyPad[0] - '0';
        //Cada vez que se pulsa un nuemro del teclado eliminamos esa tabla de la lista 
        //seteando ese elemento a 0. Si se vuelve a pulsar el mismo número volvemos a poner el valor 
        //correcto en el indice del array de tablas.
        if(tablas[indice-1] == 0){
          tablas[indice-1] = indice;
        }else{
          tablas[indice-1] = 0;
        }
    } else if(datosKeyPad[0] == '*'){
      //Pulsando el *, el juego comienza con las tablas mostradas en la pantalla
      //que han sido seleccionadas
      int algunaTabla = 0;

      //En este for se comprueba que al menos una tabla esté seleccionada en 
      //caso contrario se vuelven a mostrar todas
      for (int i = 0; i < 9; i++)
      {
        if(tablas[i] != 0){
          algunaTabla++;
        }
      }
      if (algunaTabla)
      {
        break;
      }
    } else{
      for (int i = 0; i < 9; i++)
      {
        tablas[i]=i+1;  
      }
    }
  }
}


/*
* En este metodo se inicializa un puntero a un conjunto de estructuras de tipo Operación
* con los datos selecionados en el array tablas
*/
void inicializarTablas(int tablas[], int tamanio){

    int contador = 0;
    for (int i = 0; i < 9; i++)
    {
        if (tablas[i] != 0)
        {
            contador++;
        }
    }

    totalOperaciones = 10 * contador;
    datos = (Operacion *) malloc(totalOperaciones * sizeof(Operacion));
    
    int desplazamiento = 0;
    for (int i = 0; i < 9; i++)
    {
        if (tablas[i] != 0)
        {
          for (int j = 0; j < 10; j++)
          {

            datos[j+(desplazamiento*10)].a = tablas[i];
            datos[j+(desplazamiento*10)].b = j + 1;
            datos[j+(desplazamiento*10)].c = tablas[i] * (j + 1);
          }
          desplazamiento++;
        }     
    }
}

/*
* Metodo auxiliar para imprimir el conjunto de datos, para comprobar que se han generado correctamente
*/
void printData(){
  if(MOSTRAR_DATOS){
    for (int i = 0; i < totalOperaciones; i++) {
      lcd.clear();
      lcd.printf("%d x %d = %d\n", datos[i].a, datos[i].b, datos[i].c);
      delay(500);
    }
  }
}

/*
* Metodo que genera un numero aleatorio entre 1 y el total de operaciones que restan por mostrar (totalOperaciones)
* este numero aleatorio representa la posicion del elemento dentro del array
*/
int generarOperacionAleatoria(){
    srand(time(NULL));
    int random = (rand() % totalOperaciones);
    return random;
}

/*
* Metodo que eliminar un elemento del array de datos del conjunto de operaciones por la posicion dada
* La posicion es el numero aleatorio generado para selecionar la Operacion a realizar.
* Cada Operacion acertada se reduce el array y por lo tanto totalOperaciones se decrementa en una unidad
* para la generación del siguiente número aleatorio.
*/
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

/*
* Metodo para conocer la longitud del resultado esperado, se usa para poder conocer cuantos simbolos ?
* mostrar como pista al jugador
*/
int longitudNumero(int numero){
  return floor(log10(abs(numero)) + 1);
}

/*
* metodo que genera un array con la cantidad de simbolos ? que se pasen por parámetro(longitud)
*/
char* cadenaResultado(int longitud){
  char* cadena = (char*)malloc((longitud + 1) * sizeof(char));
  memset(cadena, '?', longitud);
  cadena[longitud] = '\0';
  return cadena;
}

/*
* Metodo que interpreta melodia cuando hay acierto
*/
void playLevelUpSound() {
  tone(SPEAKER_PIN, NOTE_E4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5);
  delay(150);
  noTone(SPEAKER_PIN);
}


/*
* Metodo que mostrará la jugada correspondiente al numero aleatorio generado, que representa la posición de la 
* Operacion a realizar dentro del array
* devuelve 1 o 0 si se ha acertado o no.
*/
int mostrarJugada(int posicion){

    int respuesta;
    //Operacion selecionada aleatoriamente
    Operacion o = datos[posicion];

    srand(time(NULL));

    //Tipo de jugada sera 1,2,3 .
    //Si el usuario seleciona el 4 en cada jugada se genera un numero aleatorio entre 1 y 3
    int tipoJugada = (opcion != 4) ? opcion : (rand()%3 +1);

    int acierto = 0;

    lcd.clear();
      
    //Variables para concer la posición del cursor(LCD) para mostrar los resultados
    int longitud_a = longitudNumero(o.a);
    int longitud_b = longitudNumero(o.b);
    int longitud_c = longitudNumero(o.c);
    int solucion;

    switch (tipoJugada)
    {
        case 1:
                solucion = o.c;
                cadena_resultado = cadenaResultado(longitud_c); 
                lcd.printf("%d X %d = %s", o.a, o.b, cadena_resultado);
                longitud_total = longitud_a + longitud_b + longitud_c + 6;
                readKeyPadFromMenu(longitud_c, ' ', 0, longitud_total-longitud_c);
                respuesta = atoi(datosKeyPad);
                acierto = (o.c == respuesta) ? 1 : 0;
            break;
        case 2:
                solucion = o.a;
                cadena_resultado = cadenaResultado(longitud_a); 
                lcd.printf("%s X %d = %d", cadena_resultado, o.b, o.c);
                readKeyPadFromMenu(longitud_a, ' ', 0, 0);
                respuesta = atoi(datosKeyPad);
                acierto = (o.a == respuesta) ? 1 : 0;
            break;    
        case 3:
                solucion = o.b;
                cadena_resultado = cadenaResultado(longitud_b); 
                lcd.printf("%d X %s = %d\n", o.a, cadena_resultado, o.c);
                longitud_total = longitud_a + 3;
                readKeyPadFromMenu(longitud_b, ' ', 0, longitud_total);
                respuesta = atoi(datosKeyPad);   
                acierto = (o.b == respuesta) ? 1 : 0;
            break;
        default:
            break;
    }

    //Si falla el jugador se le meuestra la solucion
    if(!acierto){
        imprimirSolucion(solucion);
    }

    return acierto;
}

/*
* Método que imprime el mensaje de fin de partida y ejecuta un una melodia
* Mestra la puntuación obtenida e inicializa la puntuacion a 0.
*/
void fin_de_partida(int perdida) {
  lcd.clear();
  lcd.print("Fin de partida!!");

  // Play a Wah-Wah-Wah-Wah sound
  if(perdida){
    tone(SPEAKER_PIN, NOTE_DS5);
    delay(300);
    tone(SPEAKER_PIN, NOTE_D5);
    delay(300);
    tone(SPEAKER_PIN, NOTE_CS5);
    delay(300);
    for (byte i = 0; i < 10; i++) {
      for (int pitch = -10; pitch <= 10; pitch++) {
        tone(SPEAKER_PIN, NOTE_C5 + pitch);
        delay(5);
      }
    }
    noTone(SPEAKER_PIN);
  }
  
  displayScore();
  score = 0;
  delay(500);
}

/*
* Método que realiza cada una de las jugadas
*/
void jugar(){
  while (1)
    {
        //genera una opracion aletario por la posición de los datos dentro del array
        int posicion = generarOperacionAleatoria();
        //Se muestra la jugada y se comprueba si se ha acertado o no
        //Si acierta eliminamos esa operación para que no vuelva a aparecer incrementamos la puntuación
        //Si no acierta fin de partida. También acaba la partida si no hay más operaciones a realizar.
        if (mostrarJugada(posicion))
        {
            playLevelUpSound();
            eliminarElemento(posicion);
            score++;
        } else {
          fin_de_partida(1);
          break;
        }
        printData();
        if(totalOperaciones <= 0){
          fin_de_partida(0);
          break;
        }
    }
    opcion = 0;
}

void setup(){
  Serial.begin(11600);
  initRecord();
  initLCD();
  initSpeaker();
  wellcome();
}

void loop(){
  //Inicializamos las variables del juego
  initGame();
  //Mostramos las opciones del juego
  mainMenu();

  char key = datosKeyPad[0];
  opcion = checkOpcion(key);
  //Si la opción es correcta , selecionamos las tablas para jugar e inicializamos la partida.
  if(opcion > 0 && opcion < 5){
    seleccionarTablas();
    inicializarTablas(tablas, 10);
    printData();
    jugar();
  }
}