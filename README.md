## Simple Arduino Project - RFID Card reading
NOTE: code is only applicable for RFID cards containing datas of First Name and Last Name located at block 4 & 1, respectively.
___
#### Main Concept:
This project shows the technical perspective of Bank Cards/ATMs in the simplest form
#### Flow:
- Firstly, the RFID must contain the card owner's UUID and Name data, then
- The RFID waits for card presence
  - The waiting state ignores button-press events
- If a card is present on the reader, Arduino runs card checks
  - checks if the card is registered in storage; if not, then register as new
  - acccess the card's internal data blocks when authentication is successful
  - save card data into the Arduino storage, specifically in `line:35 - struct registeredCards` structure variable
- Information about the current card is displayed
- User is now able to interact with the buttons, such as:
  - Withdrawing balance, and Deposit balance
  - Note: Buttons are just used as +(increment) & -(decrement) for the balance value
  - Card balance is stored in the Arduino, not in the card itself.
- Upon removal of the card, the system returns to waiting state.
---
#### Libraries used:
- LiquidCrystal_I2C
- MFRC522

#### Hardware used:
- 💻 Arduino Uno R3
- 📡 RFID Reader
- 📺 LCD
- 🔘 Buttons
- 🔊 Buzzer
- ➰ Jumper wires
#### Hardware Roles:
📡 RFID Reader
Used to:
- Detect card presence
- Read UID
- Authenticate memory blocks
- Read stored name data

📺 LCD
Used for:
- Displaying information

🔘 Buttons
Used for:
- Incrementing/Decrementing card balance

🔊 Buzzer
Used for:
- Sound queues
___
I'm unable to provide a diagram of the circuit since this project was archived a long time ago. But the RFID reader and LCD-i2c components should have used their default pin location, just like any of its use cases.
