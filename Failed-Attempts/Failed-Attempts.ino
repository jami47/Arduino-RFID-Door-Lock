#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define BLOCK_NUMBER 4 // The block number where you wrote the unique number
String UID = "A3 FE 4C 16"; // RFID card
int uniqueNumber = 4755; // Unique number for the card
byte lock = 0;

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);

int failattempt = 0;
void scrollText(String message) {
  int messageLength = message.length();

  for (int positionCounter = 0; positionCounter < messageLength; positionCounter++) {
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      // Check if the positionCounter+i is within the string length
      if (positionCounter + i < messageLength) {
        // Print the character at positionCounter+i
        lcd.print(message[positionCounter + i]);
      } else {
        // Print a blank space if the positionCounter+i is outside the string length
        lcd.print(' ');
      }
    }
    delay(300); // Adjust the delay to control the speed of the scrolling
    //lcd.clear();
  }
}

void setup() {
  Serial.begin(9600);
  servo.write(70);
  lcd.init();
  lcd.backlight();
  servo.attach(5);
  SPI.begin();
  rfid.PCD_Init();
}

void loop() {
  //Nicher line kheyal rakba
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Welcome! ");
  lcd.setCursor(1, 1);
  lcd.print("Put your card");

  if ( ! rfid.PICC_IsNewCardPresent()) 
  {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }
  if ( ! rfid.PICC_ReadCardSerial()) 
  {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning");
  //Serial.print("NUID tag is :");
  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    lcd.print(".");
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
    delay(300);
  }
  delay(100); //This line for testing
  ID.toUpperCase();

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
    return;
  }

  // Convert the first 4 bytes of the buffer to an integer
  int readUniqueNumber = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];

  // Check both the UID and the unique number
  if (ID.substring(1) == UID && readUniqueNumber == uniqueNumber && lock == 0 ) {
    servo.write(10);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is open");

    String message = " Wrong attempts: " + String(failattempt);
    scrollText(message);

    delay(1500); //default: 1500
    lcd.clear();
    lock = 1;
  } else if (ID.substring(1) == UID && readUniqueNumber == uniqueNumber && lock == 1 ) {
    servo.write(160);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is locked");

    String message = " Wrong attempts: " + String(failattempt);
    scrollText(message);

    delay(1500); //default: 1500
    lcd.clear();
    lock = 0;
  } else {
    ++failattempt;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong card!");
    //lcd.setCursor(0,1);
    //lcd.print("Failed attempts: ");
    //String message = " Wrong attempts: " + String(failattempt);
    //scrollText(message); // Call the scrollText function

    //lcd.print(failattempt);
    delay(1500); //default: 1500
    lcd.clear();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
