#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define BLOCK_NUMBER 4 // The block number where you want to write data
//String UID = "A3 FE 4C 16"; // RFID card
//int uniqueNumber = 4755; // Unique number for the card

//String UID = "03 22 13 15"; // RFID card
String UID = "30 37 72 51";
int uniqueNumber = 5547; // Unique number for the card (4 digit as of now)

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
  }
  ID.toUpperCase();

  if (ID.substring(1) == UID) {
    byte buffer[16]; // Buffer to store the data
    buffer[0] = uniqueNumber >> 24;
    buffer[1] = uniqueNumber >> 16;
    buffer[2] = uniqueNumber >> 8;
    buffer[3] = uniqueNumber; // Store the unique number in the first 4 bytes

    // Fill the rest of the buffer with 0s
    for (int i = 4; i < 16; i++) {
      buffer[i] = 0;
    }

    // Define the key
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF; // Default key
    }

    // Authenticate the block
    MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, BLOCK_NUMBER, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print("PCD_Authenticate() failed: ");
      Serial.println(rfid.GetStatusCodeName(status));
      return;
    }

    // Write the data to the card
    status = rfid.MIFARE_Write(BLOCK_NUMBER, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print("MIFARE_Write() failed: ");
      Serial.println(rfid.GetStatusCodeName(status));
    } else {
      String successMessage = "MIFARE_Write() success: Unique number " + String(uniqueNumber) + " written to card";
      Serial.println(successMessage);
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
