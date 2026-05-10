// ======================================================
//                     LIBRARIES
// ======================================================
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <math.h>

// ======================================================
//                   PIN DEFINITIONS
// ======================================================
#define TFT_CS      5
#define TFT_RST     4
#define TFT_DC      2

#define BTN1_PIN    13
#define BTN2_PIN    25
#define BTN3_PIN    14

#define BUZZER_PIN  27
#define FSR_PIN     34

// ======================================================
//                    CONSTANTS
// ======================================================

// ======================================================
//                     DISPLAY OBJECT
// ======================================================
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


// ======================================================
//                    FACE CONSTANTS
// ======================================================
const int LEFT_CX   = 84;
const int RIGHT_CX  = 236;
const int EYE_CY    = 120;

const int EYE_RX    = 62;
const int EYE_RY    = 66;
const int EYE_TILT  = 12;

// ======================================================
//                    TIMER VARIABLES
// ======================================================
unsigned long pomodoroStartTime = 0;
unsigned long pomodoroDuration = 25UL * 60UL * 1000UL;
bool pomodoroRunning = false;
unsigned long postureStartTime = 0;
bool postureActive = false;
const unsigned long POSTURE_DURATION = 3000;
unsigned long lastBlinkTime = 0;
bool idleEyesClosed = false;

const unsigned long BLINK_INTERVAL = 2500;   // time between blinks
const unsigned long BLINK_DURATION = 180;    // how long eyes stay closed

// ======================================================
//                 INPUT / SENSOR VARIABLES
// ======================================================
bool lastBtn1State = HIGH;
bool lastBtn2State = HIGH;
bool lastBtn3State = HIGH;

int fsrValue = 0;

// ======================================================
//                  STATE DEFINITIONS
// ======================================================
enum RobotState {
  NO_PHONE,
  IDLE,
  FOCUS,
  BREAK_WARNING,
  BREAK,
  FOCUS_WARNING,
  RECALL,
  REWARD,
  BAD_POSTURE
};


// ======================================================
//                 GLOBAL VARIABLES
// ======================================================
RobotState currentState = NO_PHONE;

const unsigned long STUDY_DURATION = 10UL * 1000UL;  // 10 seconds (TEST)
const unsigned long BREAK_DURATION = 5UL * 1000UL;   // 5 seconds (TEST)

unsigned long rewardStartTime = 0;
const unsigned long REWARD_DURATION = 2000; // 2 seconds
bool rewardActive = false;

// ======================================================
//               FUNCTION DECLARATIONS
// ======================================================
void updateButtons();
void updateFSR();
void updateStateFromFSR();
void updateTimerTransitions();

void startFocusSession();
unsigned long getRemainingTime();

void drawCurrentScreen();
void drawTimerScreen();
void drawBreakWarningFace();

void beepShort();
void readSerialCommands();

void updatePostureState();

// your eye helpers
void drawTiltedEye(int, int, int, int, int, uint16_t);
void drawGlasses();


// ======================================================
//                        SETUP
// ======================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  currentState = NO_PHONE;

}



// ======================================================
//                         LOOP
// ======================================================
void loop() {
  updateButtons();
  updateFSR();
  updateStateFromFSR();
  updateTimerTransitions();

  readSerialCommands();
  updatePostureState();

  drawCurrentScreen();

  updateRewardState();

  delay(50);
}


