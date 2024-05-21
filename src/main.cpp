
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

#define ROW_NUM_10     10 // ten rows
#define COLUMN_NUM_10  10 // ten columns

void iniciarTabla();
int leerRespuesta();

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
  iniciarTabla();
}


void loop(){
  int a = 0;
  int b = 0;
  //Generar dos números aleatorios
  do{
    a = random(1, 11);
    b = random(1, 11);
  }while (tablaMultiplicar[a-1][b-1] == 1);

  int c = a * b;

    // Mostrar la operación en la pantalla LCD
  lcd.clear();
  lcd.print(a);
  lcd.print(" x ");
  lcd.print(b);
  lcd.print(" = ?");


  // Leer la respuesta del usuario

  int r = leerRespuesta();
  Serial.println(r);
  lcd.print(r);
    
  if (r == c) {
    puntos++; // Aumentar la puntuación
    lcd.setCursor(0, 1);
    lcd.print("Puntuación: ");
    lcd.print(puntos);
    tablaMultiplicar[a-1][b-1] = 1;
  }
}


void iniciarTabla(){
  // Generar tabla de multiplicar
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      tablaMultiplicar[i][j] = 0;
    }
  }
}


int leerRespuesta(){
  int r = 0;
  // Lee continuamente desde el keypad hasta que se presione el asterisco
  while (true) {
    char caracter = keypad.getKey();
    if (caracter != NO_KEY) {
      // Verifica si se presionó el asterisco (*)
      if (caracter == '*') {
        // Se presionó el asterisco, se puede realizar alguna acción adicional aquí
        Serial.println("Se presionó el asterisco (*)");
        break; // Sale del bucle
      }
      
      // Verifica si el caracter es un dígito
      if (isdigit(caracter)) {
        // Convierte el caracter a número entero y lo agrega al número acumulado
        r = r * 10 + (caracter - '0');
      }
    }
  }
  return r;
}