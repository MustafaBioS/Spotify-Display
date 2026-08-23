#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "mbedtls/base64.h"

const char* CLIENT_ID = "CLIENT_ID";
const char* CLIENT_SECRET = "CLIENT_SECRET";
const char* REFRESH_TOKEN = "REFRESH_TOKEN";

const int firstBtn = 23;
const int secBtn = 18;
const int thirdBtn = 19;

bool playing;

HTTPClient http;

LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiManager wifiManager;

String accessToken;

String encodedCreds() {
  String credentials = String(CLIENT_ID) + ":" + CLIENT_SECRET;

  size_t outputLen = 0;

  mbedtls_base64_encode(
      nullptr,
      0,
      &outputLen,
      (const unsigned char*)credentials.c_str(),
      credentials.length()
  );

  unsigned char* buffer = new unsigned char[outputLen + 1];

  mbedtls_base64_encode(
      buffer,
      outputLen,
      &outputLen,
      (const unsigned char*)credentials.c_str(),
      credentials.length()
  );

  buffer[outputLen] = '\0';

  String result = String((char*)buffer);

  delete[] buffer;

  return result;
}

String getAccessToken() {

  http.begin("https://accounts.spotify.com/api/token");

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String encoded = encodedCreds();

  http.addHeader("Authorization", "Basic " + encoded);

  String body = "grant_type=refresh_token" "&refresh_token=" + String(REFRESH_TOKEN);

  int httpCode = http.POST(body);

  if (httpCode == 200) {
    String response = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.println("JSON error");
        http.end();
        return "";
    }

    String token = doc["access_token"].as<String>();

    Serial.println("Got access token!");
    Serial.println(token);

    http.end();
    return token;
  }

  Serial.printf("Token request failed: %d\n", httpCode);
  Serial.println(http.getString());

  http.end();
  return "";
}

void getCurrentlyPlaying() {

  http.begin(
    "https://api.spotify.com/v1/me/player/currently-playing"
  );

  http.addHeader(
    "Authorization",
    "Bearer " + accessToken
  );

  int httpCode = http.GET();

  Serial.print("Currently playing HTTP code: ");
  Serial.println(httpCode);

  if (httpCode == 200) {

    String response = http.getString();

    // Serial.println(response);

    JsonDocument doc;

    DeserializationError error =
      deserializeJson(doc, response);

    if (error) {
      Serial.println("JSON error");
      http.end();
      return;
    }

    const char* track =
      doc["item"]["name"];

    const char* artist =
      doc["item"]["artists"][0]["name"];

    playing =
      doc["is_playing"];

    Serial.print("Track: ");
    Serial.println(track);

    Serial.print("Artist: ");
    Serial.println(artist);

    Serial.print("Playing: ");
    Serial.println(playing ? "YES" : "NO");

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print(track);

    lcd.setCursor(0,1);
    lcd.print(artist);

  }

  else if (httpCode == 204) {

    Serial.println("Nothing is currently playing.");

  }

  else {

    Serial.print("Spotify error: ");
    Serial.println(http.getString());
  }

  http.end();
}

void previousSong() {
  http.begin("https://api.spotify.com/v1/me/player/previous");

  http.addHeader(
    "Authorization",
    "Bearer " + accessToken
  );

  http.addHeader(
    "Content-Length",
    "0"
  );

  int httpCode = http.POST("");

  Serial.print("previous HTTP code: ");
  Serial.println(httpCode);

  http.end();
}

void pauseSong() {
  http.begin("https://api.spotify.com/v1/me/player/pause");

  http.addHeader(
    "Authorization",
    "Bearer " + accessToken
  );

  http.addHeader(
    "Content-Length",
    "0"
  );

  int httpCode = http.PUT("");

  Serial.print("pause HTTP code: ");
  Serial.println(httpCode);

  http.end();
}

void resumeSong() {
  http.begin("https://api.spotify.com/v1/me/player/play");

  http.addHeader(
    "Authorization",
    "Bearer " + accessToken
  );

  http.addHeader(
    "Content-Length",
    "0"
  );

  int httpCode = http.PUT("");

  Serial.print("resume HTTP code: ");
  Serial.println(httpCode);

  http.end();
}

void nextSong() {
  http.begin("https://api.spotify.com/v1/me/player/next");

  http.addHeader(
    "Authorization",
    "Bearer " + accessToken
  );

  http.addHeader(
    "Content-Length",
    "0"
  );

  int httpCode = http.POST("");

  Serial.print("next HTTP code: ");
  Serial.println(httpCode);

  http.end();
}

void setup() {
  // put your setup code here, to run once:

  Wire.begin(21, 22);
  pinMode(firstBtn, INPUT_PULLUP);
  pinMode(secBtn, INPUT_PULLUP);
  pinMode(thirdBtn, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("Hello!");

  lcd.begin(16, 2);
  Serial.println("LCD Initialized");
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("HELLO");
  lcd.setCursor(0, 1);
  lcd.print("WORLD");

  if (!wifiManager.autoConnect("ESP32-Setup")) {
    Serial.println("Failed to connect");
    ESP.restart();
  }

  Serial.println("Connected Successfully");

  accessToken = getAccessToken();

  if (accessToken != "") {
      Serial.println("Ready to use Spotify API!");
  }

}

unsigned long lastSpotifyUpdate = 0;

void loop() {

  if (digitalRead(firstBtn) == LOW) {
    Serial.println("PREVIOUS SONG");
    previousSong();
    delay(300);
  }

  if (digitalRead(secBtn) == LOW) {
    Serial.println("PAUSE/PLAY");

    if (playing) {
      Serial.println("Paused");
      pauseSong();
    } else {
      Serial.println("Resumed");
      resumeSong();
    }

    delay(300);
  }

  if (digitalRead(thirdBtn) == LOW) {
    Serial.println("NEXT SONG");
    nextSong();
    delay(300);
  }

  if (millis() - lastSpotifyUpdate >= 2000) {
    lastSpotifyUpdate = millis();
    getCurrentlyPlaying();
  }
}