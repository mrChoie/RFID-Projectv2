#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define withdrawBtn   2
#define depositBtn    3
#define infoBtn       4
#define buzzerPin     6
#define RST_PIN       9
#define SS_PIN        10
#define PhotoResistorPin     A0
#define tone1         1200

LiquidCrystal_I2C lcd (0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  pinMode(withdrawBtn, OUTPUT);
  pinMode(depositBtn, OUTPUT);
  pinMode(infoBtn, OUTPUT);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
}


//vvvvvvvvvvvvvvvvvvvvvvvvvvvvv  Global variables  vvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
struct registeredCards {  // Structure declaration
  String cardUID = " ";
  String cardOwner="";
  int cardBalance;
};

struct registeredCards Account[10]; //registeredCards structure can have 10 Account
// String firstAccountCardUID="";
// String firstAccountCardOwner="";
// int firstAccountCardBalance="";
// String secondAccountCardUID="";
// String secondAccountCardOwner="";
// int secondAccountCardBalance="";
// ...

// Account[1] has: cardUID, cardOwner, cardBalance
// Account[2] has: cardUID, cardOwner, cardBalance
// ...
// Account[n] has: cardUID, cardOwner, cardBalance

bool insertStatus=0;
int PhotoResistorValue, accIndex;

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  // read and map PhotoResistorPin pin value (phtrstr)
  // then assign value to PhotoResistorValue
  PhotoResistorValue = map(analogRead(PhotoResistorPin), 0, 550, 0, 8);

  // Do function if card is inserted/reader is active
  if (PhotoResistorValue<4) {                     
    readerActive();
    if (insertStatus==1){
        tone(buzzerPin, 1000);delay(90);
        tone(buzzerPin, 2500);delay(160);
        tone(buzzerPin, 2500);delay(110);noTone(buzzerPin);
      }
    
    // Display alert if buttons is pressed but no card is inserted
  } else if ( digitalRead(withdrawBtn)==1||digitalRead(depositBtn)==1||digitalRead(infoBtn)==1){
      if (insertStatus==0){
        tone(buzzerPin, 2500);delay(100);
        tone(buzzerPin, 1000);delay(150);noTone(buzzerPin);
      }
      insertAlert();

    // Do nothing / clear lcd display
  } else {
    lcd.clear();
  }
  insertStatus=0;

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;// <------- this is resets the loop back to the top
  } 
  //-----------------------------------------------------------------------------------------------------
  //---------   statements below this line are executed when above statement is false    -----------------
  //-----------------------------------------------------------------------------------------------------
 
  int sizeofCurrUID = mfrc522.uid.size;       // For loop counter
  
  String firstName =" ", lastName =" ";
  String currentCard;
  bool nameSaved;
  insertStatus=1;
  
  
  
  // Access card uid from buffer array and save as currentCard string value 
  for (int i = 0; i < sizeofCurrUID; i++){
    currentCard += mfrc522.uid.uidByte[i], HEX;
  }
  
  // Save current card to registeredCards structure
  for (int i = 0; i < 10; i++){
    if (currentCard == Account[i].cardUID){   // Ignore if already saved
      accIndex=i;
      nameSaved=0;// <---
      break;         
    }
    else if (Account[i].cardUID==" "){
      Account[i].cardUID = currentCard;       // Save if not yet registered
      Account[i].cardBalance=50;
      accIndex=i;
      nameSaved=1;// <---
      break;
    }
  }
  
  //vvvvvvvvvvvvvvvvvvvvvvvvvvvvv  Get first name from card  vvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
  byte buffer1[18];
  block = 4;
  len = 18;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  // Store the fetched first name from buffer1 array into cardOwner--member of stucture Account[accIndex]
  for (int i = 0; i < 16; i++){
    if (nameSaved){
      if (buffer1[i] != 32 && buffer1[i] != 10 && buffer1[i] != 11){
        Account[accIndex].cardOwner += char(buffer1[i]);
      }
    }
  }
  //vvvvvvvvvvvvvvvvvvvvvvvvvvvvv  Get last name from card  vvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  // Store the fetched last name from buffer2 array into cardOwner--member of stucture Account[accIndex]
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
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
  displayLCD(accIndex);
  delay(100);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
}
//------------------------------------------------------------------------------------------
//--------  End of main loop  --------------------------------------------------------------
//------------------------------------------------------------------------------------------

void insertAlert(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Insert card");
  lcd.setCursor(5,1);
  lcd.print("first");
  insertStatus=0;
  delay(1000);
}

void readerActive(){
  if (digitalRead(withdrawBtn)==1){
    withdrawBal();
  }
  if (digitalRead(depositBtn)==1){
    depositBal();
  }
  if (digitalRead(infoBtn)==1){
    displayInfo();
  }
}

//*****************************************************************************************//

void displayLCD(int accIndex){
  delay(1000);
  if (PhotoResistorValue<3){  // If reader LED is blocked by card
                          // blocked == phtrstr is LOW
                          // unblocked == phtrstr is HIGH
    // LCD Display animation
    lcd.clear();lcd.setCursor(0,0);lcd.print("Card Detected");
    tone(buzzerPin, 1000);delay(100);noTone(buzzerPin);delay(1000);
    lcd.setCursor(0,0);lcd.print("Getting Info.  ");
    tone(buzzerPin, tone1);delay(50);noTone(buzzerPin);delay(210);
    lcd.setCursor(0,0);lcd.print("Getting Info.. ");
    tone(buzzerPin, tone1);delay(50);noTone(buzzerPin);delay(210);
    lcd.setCursor(0,0);lcd.print("Getting Info...");
    tone(buzzerPin, tone1);delay(50);noTone(buzzerPin);delay(210);
    lcd.setCursor(0,0);lcd.print("Getting Info.  ");
    tone(buzzerPin, tone1);delay(50);noTone(buzzerPin);delay(210);
    lcd.setCursor(0,0);lcd.print("Getting Info.. ");
    tone(buzzerPin, tone1);delay(50);noTone(buzzerPin);delay(210);
    lcd.setCursor(0,0);lcd.print("Getting Info...");
    tone(buzzerPin, tone1);delay(50);noTone(buzzerPin);delay(210);
    lcd.setCursor(0,0);lcd.clear();
    tone(buzzerPin, 1000);delay(100);noTone(buzzerPin);

    // Display information of the current Account index
    lcd.print(Account[accIndex].cardOwner);
    if (Account[accIndex].cardBalance>0){
      lcd.setCursor(0,1);lcd.print("Balance: $");
      lcd.setCursor(10,1);lcd.print(Account[accIndex].cardBalance);
    } else {
      lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
      lcd.setCursor(0,1);lcd.print("Balance:-$");
    }
    insertStatus=0;
  }
}

//*****************************************************************************************//

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
    lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
    lcd.setCursor(0,1);lcd.print("Balance:-$");
    Account[accIndex].cardBalance--;
  }
}

//*****************************************************************************************//

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

//*****************************************************************************************//

void displayInfo(){
  tone(buzzerPin, 1000);
  delay(100);
  noTone(buzzerPin);

  lcd.setCursor(0,0);lcd.clear();
  lcd.print(Account[accIndex].cardOwner);

  if (Account[accIndex].cardBalance>0){
    lcd.setCursor(0,1);lcd.print("Balance: $");
    lcd.setCursor(10,1);lcd.print(Account[accIndex].cardBalance);
  } else {
    lcd.setCursor(9,1);lcd.print(Account[accIndex].cardBalance);
    lcd.setCursor(0,1);lcd.print("Balance:-$");
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}