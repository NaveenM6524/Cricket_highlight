#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* HTTPS PIPEDREAM URL */
const char* pipedreamURL =
"https://eoqtyfj75xg5ypx.m.pipedream.net";

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DOT_BTN 18
#define SINGLE_BTN 19
#define ATTACK_BTN 23

#define LED_PIN 5
#define BUZZER 4

int totalRuns = 0;
int ballsLeft = 120;
int wickets = 0;

int singles = 0;
int dotBalls = 0;          
int lowRunStreak = 0;
int consecutiveWickets = 0;

void processEvent(String event, int runs);
void sendHighlight(String message);
void monitorStream(String event);
void updateLCD();

void setup() {

  Serial.begin(115200);

  pinMode(DOT_BTN, INPUT_PULLUP);
  pinMode(SINGLE_BTN, INPUT_PULLUP);
  pinMode(ATTACK_BTN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();

  randomSeed(micros());

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected ✅");

  sendHighlight("Match Started");

  updateLCD();
}

void loop() {

  if (ballsLeft <= 0) {
    sendHighlight("Match Finished");
    while (true);
  }

  if (digitalRead(DOT_BTN) == LOW) {
    processEvent("DOT BALL", 0);
    delay(600);
  }

  if (digitalRead(SINGLE_BTN) == LOW) {
    processEvent("SINGLE", 1);
    delay(600);
  }

  if (digitalRead(ATTACK_BTN) == LOW) {

    int r = random(10);

    if (r < 2)
      processEvent("WICKET", 0);
    else if (r < 6)
      processEvent("FOUR", 4);
    else
      processEvent("SIX", 6);

    delay(800);
  }
}

void sendHighlight(String message) {

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  https.begin(client, pipedreamURL);
  https.addHeader("Content-Type", "application/json");

  String json =
    "{\"highlight\":\"" + message + "\"}";

  int code = https.POST(json);

  Serial.print("Highlight Sent → HTTP Code: ");
  Serial.println(code);

  https.end();

  delay(300);
}

void monitorStream(String event) {

  int ballsPlayed = 120 - ballsLeft;
  int overs = ballsPlayed / 6;
  int balls = ballsPlayed % 6;

  Serial.println("\n===== MATCH STREAM =====");

  Serial.print("Event: ");
  Serial.println(event);

  Serial.print("Score: ");
  Serial.print(totalRuns);
  Serial.print("/");
  Serial.println(wickets);

  Serial.print("Overs: ");
  Serial.print(overs);
  Serial.print(".");
  Serial.println(balls);

  Serial.print("Balls Left: ");
  Serial.println(ballsLeft);

  Serial.print("Singles: ");
  Serial.println(singles);

  Serial.print("Dot Balls: ");    
  Serial.println(dotBalls);

  Serial.print("Low Run Streak: ");
  Serial.println(lowRunStreak);

  Serial.println("========================");
}

void processEvent(String event, int runs) {

  ballsLeft--;
  totalRuns += runs;

  if (runs == 1) singles++;

  if (runs == 0) dotBalls++;   

  if (runs <= 2)
    lowRunStreak++;
  else
    lowRunStreak = 0;

  if (event == "WICKET") {
    wickets++;
    consecutiveWickets++;

    tone(BUZZER, 1000);
    delay(250);
    noTone(BUZZER);

    sendHighlight("WICKET FALLS");
  } else {
    consecutiveWickets = 0;
  }

  if (runs == 4)
    sendHighlight("Beautiful Four");

  if (runs == 6) {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);

    sendHighlight("Massive Six");
  }

  if (ballsLeft == 30)
    sendHighlight("Death Overs Begin");

  if (ballsLeft == 6)
    sendHighlight("Final Over Drama");

  monitorStream(event);
  updateLCD();
}

void updateLCD() {

  int ballsPlayed = 120 - ballsLeft;
  int overs = ballsPlayed / 6;
  int balls = ballsPlayed % 6;

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Score:");
  lcd.print(totalRuns);
  lcd.print("/");
  lcd.print(wickets);

  lcd.setCursor(0,1);
  lcd.print("Ov:");
  lcd.print(overs);
  lcd.print(".");
  lcd.print(balls);
}
