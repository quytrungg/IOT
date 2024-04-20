

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6USGalAgp"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "SJCJfuRTHn_ULKYX5o77T-R5nd4CnNeT"

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <SPI.h>
#include <MFRC522.h>
#include <FS.h>

// Wifi Configure: 
WiFiServer server(80);
String header; 
bool wifi_connected = 0; 

char auth[] = BLYNK_AUTH_TOKEN;

// Device: Signal to Blynk
SimpleTimer timer;
int fflag = 0;
int eflag = 0;
int jflag = 0;
int gflag = 0;

// Device: LEDs
// Green LED:
#define LED_G 16
// Red LED:
#define LED_R 3

// Decvice: Locker
#define LOCKER 2

// Device: MRF522
#define SS_PIN 4
#define RST_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// Device: Buzzer:
#define BUZZER 15

// Blynk setup:
WidgetTerminal terminal(V2);

void setup() {
  Serial.begin(115200);

  // WifiManager 
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  if (wifiManager.autoConnect("SmartDoorAP", "projectiot")) {
    Serial.println("Connected.");
    wifi_connected = 1;
  }

  if (wifi_connected) {
    // Device: LED setup
    pinMode(LED_G, OUTPUT);
    pinMode(LED_R, OUTPUT);
    digitalWrite(LED_R, 0);
    digitalWrite(LED_G, 0);

    //Device: Buzzer
    pinMode(BUZZER, OUTPUT);
    noTone(BUZZER);

    // Device: Relay - Locker
    pinMode(LOCKER, OUTPUT);
    digitalWrite(LOCKER, HIGH);

    // Device: RFID
    SPI.begin();      // Initiate  SPI bus
    mfrc522.PCD_Init();   // Initiate MFRC522
    Serial.println("Put your card to the reader...");
    Serial.println();
    timer.setInterval(1000L, iot_rfid);

    const char* ssid_blynk = WiFi.SSID().c_str();
    const char* pssd_blynk = WiFi.psk().c_str();

    Blynk.begin(auth, ssid_blynk, pssd_blynk, "blynk.cloud", 80);
  }
  
}

void loop() 
{
  timer.run();
  Blynk.run();
}

// Function handle RFID:
void iot_rfid() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) { return;}
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) { return;}

  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;

  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], DEC);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], DEC));
  }

  Serial.println();
  Serial.print("Message: ");
  content.toUpperCase();

  // Set up cards:
  if ((content.substring(1) == "53 112 221 41") && (fflag == 1)) {
    access_sucess("Phuoc Nguyen");
  } 
  else if ((content.substring(1) == "192 205 223 29") && (eflag == 1)) {
    access_sucess("Quy Trung");
  }
  else if ((content.substring(1) == "35 118 16 186") && (jflag == 1)) {
    access_sucess("Quoc Huy");
  }
  else  {
    Serial.println("Unregistered user");
    Blynk.virtualWrite(V2, "Unregistered user Trying to Access your Door Lock " );
    Blynk.logEvent("Unregistered user Trying to Access your Door Lock");
    Serial.println("Access denied");
    Serial.println();

    digitalWrite(LED_R, HIGH);
    tone(BUZZER, 300);
    delay(1500);
    digitalWrite(LED_R, LOW);
    noTone(BUZZER);
  }
}

BLYNK_WRITE(V0) {
  fflag = param.asInt();
}

BLYNK_WRITE(V1) {
  eflag = param.asInt();
}

BLYNK_WRITE(V3) {
  jflag = param.asInt();
}

BLYNK_WRITE(V4) {
  gflag = param.asInt();

  if (gflag == 1) {
    access_sucess("Remote/Admin"); 
    timer.setTimeout(1000L,  closeDoor);
  }
}

void closeDoor() {
  Blynk.virtualWrite(V4, 0);
  gflag = 0; 
}

void access_sucess(String name) {
  
  Serial.println(name);
  String message = name + (" acessing ");
  Blynk.logEvent("notification", message + (" the door right now!"));
  Blynk.virtualWrite(V2, message);
  Serial.println();

  delay(500);
  digitalWrite(LED_G, 1);
  digitalWrite(LOCKER, HIGH);
  tone(BUZZER, 500);
  delay(250);
  noTone(BUZZER);
  delay(2000);
  digitalWrite(LED_G, 0);
  delay(3000);
  digitalWrite(LOCKER, LOW);
}