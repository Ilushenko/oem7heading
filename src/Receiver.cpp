/// \file       Receiver.cpp
/// \brief      This file is part of OEM7 Heading
///	\copyright  &copy; https://github.com/Ilushenko Oleksandr Ilushenko 
///	\author     Oleksandr Ilushenko
/// \date       2024
#include "Receiver.h"

#ifdef DEBUGLOG
# if defined(ESP8266) || defined(ESP32)
#  define xLog(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#  define xDebug(fmt, ...) Serial.printf("[%s %s:%i] ", __FUNCTION__, __FILENAME__, __LINE__); Serial.printf(fmt, ##__VA_ARGS__)
# else
# include <cstdio>
# include <chrono>
#  ifdef _WIN32
#   define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#  else
#   define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#  endif // _WIN32
#  define xLog(fmt, ...) printf(fmt, ##__VA_ARGS__)
#  define xDebug(fmt, ...) printf("[%s %s:%i] ", __FUNCTION__, __FILENAME__, __LINE__); printf(fmt, ##__VA_ARGS__)
# endif
#else
# ifdef _WIN32
#  pragma warning(push)
#  pragma warning(disable : 4390)
# endif
# define xLog(fmt, ...)
# define xDebug(fmt, ...)
#endif

oem7::Receiver::Receiver(SERIALPORT& serial) : _serial(serial)
{
}

void oem7::Receiver::reset()
{
	// Factory Reset
#if defined(ESP8266) || defined(ESP32)
	setCommand("FRESET STANDARD");
	delay(5000);
	// Default baud rate
	_serial.begin(9600, SERIAL_8N1, 5, 18);
	// Restore baud rate
	setCommand("UNLOGALL TRUE");
	setCommand("SERIALCONFIG COM1 115200 N 8 1 N ON");
	delay(1000);
	_serial.begin(115200, SERIAL_8N1, 5, 18);
	setCommand("SAVECONFIG");
#endif
}

void oem7::Receiver::begin()
{
	setCommand("UNLOGALL TRUE");

	// setCommand("STATUSCONFIG PRIORITY STATUS 0");
	// setCommand("STATUSCONFIG PRIORITY AUX1 0x00000008");
	// setCommand("STATUSCONFIG PRIORITY AUX2 0");
	
	// setCommand("STATUSCONFIG SET STATUS 0x00000000");
	// setCommand("STATUSCONFIG SET AUX1 0");
	// setCommand("STATUSCONFIG SET AUX2 0");
	// setCommand("STATUSCONFIG SET AUX4 0xFFFFFFFF");
	
	// setCommand("STATUSCONFIG CLEAR STATUS 0x00000000");
	// setCommand("STATUSCONFIG CLEAR AUX1 0");
	// setCommand("STATUSCONFIG CLEAR AUX2 0");
	// setCommand("STATUSCONFIG CLEAR AUX4 0");

	// ITDETECTCONFIG

	// RTKANTENNA
	// BestPos::solutionStatusEx
	// DualAntHeading::solutionStatusEx

	setCommand("ANTENNAPOWER ON");
	setCommand("RFINPUTGAIN L1 AUTO");
	setCommand("RFINPUTGAIN L2 AUTO");
	//setCommand("RFINPUTGAIN L5 AUTO");
	setCommand("ASSIGNALL ALL AUTO");
	setCommand("DUALANTENNAALIGN ENABLE 5 5");

	setCommand("SAVECONFIG");

	setCommand("LOG COM1 VERSIONB ONCE");
	waitAvailable(100);
	if (getData() != MSG_VERSION) {
		_versionIdx = 0;
		xDebug("#VERSION Read Error!\n");
	}
	setCommand("LOG COM1 RXSTATUSB ONTIME 1");
	setCommand("LOG COM1 TIMEB ONTIME 1");
	setCommand("LOG COM1 BESTPOSB ONTIME 0.25");
	setCommand("LOG COM1 DUALANTENNAHEADINGB ONTIME 0.25");
}

void oem7::Receiver::stop()
{
	setCommand("UNLOGALL TRUE");
}

