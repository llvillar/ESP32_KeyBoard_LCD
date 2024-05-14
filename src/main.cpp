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

void setup(){
  lcd.init(); // initialize the lcd
  lcd.backlight();
}

void loop(){
  char key = keypad.getKey();

  if (key) {
    lcd.setCursor(cursorColumn, 0); // move cursor to   (cursorColumn, 0)
    lcd.print(key);                 // print key at (cursorColumn, 0)

    cursorColumn++;                 // move cursor to next position
    if(cursorColumn == 16) {        // if reaching limit, clear LCD
      lcd.clear();
      cursorColumn = 0;
    }
  }
}
