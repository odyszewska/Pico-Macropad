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

// shf = Shift tylko na jeden kolejny znak
bool shiftOnce = false;

// cap = CapsLock logiczny dla liter
bool capsLockEnabled = false;

// spc = tryb znaków po ukośniku:
// T11/T12: a/ą, c/ć, e/ę...
// T13: (/), {/}, [/]
bool spcMode = false;

// który zestaw własnych znaków LCD jest aktualnie załadowany
// 0 = media
// 11 = polskie znaki dla T11
// 12 = polskie znaki dla T12
uint8_t loadedCustomSet = 255;

// --------------------
// MAPY ZNAKÓW
// --------------------

const char mode2Map[15] = {
  '0', '1', '2', '3',
  '4', '5', '6', '7',
  '8', '9', '+', '-',
  '*', '/', '%'
};

const char mode13NormalMap[15] = {
  '.', ',', '!', '?',
  '-', '_', ':', ';',
  '\'', '"', '(', '{',
  '[', ' ', ' '
};

const char mode13AltMap[15] = {
  '.', ',', '!', '?',
  '-', '_', ':', ';',
  '\'', '"', ')', '}',
  ']', ' ', ' '
};

const char mode14Map[15] = {
  '@', '#', '$', '^',
  '&', '=', '+', '*',
  '/', '\\', '|', '~',
  '`', '<', '>'
};

// T05 — szybkie wiadomości
const char* mode5TextMap[15] = {
  "Yes", "No", "Ok", "Hi!",
  "Thank you", "Sorry", "Please", "Done",
  "No problem", "Sure", "You're welcome", "Nice",
  "", "", ""
};

// T10 — tekstowe emotki
const char* mode10EmoteMap[15] = {
  ":)", ":D", "XD", ";)",
  ":>", "c:", ":P", "<3",
  ":(", "T_T", ";(", "^_^",
  ":o", "0_0", "-_-"
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

// prev
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

// line
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

// --------------------
// POLSKIE ZNAKI NA LCD
// HD44780 ma tylko 8 własnych znaków naraz,
// więc dla T11 i T12 ładujemy różne zestawy.
// --------------------

// ą
byte lcd_a_ogonek[8] = {
  0b00000,
  0b01110,
  0b00001,
  0b01111,
  0b10001,
  0b01111,
  0b00010,
  0b00001
};

// ć
byte lcd_c_acute[8] = {
  0b00010,
  0b00100,
  0b01110,
  0b10000,
  0b10000,
  0b10000,
  0b01110,
  0b00000
};

// ę
byte lcd_e_ogonek[8] = {
  0b00000,
  0b01110,
  0b10001,
  0b11111,
  0b10000,
  0b01110,
  0b00010,
  0b00001
};

// ł
byte lcd_l_stroke[8] = {
  0b01100,
  0b00100,
  0b00110,
  0b01100,
  0b00100,
  0b00100,
  0b01110,
  0b00000
};

// ń
byte lcd_n_acute[8] = {
  0b00010,
  0b00100,
  0b10110,
  0b11001,
  0b10001,
  0b10001,
  0b10001,
  0b00000
};

// ó
byte lcd_o_acute[8] = {
  0b00010,
  0b00100,
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b01110,
  0b00000
};

// ś
byte lcd_s_acute[8] = {
  0b00010,
  0b00100,
  0b01111,
  0b10000,
  0b01110,
  0b00001,
  0b11110,
  0b00000
};

// ż
byte lcd_z_dot[8] = {
  0b00100,
  0b00000,
  0b11111,
  0b00010,
  0b00100,
  0b01000,
  0b11111,
  0b00000
};

// ź
byte lcd_z_acute[8] = {
  0b00010,
  0b00100,
  0b11111,
  0b00010,
  0b00100,
  0b01000,
  0b11111,
  0b00000
};

// --------------------
// EKRANY TRYBÓW
// --------------------

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
    "    NPr|Sur|YW |Nice",
    "       |   |   |MOD "
  },

  {
    "T06 Ggl|Gma|YT |GTrs",
    "   USOS|UPL|gtb|GPT ",
    "   pAGH|AGH|gmn|Clad",
    "   biel|ard|dok|MOD "
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
    "T10 :) |:D |XD |;) ",
    "    :> |c: |:P |<3 ",
    "    :( |T_T|;( |^_^",
    "    :o |0_0|-_-|MOD"
  },

  {
    "", "", "", ""
  },

  {
    "", "", "", ""
  },

  {
    "T13  . | , | ! | ?  ",
    "     - | _ | : | ;  ",
    "     ' | \" |(/)|{/}",
    "    [/]|   |spc|MOD "
  },

  {
    "T14  @ | # | $ | ^  ",
    "     & | = | + | *  ",
    "     / | \\ | | | ~  ",
    "     ` | < | > |MOD "
  },

  {
    "T15    |   |   |    ",
    "       |   |   |    ",
    "       |   |   |    ",
    "       |   |   |MOD "
  }
};

