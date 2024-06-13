/*
    ''' Прога управления поворотниками и габаритами авто.

    Платформа нано.
    Использует сигналы с реле поворотников.
    На выходуе бегущая светодиодная лента или габариты.
    '''
*/

#include <Adafruit_NeoPixel.h> 
#include <TaskScheduler.h>

#define VERSION 01001
#define LED_STEP_DELAY 12   // скорость мигания поворота, по сути пауза между загораниями светодиодов;
#define NUM_LEDS 44         // количество светодиодов в поворотнике;
#define NUM_LEDS_GABARIT 26 // количество светодиодов в габарите;
#define PINOUT_LEFT 7       // номер выхода на левый поворот;
#define PINOUT_RIGHT 8      // номер выхода на правый поворот;
#define PININ_LEFT 3        // вход от реле для левого поворота;
#define PININ_RIGHT 6       // вход от реле для правого поворота;
#define STOP_DELAY 1000     // задержка отключения поворота;
#define LENGTH_LINE 8       // количество бегущих светодиодов;
#define GABARIT_R 200       // габарит красный;
#define GABARIT_G 200       // габарит зеленый;
#define GABARIT_B 200       // габарит синий;

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

    for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
      for(int i = 0; i < left_strp.numPixels(); i++) { 
        int pixelHue = firstPixelHue + (i * 65536L / left_strp.numPixels());
        left_strp.setPixelColor(i, left_strp.gamma32(left_strp.ColorHSV(pixelHue)));
      }
      left_strp.show();
      delay(1000);
    }
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
            left_strp.setPixelColor(i, left_strp.Color(GABARIT_R, GABARIT_G, GABARIT_B));
            right_strp.setPixelColor(i, right_strp.Color(GABARIT_R, GABARIT_G, GABARIT_B));
        }
        left_strp.show();
        right_strp.show();
    }
}

void F_left(){
    if (leftCount < NUM_LEDS) {
            left_strp.setPixelColor(leftCount, left_strp.Color(255, 69, 0));    // устанавливаем цвет;
            left_strp.show();                                           // засвечиваем следующий светодиод; 
            leftCount++;
    }
    if (leftCount >= LENGTH_LINE){
            int c = leftCount - LENGTH_LINE + 2;
            left_strp.setPixelColor(c-2, left_strp.Color(0, 0, 0));     // плавно тушим свет;
            left_strp.setPixelColor(c-1, left_strp.Color(63, 17, 0));
            left_strp.setPixelColor(c, left_strp.Color(127, 34, 0));
            left_strp.show();                                                   // в ленту; 
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
            right_strp.setPixelColor(rightCount, right_strp.Color(255, 69, 0));    // устанавливаем цвет;
            right_strp.show();                                           // засвечиваем следующий светодиод; 
            rightCount++;
    }
    if (rightCount >= LENGTH_LINE){
            int c = rightCount - LENGTH_LINE + 2;
            right_strp.setPixelColor(c-2, right_strp.Color(0, 0, 0));     // плавно тушим свет;
            right_strp.setPixelColor(c-1, right_strp.Color(63, 17, 0));
            right_strp.setPixelColor(c, right_strp.Color(127, 34, 0));
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

