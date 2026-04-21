#include <LiquidCrystal.h>
#include <Keyboard.h>
#include <USB.h>

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

const int rows[4] = {4, 5, 6, 7};
const int cols[4] = {0, 1, 2, 3};

// zapamiętanie stanu klawiszy, żeby nie wpisywało w kółko
bool lastState[4][4] = {false};

// --------------------
// TRYBY
// --------------------
uint8_t currentMode = 1;
bool modeSelectActive = false;


const char mode2Map[15] = {
  '0', '1', '2', '3',
  '4', '5', '6', '7',
  '8', '9', '+', '-',
  '*', '/', '%'
};

// --------------------
// WŁASNE OBRAZKI JAKO ZNAKI
// --------------------

// play
byte customChar1[8] = { 
  0b01000,
  0b01100,
  0b01110,
  0b01111,
  0b01110,
  0b01100,
  0b01000,
  0b00000
};

// pause
byte customChar2[8] = {
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b00000
};

//prev
byte customChar3[8] = {
  0b00010,
  0b00110,
  0b01110,
  0b11110,
  0b01110,
  0b00110,
  0b00010,
  0b00000
};

//line
byte customChar4[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

const char* modeScreen[16][4] = {
  {
    "", "", "", ""
  },

  {
    "T01 Ggl|AGH|DSC|HELP",
    "   USOS|UPL|   |    ",
    "",
    "    V+5|V-5|DOK|MOD "
  },

  {
    "T02  0 | 1 | 2 | 3  ",
    "     4 | 5 | 6 | 7  ",
    "     8 | 9 | + | -  ",
    "     * | / | % |MOD "
  },

  {
    "T03 CtA|Ct<|Ct>|Del ",
    "   CtBs|Sh<|Sh>|Bcsp",
    "       |Tab|   |Ent ",
    "    CtB|CtI|CtU|MOD "
  },

  {
    "T04    |   |End|Home",
    "    Tab|STb|   |    ",
    "       | ^ |PUp|PDn ",
    "     < | v | > |MOD "
  },

  {
    "T05 Yes|No |Ok |Hi! ",
    "    Thx|Sry|Pls|Done",
    "   NPrb|Sur|YWl|Nice ",
    "       |   |   |MOD "
  },

  { 
    "T06 Gma|Ggl|YT |GTrs", 
    "   USOS|UPL|gtb|GPT ",
    "   pAGH|AGH|   |    ",
    "       |   |dok|MOD "
  },

  {
    "",
    "   v+20|v+5|v-5|v-20",
    "   b+20|b+5|b-5|b-20",
    "       |   |   |MOD "
  },

  {
    "T08 cd |ls |pwd|mkdr",
    "    rm |clr|ext|gsta",
    "   gadd|gcm|gpu|gpul",
    "    pip|pyt|   |MOD "
  },

  {
    "T09 CtT|CtW|CtN|Atab",
    "   CtST|CtF|CtR|CtL ",
    "   hist|dow|ss |Inco",
    "    zm+|zm-|zm0|MOD "
  },

  {
    "T10  0 | 1 | 2 | 3  ", //TODO: emoji
    "     4 | 5 | 6 | 7  ",
    "     8 | 9 | + | -  ",
    "     * | / | % |MOD "
  },

  {
    "T11 a/ą| b |c/ć| d ", //TODO: polish characters
    "    e/ę| f | g | h  ",
    "     i | j | k |l/ł ",
    "     m |shf|cap|MOD "
  },

  {
    "T12 n/ń|o/ó| p | q  ", //TODO: polish characters
    "     r |s/ś| t | u  ",
    "     v | w |x/ż| y  ",
    "    z/ż|shf|cap|MOD "
  },

  {
    "T13  . | , | ! | ?  ",
    "     - |  | : | ;  ",  //TODO: long dash
    "     ' | \"\" |(/)|{/} ", //TODO: "
    "    [/]|   |spc|MOD "
  },

  {
    "T14  @ | # | $ | ^  ",
    "     & |   |   |    ",
    "       |   |   |    ",
    "       |   |   |MOD "
  },

  {
    "T15    |   |   |    ",
    "       |   |   |    ",
    "       |   |   |    ",
    "       |   |   |MOD "
  }
};

// --------------------
// LCD
// --------------------
void lcdPrintLine(uint8_t row, const char *text) {
  lcd.setCursor(0, row);
  lcd.print("                    ");
  lcd.setCursor(0, row);
  lcd.print(text);
}

void renderMode1CustomRow2() {
  lcd.setCursor(0, 2);
  lcd.print("                    ");

  lcd.setCursor(0, 2);
  lcd.print("    ");
  lcd.write(byte(0));
  lcd.print("/");
  lcd.write(byte(1));

  lcd.print("|");

  lcd.write(byte(3));
  lcd.write(byte(2));
  lcd.write(byte(2));

  lcd.print("|");

  lcd.write(byte(0));
  lcd.write(byte(0));
  lcd.write(byte(3));

  lcd.print("|mute");
}

void renderMode7CustomRow0() {
  lcd.setCursor(0, 0);
  lcd.print("                    ");

  lcd.setCursor(0, 0);
  lcd.print("T07 ");

  lcd.write(byte(0));
  lcd.print("/");
  lcd.write(byte(1));

  lcd.print("|");

  lcd.write(byte(3));
  lcd.write(byte(2));
  lcd.write(byte(2));

  lcd.print("|");

  lcd.write(byte(2));
  lcd.write(byte(2));
  lcd.write(byte(3));

  lcd.print("|mute");
}

void renderScreen() {
  if (modeSelectActive) {
    char line0[21];
    snprintf(line0, sizeof(line0), "T%02u dom|123|Ctr|<^> ", currentMode);

    lcdPrintLine(0, line0);
    lcdPrintLine(1, "    txt|:-D|url|ster");
    lcdPrintLine(2, "    trm|bro|a-m|n-z");
    lcdPrintLine(3, "    ?{!|   |   |MOD");
  } else {
    lcdPrintLine(0, modeScreen[currentMode][0]);
    lcdPrintLine(1, modeScreen[currentMode][1]);

    if (currentMode == 1) {
      renderMode1CustomRow2();
    } else if (currentMode == 7){
      renderMode7CustomRow0();
      lcdPrintLine(2, modeScreen[currentMode][2]);
    } else {
      lcdPrintLine(2, modeScreen[currentMode][2]);
    }

    lcdPrintLine(3, modeScreen[currentMode][3]);
  }
}

// --------------------
// PRZYCISKI
// --------------------
uint8_t getButtonNumber(uint8_t r, uint8_t c) {
  // numeracja:
  // S1  S2  S3  S4
  // S5  S6  S7  S8
  // S9  S10 S11 S12
  // S13 S14 S15 S16
  return (r * 4 + c + 1);
}

void executeModeAction(uint8_t mode, uint8_t buttonNumber) {
  switch (mode) {
    case 1:
      Serial.print("Tryb 1, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 2:
      if (buttonNumber >= 1 && buttonNumber <= 15) {
        char out = mode2Map[buttonNumber - 1];
        Keyboard.write(out);

        Serial.print("Tryb 2 -> wyslano: ");
        Serial.println(out);
      }
      break;

    case 3:
      Serial.print("Tryb 3, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 4:
      Serial.print("Tryb 4, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 5:
      Serial.print("Tryb 5, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 6:
      Serial.print("Tryb 6, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 7:
      Serial.print("Tryb 7, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 8:
      Serial.print("Tryb 8, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 9:
      Serial.print("Tryb 9, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 10:
      Serial.print("Tryb 10, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 11:
      Serial.print("Tryb 11, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 12:
      Serial.print("Tryb 12, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 13:
      Serial.print("Tryb 13, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 14:
      Serial.print("Tryb 14, przycisk S");
      Serial.println(buttonNumber);
      break;

    case 15:
      Serial.print("Tryb 15, przycisk S");
      Serial.println(buttonNumber);
      break;
  }
}

void handleButtonPress(uint8_t buttonNumber) {
  Serial.print("Przycisk S");
  Serial.println(buttonNumber);

  if (buttonNumber == 16) {
    if (!modeSelectActive) {
      modeSelectActive = true;
      Serial.println("Wejscie do wyboru trybu");
    } else {
      modeSelectActive = false;
      Serial.println("Wyjscie z wyboru trybu");
    }

    renderScreen();
    return;
  }

  if (modeSelectActive) {
    currentMode = buttonNumber;
    modeSelectActive = false;

    Serial.print("Ustawiono tryb T");
    if (currentMode < 10) Serial.print("0");
    Serial.println(currentMode);

    renderScreen();
    return;
  }

  executeModeAction(currentMode, buttonNumber);
}

void setup() {
  Serial.begin(115200);

  lcd.begin(20, 4);

  lcd.createChar(0, customChar1);
  lcd.createChar(1, customChar2);
  lcd.createChar(2, customChar3);
  lcd.createChar(3, customChar4);

  lcd.clear();
  renderScreen();

  USB.disconnect();
  USB.setManufacturer("AO");
  USB.setProduct("MacroPad Pico");
  USB.connect();

  delay(2000);
  Keyboard.begin();

  for (int i = 0; i < 4; i++) {
    pinMode(rows[i], INPUT_PULLDOWN);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(cols[i], OUTPUT);
    digitalWrite(cols[i], LOW);
  }
}

void loop() {
  for (int c = 0; c < 4; c++) {
    digitalWrite(cols[c], HIGH);
    delayMicroseconds(5);

    for (int r = 0; r < 4; r++) {
      bool pressed = (digitalRead(rows[r]) == HIGH);

      if (pressed && !lastState[r][c]) {
        uint8_t buttonNumber = getButtonNumber(r, c);
        handleButtonPress(buttonNumber);
      }

      lastState[r][c] = pressed;
    }

    digitalWrite(cols[c], LOW);
  }

  delay(20);
}