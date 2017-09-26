/* BeanMidiWireMaster
 * 
 * USB MIDI to BLE MIDI converter for LightBlue Bean+
 * 
 * MIDI messages from a USB MIDI instrument are received by a USB Host Shield
 * attached to another Arduino, fowarded to the Bean+ running this sketch, and
 * then transmitted as BLE MIDI messages.
 * 
 * Run this sketch on a Bean+ and run the UsbMidiWireSlave sketch on an Arduino
 * connected to a USB Host Shield. The two sketches communicate using I2C.
 * 
 * Connect Bean+'s I2C Grove Connector to Arduino's SDA/SCL pins, 5v and GND.
 * 
 * NOTE: Configure Bean+ for 5v operation.
 */

#include <Wire.h>

const uint8_t SLAVE_ADDRESS = 0x42;

boolean connected;

void setup() {
  connected = false;
  displayConnectionState();
  BeanMidi.enable();
  Wire.begin();
  Bean.sleep(1000); // wait for slave setup
}

void loop() {
  // Connection
  if (Bean.getConnectionState() != connected) {
    connected = !connected;
    displayConnectionState();
  }

  uint8_t message[3];
  boolean overflow = false;
  while (!overflow && receive(message) && connected) {
    overflow = !dispatch(message);
  }
  while (BeanMidi.sendMessages());
  if (overflow) {
    BeanMidi.loadMessage(message[0], message[1], message[2]);
  }
}

void displayConnectionState() {
  // LED is blue until we're connected
  Bean.setLedBlue(connected ? 0 : 255);
}

boolean receive(uint8_t message[]) {
  if (Wire.requestFrom(SLAVE_ADDRESS, 3)) {
    uint8_t status = Wire.read();
    uint8_t byte1 = Wire.read();
    uint8_t byte2 = Wire.read();
    if (status) {
      message[0] = status;
      message[1] = byte1;
      message[2] = byte2;
      return true;
    }
  }
  return false;
}

// returns false if loadMessage failed due to overflow 
boolean dispatch(uint8_t message[]) {
  switch (message[0] & 0xF0) {
    case 0x80: // Note Off
    case 0x90: // Note On
    case 0xA0: // After Touch Poly
    //case 0xB0: // Control Change
    case 0xE0: // Pitch Bend
      return BeanMidi.loadMessage(message[0], message[1], message[2]);

    case 0xC0: // Program Change
    //case 0xD0: // After Touch Channel
      return BeanMidi.loadMessage(message[0], message[1], 0);
  }
  return true;
}