// --------------------
// ŁADOWANIE CUSTOM ZNAKÓW LCD
// --------------------

void loadMediaCustomChars() {
  if (loadedCustomSet == 0) return;

  lcd.createChar(0, customChar1);
  lcd.createChar(1, customChar2);
  lcd.createChar(2, customChar3);
  lcd.createChar(3, customChar4);

  loadedCustomSet = 0;
}

void loadPolishT11CustomChars() {
  if (loadedCustomSet == 11) return;

  lcd.createChar(0, lcd_a_ogonek);
  lcd.createChar(1, lcd_c_acute);
  lcd.createChar(2, lcd_e_ogonek);
  lcd.createChar(3, lcd_l_stroke);

  loadedCustomSet = 11;
}

void loadPolishT12CustomChars() {
  if (loadedCustomSet == 12) return;

  lcd.createChar(0, lcd_n_acute);
  lcd.createChar(1, lcd_o_acute);
  lcd.createChar(2, lcd_s_acute);
  lcd.createChar(3, lcd_z_dot);
  lcd.createChar(4, lcd_z_acute);

  loadedCustomSet = 12;
}

// --------------------
// LCD
// --------------------

void lcdPrintLine(uint8_t row, const char *text) {
  lcd.setCursor(0, row);
  lcd.print("                    ");
  lcd.setCursor(0, row);
  lcd.print(text);
}

