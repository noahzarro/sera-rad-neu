//Version: Holzrad Test für Trudelhaus 30.6.20
// define pin numbers
#define GND_PIN 7
#define PWM_PIN 6
#define REED_PIN 3
// set new speed
#define SPEED_DEFAULT 255
#define SPEED_CHANGE_DELAY 100 //change speed every SPEED_CHANGE_DELAY milliseconds about SPEED_CHANGE towards speed_target
#define SPEED_CHANGE 20
#define SPEED_RANDOM 0 //speed_target = SPEED_DEFAULT + random(-SPEED_RANDOM, + SPEED_RANDOM)
#define SPEED_STOP 255 //Minimaler Speed, bei welchem der Motor sich noch bewegen kann
// max time allowed to stand still for safety reasons in ms
#define MAX_NOT_MOVE_TIME 100000 //Falls er X Sekunden lang keine Umdrehung schafft, wird speed erhöht
// set subtarget
#define SUBTARGET_DISTANCE 1
#define SUBTARGET_RANDOM 0
// set target
#define TARGET_MIN_DISTANCE 1 // minimum distance between two targets
#define TARGET_MAX_DISTANCE 4 // maximum distance between two targets (hat Serafin gecodet)
// debug
#define DEBUG_SPEED_CHANGE 16    //Veränderungswert bei ein mal > oder < Drücken im App
#define DEBUG_SLOW_DOWN_DELAY 80 //Bremsgeschw. im Manuell-Modus
#define DEBUG_SLOW_DOWN_CHANGE 9 //Bremsgeschw. im Manuell-Modus
// take a break
#define TIME_RANDOM_MIN 2000 //Minimale Pausenlänge zwischen Fahrsequenzen
#define TIME_RANDOM 80000 //Maximale Pausenlänge zwischen Fahrsequenzen
// debounce
#define TIME_DEBOUNCE 35 //Wert um Reed-Sensor zu optimieren

// position variables
volatile int position;
enum directions
{
  FORWARD, // increasing position
  BACKWARD // decreasing position
};
int speed = 255;
directions direction = FORWARD;
bool changed = true; // did speed or direction change?
int break_time;
int limit = 17;
int target = 0;    // position to reach
int subtarget = 0; // next time to change speed
int speed_target = SPEED_DEFAULT;
unsigned long last_speed_change = 0;
unsigned long last_position_change = 0;

// debounce
unsigned long first_time_rising = 0;
bool incremented = true;

// control variables
bool debug_mode = false; // is it controlled manually?
unsigned long continue_at = 0;
bool slow_down = false;
unsigned long last_time_slowed = 0;

void setup()
{
  // initialize pins
  pinMode(GND_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(REED_PIN, INPUT_PULLUP);

  // register interrupts
  attachInterrupt(digitalPinToInterrupt(REED_PIN), reed_count, FALLING);
}

void loop()
{
  // check if target is reached
  if ((direction == FORWARD and position >= target) or (direction == BACKWARD and position <= target))
  {

    // maybe take a break
    break_time = random(TIME_RANDOM_MIN, TIME_RANDOM_MAX);
    continue_at = millis() + break_time;

    // do not take a break at start
    if (target == 0)
    {
      continue_at = millis();
    }

    // set new target
    // try to find a new target that is at least TARGET_MIN_DISTANCE away
    int old_target = target;
    while (abs(old_target - target) < TARGET_MIN_DISTANCE)
    {
      target = random(old_target - TARGET_MAX_DISTANCE, old_target + TARGET_MAX_DISTANCE);
    }

    if (target < 0)
    {
      target = random(0, TARGET_MAX_DISTANCE);
    }

    if (target > limit)
    {
      target = random(limit - TARGET_MAX_DISTANCE, limit);
    }

    // set direction
    if (target < position)
    {
      if (!digitalRead(REED_PIN) and direction == FORWARD)
      {
        position--;
        last_position_change = millis();
      }
      direction = BACKWARD;
    }
    else
    {
      if (!digitalRead(REED_PIN) and direction == BACKWARD)
      {
        position++;
        last_position_change = millis();
      }
      direction = FORWARD;
    }

    changed = true;
  }


  // check if at break
  if (millis() > continue_at)
  {
    // check if speed/direction must be changed
    if (changed)
    {
      // set speed and direction
      if (direction == FORWARD)
      {
        digitalWrite(GND_PIN, LOW);
        analogWrite(PWM_PIN, speed);
      }
      else // invert both pins
      {
        digitalWrite(GND_PIN, HIGH);
        analogWrite(PWM_PIN, 255 - speed);
      }
    }
  }
  else
  {
    digitalWrite(GND_PIN, LOW);
    analogWrite(PWM_PIN, 0);
  }

  // check if debounce time is over and reed still high
  if (first_time_rising + TIME_DEBOUNCE < millis())
  {
    if (!incremented && !digitalRead(REED_PIN))
    {
      if (direction == FORWARD)
      {
        position++;
        last_position_change = millis();
      }
      else
      {
        position--;
        last_position_change = millis();
      }
      incremented = true;
    }
  }

  /*
  Serial.print("D: ");
  Serial.print(direction);
  Serial.print(" ,T: ");
  Serial.print(target);
  Serial.print(" ,U: ");
  Serial.print(subtarget);
  Serial.print(" ,S: ");
  Serial.print(speed);
  Serial.print(", P: ");
  Serial.println(position);
*/

  Serial.print("D: ");
  Serial.print(direction);
  Serial.print(" ,T: ");
  Serial.print(target);
  Serial.print(" ,U: ");
  Serial.print(subtarget);
  Serial.print(" ,S: ");
  Serial.print(speed);
  Serial.print(", P: ");
  Serial.print(position);
  Serial.print(", Br: ");
  Serial.println(break_time);

  //Serial.println(digitalRead(REED_PIN));
}

// ISR
void reed_count()
{
  if (first_time_rising + TIME_DEBOUNCE < millis())
  {
    if (incremented)
    {
      first_time_rising = millis();
      incremented = false;
    }
  }
}