void oem7::Receiver::update()
{
	bool isRx = false;
	bool isTime = false;
	bool isBestPos = false;
	bool isHeading2 = false;
	while (_serial.available() > 0) {
		switch (getData()) {
		case MSG_RXSTATUS:
			isRx = true;
			break;
		case MSG_TIME:
			isTime = true;
			break;
		case MSG_BESTPOS:
			isBestPos = true;
			break;
		case MSG_DUALANTHEADING:
			isHeading2 = true;
			break;
		}
	}
	// Status
    _valid = false;
	if (isRx) {
		xDebug("#RXSTATUS[ Error: %u, Number: %u\n"
			"\tRX:   %08X %08X %08X %08X\n"
			"\tAUX1: %08X %08X %08X %08X\n"
			"\tAUX2: %08X %08X %08X %08X\n"
			"\tAUX3: %08X %08X %08X %08X\n"
			"\tAUX4: %08X %08X %08X %08X ]\n",
			_rxstatus.error, _rxstatus.numStats,
			_rxstatus.rxstat, _rxstatus.rxstatPri, _rxstatus.rxstatSet, _rxstatus.rxstatClear,
			_rxstatus.aux1stat, _rxstatus.aux1statPri, _rxstatus.aux1statSet, _rxstatus.aux1statClear,
			_rxstatus.aux2stat, _rxstatus.aux2statPri, _rxstatus.aux2statSet, _rxstatus.aux2statClear,
			_rxstatus.aux3stat, _rxstatus.aux3statPri, _rxstatus.aux3statSet, _rxstatus.aux3statClear,
			_rxstatus.aux4stat, _rxstatus.aux4statPri, _rxstatus.aux4statSet, _rxstatus.aux4statClear
		);
		if (!checkDevice()) return;
		if (!checkAntenna()) return;
	    _jamming = checkJamming();
    	_spoofing = checkSpoofing();
		checkRTK();
	}
	if (isTime) {
		xDebug("#TIME[Status: %u, %04u-%02u-%02u %02u:%02u:%02u UTC: %u]\n",
			_time.clock_status, _time.utc_year, _time.utc_month, _time.utc_day,
			_time.utc_hour, _time.utc_min, _time.utc_ms / 1000, _time.utc_status
		);
	}
	if (isBestPos) {
		xDebug("#BESTPOS[Status: %u, PosType: %u, lat: %0.9f, lon: %0.9f, alt: %.02f, SatView: %u, SatUsed: %u]\n",
			_bestpos.solutionStatus, _bestpos.positionType,
			_bestpos.lat, _bestpos.lon, _bestpos.alt,
			_bestpos.satellitesTracked, _bestpos.satellitesUsed
		);
		if (_bestpos.solutionStatus != SOL_COMPUTED) isBestPos = false;
	}
	if (isHeading2) {
		xDebug("#HEADING[Status: %u, PosType: %u, Lenght: %.02f, Heading: %.02f, HeadingDev: %.02f Pitch: %.02f PitchDev: %.02f, SatView: %u, SatUsed: %u]\n",
			_heading.solutionStatus, _heading.positionType,
			_heading.length, _heading.heading, _heading.hdgStdDev, _heading.pitch, _heading.ptchStdDev,
			_heading.satellitesTracked, _heading.satellitesUsed
		);
		if (_heading.solutionStatus != SOL_COMPUTED) isHeading2 = false;
	}
	_valid = isBestPos && isHeading2;
}

void oem7::Receiver::setCommand(const char *cmd)
{
	// Write Abbreviated ASCII Command.
#if defined(ESP8266) || defined(ESP32)
	_serial.write(cmd, strlen(cmd));
	_serial.write("\n", 1);
#else
	_serial.writeBytes(cmd, strlen(cmd));
	_serial.writeBytes("\n", 1);
	auto yield = [&]() {
#ifdef WIN32
		::Sleep(0);
#else
		usleep(0);
#endif
	};
#endif
	xLog(">%s\n", cmd);
	// Read Abbreviated ASCII Response. Example: \r\n<OK\r\n[COM1]
	if (!waitAvailable(100)) return;
	uint8_t data = 0;
	while (_serial.available() > 0) {
		if (_serial.readBytes(&data, 1) != 1) continue;
		if (data != 0x0D && data != 0x0A) xLog("%c", data);
		if (data == ']') break;
	}
	xLog("\n");
}

