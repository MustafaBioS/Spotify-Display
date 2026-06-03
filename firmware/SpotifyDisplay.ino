#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ILI9341.h>
#include <SpotifyEsp32.h>

const char* CLIENT_ID = "ID";
const char* CLIENT_SECRET = "SECRET";

const int firstBtn = 13;
const int secBtn = 14;
const int thirdBtn = 21;

bool isPlaying = false;

// const int TFT_CS = 46;
// const int TFT_REST = 9;
// const int TFT_DC = 10;
// const int TFT_MOSI = 11;
// const int TFT_SCK = 12;

String lastArtist;
String lastSong;

Spotify sp(CLIENT_ID, CLIENT_SECRET);

// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_REST);

LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiManager wifiManager;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(32, 25);
  pinMode(firstBtn, INPUT_PULLUP);
  pinMode(secBtn, INPUT_PULLUP);
  pinMode(thirdBtn, INPUT_PULLUP);

  Serial.begin(115200);
  
  // tft.begin();
  // Serial.println("TFT Initialized");
  // tft.fillScreen(ILI9341_BLACK);
  lcd.init();
  Serial.println("LCD Initialized");
  lcd.backlight();

  wifiManager.autoConnect("ESP32-Setup");

  if (!wifiManager.autoConnect("ESP32-Setup")) {
    Serial.println("Failed to connect");
    ESP.restart();
  }

  Serial.println("Connected Successfully");

  // tft.setCursor(0,0);

  sp.begin();
  while (!sp.is_auth()) {
    sp.handle_client();
    delay(2000);
  }

  Serial.printf("Authenticated, Refresh Token: %s\n", sp.get_user_tokens().refresh_token);

}

void loop() {
  // put your main code here, to run repeatedly:

  String currentArtist = sp.current_artist_names();
  String currentSong = sp.current_track_name();

  if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
    // tft.fillScreen(ILI9341_BLACK);
    lcd.clear();
    lastArtist = currentArtist;
    // tft.setCursor(120, 120);
    lcd.setCursor(0, 0);
    // tft.write(lastArtist.c_str());
    lcd.print(lastArtist);
  }

  if (lastSong != currentSong && currentSong != "null" && !currentSong.isEmpty()) {
    // tft.fillScreen(ILI9341_BLACK);
    lcd.clear();
    lastSong = currentSong;
    // tft.setCursor(120, 160);
    lcd.setCursor(0, 1);
    // tft.write(lastSong.c_str());
    lcd.print(lastSong);
  }
  delay(2000);

  if (digitalRead(firstBtn) == LOW) {
    Serial.println("PREVIOUS SONG");
    sp.skip_to_previous();

    delay(300);
  }

  if (digitalRead(secBtn) == LOW) {
    Serial.println("PAUSE/PLAY");
    if (isPlaying == true) {
      sp.pause_playback();
      isPlaying = false;
    } else if (isPlaying == false) {
      sp.start_a_users_playback();
      isPlaying = true;
    }

    delay(300);
  }

  if (digitalRead(thirdBtn) == LOW) {
    Serial.println("NEXT SONG");
    sp.skip_to_next();

    delay(300);
  }

}