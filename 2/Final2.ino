#define MAP(x, in_min, in_max, out_min, out_max)\
  ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
#define DEG_TO_US(deg) ((deg * 1000ul + 180ul * 1000ul) / 180ul)

#define PROD_A  A3
#define PROD_B  A2
#define MIXER_A A1
#define MIXER_B A0

#define VALVE   0
#define HEATER  1
#define ARM     2
#define CAKE_A  3
#define CAKE_B  4
#define RASP    5
#define PINE    6

#define KEYPAD_0      8
#define KEYPAD_1      9
#define KEYPAD_ACTIVE 10

#define CLOSE         0ul
#define OPEN          180ul
#define OUTER         0ul
#define MIDDLE        90ul
#define INNER         180ul

struct Stepper {
  byte pinA, pinB;
  byte speed;       // rev/sec
  byte steps;
  byte step_phase;
  short step_number;
  unsigned long last_step_time;
  Stepper(byte _pinA, byte _pinB, short _steps, byte _speed);
  void move();
};
Stepper mixer = { MIXER_A, MIXER_B, 32, 10 };
Stepper prod = { PROD_A, PROD_B, 32, 1 };
Stepper cake = { CAKE_A, CAKE_B, 56, 1 };

struct Servo {
  byte pin;
  void move(unsigned long deg);
};
Servo valve = { VALVE };
Servo arm = { ARM };

bool buttonState = false;
void decorateCake(byte decoration);

void setup() {
  DDRC |= 0b1111;
  DDRD |= 0b1111111;
  DDRB &= ~(0b111);
}

void loop() {
  /// Make the cake
  // Open the eggs valve for 500ms
  valve.move(OPEN);
  delay(500);
  valve.move(CLOSE);

  // Start the mixer for 5 rotations
  while (mixer.step_number < 5 * mixer.steps) 
    mixer.move();
  mixer.step_number = 0;

  // Open the vanilla valve for 100ms
  // Keep rotating the mixer for 4 more rotations
  unsigned long now = millis();
  valve.move(OPEN);
  while (mixer.step_number < 4 * mixer.steps) {
    mixer.move();
    if (millis() - now >= 100)
      valve.move(CLOSE);
  }
  mixer.step_number = 0;

  // Open the sugar valve for 200ms
  valve.move(OPEN);
  delay(200);
  valve.move(CLOSE);

  // Make another 10 mixer rotations
  while (mixer.step_number < 10 * mixer.steps) 
    mixer.move();
  mixer.step_number = 0;
  mixer.speed /= 2;

  // Do the following three times
  for (byte t = 0; t < 3; t++) {
    // Open the flour valve for 100ms
    valve.move(OPEN);
    delay(100);
    valve.move(CLOSE);

    // Make 4 mixer rotations
    while (mixer.step_number < 4 * mixer.steps) 
      mixer.move();
    mixer.step_number = 0;
  }

  /// Bake the cake
  // Move the production line one full rotation
  while (prod.step_number < prod.steps)
    prod.move();
  prod.step_number = 0;

  // Light the heater for 1000ms
  digitalWrite(HEATER, HIGH);
  delay(1000);
  digitalWrite(HEATER, LOW);

  // Move the production line again for one full rotation
  while (prod.step_number < prod.steps)
    prod.move();
  prod.step_number = 0;

  // Stop everything for 1000ms
  delay(1000);

  /// Decorate the cake
  // Read input from user
  while (!digitalRead(KEYPAD_ACTIVE));
  byte decoration = PINB & 0b11;
  decorateCake(decoration);
}

Stepper::Stepper(byte _pinA, byte _pinB, short _steps, byte _speed) {
  pinA = _pinA;
  pinB = _pinB;
  steps = _steps;
  speed = _speed;
  last_step_time = 0;
  step_number = 0;
  step_phase = 0;
}

void Stepper::move() {
  // Skip step if not enough time has passed
  if ((millis() - last_step_time) * speed * steps < 1000)
    return;

  switch (step_phase) {
  case 0:
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
    break;

  case 1:
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, HIGH);
    break;

  case 2:
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
    break;

  case 3:
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    break;
  }

  last_step_time = millis();
  step_number++;
  step_phase = (step_phase+1) % 4;
}

void Servo::move(unsigned long deg) {
  unsigned long pulse = DEG_TO_US(deg);
  digitalWrite(pin, HIGH);
  delayMicroseconds(pulse);
  digitalWrite(pin, LOW);
}

void decorateCake(byte decoration) {
  switch (decoration) {
  case 0:
    arm.move(OUTER);                      // Move to outer sector
    digitalWrite(RASP, HIGH);             // Open raspberry valve
    while (cake.step_number < cake.steps) // Move cake stand one full rotation
      cake.move();
    cake.step_number = 0;
    digitalWrite(RASP, LOW);              // Close raspberry valve
    delay(300);

    arm.move(MIDDLE);                     // Move to middle sector
    digitalWrite(PINE, HIGH);             // Open pineapple valve
    while (cake.step_number < cake.steps) // Move cake stand one full rotation
      cake.move();
    cake.step_number = 0;
    digitalWrite(PINE, LOW);                  // Close pineapple valve
    break;

  case 1:
    for (byte r = 0; r < 4; r++) {          
      arm.move(OUTER);                        // Move to outer sector
      digitalWrite(RASP, HIGH);               // Open raspberry valve
      while (cake.step_number < cake.steps/8) // Move cake stand 1/8 rotation
        cake.move();                  
      cake.step_number = 0;
      digitalWrite(RASP, LOW);                // Close raspberry valve
      delay(300);

      arm.move(MIDDLE);                       // Move to middle sector
      digitalWrite(PINE, HIGH);               // Open pineapple valve
      while (cake.step_number < cake.steps/8) // Move cake stand 1/8 rotation
        cake.move();
      cake.step_number = 0;
      digitalWrite(PINE, LOW);                // Close pineapple valve
      delay(300);
    }
    break;

  case 2:
    arm.move(OUTER);                           // Move to outer sector
    for (byte r = 0; r < 14; r++) {
      while (cake.step_number < cake.steps/14) // Move cake stand 1/14 rotation
        cake.move();
      cake.step_number = 0;
      digitalWrite(RASP, HIGH);                // Open raspberry valve
      delay(300);
      digitalWrite(RASP, LOW);                 // Close raspberry valve
      delay(300);
    }
    arm.move(INNER);                           // Move to inner sector
    digitalWrite(PINE, HIGH);                  // Open pineapple valve
    delay(300);
    digitalWrite(PINE, LOW);                   // Close pineapple valve
    break;

  case 3:
    arm.move(OUTER);                           // Move to outer sector
    for (byte r = 0; r < 14; r++) {
      while (cake.step_number < cake.steps/14) // Move cake stand 1/14 rotation
        cake.move();
      cake.step_number = 0;
      digitalWrite(r % 2 ? PINE : RASP, HIGH); // Open respective valve
      delay(300);
      digitalWrite(r % 2 ? PINE : RASP, LOW);  // Close respective valve
      delay(300);
    }
    digitalWrite(RASP, HIGH);                  // Open pineapple valve
    // Move gradually towards the center
    for (unsigned long deg = MIDDLE; deg < INNER; deg += 10ul) {
      arm.move(deg);                           // Move to respective sector
      while (cake.step_number < cake.steps)    // Move cake stand one full rotation
        cake.move();
      cake.step_number = 0;
    }
    break;
  }
  arm.move(OUTER); // Move to outer sector
}
