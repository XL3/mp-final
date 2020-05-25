// Bit manipulation macros
#define BIT_SET(reg, bit)    reg |= (1u << bit)
#define BIT_CLEAR(reg, bit)  reg &= ~(1u << bit)
#define BIT_TOGGLE(reg, bit) reg ^= (1u << bit)
#define BIT_READ(reg, bit)  (reg & (1u << bit))
#define NIB_ASSIGN(dim, nib) {\
  digitalWrite(EN_##dim##_0, BIT_READ(nib, 0) ? HIGH : LOW);\
  digitalWrite(EN_##dim##_1, BIT_READ(nib, 1) ? HIGH : LOW);\
  digitalWrite(EN_##dim##_2, BIT_READ(nib, 2) ? HIGH : LOW);\
  digitalWrite(EN_##dim##_3, BIT_READ(nib, 3) ? HIGH : LOW);\
}
#define CLEAR_LEDS() {\
  BIT_CLEAR(PORTB, ADR_EN);\
  BIT_SET(PORTB, CLR);\
  PORTD = 0xFF;\
  BIT_CLEAR(PORTB, CLR);\
}

#define NUMBER 0
#define EMOJI  1

// Delay macro
#define DELAY_MS(ms) {\
  unsigned long __dt = millis();\
  while(millis() - __dt < ms);\
}

// Pin environment
// Port D
#define EN_C_0          0   // 0
#define EN_C_1          1   // 1
#define EN_C_2          2   // 2
#define EN_C_3          3   // 3
#define EN_R_0          4   // 4
#define EN_R_1          5   // 5
#define EN_R_2          6   // 6
#define EN_R_3          7   // 7

// Port B
#define DATA            0   // 8
#define DIAG            1   // 9
#define CLR             2   // 10
#define AUDIO_OUT       3   // 11
#define ADR_EN          4   // 12

// Port C
#define KEYPAD_ACTIVE   4   // A4
#define KEYPAD_3        3   // A3
#define KEYPAD_2        2   // A2
#define KEYPAD_1        1   // A1
#define KEYPAD_0        0   // A0

// Audio
// Tones
#define TONE_c  3830    // 261 Hz
#define TONE_d  3400    // 294 Hz
#define TONE_e  3038    // 329 Hz
#define TONE_f  2864    // 349 Hz
#define TONE_g  2550    // 392 Hz
#define TONE_a  2272    // 440 Hz
#define TONE_b  2028    // 493 Hz
#define TONE_C  1912    // 523 Hz
#define TONE_D  1703    // 587 Hz
#define TONE_E  1517    // 659 Hz
#define TONE_R  0

// Utility functions
void runDiagnostics();
void display(byte type, byte x);
bool checkPassword();
void displayStep(byte type, byte x);
void playTone();

// Global Variables
const byte changePW[] = { 1, 2, 1, 2 };
byte password[] = { 1, 2, 3, 4 };
byte current_mode = NUMBER;

byte input[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
char input_ptr = 0;

byte key_press = 0xFF;
bool keypad_state = false;
bool just_toggled = false;

int audio_melody[] = { TONE_c, TONE_d, TONE_e, TONE_f, TONE_g, TONE_a, TONE_b, TONE_C, TONE_D, TONE_E };
int audio_beat = 8;
long audio_tempo = 8192;
int audio_rest_count = 100;
// ---
int audio_tone = 0;
long audio_duration = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  DDRD |= 0b11111111;   // 0-7   Output
  DDRB |= 0b111111;     // 8-13  Output
  DDRC &= ~(0b11111);   // A0-A4 Input
  CLEAR_LEDS();

  runDiagnostics();
}

void loop() {
  if (!BIT_READ(PINC, KEYPAD_ACTIVE)) {
    keypad_state = false;
    displayStep(current_mode ^ just_toggled, key_press);
    return;
  }
  // If a button was just pressed
  if (!keypad_state) {
    // Set the button to pressed
    keypad_state = true;

    // Read key press
    key_press = PINC & 0b1111;

    // Accumulate input
    input[input_ptr] = key_press;
    input_ptr = (input_ptr+1) % 8;

    // Clear the recent toggle flag
    just_toggled = false;

    // Play audio cue
    audio_tone = audio_melody[key_press];
    audio_duration = audio_beat * audio_tempo;
    playTone();
  }
  displayStep(current_mode ^ just_toggled, key_press);

  // Toggle current mode
  changePassword();
  if (checkPassword()) {
    current_mode = !current_mode;
    just_toggled = true;

    // Clear the input buffer
    for (byte i = 0; i < 8; i++) {
      input[i] = 0xFF;
    }
    input_ptr = 0;

    // Play longer audio cue
    audio_tone = audio_melody[key_press];
    audio_duration = audio_beat * audio_tempo * 2;
    playTone();
  } 
}

void runDiagnostics() {
  CLEAR_LEDS();
  BIT_SET(PORTB, DATA);
  BIT_SET(PORTB, DIAG);

  for (byte i = 0; i < 10; i++) {
    digitalWrite(8 + ADR_EN, LOW);
    PORTD = 0xF0 + i;
    digitalWrite(8 + ADR_EN, HIGH);
    DELAY_MS(8);

    CLEAR_LEDS();
  }

  for (byte i = 0; i < 10; i++) {
    digitalWrite(8 + ADR_EN, LOW);
    PORTD = (i << 4) + 0xF;
    digitalWrite(8 + ADR_EN, HIGH);
    DELAY_MS(8);

    CLEAR_LEDS();
  }
  BIT_CLEAR(PORTB, DIAG);
}

void changePassword() {
  char ptr = (input_ptr - 8);
  byte size = 8;  // sizeof(input) - null terminator
  ptr = ptr > -1 ? ptr : ptr + size;

  bool match = true;
  for (byte i = 0; i < 4; i++) {
    if (input[ptr] != changePW[i]) {
      match = false;
      break;
    }
    ptr++;
    ptr = ptr < size ? ptr : ptr - size; 
  }

  byte temp[] = { 0xFF, 0xFF, 0xFF, 0xFF };
  if (match) {
    for (byte i = 0; i < 4; i++) {
      if (input[ptr] == 0xFF) {
        match = false;
        break;
      } else {
        temp[i] = input[ptr];
      }
      ptr++;
      ptr = ptr < size ? ptr : ptr - size; 
    }
  }
  if (match) {
    for (byte i = 0; i < 4; i++) {
      password[i] = temp[i];
    }
    digitalWrite(LED_BUILTIN, HIGH);
    DELAY_MS(8);
    // Clear the input buffer
    for (byte i = 0; i < 8; i++) {
      input[i] = 0xFF;
    }
    input_ptr = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }
}

bool checkPassword() {
  char ptr = (input_ptr - 4);
  byte size = 8;  // sizeof(input) - null terminator
  ptr = ptr > -1 ? ptr : ptr + size;

  bool match = true;
  for (byte i = 0; i < 4; i++) {
    if (current_mode == NUMBER) {
      if (input[ptr] != password[i]) {
        match = false;
        break;
      }
    } else if (current_mode == EMOJI) {  // Reverse password
      if (input[ptr] != password[3 - i]) {
        match = false;
        break;
      }
    }
    ptr++;
    ptr = ptr < size ? ptr : ptr - size; 
  }

  return match;
}

// EMOJIS
// smile
const byte emoji_0[] = { 0x03, 0x04, 0x05, 0x06, 0x12, 0x17, 0x21, 0x28, 0x31, 0x33, 0x36, 0x38, 0x40, 0x49, 0x50, 0x53, 0x56, 0x59, 0x61, 0x64, 0x65, 0x68, 0x71, 0x78, 0x82, 0x87, 0x93, 0x94, 0x95, 0x96, };
// frown
const byte emoji_1[] = { 0x03, 0x04, 0x05, 0x06, 0x12, 0x17, 0x21, 0x28, 0x31, 0x33, 0x36, 0x38, 0x40, 0x49, 0x50, 0x54, 0x55, 0x59, 0x61, 0x63, 0x66, 0x68, 0x71, 0x78, 0x82, 0x87, 0x93, 0x94, 0x95, 0x96, };
// devil
const byte emoji_2[] = { 0x00, 0x09, 0x10, 0x11, 0x13, 0x14, 0x15, 0x16, 0x18, 0x19, 0x21, 0x22, 0x27, 0x28, 0x31, 0x33, 0x36, 0x38, 0x40, 0x49, 0x50, 0x52, 0x57, 0x59, 0x61, 0x63, 0x66, 0x68, 0x71, 0x74, 0x75, 0x78, 0x82, 0x87, 0x93, 0x94, 0x95, 0x96, };
// poker
const byte emoji_3[] = { 0x03, 0x04, 0x05, 0x06, 0x12, 0x17, 0x21, 0x28, 0x30, 0x31, 0x33, 0x36, 0x38, 0x39, 0x40, 0x49, 0x50, 0x59, 0x60, 0x63, 0x64, 0x65, 0x66, 0x69, 0x71, 0x78, 0x82, 0x87, 0x93, 0x94, 0x95, 0x96, };
// heart
const byte emoji_4[] = { 0x01, 0x02, 0x07, 0x08, 0x10, 0x13, 0x16, 0x19, 0x20, 0x24, 0x25, 0x29, 0x30, 0x39, 0x41, 0x48, 0x51, 0x58, 0x62, 0x67, 0x72, 0x77, 0x83, 0x86, 0x94, 0x95, };
// broken
const byte emoji_5[] = { 0x01, 0x02, 0x03, 0x06, 0x07, 0x08, 0x10, 0x14, 0x15, 0x19, 0x20, 0x23, 0x29, 0x30, 0x34, 0x39, 0x41, 0x45, 0x48, 0x51, 0x54, 0x58, 0x62, 0x65, 0x67, 0x72, 0x74, 0x77, 0x83, 0x86, 0x94, 0x95, };
// bomb
const byte emoji_6[] = { 0x07, 0x16, 0x25, 0x33, 0x34, 0x35, 0x36, 0x42, 0x47, 0x51, 0x58, 0x61, 0x68, 0x71, 0x78, 0x82, 0x87, 0x93, 0x94, 0x95, 0x96, };
// check
const byte emoji_7[] = { 0x29, 0x38, 0x39, 0x47, 0x48, 0x50, 0x56, 0x57, 0x60, 0x61, 0x65, 0x66, 0x71, 0x72, 0x74, 0x75, 0x82, 0x83, 0x84, 0x93, };
// gun
const byte emoji_8[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x39, 0x40, 0x49, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x59, 0x64, 0x67, 0x69, 0x75, 0x76, 0x77, 0x79, 0x87, 0x89, 0x97, 0x98, 0x99, };
// tori
const byte emoji_9[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x22, 0x24, 0x25, 0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x42, 0x47, 0x52, 0x57, 0x62, 0x67, 0x72, 0x77, 0x82, 0x87, 0x92, 0x97, };

const byte* emojis[10] = { emoji_0, emoji_1, emoji_2, emoji_3, emoji_4, emoji_5, emoji_6, emoji_7, emoji_8, emoji_9, };
const byte emojis_n[10] = { 30, 30, 38, 32, 26, 32, 21, 20, 35, 36, };

// NUMBERS
// 0
const byte num_0[] = { 0x13, 0x14, 0x15, 0x16, 0x23, 0x26, 0x33, 0x36, 0x43, 0x46, 0x53, 0x56, 0x63, 0x66, 0x73, 0x76, 0x83, 0x84, 0x85, 0x86, };
// 1
const byte num_1[] = { 0x15, 0x16, 0x24, 0x25, 0x26, 0x33, 0x34, 0x35, 0x36, 0x45, 0x46, 0x55, 0x56, 0x65, 0x66, 0x75, 0x76, 0x85, 0x86, };
// 2
const byte num_2[] = { 0x13, 0x14, 0x15, 0x16, 0x26, 0x36, 0x46, 0x53, 0x54, 0x55, 0x56, 0x63, 0x73, 0x83, 0x84, 0x85, 0x86, };
// 3
const byte num_3[] = { 0x13, 0x14, 0x15, 0x16, 0x26, 0x36, 0x43, 0x44, 0x45, 0x46, 0x56, 0x66, 0x76, 0x83, 0x84, 0x85, 0x86, };
// 4
const byte num_4[] = { 0x13, 0x23, 0x26, 0x33, 0x36, 0x43, 0x44, 0x45, 0x46, 0x47, 0x56, 0x66, 0x76, 0x86, };
// 5
const byte num_5[] = { 0x13, 0x14, 0x15, 0x16, 0x23, 0x33, 0x43, 0x44, 0x45, 0x46, 0x56, 0x66, 0x76, 0x83, 0x84, 0x85, 0x86, };
// 6
const byte num_6[] = { 0x13, 0x14, 0x15, 0x16, 0x23, 0x33, 0x43, 0x53, 0x54, 0x55, 0x56, 0x63, 0x66, 0x73, 0x76, 0x83, 0x84, 0x85, 0x86, };
// 7
const byte num_7[] = { 0x12, 0x13, 0x14, 0x15, 0x16, 0x26, 0x36, 0x46, 0x53, 0x54, 0x55, 0x56, 0x66, 0x76, 0x86, };
// 8
const byte num_8[] = { 0x13, 0x14, 0x15, 0x16, 0x22, 0x23, 0x26, 0x27, 0x32, 0x33, 0x36, 0x37, 0x43, 0x44, 0x45, 0x46, 0x52, 0x53, 0x56, 0x57, 0x62, 0x63, 0x66, 0x67, 0x72, 0x73, 0x76, 0x77, 0x83, 0x84, 0x85, 0x86, };
// 9
const byte num_9[] = { 0x13, 0x14, 0x15, 0x16, 0x23, 0x26, 0x33, 0x36, 0x43, 0x44, 0x45, 0x46, 0x56, 0x66, 0x76, 0x83, 0x84, 0x85, 0x86, };

const byte* numbers[10] = { num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9, };
const byte numbers_n[10] = { 20, 19, 17, 17, 14, 17, 19, 15, 32, 19  };

void displayStep(byte type, byte x) {
  static byte previousX = 0xFF;
  static byte previousType = 0xFF;
  static byte iX = 0;
  static unsigned long previousTime = 0;

  // If we're drawing a new shape, reset the index
  if (x != previousX || type != previousType) {
    iX = 0;
    CLEAR_LEDS();
    BIT_SET(PORTB, DATA);
    previousX = x;
    previousType = type;
  }
  byte count;
  byte* array;

  if (type == NUMBER) {
    count = numbers_n[x];
    array = numbers[x];
  } else if (type == EMOJI) {
    count = emojis_n[x];
    array = emojis[x];
  } else return;

  // If the shape is complete, return
  // If not enough time has passed, return
  if (iX == count) return;
  if (millis() - previousTime < 5) return;

  // Display and increment index
  digitalWrite(8 + ADR_EN, LOW);
  PORTD = array[iX++];
  digitalWrite(8 + ADR_EN, HIGH);
  previousTime = millis();
}

// audio_tone
// audio_melody
void playTone() {
  long elapsed_time = 0;
  // If this isn't a Rest beat
  if (audio_tone > 0) {
    // While the tone has played less long than 'duration', pulse speaker HIGH and LOW
    while (elapsed_time < audio_duration) {
      digitalWrite(8 + AUDIO_OUT, HIGH);
      delayMicroseconds(audio_tone / 2);
      digitalWrite(8 + AUDIO_OUT, LOW);
      delayMicroseconds(audio_tone / 2);

      // Keep track of how long we pulsed
      elapsed_time += (audio_tone);
    }
  }
  // Rest beat; loop times delay
  else {
    for (int j = 0; j < audio_rest_count; j++) {
      delayMicroseconds(audio_duration);  
    }                                
  }                                
}