// ======================================================
//               SERIAL COMMUNICATION
// ======================================================
void readSerialCommands(){
  if(Serial.available()){
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "POSTURE_BAD"){
      if (currentState == FOCUS){
        postureActive = true;
        postureStartTime = millis();
        currentState = BAD_POSTURE;
        beepShort();
        delay(200);
        beepShort();
        delay(200);
      }
      
    }
    else if (cmd == "REWARD_SHOW") {
    currentState = REWARD;
    rewardActive = true;
    rewardStartTime = millis();
  }

    else if(cmd == "RECALL_DONE"){
      currentState = IDLE;
      Serial.println("Recall finished, back to IDLE");
    }
  }
}
// ======================================================
//                  INPUT FUNCTIONS
// ======================================================
void updateButtons() {
  bool btn1State = digitalRead(BTN1_PIN);
  bool btn2State = digitalRead(BTN2_PIN);
  bool btn3State = digitalRead(BTN3_PIN);

  // Button 1 = confirm warnings
  if (lastBtn1State == HIGH && btn1State == LOW) {

    if (currentState == BREAK_WARNING) {
      startBreakSession();
      beepShort();
      delay(200);
      
         // simple debounce for now

    }

    else if (currentState == FOCUS_WARNING) {
      Serial.println("SESSION_DONE");
      startFocusSession();
      beepShort();
      delay(200);   // simple debounce for now
    }
  }

  // Button 2 = start study only from IDLE
  if (lastBtn2State == HIGH && btn2State == LOW) {
    if (currentState == IDLE) {
      startFocusSession();
      beepShort();
      delay(200);   // simple debounce for now
    }
  }

  // Button 3(recall)
  if (lastBtn3State == HIGH && btn3State == LOW) {

  if (currentState == IDLE || currentState == FOCUS) {
    currentState = RECALL;

    Serial.println("RECALL_REQUEST");
    Serial.println("State changed to RECALL");

    beepShort();
    delay(200);
  }
}

  lastBtn1State = btn1State;
  lastBtn2State = btn2State;
  lastBtn3State = btn3State;
}
void updateFSR() {
  fsrValue = analogRead(FSR_PIN);
  }

// ======================================================
//                  STATE FUNCTIONS
// ======================================================
void updateStateFromFSR(){
  int threshold = 1000;
  if (fsrValue < threshold){
    //No phone detected
    if(currentState != NO_PHONE){
        currentState = NO_PHONE;
        Serial.println("State changed to NO_PHONE");
    }
  } else{
    //Phone detected
    if (currentState == NO_PHONE){
      currentState = IDLE;
      Serial.println("State changed to IDLE");
    }
    
  }
}


unsigned long getRemainingTime() {
  if (!pomodoroRunning) {
    return pomodoroDuration;
  }

  unsigned long elapsed = millis() - pomodoroStartTime;

  if (elapsed >= pomodoroDuration) {
    return 0;
  }

  return pomodoroDuration - elapsed;
}

void updateTimerTransitions() {

  if (currentState == FOCUS && pomodoroRunning && getRemainingTime() == 0) {
    pomodoroRunning = false;
    currentState = BREAK_WARNING;

    beepShort();
    Serial.println("State changed to BREAK_WARNING");
  }

  else if (currentState == BREAK && pomodoroRunning && getRemainingTime() == 0) {
    pomodoroRunning = false;
    currentState = FOCUS_WARNING;

    beepShort();
    Serial.println("State changed to FOCUS_WARNING");
  }
}
void updateRewardState() {
  if (rewardActive && millis() - rewardStartTime >= REWARD_DURATION) {
    rewardActive = false;
    currentState = IDLE;   // clean return
  }
}

void updatePostureState() {
  if (postureActive && millis() - postureStartTime >= POSTURE_DURATION) {
    postureActive = false;
    currentState = FOCUS;

    Serial.println("Back to FOCUS");
  }
}


// ======================================================
//                  TIMER FUNCTIONS
// ======================================================
void startFocusSession() {
  pomodoroStartTime = millis();
  pomodoroDuration = STUDY_DURATION;
  pomodoroRunning = true;

  currentState = FOCUS;

  drawTimerScreen();

  Serial.println("FOCUS session started");
}

void startBreakSession(){
  pomodoroStartTime = millis();
  pomodoroDuration = BREAK_DURATION;
  pomodoroRunning = true;

  currentState = BREAK;

  Serial.println("BREAK started");
}

// ======================================================
//                 DISPLAY FUNCTIONS
// ======================================================
void drawTimerScreen() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setTextSize(2);

  if (currentState == FOCUS) {
    tft.setCursor(110, 60);
    tft.print("FOCUS");
  }

  else if (currentState == BREAK) {
    tft.setCursor(110, 60);
    tft.print("BREAK");
  }
}

