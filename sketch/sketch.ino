#define TRUE 1
#define FALSE 0

#define chipEnable 10 //PD2 - 12
#define outputEnable 11 //PD3 -13
#define programVoltageEnable 12 //PD4 -14
#define PGM 6 //A14 on 27C256 //PA6 - 33 / PB6 - 7

#define addressHighPort 1 //PORT A // also change PGM
#define addressLowPort 0 //PORT B
#define dataPort 2 //PORT C

typedef enum chipType {
  NONE,
  C16,
  C64,
  C128,
  C256
} Chip;

typedef enum mode {
  WAIT,
  READ,
  WRITE,
  VERIFY,
  CLEAR_CHECK
} Modes;

uint16_t gen_address(uint16_t address);
void set_address(uint16_t address);

uint8_t read_byte();
void write_byte(uint16_t address, uint8_t data);

void select_chip(chipType new_chip);
void program_voltage_set(bool state);

void write_begin();
void write_end();

void dump_chip();
bool recieve_HEX_record();
bool write_HEX_record();
uint8_t read_HEX();

chipType chip = NONE;
Modes mode = WAIT;

uint8_t nmos = 0;
uint8_t rec_len; // recieved Intel HEX record len
uint16_t load_offset; // recieved Intel HEX load offset
uint16_t end_address;
uint8_t buf[16];

void setup() {
  pinMode(chipEnable, OUTPUT);
  digitalWrite(chipEnable, HIGH);

  pinMode(outputEnable, OUTPUT);
  digitalWrite(outputEnable, HIGH);

  pinMode(programVoltageEnable, OUTPUT);
  digitalWrite(programVoltageEnable, LOW);

  portMode(dataPort, INPUT_PULLUP); //MCUDude lib
  portMode(addressLowPort, OUTPUT);
  portMode(addressHighPort, OUTPUT);

  digitalWrite(PGM, HIGH); //A14 in addressHighPort
  
  Serial.begin(57600);
  // One Intel HEX byte is 2 UART bytes @ 57600  baud =
  // = 0,000277777 s = 277,277 us > 110 us write.
  // (baud as bit/s)
  // And for NMOS @ 300 baud = 0,053333 s = 53,3 ms > 50 ms
}

void loop() {
  switch (mode) {
    case READ:
      dump_chip();
      mode = WAIT;
      break;
    case WRITE:
      if (!write_HEX_record()) {
        mode = WAIT;
        write_end();
      }
      break;
    case VERIFY:
    if (!verify_HEX_record()) {
      mode = WAIT;
      write_end();
    }
    break;
    case CLEAR_CHECK:
      clear_check();
      mode = WAIT;
      break;
    default:
      do {} while (Serial.available() == 0);
      switch (Serial.read()) {
        case 'r':
          if (chip == NONE) Serial.println("ENTER CHIP");
          else mode = READ;
          break;
        case 'w':
          if (chip == NONE) Serial.println("ENTER CHIP");
          else {
            mode = WRITE;
            Serial.println("Paste HEX:");
            write_begin();
          }
          break;
        case 'v':
          if (chip == NONE) Serial.println("ENTER CHIP");
          else {
            mode = VERIFY;
            Serial.println("Paste HEX:");
          }
          break;
        case 'f':
          if (chip == NONE) Serial.println("ENTER CHIP");
          else mode = CLEAR_CHECK;
          break;
        case 'a': select_chip(C16); break;
        case 'b': select_chip(C64); break;
        case 'c': select_chip(C128); break;
        case 'd': select_chip(C256); break;
        case 'n': toggle_nmos(); break;
      }
  }
}

void select_chip (chipType new_chip) {
  Serial.print("27");
  if (!nmos) Serial.print("C");
  switch (new_chip) {
    case C16:
      chip = new_chip;
      end_address = 0x7ff;
      Serial.println("16");
      break;
    case C64:
      chip = new_chip;
      end_address = 0x1fff;
      Serial.println("64");
      break;
    case C128:
      chip = new_chip;
      end_address = 0x3fff;
      Serial.println("128");
      break;
    case C256:
      chip = new_chip;
      end_address = 0x7fff;
      Serial.println("256");
      break;
    default:
      chip = NONE;
      end_address = 0x0000;
      Serial.println("NONE");
  }
}

void toggle_nmos() {
  if (nmos) {
    nmos = FALSE;
    Serial.println("Switching to 57600 baud");
    Serial.flush();
    Serial.end();
    Serial.begin(57600);
  } else {
    nmos = TRUE;
    Serial.println("Switching to 300 baud");
    Serial.flush();
    Serial.end();
    Serial.begin(300);
  }
}

void clear_check() {
  uint8_t clear_check = 0xFF;
  for (uint16_t i = 0; i <= end_address; set_address(i++)) clear_check &= read_byte();
  if (clear_check != 0xFF) Serial.println("Chip is dirty");
  else Serial.println("Chip is clean");
}

