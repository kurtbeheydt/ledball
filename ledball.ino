#include <Adafruit_NeoPixel.h>

#define TRIGGER1  3
#define ECHO1     2
#define STATUS_1  4
#define TRIGGER2  10
#define ECHO2     9
#define STATUS_2  5

#define BASE_DIST 1500 // in ms

#define LED_PIN   6
#define LED_COUNT 100
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ400);
int middleLed = LED_COUNT / 2;
const int fieldZones = LED_COUNT / 10;
const int startingPoints = 5;

bool ready_player1 = false;
bool input_player1 = false;
bool newInput_player1 = false;
int score_player1 = startingPoints;
bool ready_player2 = false;
bool input_player2 = false;
bool newInput_player2 = false;
int score_player2 = startingPoints;

int scoringPlayer;
int ball_position = middleLed;
bool ball_direction = true;

unsigned long currentMillis;
unsigned long previousMillisPlaying = 0;
const long ballIntervalStart = (LED_COUNT > 50) ? 40 : 70;
unsigned long ballInterval = ballIntervalStart;

unsigned long previousMillisPlayerCheck = 0;
const long playerInterval = 100;
const int triggerDelay = 3;

enum State {STATE_PLAYINGSHOWINGSCORE, STATE_PLAYINGCOUNTDOWN, STATE_PLAYINGGAME, STATE_PLAYINGPOINT, STATE_PLAYINGWINNER, STATE_WAITINGFORPLAYERS };
State state = STATE_WAITINGFORPLAYERS;

int i;
 
void setup() {
  pinMode(TRIGGER1, OUTPUT);
  digitalWrite(TRIGGER1, LOW);
  pinMode(ECHO1, INPUT);
  pinMode(STATUS_1, OUTPUT);

  pinMode(TRIGGER2, OUTPUT);
  digitalWrite(TRIGGER2, LOW);
  pinMode(ECHO2, INPUT);
  pinMode(STATUS_2, OUTPUT);
  
  strip.begin();
  strip.show();
  strip.setBrightness(80);

  randomSeed(analogRead(0));
}

void loop() {
  currentMillis = millis();
  
  checkInput();
  
  if (state == STATE_WAITINGFORPLAYERS) {
    checkNewPlayer();
    
    if (ready_player1 && ready_player2) {
      score_player1 = startingPoints;
      score_player2 = startingPoints;
      scoringPlayer = 0;
      ballInterval = ballIntervalStart;
      state = STATE_PLAYINGCOUNTDOWN;
    }

    if (currentMillis - previousMillisPlaying >= ballInterval) {
      previousMillisPlaying = currentMillis;
      ball_position = random(LED_COUNT);
    }
  }

  if (state == STATE_PLAYINGCOUNTDOWN) {
    ballInterval = ballIntervalStart;
    if (scoringPlayer == 1) {
      ball_direction = true;
      ball_position = middleLed - 1;
    } else if (scoringPlayer == 2) {
      ball_direction = false;
      ball_position = middleLed;
    } else {
      ball_direction = (random(2) >= 1);
      ball_position = (ball_direction) ? (2 * LED_COUNT/10) : (LED_COUNT - 1 - (2 * LED_COUNT/10));
    }
  }
  if (state == STATE_PLAYINGGAME) {
    if (currentMillis - previousMillisPlaying >= ballInterval) {
      previousMillisPlaying = currentMillis;

      checkPlayerMove();
      moveBall();
    }
  }
  if (state == STATE_PLAYINGPOINT) {
    if ((score_player1 <= 0) || (score_player2 <= 0)) {
      state = STATE_PLAYINGWINNER;
      ballInterval = ballIntervalStart;
      ready_player1 = false;
      ready_player2 = false;
    }
  }

  drawStrip();
}

void checkInput() {
  int duration_player1, duration_player2;

  if (currentMillis - previousMillisPlayerCheck >= playerInterval) {
    previousMillisPlayerCheck = currentMillis;
    digitalWrite(TRIGGER1, HIGH);
    delayMicroseconds(triggerDelay);
    digitalWrite(TRIGGER1, LOW);
    duration_player1 = pulseIn(ECHO1, HIGH);
    input_player1 = (duration_player1 <= BASE_DIST) ? true : false;
    digitalWrite(STATUS_1, input_player1);
    if (!input_player1) {
      newInput_player1 = true;
    }
    digitalWrite(TRIGGER2, HIGH);
    delayMicroseconds(triggerDelay);
    digitalWrite(TRIGGER2, LOW);
    duration_player2 = pulseIn(ECHO2, HIGH);
    input_player2 = (duration_player2 <= BASE_DIST) ? true : false;
    digitalWrite(STATUS_2, input_player2);
    if (!input_player2) {
      newInput_player2 = true;
    }
  }
}

