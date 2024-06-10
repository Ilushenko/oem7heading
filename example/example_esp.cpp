#include "Receiver.h"

HardwareSerial serial(1);

oem7::Receiver gnss(serial);

void setup()
{
  Serial.begin(115200);
  serial.begin(115200, SERIAL_8N1, 5, 18);
  gnss.begin();
}

void loop()
{
  static uint32_t year;
  static uint8_t month, day, hour, minute, second;

  gnss.update();
  if (!gnss.isValid()) return;

  if (gnss.utcTime(year, month, day, hour, minute, second)) {
    Serial.printf("UTC Time: %04u-%02u-%02u %02u:%02u:%02u\n", year, month, day, hour, minute, second);
  }

  switch (gnss.headingPositionType()) {
  case oem7::POS_NARROW_FLOAT:
    Serial.printf("RTK: solution with unresolved, float carrier phase ambiguities\n");
    break;
  case oem7::POS_WIDE_INT:
    Serial.printf("RTK: solution with carrier phase ambiguities resolved to widelane integers\n");
    break;
  case oem7::POS_NARROW_INT:
    Serial.printf("RTK: solution with carrier phase ambiguities resolved to narrow-lane integers\n");
    break;
  default:
    Serial.printf("RTK: solution with no carrier: %u\n", gnss.headingPositionType());
    return;
  }

  Serial.printf("Jamming: %s, Spoofing: %s\n", (gnss.isJamming() ? "yes" : "no"), (gnss.isSpoofing() ? "yes" : "no"));

  Serial.printf("Position: lat = %.09f, lon = %.09f, alt = %.02f\n", gnss.lat(), gnss.lon(), gnss.alt());

  Serial.printf("Heading: heading = %.02f, heading accuracy = %.02f, pitch = %.02f, pitch accuracy = %.02f\n",
      gnss.heading(), gnss.headingDev(), gnss.pitch(), gnss.pitchDev()
  );

  Serial.println();
}