uint16_t oem7::Receiver::getData()
{
	static Head head = { 0 };
	static uint8_t crcbuf[4] = { 0 };
	static uint8_t buffer[1024] = { 0 };
	// 0xAA Sync Byte
	if (_serial.readBytes(&buffer[0], 1) != 1) {
		xLog("Error read byte\n");
		return 0;
	}
	//xDebug("1 0x%02X %c\n", buffer[0], buffer[0]);
	if (buffer[0] != HEAD_SYNC_1) return 0;
	// 0x44 Sync Byte
	if (_serial.readBytes(&buffer[1], 1) != 1) {
		xLog("Error read byte\n");
		return 0;
	}
	//xDebug("2 0x%02X %c\n", buffer[1], buffer[1]);
	if (buffer[1] != HEAD_SYNC_2) return 0;
	// 0x12 Sync Byte
	if (_serial.readBytes(&buffer[2], 1) != 1) {
		xLog("Error read byte\n");
		return 0;
	}
	//xDebug("3 0x%02X %c\n", buffer[2], buffer[2]);
	if (buffer[2] != HEAD_SYNC_3) return 0;
	// 0x1C Head Size
	if (_serial.readBytes(&buffer[3], 1) != 1) {
		xLog("Error read byte\n");
		return 0;
	}
	//xDebug("4 0x%02X %c\n", buffer[3], buffer[3]);
	if (buffer[3] != HEAD_LENGHT) {
		xLog("Head Size Wrong: %i\n", static_cast<int>(buffer[3]));
		return 0;
	}
	// Read Header
	if (_serial.readBytes(&buffer[4], sizeof(Head)) != sizeof(Head)) {
		xLog("Head Read Error\n");
		return 0;
	}
	memcpy(&head, &buffer[4], sizeof(Head));
	const size_t size = static_cast<size_t>(head.msgLenght);
	//xDebug("Msg ID: %u Size: %u\n", head.msgId, size);
	// Read Message
	if (_serial.readBytes(&buffer[HEAD_LENGHT], size) != size) {
		xLog("Message Read Error\n");
		return 0;
	}
	// Read CRC
	if (_serial.readBytes(&crcbuf[0], sizeof(uint32_t)) != sizeof(uint32_t)) {
		xLog("CRC Read Error\n");
		return 0;
	}
	uint32_t crc = 0;
	memcpy(&crc, &crcbuf[0], sizeof(crc));
	// Check CRC
	const uint32_t chk = crc32block(&buffer[0], size + HEAD_LENGHT);
	if (crc != chk) {
		xLog("CRC Error! Contain: %u Computed: %u\n", crc, chk);
		return 0;
	}
	// to Data
	switch (head.msgId) {
	case MSG_VERSION:
		if (size < sizeof(uint32_t)) {
			xLog("OEM7Version Wrong Size\n");
			return 0;
		}
		memcpy(&_versionIdx, &buffer[HEAD_LENGHT], sizeof(uint32_t));
		//xDebug("OEM7Version Number: %u\n", _versionIdx);
		if (size - sizeof(uint32_t) != _versionIdx * sizeof(Version)) {
			xLog("OEM7Version Wrong Size\n");
			return 0;
		}
		memcpy(&_version[0], &buffer[HEAD_LENGHT + sizeof(uint32_t)], size - sizeof(uint32_t));
		break;
	case MSG_RXSTATUS:
		if (size != sizeof(oem7::RxStatus)) {
			xLog("OEM7RxStatus Wrong Size\n");
			return 0;
		}
		memcpy(&_rxstatus, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_TIME:
		if (size != sizeof(oem7::Time)) {
			xLog("OEM7Time Wrong Size\n");
			return 0;
		}
		memcpy(&_time, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_BESTPOS:
		if (size != sizeof(oem7::BestPos)) {
			xLog("OEM7BestPos Wrong Size\n");
			return 0;
		}
		memcpy(&_bestpos, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_DUALANTHEADING:
		if (size != sizeof(oem7::DualAntHeading)) {
			xLog("OEM7DualAntHeading Wrong Size\n");
			return 0;
		}
		memcpy(&_heading, &buffer[HEAD_LENGHT], size);
	}
	return head.msgId;
}

bool oem7::Receiver::waitAvailable(const unsigned long timeout) const
{
#if !defined(ESP8266) && !defined(ESP32)
	auto millis = []() {
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		return static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
	};
	auto yield = [&]() {
# ifdef WIN32
		::Sleep(0);
# else
		usleep(0);
# endif
	};
#endif
	unsigned long ms = millis();
	if (static_cast<unsigned long>(ms + timeout) == 0) ms = 0;
	while (_serial.available() == 0) {
		if (millis() - ms > timeout) return false;
		yield();
	}
	return true;
}

bool oem7::Receiver::checkDevice()
{
	if (_rxstatus.error != 0) {
		if (_rxstatus.error & ERR_DRAM) 		xLog("RX ERROR! RAM failure on an OEM7 card may also be indicated by a flashing red LED");
		if (_rxstatus.error & ERR_FIRMWARE) 	xLog("RX ERROR! Invalid firmware");
		if (_rxstatus.error & ERR_ROM) 			xLog("RX ERROR! ROM status");
		if (_rxstatus.error & ERR_ESN) 			xLog("RX ERROR! Electronic Serial Number (ESN) access status");
		if (_rxstatus.error & ERR_AUTH) 		xLog("RX ERROR! Authorization code status");
		if (_rxstatus.error & ERR_VOLTAGE) 		xLog("RX ERROR! Supply voltage status");
		if (_rxstatus.error & ERR_TEMPERATURE)	xLog("RX ERROR! Temperature status (as compared against acceptable limits)");
		if (_rxstatus.error & ERR_MINOS) 		xLog("RX ERROR! MINOS status");
		if (_rxstatus.error & ERR_PLLRF) 		xLog("RX ERROR! PLL RF status. Error with an RF PLL. See AUX2 status bits");
		if (_rxstatus.error & ERR_NVM) 			xLog("RX ERROR! NVM status");
		if (_rxstatus.error & ERR_SOFT_LIMIT) 	xLog("RX ERROR! Software resource limit exceeded");
		if (_rxstatus.error & ERR_MODEL) 		xLog("RX ERROR! Model invalid for this receiver");
		if (_rxstatus.error & ERR_REMOTE) 		xLog("RX ERROR! Remote loading has begun");
		if (_rxstatus.error & ERR_EXPORT) 		xLog("RX ERROR! Export restriction");
		if (_rxstatus.error & ERR_SAFEMODE) 	xLog("RX ERROR! Safe Mode");
		if (_rxstatus.error & ERR_HARDWARE) 	xLog("RX ERROR! Component hardware failure");
		return false;
	}
	uint8_t err = 0;
	if (_rxstatus.rxstat & 0x00000002) {
		xLog("RX WARNING! Temperature status\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000004) {
		xLog("RX WARNING! Voltage supply status\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000080) {
		xLog("RX WARNING! CPU overload\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000100) {
		xLog("RX WARNING! COM buffer overrun\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000800) {
		xLog("RX WARNING! Link overrun\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00001000) {
		xLog("RX WARNING! Input overrun\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00002000) {
		xLog("RX WARNING! Aux transmit overrun\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00010000) {
		xLog("RX WARNING! INS reset\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00020000) {
		xLog("RX WARNING! IMU communication failure\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00800000) {
		xLog("RX WARNING! External oscillator locked\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x01000000) {
		xLog("RX WARNING! Software resource warning\n");
		err++;
	}
	return 0 == err;
}

bool oem7::Receiver::checkAntenna()
{
	uint8_t err = 0;
	if (_rxstatus.rxstat & 0x00000008) {
		xLog("RX WARNING! Primary antenna not powered\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000010) {
		xLog("RX WARNING! LNA Failure\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000020) {
		xLog("OEM7 WARNING! Primary antenna disconnected\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00000040) {
		xLog("RX WARNING! Primary antenna short circuit detected\n");
		err++;
	}
	if (_rxstatus.aux2stat & 0x10000000) {
		xLog("AUX2 WARNING! Secondary antenna not powered\n");
		err++;
	}
	if (_rxstatus.aux2stat & 0x20000000) {
		xLog("AUX2 WARNING! Secondary antenna disconnected\n");
		err++;
	}
	if (_rxstatus.aux2stat & 0x40000000) {
		xLog("AUX2 WARNING! Secondary antenna short circuit detected\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00004000) {
		xLog("RX WARNING! Antenna gain out of range\n");
		err++;
	}
	return 0 == err;
}

bool oem7::Receiver::checkRTK()
{
	uint8_t err = 0;
	if (_rxstatus.rxstat & 0x00040000) {
		xLog("RX WARNING! GPS almanac flag/UTC known invalid\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00080000) {
		xLog("RX WARNING! Position solution flag invalid\n");
		err++;
	}
	if (_rxstatus.rxstat & 0x00400000) {
		xLog("RX WARNING! Clock model flag invalid\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00000002) {
		xLog("AUX4 WARNING! <15%% of available satellites are tracked well\n");
		err++;
	}
	else if (_rxstatus.aux4stat & 0x00000001) {
		xLog("AUX4 WARNING! <60%% of available satellites are tracked well\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00008000) {
		xLog("AUX4 WARNING! <15%% of expected corrections available\n");
		err++;
	}
	else if (_rxstatus.aux4stat & 0x00004000) {
		xLog("AUX4 WARNING! <60%% of expected corrections available\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00010000) {
		xLog("AUX4 WARNING! Bad RTK Geometry. PDOP >5.0\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00080000) {
		xLog("AUX4 WARNING! Long RTK Baseline. Baseline >50 km\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00100000) {
		xLog("AUX4 WARNING! Poor RTK COM Link. Corrections quality <=60%%\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00200000) {
		xLog("AUX4 WARNING! Poor ALIGN COM Link. Corrections quality <=60%%\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00400000) {
		xLog("AUX4 WARNING! GLIDE not active\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x00800000) {
		xLog("AUX4 WARNING! Bad PDP Geometry. PDOP >5.0\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x01000000) {
		xLog("AUX4 WARNING! No TerraStar Subscription\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x10000000) {
		xLog("AUX4 WARNING! Bad PPP Geometry. PDOP >5.0\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x40000000) {
		xLog("AUX4 WARNING! No INS alignment\n");
		err++;
	}
	if (_rxstatus.aux4stat & 0x80000000) {
		xLog("AUX4 WARNING! INS not converged\n");
		err++;
	}
	return 0 == err;
}

bool oem7::Receiver::checkJamming()
{
	uint8_t err = 0;
	if (_rxstatus.rxstat & 0x00008000) {
		xLog("RX WARNING! Jammer detected\n");
		err++;
	}
	if (_rxstatus.aux1stat & 0x00000001) {
		xLog("AUX1 WARNING! Jammer detected on RF1\n");
		err++;
	}
	if (_rxstatus.aux1stat & 0x00000002) {
		xLog("AUX1 WARNING! Jammer detected on RF2\n");
		err++;
	}
	if (_rxstatus.aux1stat & 0x00000004) {
		xLog("AUX1 WARNING! Jammer detected on RF3\n");
		err++;
	}
	if (_rxstatus.aux1stat & 0x00000010) {
		xLog("AUX1 WARNING! Jammer detected on RF4\n");
		err++;
	}
	if (_rxstatus.aux1stat & 0x00000020) {
		xLog("AUX1 WARNING! Jammer detected on RF5\n");
		err++;
	}
	if (_rxstatus.aux1stat & 0x00000040) {
		xLog("AUX1 WARNING! Jammer detected on RF6\n");
		err++;
	}
	return err > 0;
}

bool oem7::Receiver::checkSpoofing()
{
    return _rxstatus.rxstat & 0x00000200;
}

#ifndef DEBUGLOG
# ifdef _WIN32
#  pragma warning(pop)
# endif
#endif