void moveBall() {
  ball_position = (ball_direction) ? (ball_position + 1) :  (ball_position - 1);
  if (ball_position >= LED_COUNT) { // player one scores
    scoringPlayer = 1;
    score_player2--;
    state = STATE_PLAYINGPOINT;
  }
  if (ball_position < 0) { // player two scores
    scoringPlayer = 2;
    score_player1--;
    state = STATE_PLAYINGPOINT;
  }
}

void checkNewPlayer() {
  if (input_player1 && newInput_player1) {
    newInput_player1 = false;
    ready_player1 = true;
    digitalWrite(STATUS_1, LOW);
  }
  if (input_player2 && newInput_player2) {
    newInput_player2 = false;
    ready_player2 = true;
    digitalWrite(STATUS_2, LOW);
  }
}

void checkPlayerMove() {
  if (input_player1 && newInput_player1) {
    newInput_player1 = false;
    if (ball_position < (2 * fieldZones)) {
      ball_direction = true;      
    }
    if (ball_position < fieldZones) {
      ballInterval = ballInterval - (ballInterval / 4);
    }
  }
  if (input_player2 && newInput_player2) {
    newInput_player2 = false;
    if (ball_position >= (LED_COUNT - 2 * fieldZones)) {
      ball_direction = false;      
    }
    if (ball_position >= (LED_COUNT - fieldZones)) {
      ballInterval = ballInterval - (ballInterval / 4);
    }
  }
}

void drawStrip() {
  strip.clear();
  if (state == STATE_WAITINGFORPLAYERS) {
    if (ready_player1) {
      for (i = 0; i < middleLed; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
      }
    } else if (ready_player2) {
      for (i = middleLed; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
      }    
    } else {
      int pixelHue = ((ball_position + 1) * 65536L / LED_COUNT);
      strip.setPixelColor(ball_position, strip.ColorHSV(pixelHue));
    }
  }
  if (state == STATE_PLAYINGSHOWINGSCORE) {
    for (int j = 0; j < 1; j++) {
      for (i = score_player1; i > 0; i--) {
        strip.setPixelColor(middleLed - (2 * i) - 5, strip.Color(255, 0, 0));
      }
      for (i = score_player2; i > 0; i--) {
        strip.setPixelColor(middleLed + (2 * i) + 4, strip.Color(255, 0, 0));
      }

      strip.show();
      delay(1700);
      strip.clear();
      strip.show();
      delay(300);
    }
    
    state = STATE_PLAYINGCOUNTDOWN;
  } else if (state == STATE_PLAYINGCOUNTDOWN) {
    for (int j = 0; j < 3; j++) {
      for (i = (0 + (j * LED_COUNT/5)); i < (LED_COUNT - (j * LED_COUNT/5)); i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
      strip.show();
      delay(600);
      strip.clear();
      strip.show();
      delay(400);
    }
    state = STATE_PLAYINGGAME;
  } else if (state == STATE_PLAYINGGAME) {
    // draw field
    for (i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 50));
    }
    for (i = 0; i < (fieldZones); i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
      strip.setPixelColor(i + (fieldZones), strip.Color(255, 255, 0));
      strip.setPixelColor((LED_COUNT - 1) - i, strip.Color(255, 0, 0));
      strip.setPixelColor((LED_COUNT - 1) - (i + (fieldZones)), strip.Color(255, 255, 0));
    }
    // draw ball
    strip.setPixelColor(ball_position, strip.Color(255, 255, 255));
  } else if (state == STATE_PLAYINGPOINT) {
    for (int j = 0; j < 6; j++) {
      if (scoringPlayer == 1) {
        for (i = 0; i < (middleLed); i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 255));
        }
      } else if (scoringPlayer == 2) {
        for (i = middleLed; i < LED_COUNT; i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 255));
        }
      }
      strip.show();
      delay(300);
      strip.clear();
      strip.show();
      delay(300);
    }
    state = STATE_PLAYINGSHOWINGSCORE;
  } else if (state == STATE_PLAYINGWINNER) {
    for (int j = 0; j < 4; j++) {
      for (i = 0; i < (LED_COUNT/2); i++) {
        int pixelHue = (i * 65536L / (LED_COUNT/2));
        if (scoringPlayer == 1) {
          strip.setPixelColor(i, strip.ColorHSV(pixelHue));
        } else if (scoringPlayer == 2) {
          strip.setPixelColor((LED_COUNT - 1) - i, strip.ColorHSV(pixelHue));
        }
        strip.show();
        delay(20);
      }
      delay(300);
      strip.clear();
      strip.show();
      delay(300);
    }
    state = STATE_WAITINGFORPLAYERS;
  }
  
  strip.show();
}
