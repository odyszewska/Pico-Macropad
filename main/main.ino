#include <LiquidCrystal.h>
#include <Keyboard.h>
#include <USB.h>

#ifndef HID_USAGE_CONSUMER_VOLUME_INCREMENT
#define HID_USAGE_CONSUMER_VOLUME_INCREMENT 0x00E9
#endif

#ifndef HID_USAGE_CONSUMER_VOLUME_DECREMENT
#define HID_USAGE_CONSUMER_VOLUME_DECREMENT 0x00EA
#endif

#ifndef HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT
#define HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT 0x006F
#endif

#ifndef HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT
#define HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT 0x0070
#endif

#ifndef HID_USAGE_CONSUMER_PLAY_PAUSE
#define HID_USAGE_CONSUMER_PLAY_PAUSE 0x00CD
#endif

#ifndef HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK
#define HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK 0x00B6
#endif

#ifndef HID_USAGE_CONSUMER_SCAN_NEXT_TRACK
#define HID_USAGE_CONSUMER_SCAN_NEXT_TRACK 0x00B5
#endif

#ifndef HID_USAGE_CONSUMER_STOP
#define HID_USAGE_CONSUMER_STOP 0x00B7
#endif

#ifndef HID_USAGE_CONSUMER_MUTE
#define HID_USAGE_CONSUMER_MUTE 0x00E2
#endif

class MacroKeyboard_ : public Keyboard_ {
public:
  void pressConsumer(uint16_t key) {
    sendConsumerReport(key);
  }

  void releaseConsumer() {
    sendConsumerReport(0);
  }
};

MacroKeyboard_ MacroKeyboard;

#define Keyboard MacroKeyboard

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

// alt = tryb alternatywnych znaków:
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

// T08 — komendy terminalowe
const char* mode8TerminalMap[15] = {
  "cd ", "ls", "pwd", "mkdir ",
  "rm ", "clear", "exit", "git status",
  "git add .", "git commit -m \"\"", "git push", "git pull",
  "python main.py", "pip install ", ""
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
    "    V-6|V+6|DOK|MOD "
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
    "    spc|Tab|   |Ent ",
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
    "   v-20|v-6|v+6|v+20",
    "   b-20|b-6|b+6|b+20",
    "       |   |   |MOD "
  },

  {
    "T08 cd |ls |pwd|mkdr",
    "    rm |clr|ext|gsta",
    "   gadd|gcm|gpu|gpul",
    "    pyt|pip|CMD|MOD "
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
    "    [/]|spc|alt|MOD "
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
  lcd.print(spcMode ? "ALT" : "   ");
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
  lcd.print("    shf|cap|alt|MOD");

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
  lcd.print("    shf|cap|alt|MOD");

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
    lcd.print(spcMode ? "ALT" : "   ");
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
      Serial.print("ALT: ");
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
      Serial.print("ALT: ");
      Serial.println(spcMode ? "ON" : "OFF");
      renderScreen();
      break;
  }
}

