/*
    ''' Прога управления поворотниками и габаритами авто.

    Платформа нано.
    Использует сигналы с реле поворотников.
    На выходуе бегущая полоса или габариты.
    '''
*/

#include <Adafruit_NeoPixel.h> 
#include <TaskScheduler.h>

#define VERSION 01004
#define LED_STEP_DELAY 12   // скорость мигания поворота, по сути пауза между загораниями светодиодов;
#define NUM_LEDS 26         // количество светодиодов в поворотнике;
#define NUM_LEDS_GABARIT 26 // количество светодиодов в габарите;
#define PINOUT_LEFT 7       // номер выхода на левый поворот;
#define PINOUT_RIGHT 8      // номер выхода на правый поворот;
#define PININ_LEFT 3        // вход от реле для левого поворота;
#define PININ_RIGHT 6       // вход от реле для правого поворота;
#define STOP_DELAY 1000     // задержка отключения поворота ms;
#define LENGTH_LINE 8       // количество бегущих светодиодов;

// поворотники;
#define TURN_HUE 2000      // оттенок 0-65535;
#define TURN_SAT 255      // насыщенность 0-255 (0 - оттенки серого);
#define TURN_VAL 200      // яркость 0-255;

// габарит;
#define GABARIT_HUE 65000      // оттенок 0-65535;
#define GABARIT_SAT 0      // насыщенность 0-255 (0 - оттенки серого);
#define GABARIT_VAL 200      // яркость 0-255;

// набегающий свет при старте;
#define  RUNL_HUE 65000      // оттенок 0-65535;
#define  RUNL_SAT 255          // насыщенность 0-255 (0 - оттенки серого);
#define  RUNL_VAL 255        // яркость 0-255;
#define  RUN_SPEED 10        // по сути задержки между итерациями цикла;
#define  RUN_LENGTH 4        // длина бегущей линии;

#define  RUNR_HUE 65000      // оттенок 0-65535;
#define  RUNR_SAT 255          // насыщенность 0-255 (0 - оттенки серого);
#define  RUNR_VAL 255        // яркость 0-255;

// постоянный свет при старте;
#define  STARTL_HUE 20000      // оттенок 0-65535;
#define  STARTL_SAT 255          // насыщенность 0-255 (0 - оттенки серого);
#define  STARTL_VAL 255        // яркость 0-255;
#define  STARTL_DEL 1000        // время засвета;

#define  STARTR_HUE 20000      // оттенок 0-65535;
#define  STARTR_SAT 255          // насыщенность 0-255 (0 - оттенки серого);
#define  STARTR_VAL 255        // яркость 0-255;
#define  STARTR_DEL 1000        // время засвета;

// ================================ Task scheduler ============================================================;
#define _TASK_SLEEP_ON_IDLE_RUN

Scheduler ts;       // создаем обьект класса планировщик;

// прототипы функций задач;
void F_left();      // мигание левого поворота;
void F_right();     // мигание правого поворота;
void F_leftOn();    // обработка задержки выключения;
void F_rightOn();   // обработка задержки выключения;

// инициализация задач;
Task T_leftOn (STOP_DELAY, TASK_ONCE, &F_leftOn, &ts, true);
Task T_rightOn (STOP_DELAY, TASK_ONCE, &F_rightOn, &ts, true);
Task T_left (LED_STEP_DELAY, NUM_LEDS + LENGTH_LINE, &F_left, &ts, true);
Task T_right (LED_STEP_DELAY, NUM_LEDS + LENGTH_LINE, &F_right, &ts, true); 
// =============================================================================================================

