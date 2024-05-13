#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define BLOCK_NUMBER 4 // The block number where you want to read data

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

  // Read the data from the card
  byte buffer[18]; // Buffer to store the data
  byte size = sizeof(buffer);
  status = rfid.MIFARE_Read(BLOCK_NUMBER, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_Read() failed: ");
    Serial.println(rfid.GetStatusCodeName(status));
  } else {
    // Convert the first 4 bytes of the buffer to an integer
    int uniqueNumber = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
    Serial.print("Unique number read from card: ");
    Serial.println(uniqueNumber);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