void sendMode13Char(uint8_t buttonNumber) {
  if (buttonNumber < 1 || buttonNumber > 15) return;

  if (buttonNumber == 15) {
    spcMode = !spcMode;
    Serial.print("ALT: ");
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

void sendMode8Terminal(uint8_t buttonNumber) {
  if (buttonNumber < 1 || buttonNumber > 15) return;

  if (buttonNumber == 15) {
    openCmd();
    Serial.println("Tryb 8 -> otwarto CMD");
    return;
  }

  const char* command = mode8TerminalMap[buttonNumber - 1];

  if (command == nullptr) return;
  if (command[0] == '\0') return;

  Keyboard.print(command);

  shiftOnce = false;
  renderScreen();

  Serial.print("Tryb 8 -> wpisano: ");
  Serial.println(command);
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

void openCmd() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();

  delay(300);

  Keyboard.print("cmd");
  delay(100);

  Keyboard.press(KEY_RETURN);
  delay(50);
  Keyboard.releaseAll();

  shiftOnce = false;
  renderScreen();
}

void pressCtrlKey(uint8_t key) {
  Keyboard.press(KEY_LEFT_CTRL);
  delay(5);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void pressShiftKey(uint8_t key) {
  Keyboard.press(KEY_LEFT_SHIFT);
  delay(5);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void pressCtrlShiftKey(uint8_t key) {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_SHIFT);
  delay(5);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void pressAltKey(uint8_t key) {
  Keyboard.press(KEY_LEFT_ALT);
  delay(5);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void pressWinKey(uint8_t key) {
  Keyboard.press(KEY_LEFT_GUI);
  delay(5);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void pressWinShiftKey(uint8_t key) {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(KEY_LEFT_SHIFT);
  delay(5);
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void pressSingleKey(uint8_t key) {
  Keyboard.press(key);
  delay(20);
  Keyboard.releaseAll();
  delay(20);

  renderScreen();
}

void handleMode3TextEdit(uint8_t buttonNumber) {
  switch (buttonNumber) {
    case 1:
      pressCtrlKey('a');
      Serial.println("Tryb 3 -> Ctrl + A");
      break;

    case 2:
      pressCtrlKey(KEY_LEFT_ARROW);
      Serial.println("Tryb 3 -> Ctrl + Left");
      break;

    case 3:
      pressCtrlKey(KEY_RIGHT_ARROW);
      Serial.println("Tryb 3 -> Ctrl + Right");
      break;

    case 4:
      pressSingleKey(KEY_DELETE);
      Serial.println("Tryb 3 -> Delete");
      break;

    case 5:
      pressCtrlKey(KEY_BACKSPACE);
      Serial.println("Tryb 3 -> Ctrl + Backspace");
      break;

    case 6:
      pressShiftKey(KEY_LEFT_ARROW);
      Serial.println("Tryb 3 -> Shift + Left");
      break;

    case 7:
      pressShiftKey(KEY_RIGHT_ARROW);
      Serial.println("Tryb 3 -> Shift + Right");
      break;

    case 8:
      pressSingleKey(KEY_BACKSPACE);
      Serial.println("Tryb 3 -> Backspace");
      break;

    case 9:
      pressSingleKey(' ');
      Serial.println("Tryb 3 -> Space");
      break;

    case 10:
      pressSingleKey(KEY_TAB);
      Serial.println("Tryb 3 -> Tab");
      break;

    case 12:
      pressSingleKey(KEY_RETURN);
      Serial.println("Tryb 3 -> Enter");
      break;

    case 13:
      pressCtrlKey('b');
      Serial.println("Tryb 3 -> Ctrl + B");
      break;

    case 14:
      pressCtrlKey('i');
      Serial.println("Tryb 3 -> Ctrl + I");
      break;

    case 15:
      pressCtrlKey('u');
      Serial.println("Tryb 3 -> Ctrl + U");
      break;

    default:
      Serial.print("Tryb 3, brak akcji dla S");
      Serial.println(buttonNumber);
      break;
  }
}

void handleMode4Navigation(uint8_t buttonNumber) {
  switch (buttonNumber) {
    case 3:
      pressSingleKey(KEY_END);
      Serial.println("Tryb 4 -> End");
      break;

    case 4:
      pressSingleKey(KEY_HOME);
      Serial.println("Tryb 4 -> Home");
      break;

    case 5:
      pressSingleKey(KEY_TAB);
      Serial.println("Tryb 4 -> Tab");
      break;

    case 6:
      pressShiftKey(KEY_TAB);
      Serial.println("Tryb 4 -> Shift + Tab");
      break;

    case 10:
      pressSingleKey(KEY_UP_ARROW);
      Serial.println("Tryb 4 -> Up");
      break;

    case 11:
      pressSingleKey(KEY_PAGE_UP);
      Serial.println("Tryb 4 -> Page Up");
      break;

    case 12:
      pressSingleKey(KEY_PAGE_DOWN);
      Serial.println("Tryb 4 -> Page Down");
      break;

    case 13:
      pressSingleKey(KEY_LEFT_ARROW);
      Serial.println("Tryb 4 -> Left");
      break;

    case 14:
      pressSingleKey(KEY_DOWN_ARROW);
      Serial.println("Tryb 4 -> Down");
      break;

    case 15:
      pressSingleKey(KEY_RIGHT_ARROW);
      Serial.println("Tryb 4 -> Right");
      break;

    default:
      Serial.print("Tryb 4, brak akcji dla S");
      Serial.println(buttonNumber);
      break;
  }
}

void handleMode9Browser(uint8_t buttonNumber) {
  switch (buttonNumber) {
    case 1:
      pressCtrlKey('t');
      Serial.println("Tryb 9 -> Ctrl + T");
      break;

    case 2:
      pressCtrlKey('w');
      Serial.println("Tryb 9 -> Ctrl + W");
      break;

    case 3:
      pressCtrlKey('n');
      Serial.println("Tryb 9 -> Ctrl + N");
      break;

    case 4:
      pressAltKey(KEY_TAB);
      Serial.println("Tryb 9 -> Alt + Tab");
      break;

    case 5:
      pressCtrlShiftKey('t');
      Serial.println("Tryb 9 -> Ctrl + Shift + T");
      break;

    case 6:
      pressCtrlKey('f');
      Serial.println("Tryb 9 -> Ctrl + F");
      break;

    case 7:
      pressCtrlKey('r');
      Serial.println("Tryb 9 -> Ctrl + R");
      break;

    case 8:
      pressCtrlKey('l');
      Serial.println("Tryb 9 -> Ctrl + L");
      break;

    case 9:
      pressCtrlKey('h');
      Serial.println("Tryb 9 -> Open history");
      break;

    case 10:
      pressCtrlKey('j');
      Serial.println("Tryb 9 -> Open downloads");
      break;

    case 11:
      pressWinShiftKey('s');
      Serial.println("Tryb 9 -> Screenshot");
      break;

    case 12:
      pressCtrlShiftKey('n');
      Serial.println("Tryb 9 -> Incognito");
      break;

    case 13:
      pressCtrlKey('+');
      Serial.println("Tryb 9 -> Zoom +");
      break;

    case 14:
      pressCtrlKey('-');
      Serial.println("Tryb 9 -> Zoom -");
      break;

    case 15:
      pressCtrlKey('0');
      Serial.println("Tryb 9 -> Reset zoom");
      break;

    default:
      Serial.print("Tryb 9, brak akcji dla S");
      Serial.println(buttonNumber);
      break;
  }
}

void pressConsumerKeyNTimes(uint16_t key, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    Keyboard.pressConsumer(key);
    delay(30);
    Keyboard.releaseConsumer();
    delay(30);
  }

  renderScreen();
}

void volumeUp(uint8_t amount) {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_VOLUME_INCREMENT, amount);
}

void volumeDown(uint8_t amount) {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_VOLUME_DECREMENT, amount);
}

void volumeMute() {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_MUTE, 1);
}

void brightnessUp(uint8_t amount) {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT, amount);
}

void brightnessDown(uint8_t amount) {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT, amount);
}

void mediaPlayPause() {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_PLAY_PAUSE, 1);
}

