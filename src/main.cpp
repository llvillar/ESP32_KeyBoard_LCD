/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-keypad-lcd
 */

#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

#define ROW_NUM_10     10 // ten rows
#define COLUMN_NUM_10  10 // ten columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM]      = {12, 14, 27, 26}; // GPIO19, GPIO18, GPIO5, GPIO17 connect to the row pins
byte pin_column[COLUMN_NUM] = {25, 33, 32, 35};   // GPIO16, GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

int cursorColumn = 0;

int puntos = 0;
int vidas = 10;
int tablaMultiplicar[ROW_NUM_10][COLUMN_NUM_10];


void setup(){
  Serial.begin(9600);

  lcd.init(); // initialize the lcd
  lcd.backlight();

    // Generar tabla de multiplicar
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      //tabla[i][j] = (i + 1) * (j + 1);
      tablaMultiplicar[i][j] = 0;
    }
  }
}

void loop(){
  int fila = 0;
  int columna = 0;
  //Generar dos números aleatorios
  do{
    fila = random(1, 11);
    columna = random(1, 11);
  }while (tablaMultiplicar[fila][columna] ==1);

    int resultado = fila * columna;

      // Mostrar la operación en la pantalla LCD
    lcd.clear();
    lcd.print(fila);
    lcd.print(" x ");
    lcd.print(columna);
    lcd.print(" = ?");

  // Leer la respuesta del usuario
  char respuesta = keypad.getKey();


  if (respuesta != NO_KEY) {
    Serial.println(respuesta);
    lcd.print(respuesta);
    int respuestaUsuario = respuesta - '0'; // Convertir la respuesta del usuario a un número entero
    
    if (respuestaUsuario == resultado) {
      puntos++; // Aumentar la puntuación
      lcd.setCursor(0, 1);
      lcd.print("Puntuación: ");
      lcd.print(puntos);
      tablaMultiplicar[fila][columna] = 1;
    }
  }
}
