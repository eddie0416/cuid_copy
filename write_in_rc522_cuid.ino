#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

// 預設密碼KeyA 6 bytes
byte keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 新UID＋block0完整16 bytes資料，UID為前4 bytes
byte new_block0[16] = {
  0x75, 0xAC, 0xBB, 0x74,   // 新UID 4 bytes
  0x08, 0x04, 0x00, 0x00,   // 後面12 bytes 建議使用原卡block0的資料
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("等待新卡片，準備寫入新UID...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.print("偵測到卡片 UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // 認證扇區0( Block 0 )，Key A為預設FF FF FF FF FF FF
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    0,
    &((MFRC522::MIFARE_Key){.keyByte = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}),
    &(mfrc522.uid));
    
  if (status != MFRC522::STATUS_OK) {
    Serial.print("扇區0認證失敗: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // 寫入block 0 (包含新UID)
  status = mfrc522.MIFARE_Write(0, new_block0, 16);
  if (status == MFRC522::STATUS_OK) {
    Serial.print("成功寫入新UID: ");
    for (int i = 0; i < 4; i++) {  // UID 就是 new_block0 的前 4 個 byte
    if (new_block0[i] < 0x10) Serial.print("0"); // 補零
    Serial.print(new_block0[i], HEX);
    if (i < 3) Serial.print(" "); // 中間加空格
    }
    Serial.println();
  } else {
    Serial.print("寫入失敗: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  mfrc522.PICC_HaltA();
  delay(3000);
}