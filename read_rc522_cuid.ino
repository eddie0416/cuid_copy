#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("等待卡片以讀取16個扇區資料...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.print("UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // 讀取16個扇區，每扇區4個blocks
  for (byte sector = 0; sector < 16; sector++) {
    Serial.print("讀取 Sector ");
    Serial.println(sector);

    // 證認該扇區第一個block(扇區的第0個block，地址是 sector*4)
    byte blockAddr = sector * 4;
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
      blockAddr,
      &((MFRC522::MIFARE_Key){.keyByte = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}),
      &(mfrc522.uid));
    
    if (status != MFRC522::STATUS_OK) {
      Serial.print("Sector ");
      Serial.print(sector);
      Serial.print(" 認證失敗: ");
      Serial.println(mfrc522.GetStatusCodeName(status));
      continue;
    }

    // 讀取4個block
    for (byte blockOffset = 0; blockOffset < 4; blockOffset++) {
      byte buffer[18];
      byte size = sizeof(buffer);
      byte block = blockAddr + blockOffset;

      status = mfrc522.MIFARE_Read(block, buffer, &size);
      if (status == MFRC522::STATUS_OK) {
        Serial.print(" Block ");
        Serial.print(block);
        Serial.print(": ");
        for (byte i = 0; i < 16; i++) {
          Serial.print(buffer[i] < 0x10 ? " 0" : " ");
          Serial.print(buffer[i], HEX);
        }
        Serial.println();
      } else {
        Serial.print("讀取block ");
        Serial.print(block);
        Serial.print(" 失敗: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
      }
    }
  }
  
  mfrc522.PICC_HaltA();
  delay(5000); // 等待下一次掃描
}
