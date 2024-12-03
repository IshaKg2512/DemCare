#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#define BUZZER_PIN      8
#define GSM_RX          7          // GSM Module RX pin
#define GSM_TX          6          // GSM Module TX pin

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
SoftwareSerial gsmSerial(GSM_RX, GSM_TX); // Initialize GSM Serial communication

void setup() {
    Serial.begin(9600);           // Initialize serial communications with the PC
    gsmSerial.begin(9600);        // Initialize GSM Module communication
    while (!Serial);              // Do nothing if no serial port is opened
    SPI.begin();                  // Init SPI bus
    mfrc522.PCD_Init();           // Init MFRC522
    delay(4);                     // Optional delay
    mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
    Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
    pinMode(BUZZER_PIN, OUTPUT); 
}

void sendSMS(String message) {
    gsmSerial.println("AT");                    // Check if GSM module is ready
    delay(1000);
    gsmSerial.println("AT+CMGF=1");             // Set SMS to text mode
    delay(1000);
    gsmSerial.println("AT+CMGS=\"+1234567890\"");// Replace with your phone number
    delay(1000);
    gsmSerial.println(message);                 // Message to send
    delay(1000);
    gsmSerial.write(26);                        // ASCII code for CTRL+Z to send SMS
    delay(1000);
}

void loop() {
    // Look for new cards
    digitalWrite(BUZZER_PIN, HIGH);
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }
    // Show UID on serial monitor
    Serial.print("UID tag: ");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    Serial.println();
    Serial.print("Message: ");
    content.toUpperCase();

    if (content.substring(1) == "EA 51 64 02") { // Change this UID for the authorized card
        Serial.println("Authorized access");
        sendSMS("Authorized access granted.");
        digitalWrite(BUZZER_PIN, LOW);
        delay(500);
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        Serial.println("Unauthorized access attempt!");
        sendSMS("Unauthorized access detected!");
    }
}
