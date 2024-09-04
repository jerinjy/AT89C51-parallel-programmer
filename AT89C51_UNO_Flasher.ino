#define SHFT_DATA 2
#define SHFT_CLK 3
#define ST_CLK 4
#define DATA_D0 5
#define DATA_D7 12 
#define PROG 13 
#define VPP 15
#define P2_6 16
#define P2_7 17
#define P3_6 18
#define P3_7 19

/*COMBINATIONS*/
//Write code    : 0x7
//Read code     : 0x3
//Write LockBit1: 0xF
//Write LockBit2: 0xC
//Write LockBit3: 0xA
//Chip erase    : 0x8
//Read signByte : 0x0


void setAddr(int addr)
  { 
    uint16_t VALUE = (addr << 4);
    shiftOut(SHFT_DATA, SHFT_CLK, LSBFIRST, VALUE);
    shiftOut(SHFT_DATA, SHFT_CLK, LSBFIRST, VALUE >> 8);   
  }


//void setComb(byte comb)
//  {
//    for(int i = P3_7; i >= P2_6; i--)
//    {
//      bool Bit = comb & 1;
//      digitalWrite(i, Bit);
//      comb >> 1;
//    }
//  }


void chipErase()
  { 
    /*comb = 0x8*/
    PORTC |= 4U;                // Set P2_6 to high
    PORTC &= 199U;              // Set P2_7,P3_6,P3_7 to LOW
    PORTC &= 253U;              // (VPP, LOW)
    delayMicroseconds(1);
    PORTB &= 223U;              // (PROG, LOW)
    delayMicroseconds(10);
    PORTB |= 32U;               // (PROG, HIGH)
    PORTC |= 2U;                // (VPP, HIGH)
    
    PORTC |= 2U;              
    PORTC |= 60U;               // Reset
    Serial.println("Chip Erased.....");
  }


void writeData(int addr, byte Data)
  { 
    setAddr(addr);
    DDRD |= 224U;                // Setting D0-D7 as output
    DDRB |= 31U;
    PORTC &= 195U;               // Reseting comb pins
    
    /*comb = 0x7*/
    PORTC &= 251U;               // Set P2_6 to low
    PORTC |= 56U;                // Set P2_7,P3_6,P3_7 to high
    delayMicroseconds(10);
    PORTC &= 253U;                // (VPP, LOW)
    
    delayMicroseconds(2);
    PORTD |= 16U;               // ST_CLK Set high

    for(int pin = DATA_D0; pin <= DATA_D7; pin++)
      {
        digitalWrite(pin , Data & 1);
        Data = Data >> 1;
      } 
    delayMicroseconds(15);
    PORTB &= 223U;                // (PROG, LOW)
    delayMicroseconds(15);
    PORTB |= 32U;                 // (PROG, HIGH)
    delayMicroseconds(15);
    PORTC |= 2U;                  // (VPP, HIGH)

    PORTD &= 239U;       
    PORTC |= 60U;                 // Reset
    Serial.println("Chip Programmed.....");
  }


void readData(int addr)
  { 
    DDRD &= 31U;                // Setting D0-D7 as input
    DDRB &= 224U;             
    
    setAddr(addr);
    
    /*comb = 0x3*/
    PORTD |= 16U;               // ST_CLK Set high
    PORTC &= 243U;              // P2_6, P2_7 Set as low
    PORTC |= 48U;               // P3_6, P3_7 Set high
    
    byte Data = 0;
    for(int pin = DATA_D7; pin >= DATA_D0; pin--)
      { Data = (Data << 1) + digitalRead(pin); }

    PORTC |= 60U;
    PORTD &= 239U;
      
    //Serial.println(Data,HEX);
  return Data;
  }


void readSignByte()             
  { 
    DDRD &= 31U;                
    DDRB &= 224U;               // Set D0-D7 as input
    
    for(int addr = 48; addr <= 50; addr++)
      { 
        setAddr(addr);
        
        /*comb = 0x0*/
        PORTD |= 16U;               // ST_CLK Set high
        PORTC &= 195U;        // P2_6, P2_7, P3_6, P3_7 Set as low
        
        byte Data = 0;
        for(int pin = DATA_D7; pin >= DATA_D0; pin--)
          { Data = (Data << 1) + digitalRead(pin); }

        PORTC |= 60U;
        PORTD &= 239U;              // Reset
      
      Serial.print("Signature Byte | ");
      Serial.print(addr,HEX);
      Serial.print(": ");
      Serial.println(Data,HEX);
      delayMicroseconds(50);
     }
  }


void printDatas()
  {
    for(int base = 0; base <= 255; base += 16)
    {
      byte data[16];
      for(int pos = 0; pos <= 15; pos++)
      {
        data[pos] = readData(base + pos);
      }
      char buf[100];
      sprintf(buf, " %03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
              base,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
      Serial.print(buf);
    }
  }


void setup() {
  PORTD &= 239U;       // ST_CLK Set low
  DDRD |= 28U;         // (ST_CLK,SHFT_CLK,SHFT_DATA Set as OUTPUT)
  PORTC |= 2U;         // (VPP, HIGH)
  DDRC |= 2U;          // (VPP, OUTPUT)
  PORTC |= 60U;        // (P2_6,P2_7,P3_6,P3_7 Set HIGH)
  DDRC |= 60U;         // (P2_6,P2_7,P3_6,P3_7 Set as OUTPUT)
  PORTB |= 32U;        // (PROG, HIGH)
  DDRB |= 32U;         // (PROG, OUTPUT)
  
  Serial.begin(57600);
  readSignByte();      // Testing 
  delay(10);
  chipErase();
  delay(10);
  writeData(100,200);
  delay(10);
  readData(100);
  delay(10);
  printDatas();
}

void loop() {}