void lcdPrintModifierStatus() {
  lcd.setCursor(0, 1);
  lcd.print(shiftOnce ? "SHF" : "   ");

  lcd.setCursor(0, 2);
  lcd.print(capsLockEnabled ? "CAP" : "   ");

  lcd.setCursor(0, 3);
  lcd.print(spcMode ? "SPC" : "   ");
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

void renderMode11Screen() {
  loadPolishT11CustomChars();

  lcd.setCursor(0, 0);
  lcd.print("T11 a/");
  lcd.write(byte(0)); // ą
  lcd.print("| b |c/");
  lcd.write(byte(1)); // ć
  lcd.print("| d ");

  lcd.setCursor(0, 1);
  lcd.print("    e/");
  lcd.write(byte(2)); // ę
  lcd.print("| f | g | h   ");

  lcd.setCursor(0, 2);
  lcd.print("     i | j | k |l/");
  lcd.write(byte(3)); // ł
  lcd.print(" ");

  lcd.setCursor(0, 3);
  lcd.print("    shf|cap|spc|MOD");

  lcdPrintModifierStatus();
}

void renderMode12Screen() {
  loadPolishT12CustomChars();

  lcd.setCursor(0, 0);
  lcd.print("T12  m |n/");
  lcd.write(byte(0)); // ń
  lcd.print("|o/");
  lcd.write(byte(1)); // ó
  lcd.print("|p/q ");

  lcd.setCursor(0, 1);
  lcd.print("     r |s/");
  lcd.write(byte(2)); // ś
  lcd.print("| t |u/v ");

  lcd.setCursor(0, 2);
  lcd.print("     w |x/");
  lcd.write(byte(4)); // ź
  lcd.print("| y |z/");
  lcd.write(byte(3)); // ż
  lcd.print("  ");

  lcd.setCursor(0, 3);
  lcd.print("    shf|cap|spc|MOD");

  lcdPrintModifierStatus();
}

void renderScreen() {
  if (modeSelectActive) {
    char line0[21];
    snprintf(line0, sizeof(line0), "T%02u dom|123|Ctr|<^> ", currentMode);

    lcdPrintLine(0, line0);
    lcdPrintLine(1, "    txt|url|med|ster");
    lcdPrintLine(2, "    trm|emo|a-m|n-z");
    lcdPrintLine(3, "    ?{!|   |   |MOD");
    return;
  }

  if (currentMode == 1 || currentMode == 7) {
    loadMediaCustomChars();
  }

  if (currentMode == 11) {
    renderMode11Screen();
    return;
  }

  if (currentMode == 12) {
    renderMode12Screen();
    return;
  }

  lcdPrintLine(0, modeScreen[currentMode][0]);
  lcdPrintLine(1, modeScreen[currentMode][1]);

  if (currentMode == 1) {
    renderMode1CustomRow2();
  } else if (currentMode == 7) {
    renderMode7CustomRow0();
    lcdPrintLine(2, modeScreen[currentMode][2]);
  } else {
    lcdPrintLine(2, modeScreen[currentMode][2]);
  }

  lcdPrintLine(3, modeScreen[currentMode][3]);

  if (currentMode == 13) {
    lcd.setCursor(0, 3);
    lcd.print(spcMode ? "SPC" : "   ");
  }

  if (currentMode == 14) {
    lcd.setCursor(0, 3);
    lcd.print(shiftOnce ? "SHF" : "   ");
  }
}

// --------------------
// WYSYŁANIE ZNAKÓW Z KLAWIATURY
// --------------------

bool isLowercaseLetter(char c) {
  return c >= 'a' && c <= 'z';
}

void sendNormalChar(char c) {
  bool shouldShift = false;

  if (isLowercaseLetter(c)) {
    shouldShift = shiftOnce ^ capsLockEnabled;
  } else {
    shouldShift = shiftOnce;
  }

  if (shouldShift) {
    Keyboard.press(KEY_LEFT_SHIFT);
    delay(5);
    Keyboard.write(c);
    delay(5);
    Keyboard.releaseAll();
  } else {
    Keyboard.write(c);
  }

  shiftOnce = false;
  renderScreen();
}

void sendAltGrChar(char baseKey) {
  bool shouldShift = shiftOnce ^ capsLockEnabled;

  if (shouldShift) {
    Keyboard.press(KEY_LEFT_SHIFT);
    delay(5);
  }

  Keyboard.press(KEY_RIGHT_ALT);
  delay(5);
  Keyboard.press(baseKey);
  delay(5);
  Keyboard.releaseAll();

  shiftOnce = false;
  renderScreen();
}

// Polish programmer layout:
// ą = AltGr + a
// ć = AltGr + c
// ę = AltGr + e
// ł = AltGr + l
// ń = AltGr + n
// ó = AltGr + o
// ś = AltGr + s
// ż = AltGr + z
// ź = AltGr + x
void sendPolishChar(char polishId) {
  switch (polishId) {
    case 'a':
      sendAltGrChar('a');
      break;
    case 'c':
      sendAltGrChar('c');
      break;
    case 'e':
      sendAltGrChar('e');
      break;
    case 'l':
      sendAltGrChar('l');
      break;
    case 'n':
      sendAltGrChar('n');
      break;
    case 'o':
      sendAltGrChar('o');
      break;
    case 's':
      sendAltGrChar('s');
      break;
    case 'z':
      sendAltGrChar('z'); // ż
      break;
    case 'x':
      sendAltGrChar('x'); // ź
      break;
  }
}

void sendTextMode11(uint8_t buttonNumber) {
  switch (buttonNumber) {
    case 1:
      if (spcMode) sendPolishChar('a');
      else sendNormalChar('a');
      break;

    case 2:
      sendNormalChar('b');
      break;

    case 3:
      if (spcMode) sendPolishChar('c');
      else sendNormalChar('c');
      break;

    case 4:
      sendNormalChar('d');
      break;

    case 5:
      if (spcMode) sendPolishChar('e');
      else sendNormalChar('e');
      break;

    case 6:
      sendNormalChar('f');
      break;

    case 7:
      sendNormalChar('g');
      break;

    case 8:
      sendNormalChar('h');
      break;

    case 9:
      sendNormalChar('i');
      break;

    case 10:
      sendNormalChar('j');
      break;

    case 11:
      sendNormalChar('k');
      break;

    case 12:
      if (spcMode) sendPolishChar('l');
      else sendNormalChar('l');
      break;

    case 13:
      shiftOnce = !shiftOnce;
      Serial.print("SHF: ");
      Serial.println(shiftOnce ? "ON" : "OFF");
      renderScreen();
      break;

    case 14:
      capsLockEnabled = !capsLockEnabled;
      Serial.print("CAPS: ");
      Serial.println(capsLockEnabled ? "ON" : "OFF");
      renderScreen();
      break;

    case 15:
      spcMode = !spcMode;
      Serial.print("SPC: ");
      Serial.println(spcMode ? "ON" : "OFF");
      renderScreen();
      break;
  }
}

void sendTextMode12(uint8_t buttonNumber) {
  switch (buttonNumber) {
    case 1:
      sendNormalChar('m');
      break;

    case 2:
      if (spcMode) sendPolishChar('n');
      else sendNormalChar('n');
      break;

    case 3:
      if (spcMode) sendPolishChar('o');
      else sendNormalChar('o');
      break;

    case 4:
      if (spcMode) sendNormalChar('q');
      else sendNormalChar('p');
      break;

    case 5:
      sendNormalChar('r');
      break;

    case 6:
      if (spcMode) sendPolishChar('s');
      else sendNormalChar('s');
      break;

    case 7:
      sendNormalChar('t');
      break;

    case 8:
      if (spcMode) sendNormalChar('v');
      else sendNormalChar('u');
      break;

    case 9:
      sendNormalChar('w');
      break;

    case 10:
      if (spcMode) sendPolishChar('x'); // ź
      else sendNormalChar('x');
      break;

    case 11:
      sendNormalChar('y');
      break;

    case 12:
      if (spcMode) sendPolishChar('z'); // ż
      else sendNormalChar('z');
      break;

    case 13:
      shiftOnce = !shiftOnce;
      Serial.print("SHF: ");
      Serial.println(shiftOnce ? "ON" : "OFF");
      renderScreen();
      break;

    case 14:
      capsLockEnabled = !capsLockEnabled;
      Serial.print("CAPS: ");
      Serial.println(capsLockEnabled ? "ON" : "OFF");
      renderScreen();
      break;

    case 15:
      spcMode = !spcMode;
      Serial.print("SPC: ");
      Serial.println(spcMode ? "ON" : "OFF");
      renderScreen();
      break;
  }
}

void sendMode13Char(uint8_t buttonNumber) {
  if (buttonNumber < 1 || buttonNumber > 15) return;

  if (buttonNumber == 15) {
    spcMode = !spcMode;
    Serial.print("SPC: ");
    Serial.println(spcMode ? "ON" : "OFF");
    renderScreen();
    return;
  }

  char out;

  if (spcMode) {
    out = mode13AltMap[buttonNumber - 1];
  } else {
    out = mode13NormalMap[buttonNumber - 1];
  }

  sendNormalChar(out);
}

void sendMode14Char(uint8_t buttonNumber) {
  if (buttonNumber >= 1 && buttonNumber <= 15) {
    char out = mode14Map[buttonNumber - 1];
    sendNormalChar(out);
  }
}

void sendTextString(const char* text) {
  if (text == nullptr) return;
  if (text[0] == '\0') return;

  Keyboard.print(text);

  shiftOnce = false;
  renderScreen();
}

void sendMode5Text(uint8_t buttonNumber) {
  if (buttonNumber < 1 || buttonNumber > 15) return;

  const char* text = mode5TextMap[buttonNumber - 1];
  sendTextString(text);

  Serial.print("Tryb 5 -> wyslano: ");
  Serial.println(text);
}

void sendMode10Emote(uint8_t buttonNumber) {
  if (buttonNumber < 1 || buttonNumber > 15) return;

  const char* emote = mode10EmoteMap[buttonNumber - 1];
  sendTextString(emote);

  Serial.print("Tryb 10 -> wyslano emotke: ");
  Serial.println(emote);
}

void openUrl(const char* url) {
  if (url == nullptr) return;
  if (url[0] == '\0') return;

  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();

  delay(300);

  Keyboard.print(url);
  delay(100);

  Keyboard.press(KEY_RETURN);
  delay(50);
  Keyboard.releaseAll();

  shiftOnce = false;
  renderScreen();
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
      if (buttonNumber == 1) {
        openUrl("https://www.google.com");
        Serial.println("Tryb 1 -> otwarto Google");
      } if (buttonNumber == 2) {
        openUrl("https://www.agh.edu.pl/");
        Serial.println("Tryb 1 -> otwarto stronę AGH");
      } if (buttonNumber == 5) {
        openUrl("https://web.usos.agh.edu.pl/");
        Serial.println("Tryb 1 -> otwarto USOS AGH");
      } if (buttonNumber == 6) {
        openUrl("https://upel.agh.edu.pl/");
        Serial.println("Tryb 1 -> otwarto UPeL");
      } else {
        Serial.print("Tryb 1, przycisk S");
        Serial.println(buttonNumber);
      }
      break;

    case 2:
      if (buttonNumber >= 1 && buttonNumber <= 15) {
        char out = mode2Map[buttonNumber - 1];
        sendNormalChar(out);

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
      sendMode5Text(buttonNumber);
      break;

    case 6:
      if (buttonNumber == 1) {
        openUrl("https://www.google.com");
        Serial.println("Tryb 6 -> otwarto Google");
      } if (buttonNumber == 2) {
        openUrl("https://mail.google.com/");
        Serial.println("Tryb 6 -> otwarto gmail");
      } if (buttonNumber == 3) {
        openUrl("https://youtube.com/");
        Serial.println("Tryb 6 -> otwarto YouTube");
      } if (buttonNumber == 4) {
        openUrl("https://translate.google.com/");
        Serial.println("Tryb 6 -> otwarto Google Translate");
      } if (buttonNumber == 5) {
        openUrl("https://web.usos.agh.edu.pl/");
        Serial.println("Tryb 6 -> otwarto USOS AGH");
      } if (buttonNumber == 6) {
        openUrl("https://upel.agh.edu.pl/");
        Serial.println("Tryb 6 -> otwarto UPeL");
      } if (buttonNumber == 7) {
        openUrl("https://github.com/");
        Serial.println("Tryb 6 -> otwarto GitHub");
      } if (buttonNumber == 8) {
        openUrl("https://chatgpt.com/");
        Serial.println("Tryb 6 -> otwarto ChatGPT");
      } if (buttonNumber == 9) {
        openUrl("https://poczta.agh.edu.pl/");
        Serial.println("Tryb 6 -> otwarto poczta AGH");
      } if (buttonNumber == 10) {
        openUrl("https://www.agh.edu.pl/");
        Serial.println("Tryb 6 -> otwarto stronę AGH");
      } if (buttonNumber == 11) {
        openUrl("https://gemini.google.com/");
        Serial.println("Tryb 6 -> otwarto Gemini");
      } if (buttonNumber == 12) {
        openUrl("https://claude.ai/");
        Serial.println("Tryb 6 -> otwarto Claude Code");
      } if (buttonNumber == 13) {
        openUrl("https://bielik.ai/");
        Serial.println("Tryb 6 -> otwarto Bielik");
      } if (buttonNumber == 14) {
        openUrl("https://www.arduino.cc/");
        Serial.println("Tryb 6 -> otwarto Ardiuno");
      } else {
        Serial.print("Tryb 6, przycisk S");
        Serial.println(buttonNumber);
      }
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
      sendMode10Emote(buttonNumber);
      break;

    case 11:
      sendTextMode11(buttonNumber);
      break;

    case 12:
      sendTextMode12(buttonNumber);
      break;

    case 13:
      sendMode13Char(buttonNumber);
      break;

    case 14:
      sendMode14Char(buttonNumber);
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

    shiftOnce = false;
    spcMode = false;

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

  loadMediaCustomChars();

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