void mediaPrev() {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK, 1);
}

void mediaNext() {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_SCAN_NEXT_TRACK, 1);
}

void mediaStop() {
  pressConsumerKeyNTimes(HID_USAGE_CONSUMER_STOP, 1);
}

void discoMode() {
  // volumeUp(50);
  openUrl("https://www.youtube.com/watch?v=auLu9oUTzZo");
  Serial.println("Tryb 1 -> DISCO");
}

void helpMode() {
  volumeMute();
  pressCtrlShiftKey('w');
  pressWinKey('d');
  brightnessDown(50);
  Serial.println("Tryb 1 -> HELP");
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
      } else if (buttonNumber == 2) {
        openUrl("https://www.agh.edu.pl/");
        Serial.println("Tryb 1 -> otwarto stronę AGH");
      } else if (buttonNumber == 3) {
        discoMode();
      } else if (buttonNumber == 4) {
        helpMode();
      } else if (buttonNumber == 5) {
        openUrl("https://web.usos.agh.edu.pl/");
        Serial.println("Tryb 1 -> otwarto USOS AGH");
      } else if (buttonNumber == 6) {
        openUrl("https://upel.agh.edu.pl/");
        Serial.println("Tryb 1 -> otwarto UPeL");
      } else if (buttonNumber == 9) {
        mediaPlayPause();
        Serial.println("Tryb 1 -> play/pause");
      } else if (buttonNumber == 10) {
        mediaPrev();
        Serial.println("Tryb 1 -> previous track");
      } else if (buttonNumber == 11) {
        mediaNext();
        Serial.println("Tryb 1 -> next track");
      } else if (buttonNumber == 12) {
        volumeMute();
        Serial.println("Tryb 1 -> mute");
      } else if (buttonNumber == 13) {
        volumeDown(3);
        Serial.println("Tryb 1 -> volume -6");
      } else if (buttonNumber == 14) {
        volumeUp(3);
        Serial.println("Tryb 1 -> volume +6");
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
      handleMode3TextEdit(buttonNumber);
      break;

    case 4:
      handleMode4Navigation(buttonNumber);
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
      if (buttonNumber == 1) {
        mediaPlayPause();
        Serial.println("Tryb 7 -> play/pause");
      } else if (buttonNumber == 2) {
        mediaPrev();
        Serial.println("Tryb 7 -> previous track");
      } else if (buttonNumber == 3) {
        mediaNext();
        Serial.println("Tryb 7 -> next track");
      } else if (buttonNumber == 4) {
        volumeMute();
        Serial.println("Tryb 7 -> mute");
      } else if (buttonNumber == 5) {
        volumeDown(10);
        Serial.println("Tryb 7 -> volume -20");
      } else if (buttonNumber == 6) {
        volumeDown(3);
        Serial.println("Tryb 7 -> volume -6");
      } else if (buttonNumber == 7) {
        volumeUp(3);
        Serial.println("Tryb 7 -> volume +6");
      } else if (buttonNumber == 8) {
        volumeUp(10);
        Serial.println("Tryb 7 -> volume +20");
      } else if (buttonNumber == 9) {
        brightnessDown(10);
        Serial.println("Tryb 7 -> brightness -20");
      } else if (buttonNumber == 10) {
        brightnessDown(3);
        Serial.println("Tryb 7 -> brightness -6");
      } else if (buttonNumber == 11) {
        brightnessUp(3);
        Serial.println("Tryb 7 -> brightness +6");
      } else if (buttonNumber == 12) {
        brightnessUp(10);
        Serial.println("Tryb 7 -> brightness +20");
      } else {
        Serial.print("Tryb 7, przycisk S");
        Serial.println(buttonNumber);
      }
      break;

    case 8:
      sendMode8Terminal(buttonNumber);
      break;

    case 9:
      handleMode9Browser(buttonNumber);
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