void updateTimerText() {
  static int lastTotalSeconds = -1;

  unsigned long remaining = getRemainingTime();
  int totalSeconds = remaining / 1000;

  if (totalSeconds == lastTotalSeconds) return;
  lastTotalSeconds = totalSeconds;

  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  char timeText[6];
  sprintf(timeText, "%02d:%02d", minutes, seconds);

  tft.fillRect(35, 95, 250, 70, ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setTextSize(10);
  tft.setCursor(15, 90);
  tft.print(timeText);
}

void drawCurrentScreen() {
  static RobotState lastDrawnState = NO_PHONE;

  // Draw full screen only when the state changes
  if (currentState != lastDrawnState) {
    lastDrawnState = currentState;

    if (currentState == NO_PHONE) {
      tft.fillScreen(ILI9341_WHITE);
      tft.setTextColor(ILI9341_BLACK);
      tft.setTextSize(2);
      tft.setCursor(45, 110);
      tft.print("NO PHONE DETECTED");
    }

    else if (currentState == IDLE) {
      drawOpenFace();   // it has animation of blinking
    }

    else if (currentState == FOCUS) {
      drawTimerScreen();
    }

    else if (currentState == BREAK_WARNING) {
      drawBreakWarningFace();
    }

    else if (currentState == BREAK) {
      drawTimerScreen();
    }

    else if (currentState == FOCUS_WARNING) {
      drawStudyWarningFace();  
    }

    else if (currentState == RECALL) {
      drawListeningFace();
    }

    else if (currentState == REWARD) {
      drawRewardEyes();
    }

    else if (currentState == BAD_POSTURE) {
      drawWarningFace();   // posture warning face
    }
  }
    // Update idle animation only during IDLE
  if (currentState == IDLE) {
    updateIdleAnimation();
  }
  // Update only the changing parts
  if (currentState == FOCUS || currentState == BREAK) {
    updateTimerText();
  }

}

// ======================================================
//                   FACE FUNCTIONS
// ======================================================

void updateIdleAnimation() {
  unsigned long now = millis();

  if (!idleEyesClosed && now - lastBlinkTime >= BLINK_INTERVAL) {
    idleEyesClosed = true;
    lastBlinkTime = now;
    drawClosedFace();
  }

  else if (idleEyesClosed && now - lastBlinkTime >= BLINK_DURATION) {
    idleEyesClosed = false;
    lastBlinkTime = now;
    drawOpenFace();
  }
}

void drawWarningFace() {
  tft.fillScreen(ILI9341_RED);

  // SAME big eyes (no change)
  drawTiltedEye(LEFT_CX, EYE_CY, EYE_RX, EYE_RY-10, EYE_TILT, ILI9341_BLACK);
  drawTiltedEye(RIGHT_CX, EYE_CY, EYE_RX, EYE_RY-10, -EYE_TILT, ILI9341_BLACK);

   // reversed version: big shine dot on lower inner side
  tft.fillCircle(LEFT_CX + 17,  EYE_CY + 20, 17, ILI9341_WHITE);
  tft.fillCircle(RIGHT_CX - 17, EYE_CY + 20, 17, ILI9341_WHITE);


  // BIG THICK EYEBROWS
  drawBigEyebrows();

      // text on top
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

  tft.setCursor(25, 200);
  tft.print("SIT UP STRAIGHT");
}

void drawBigEyebrows() {

  // LEFT eyebrow (thick + angled inward)
  for (int i = -10; i <= 10; i++) {
    tft.drawLine(
      LEFT_CX -65,  EYE_CY - 40 + i,
      LEFT_CX - 10,  EYE_CY - 80 + i,
      ILI9341_BLACK
    );
  }

  // RIGHT eyebrow (mirror)
  for (int i = -10; i <= 10; i++) {
    tft.drawLine(
      RIGHT_CX + 65, EYE_CY - 40 + i,
      RIGHT_CX + 10, EYE_CY - 80 + i,
      ILI9341_BLACK
    );
  }
}

void drawStudyWarningFace() {
  tft.fillScreen(ILI9341_YELLOW);

  // regular panda eyes
  drawTiltedEye(LEFT_CX, EYE_CY, EYE_RX-15, EYE_RY-10, EYE_TILT, ILI9341_BLACK);
  drawTiltedEye(RIGHT_CX, EYE_CY, EYE_RX-15, EYE_RY-10, -EYE_TILT, ILI9341_BLACK);

  // regular shine dots
  tft.fillCircle(LEFT_CX + 17,  EYE_CY - 30, 10, ILI9341_WHITE);
  tft.fillCircle(RIGHT_CX - 17, EYE_CY - 30, 10, ILI9341_WHITE);

  
  // glasses

  drawGlasses();

    // text on top
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

  tft.setCursor(50, 15);
  tft.print("TIME TO STUDY");



}

void drawGlasses() {
  int r = 73;   // glasses radius

  // left glasses frame
  tft.drawCircle(LEFT_CX-10, EYE_CY, r, ILI9341_BLACK);
  tft.drawCircle(LEFT_CX-10, EYE_CY, r + 1, ILI9341_BLACK);

  // right glasses frame
  tft.drawCircle(RIGHT_CX+10, EYE_CY, r, ILI9341_BLACK);
  tft.drawCircle(RIGHT_CX+10, EYE_CY, r + 1, ILI9341_BLACK);

  // bridge between glasses
  for (int i = -2; i <= 2; i++) {
    tft.drawLine(
      LEFT_CX-10 + r,
      EYE_CY + i,
      RIGHT_CX+10 - r,
      EYE_CY + i,
      ILI9341_BLACK
    );
  }

  

}

void drawListeningFace(){
  tft.fillScreen(ILI9341_WHITE);

  drawTiltedEye(LEFT_CX, EYE_CY, EYE_RX, EYE_RY, EYE_TILT, ILI9341_BLACK);
  drawTiltedEye(RIGHT_CX, EYE_CY, EYE_RX, EYE_RY, -EYE_TILT, ILI9341_BLACK);

  // reversed version: big shine dot on lower inner side
  tft.fillCircle(LEFT_CX + 17,  EYE_CY + 20, 17, ILI9341_WHITE);
  tft.fillCircle(RIGHT_CX - 17, EYE_CY + 20, 17, ILI9341_WHITE);

  // reversed version: big shine dot on lower inner side
  tft.fillCircle(LEFT_CX-8,  EYE_CY +40, 10, ILI9341_WHITE);
  tft.fillCircle(RIGHT_CX+8, EYE_CY +40, 10, ILI9341_WHITE);

  // reversed version: big shine dot on lower inner side
  tft.fillCircle(LEFT_CX -7,  EYE_CY + 22, 3, ILI9341_WHITE);
  tft.fillCircle(RIGHT_CX +7, EYE_CY + 22, 3, ILI9341_WHITE);

  
}

void drawOpenFace() {
  tft.fillScreen(ILI9341_WHITE);

  drawTiltedEye(LEFT_CX, EYE_CY, EYE_RX, EYE_RY, EYE_TILT, ILI9341_BLACK);
  drawTiltedEye(RIGHT_CX, EYE_CY, EYE_RX, EYE_RY, -EYE_TILT, ILI9341_BLACK);

  // reversed version: big shine dot on lower inner side
  tft.fillCircle(LEFT_CX + 17,  EYE_CY + 20, 17, ILI9341_WHITE);
  tft.fillCircle(RIGHT_CX - 17, EYE_CY + 20, 17, ILI9341_WHITE);
}

void drawClosedFace() {
  tft.fillScreen(ILI9341_WHITE);

  drawClosedEye(LEFT_CX, EYE_CY, true);
  drawClosedEye(RIGHT_CX, EYE_CY, false);
}

void drawRewardEyes() {
  int w = 20;   // width of each >< shape
  int h = 12;    // height
  int t = 4;    // thickness
  uint16_t hotPink = tft.color565(255, 20, 120);
  tft.fillScreen(hotPink);


  drawTiltedEye(LEFT_CX, EYE_CY, EYE_RX, EYE_RY, EYE_TILT, ILI9341_BLACK);
  drawTiltedEye(RIGHT_CX, EYE_CY, EYE_RX, EYE_RY, -EYE_TILT, ILI9341_BLACK);

  // centers where the shine dots used to be
  int lx = LEFT_CX + 17;
  int ly = EYE_CY + 20;

  int rx = RIGHT_CX - 17;
  int ry = EYE_CY + 20;

  for (int i = 0; i < t; i++) {
    // LEFT eye >
    tft.drawLine(lx - w, ly - h + i, lx + w, ly + i, ILI9341_WHITE);
    tft.drawLine(lx - w, ly + h + i, lx + w, ly + i, ILI9341_WHITE);

    // RIGHT eye <
    tft.drawLine(rx + w, ry - h + i, rx - w, ry + i, ILI9341_WHITE);
    tft.drawLine(rx + w, ry + h + i, rx - w, ry + i, ILI9341_WHITE);
  }
  
}

void drawBreakWarningFace() {
  int w = 20;   // width 
  int t = 4;    // thickness

  tft.fillScreen(ILI9341_YELLOW);

  drawTiltedEye(LEFT_CX, EYE_CY, EYE_RX, EYE_RY, EYE_TILT, ILI9341_BLACK);
  drawTiltedEye(RIGHT_CX, EYE_CY, EYE_RX, EYE_RY, -EYE_TILT, ILI9341_BLACK);

  // centers where the shine dots used to be
  int lx = LEFT_CX + 17;
  int ly = EYE_CY + 20;

  int rx = RIGHT_CX - 17;
  int ry = EYE_CY + 20;

  for (int i = 0; i < t; i++) {
    // LEFT eye
    tft.drawLine(lx - w, ly + i, lx + w, ly + i, ILI9341_WHITE);

    // RIGHT eye
    tft.drawLine(rx + w, ry  + i, rx - w, ry + i, ILI9341_WHITE);

  }
      // text on top
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

  tft.setCursor(50, 15);
  tft.print("TAKE A BREAK");
}

void drawTiltedEye(int cx, int cy, int rx, int ry, int tilt, uint16_t color) {
  for (int y = -ry; y <= ry; y++) {
    float yn = (float)y / (float)ry;
    float halfWidth = rx * sqrt(1.0 - yn * yn);
    float shift = -(float)tilt * yn;

    int x1 = (int)(cx + shift - halfWidth);
    int x2 = (int)(cx + shift + halfWidth);
    int yy = cy + y;

    tft.drawLine(x1, yy, x2, yy, color);
    tft.drawLine(x1, yy + 1, x2, yy + 1, color);
  }

  // soften top and bottom slightly
  tft.fillCircle(cx, cy - ry + 7, 7, color);
  tft.fillCircle(cx, cy + ry - 7, 7, color);
}

void drawClosedEye(int cx, int cy, bool isLeft) {
  int x1, y1, x2, y2;

  if (isLeft) {
    x1 = cx - 42;
    y1 = cy + 2;
    x2 = cx + 42;
    y2 = cy - 2;
  } else {
    x1 = cx - 42;
    y1 = cy - 2;
    x2 = cx + 42;
    y2 = cy + 2;
  }

  tft.drawLine(x1, y1, x2, y2, ILI9341_BLACK);
  tft.drawLine(x1, y1 + 1, x2, y2 + 1, ILI9341_BLACK);
  tft.drawLine(x1, y1 - 1, x2, y2 - 1, ILI9341_BLACK);
  tft.drawLine(x1, y1 + 2, x2, y2 + 2, ILI9341_BLACK);
}

// ======================================================
//                  SOUND FUNCTIONS
// ======================================================
void beepShort() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}