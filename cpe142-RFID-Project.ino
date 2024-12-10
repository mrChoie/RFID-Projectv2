/*
 * Initial Author: ryand1011 (https://github.com/ryand1011)
 *
 * Reads data written by a program such as "rfid_write_personal_data.ino"
 *
 * See: https://github.com/miguelbalboa/rfid/tree/master/examples/rfid_write_personal_data
 *
 * Uses MIFARE RFID card using RFID-RC522 reader
 * Uses MFRC522 - Library
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
*/

#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define withdrawBtn 2
#define depositBtn  3
#define infoBtn     4
#define insrtDetect A0

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

LiquidCrystal_I2C lcd (0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  pinMode(withdrawBtn, OUTPUT);
  pinMode(depositBtn, OUTPUT);
  pinMode(infoBtn, OUTPUT);
  //pinMode(insrtDetect, OUTPUT);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  
}

struct registeredCards {  // Structure declaration
  String cardUID = " ";
  String cardOwner="";
  int cardBalance=50;
};

struct registeredCards Account[10];
bool withdrawMode, depositMode, infoMode, insertStatus=0;
int lightSensorVal, accIndex;
//int timeInterval[50], time;

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------

void loop() {
  // time++;
  // timeInterval[time]=time;
  // Serial.print(time);
  //Serial.println(lightSensorVal);
  // Detect if insrtDetect pin is High or Low
  lightSensorVal = map(analogRead(insrtDetect), 0, 550, 0, 8);
  if (lightSensorVal<4) {                     // Do function if card is inserted/reader is active
    readerActive();
  } else if ( digitalRead(withdrawBtn)==1||   // alert if user try to do function but
              digitalRead(depositBtn)==1||    // card is inserted/reader is inactive
              digitalRead(infoBtn)==1){
    insertAlert();
  } else {
    lcd.clear();
  }

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------
  // if (time==50){
  //   Serial.println(timeInterval[time]);
  //   time=0;
  // }
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  } 
  //-----------------------------------------------------------------------------------------------------
  //---------   statements below this line are executed when above statement is false    -----------------
  //-----------------------------------------------------------------------------------------------------
  //Serial.print("Card Detected");
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
  int sizeofCurrUID = mfrc522.uid.size;       // For loop counter
  
  String firstName =" ", lastName =" ";
  String currentCard;
  bool nameSaved;

  
  
  
  // Access uid from buffer array and save as currentCard string value 
  for (int i = 0; i < sizeofCurrUID; i++){
    currentCard += mfrc522.uid.uidByte[i], HEX;
  }
  
  // Save current card to registeredCards structure
  for (int i = 0; i < 10; i++){
    if (currentCard == Account[i].cardUID){   // Ignore if already saved
      accIndex=i;
      nameSaved=0;
      
      //mfrc522.PICC_HaltA();
      //Serial.print("Card already Exist");
      break;  
    }
    else if (Account[i].cardUID==" "){
      Account[i].cardUID = currentCard;       // Save if not yet registered
      accIndex=i;
      nameSaved=1;
      
      //mfrc522.PICC_HaltA();
      //Serial.print("New Card Detected");
      break;
    }
  }
  
  //Print registered cards
  // Serial.print("ID: ");
  // for (int i =0; i < 4;i++){
  //   Serial.print(Account[i].cardUID+"\n");
  // }

  //------------------------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------------
  
  //------------------------------------------- GET FIRST NAME
  byte buffer1[18];
  block = 4;
  len = 18;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("Authentication failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("Reading failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  //PRINT FIRST NAME
  // for (uint8_t i = 0; i < 16; i++)
  // {
  //   if (buffer1[i] != 32)
  //   {
  //     Serial.write(buffer1[i]);
  //   }
  // }
  // Serial.print(" ");

  // Save First Name ------------------------------------------------------------
  // Save buffer1 elements into current account index of registeredCards
  
  for (int i = 0; i < 16; i++){
    if (nameSaved){
      if (buffer1[i] != 32 && buffer1[i] != 10 && buffer1[i] != 11){
        Account[accIndex].cardOwner += char(buffer1[i]);
      }
    }
  }
  // currentCard += mfrc522.uid.uidByte[i], HEX;
  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("Authentication failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("Reading failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  // for (uint8_t i = 0; i < 16; i++) {
  //   Serial.write(buffer2[i] );
  // }
  //-------------------------------------------

  // Save Last Name ------------------------------------------------------------
  // Save buffer2 elements into current account index of registeredCards
  for (int i = 0; i < 16; i++){
    if (nameSaved){
      if (i==0){
        Account[accIndex].cardOwner += " ";
      }
      if (buffer2[i] != 32 && buffer2[i] != 10){
        Account[accIndex].cardOwner += char(buffer2[i]);
      }
    }
  }
  //delay(1000);
  // if (lightSensorVal){
  //   displayLCD(accIndex);
  // }
  //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
  // for (int i =0; i < sizeofCurrUID;i++){
  //   // Serial.print(mfrc522.uid.uidByte[i], HEX);
    
  // }
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------
  // if (mfrc522.PICC_IsNewCardPresent()){
  //   insertStatus=0;
  //   mfrc522.PICC_HaltA();
  // }
  
  // do {
    
  // } while (lightSensorVal>3);
  // delay(1000);
  // if (lightSensorVal<3){
  //   displayLCD(accIndex);
  // }
  // Serial.print("\n\nID: \t\t");
  // Serial.print(Account[accIndex].cardUID);
  // Serial.print("\nName: \t\t");
  // Serial.print(Account[accIndex].cardOwner);
  // Serial.print("\nID Balance: \t$");
  // Serial.print(Account[accIndex].cardBalance);
  //Serial.println(lightSensorVal);
  displayLCD(accIndex);
  delay(100); //change value if you want to read cards faster
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
}
//*****************************************************************************************//
// withdrawBal();
// depositBal();
// displayInfo();
void insertAlert(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Insert card");
  lcd.setCursor(5,1);
  lcd.print("first");
  delay(1000);
}

void readerActive(){
  // if (x){
    if (digitalRead(withdrawBtn)==1){
      Serial.print("withdraw\n");

      withdrawBal();
    }
    if (digitalRead(depositBtn)==1){
      Serial.print("deposit\n");
      depositBal();
    }
    if (digitalRead(infoBtn)==1){
      Serial.print("info\n");
      displayInfo();
    }
}
void displayLCD(int accIndex){
  //Serial.println("Display function");
  delay(1000);
  if (lightSensorVal<3){
    //Serial.println("Display lcd");
    lcd.clear();
    lcd.setCursor(0,0);lcd.print("Card Detected");delay(1000);
    lcd.setCursor(0,0);lcd.print("Getting Info.  ");delay(200);
    lcd.setCursor(0,0);lcd.print("Getting Info.. ");delay(200);
    lcd.setCursor(0,0);lcd.print("Getting Info...");delay(200);
    lcd.setCursor(0,0);lcd.print("Getting Info.  ");delay(200);
    lcd.setCursor(0,0);lcd.print("Getting Info.. ");delay(200);
    lcd.setCursor(0,0);lcd.print("Getting Info...");delay(200);
    lcd.setCursor(0,0);lcd.clear();
    lcd.print(Account[accIndex].cardOwner);
    if (Account[accIndex].cardBalance>0){
      lcd.setCursor(0,1);lcd.print("Balance: $");
      lcd.setCursor(10,1);lcd.print(Account[accIndex].cardBalance);
    } else {
      lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
      lcd.setCursor(0,1);lcd.print("Balance:-$");
    }
    
    // Serial.print("\n\nID: \t\t");
    // Serial.print(Account[accIndex].cardUID);
    // Serial.print("\nName: \t\t");
    // Serial.print(Account[accIndex].cardOwner);
    // Serial.print("\nID Balance: \t$");
    // Serial.print(Account[accIndex].cardBalance);
  }
}
void withdrawBal(){
  lcd.setCursor(0,0);
  lcd.print("--Withdrawing---");
  if (Account[accIndex].cardBalance>0){
    lcd.setCursor(10,1);lcd.print("    ");
    Account[accIndex].cardBalance--;
    lcd.setCursor(10,1);lcd.print(Account[accIndex].cardBalance);
  } else if (Account[accIndex].cardBalance==0){
    lcd.setCursor(0,1);lcd.print("Balance: $0");
    Account[accIndex].cardBalance--;
  } else {
    lcd.setCursor(9,1);lcd.print("     ");
    //Account[accIndex].cardBalance*Account[accIndex].cardBalance;
    lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
    lcd.setCursor(0,1);lcd.print("Balance:-$");
    Account[accIndex].cardBalance--;
  }
}
void depositBal(){
  lcd.setCursor(0,0);
  lcd.print("---Depositing---");
  if (Account[accIndex].cardBalance>0){
    lcd.setCursor(10,1);lcd.print("    ");
    Account[accIndex].cardBalance++;
    lcd.setCursor(10,1);lcd.print(Account[accIndex].cardBalance);
  } else if (Account[accIndex].cardBalance==0){
    lcd.setCursor(0,1);lcd.print("Balance: $0");
    Account[accIndex].cardBalance++;
  } else {
    lcd.setCursor(10,1);lcd.print("    ");
    lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
    lcd.setCursor(0,1);lcd.print("Balance:-$");
    Account[accIndex].cardBalance++;
  }
}
void displayInfo(){
  lcd.setCursor(0,0);lcd.clear();
  lcd.print(Account[accIndex].cardOwner);
  if (Account[accIndex].cardBalance>0){
    lcd.setCursor(0,1);lcd.print("Balance: $");
    lcd.setCursor(10,1);lcd.print(Account[accIndex].cardBalance);
  } else {
    lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
    lcd.setCursor(0,1);lcd.print("Balance:-$");
  }
}