// инициализация лент;
Adafruit_NeoPixel left_strp = Adafruit_NeoPixel(NUM_LEDS, PINOUT_LEFT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel right_strp = Adafruit_NeoPixel(NUM_LEDS, PINOUT_RIGHT, NEO_GRB + NEO_KHZ800);

  volatile int leftCount = 0;       // счетчики для мигания;
  volatile int rightCount = 0; 
  volatile bool f_leftIsOn = false; // состояние поворота;
  volatile bool f_rightIsOn = false;
  volatile bool f_leftIsFinished = true; // флаг отработки поворотом полного цикла;
  volatile bool f_rightIsFinished = true;

void setup() {
    pinMode(PININ_LEFT, INPUT_PULLUP);
    pinMode(PININ_RIGHT, INPUT_PULLUP);
    pinMode(PINOUT_LEFT, OUTPUT);
    pinMode(PINOUT_RIGHT, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    left_strp.begin();
    left_strp.show();
    right_strp.begin();
    right_strp.show();
    sei();

    // бегущий заполняющий огонь пристарте;
    left_strp.clear();
    right_strp.clear();
    for(int j = left_strp.numPixels(); j > 0; j -= RUN_LENGTH){
        for(int i = 0; i < j; i++) {
            left_strp.setPixelColor(i, left_strp.ColorHSV(RUNL_HUE, RUNL_SAT, RUNL_VAL));
            right_strp.setPixelColor(i, right_strp.ColorHSV(RUNR_HUE, RUNR_SAT, RUNR_VAL));
            if (i > RUN_LENGTH){
                int c = i - RUN_LENGTH;
                left_strp.setPixelColor(c - 1, left_strp.ColorHSV(RUNL_HUE, RUNL_SAT, RUNL_VAL/2));
                left_strp.setPixelColor(c - 2, left_strp.Color(0, 0, 0));
                right_strp.setPixelColor(c - 1, right_strp.ColorHSV(RUNR_HUE, RUNR_SAT, RUNR_VAL/2));
                right_strp.setPixelColor(c - 2, right_strp.Color(0, 0, 0));
            }
            left_strp.show();
            right_strp.show();
            delay(RUN_SPEED);
        }
    }

    // засвет одним цветом;
    left_strp.clear();
    right_strp.clear();
    left_strp.fill(left_strp.ColorHSV(STARTL_HUE, STARTL_SAT, STARTL_VAL));
    right_strp.fill(right_strp.ColorHSV(STARTR_HUE, STARTR_SAT, STARTR_VAL));
    left_strp.show();
    right_strp.show();
    delay(STARTL_DEL);
}

void loop() {
    ts.execute();   // обрабатываем планировщик каждый прогон;

// проверка левого поворота;
    if (digitalRead(PININ_LEFT) == LOW) {
        if (not f_leftIsOn) {
            f_leftIsOn = true; 
            f_leftIsFinished = false; 
            T_left.restart();
            T_leftOn.restartDelayed();
        }
        else {
            T_leftOn.restartDelayed();
        }
    }
// проверка правого поворота;
    if (digitalRead(PININ_RIGHT) == LOW) {
        if (not f_rightIsOn) {
            f_rightIsOn = true; 
            f_rightIsFinished = false; 
            T_right.restart();
            T_rightOn.restartDelayed();
        }
        else {
            T_rightOn.restartDelayed();
        }
    }
// иначе габарит;
    if((not f_rightIsOn) & (not f_leftIsOn) & f_leftIsFinished & f_rightIsFinished) { 
        for(int i = 0; i < NUM_LEDS_GABARIT;  i++) { 
            left_strp.setPixelColor(i, left_strp.gamma32(left_strp.ColorHSV(GABARIT_HUE, GABARIT_SAT, GABARIT_VAL)));
            right_strp.setPixelColor(i, right_strp.gamma32(right_strp.ColorHSV(GABARIT_HUE, GABARIT_SAT, GABARIT_VAL)));
        }
        left_strp.show();
        right_strp.show();
    }
}

void F_left(){
    if (leftCount < NUM_LEDS) {
            left_strp.setPixelColor(leftCount, left_strp.ColorHSV(TURN_HUE, TURN_SAT, TURN_VAL));    // устанавливаем цвет;
            left_strp.show();                                           // засвечиваем следующий светодиод; 
            leftCount++;
    }
    if (leftCount >= LENGTH_LINE){
            int c = leftCount - LENGTH_LINE + 2;
            left_strp.setPixelColor(c-2, left_strp.Color(0, 0, 0));     // плавно тушим свет;
            left_strp.setPixelColor(c-1, left_strp.ColorHSV(TURN_HUE, TURN_SAT, TURN_VAL/2));
            left_strp.setPixelColor(c, left_strp.ColorHSV(TURN_HUE, TURN_SAT, TURN_VAL*2/3));
            left_strp.show();                                           // в ленту; 
            if (leftCount >= NUM_LEDS){
                leftCount++;
            }
    }
    if (T_left.isLastIteration()) {
            leftCount = 0;
            if (f_leftIsOn){
                T_left.restart();
            }
            else {
                f_leftIsFinished = true;
            }
    }
}

void F_right(){
    if (rightCount < NUM_LEDS) {
            right_strp.setPixelColor(rightCount, right_strp.ColorHSV(TURN_HUE, TURN_SAT, TURN_VAL));    // устанавливаем цвет;
            right_strp.show();                                           // засвечиваем следующий светодиод; 
            rightCount++;
    }
    if (rightCount >= LENGTH_LINE){
            int c = rightCount - LENGTH_LINE + 2;
            right_strp.setPixelColor(c-2, right_strp.Color(0, 0, 0));     // плавно тушим свет;
            right_strp.setPixelColor(c-1, right_strp.ColorHSV(TURN_HUE, TURN_SAT, TURN_VAL/2));
            right_strp.setPixelColor(c, right_strp.ColorHSV(TURN_HUE, TURN_SAT, TURN_VAL*2/3));
            right_strp.show();                                                   // в ленту; 
            if (rightCount >= NUM_LEDS){
                rightCount++;
            }
    }
    if (T_right.isLastIteration()) {
            rightCount = 0;
            if (f_rightIsOn){
                T_right.restart();
            }
            else {
                f_rightIsFinished = true;
            }
    }
}

void F_leftOn() {
    f_leftIsOn = false;
}

void F_rightOn() {
    f_rightIsOn = false;
}