void dump_chip() {
  for (uint16_t i = 0; i <= end_address; i += 16) {
    // Used MCUDude cores' printf
    Serial.printf(":10%04X00", i); // ":10" . address offset . "00"
    uint8_t crcacc = 0x10 + highByte(i) + lowByte(i);
    for(uint8_t data, j = 0; j < 16; j++) {
      set_address(i + j);
      data = read_byte();
      Serial.printf("%02X", data);
      crcacc += data;
    }
    Serial.printf("%02X\r\n", lowByte(-crcacc));
  }
  Serial.println(":00000001FF");
}

bool recieve_HEX_record() {
  while (Serial.available() == 0); // wait ":"
  if (isSpace(Serial.peek())) {
    Serial.read();
    return TRUE;
  }
  if (Serial.read() != ':') { // recieve Intel HEX start char
    Serial.println("Bad HEX");
    return FALSE;
  }
  rec_len = read_HEX(); // recieve record len
  if (rec_len > 16) {
    Serial.println("Huge HEX");
    return FALSE;
  }
  load_offset = (read_HEX() << 8) | read_HEX(); // recieve address offset
  if (read_HEX() != 0) { // if RECORD TYPE is 0x01 - End Of File Record (or unknown?)
    Serial.println((read_HEX()==0xFF&&rec_len==0)?"OK":"UNK REC TYPE"); // EOFR check(sum)
    return FALSE;
  }
  uint8_t crcacc = rec_len + highByte(load_offset) + lowByte(load_offset);
  for (uint8_t i = 0; i < rec_len; i++) {
    buf[i]=read_HEX();  // recieve DATA
    crcacc += buf[i];
  }
  crcacc += read_HEX(); // recieve CHKSUM
  if (crcacc) {
    Serial.printf("CHKSUM ERR @ %04X\r\n", load_offset);
    return FALSE;
  }
  return TRUE;
}

bool write_HEX_record() {
  if (!recieve_HEX_record()) return FALSE;
  for (uint8_t i = 0, err = 0; i < rec_len; i++) {
    do write_byte(load_offset + i, buf[i]);
    while (buf[i] != read_byte() && ++err < 10);
    if (err >= 10) { // to avoid Serial buffer overflow
      Serial.printf("BAD 0x%04X\r\n", load_offset + i);
      return FALSE;
    }
  }
  return TRUE;
}

bool verify_HEX_record() {
  if (!recieve_HEX_record()) return FALSE;
  for (uint8_t i = 0; i < rec_len; i++) {
    set_address(load_offset + i);
    if (buf[i] != read_byte()) {
      Serial.printf("BAD 0x%04X\r\n", load_offset + i);
      return FALSE;
    }
  }
  return TRUE;
}


uint8_t read_HEX() {
// Read two ASCII bytes with HEX from Serial and return unsigned 8 bit integer
  // ASCII 48...57 is 0...9, ASCII 65...70 is A...F
  while (Serial.available() < 2); // wait 2 bytes
  uint8_t incoming = Serial.read(); // high byte
  uint8_t hexed;
  hexed = ( incoming < 58 ? incoming - 48 : incoming - 65 + 10 ) << 4;
  incoming = Serial.read(); // low byte
  hexed +=  incoming < 58 ? incoming - 48 : incoming - 65 + 10;
  return hexed;
}

void program_voltage_set (bool state) {
      digitalWrite(programVoltageEnable, state);
}

uint16_t gen_address (uint16_t address) {
  byte high = highByte(address);
  byte low = lowByte(address);
  switch (chip) {
    case C64:
    case C128:
      high |= 1 << 6; // A14 (C256 and C512) is ~PGM for C64 and C128
      break;
    default:
      break;
  }
  return (high << 8) | low;
}

void set_address (uint16_t address) {
  address = gen_address(address);
  portWrite(addressHighPort, highByte(address));
  portWrite(addressLowPort, lowByte(address));
}

uint8_t read_byte () {
  digitalWrite(chipEnable, LOW);
  digitalWrite(outputEnable, LOW);
  _delay_loop_1(3); // tOE delay
  uint8_t data = portRead(dataPort);
  digitalWrite(outputEnable, HIGH);
  if (mode != WRITE) digitalWrite(chipEnable, HIGH);
  return data;
}


void write_begin () {
  program_voltage_set(true);
  switch (chip) {
    case C64:
    case C128:
      digitalWrite(chipEnable, LOW);
      break;
    default:
      break;
  }
}
void write_end () {
  switch (chip) {
    case C64:
    case C128:
      digitalWrite(chipEnable, HIGH);
      break;
    default:
      break;
  }
  program_voltage_set(false);
}

void write_byte (uint16_t address, uint8_t data) {
  portMode(dataPort, OUTPUT);
  set_address(address);
  portWrite(dataPort, data);
  // long tDS and tPW delays for slow crystals
  delayMicroseconds(5); // tDS+
  switch (chip) {
    case C64:
    case C128:
      digitalWrite(PGM, LOW);
      if (nmos) delayMicroseconds(100);
      else delay(50);
      digitalWrite(PGM, HIGH);
      break;
    default:
      digitalWrite(chipEnable, LOW);
      if (nmos) delayMicroseconds(100);
      else delay(50);
      digitalWrite(chipEnable, HIGH);
      break;
  }
  delayMicroseconds(5); // tDH+
  portMode(dataPort, INPUT_PULLUP);
}
