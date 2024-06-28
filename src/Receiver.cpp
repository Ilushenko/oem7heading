/// \file       Receiver.cpp
/// \brief      This file is part of OEM7 Heading
///	\copyright  &copy; https://github.com/Ilushenko Oleksandr Ilushenko 
///	\author     Oleksandr Ilushenko
/// \date       2024
#include "Receiver.h"

#if defined(ESP8266) || defined(ESP32)
# define xLog(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
# ifdef DEBUGLOG
#  define xDebug(fmt, ...) Serial.printf("[%s %s:%i] ", __FUNCTION__, __FILENAME__, __LINE__); Serial.printf(fmt, ##__VA_ARGS__)
# else
#  define xDebug(fmt, ...)
# endif
#else
# include <cstdio>
# include <chrono>
# include <thread>
# define xLog(fmt, ...) printf(fmt, ##__VA_ARGS__)
# ifdef DEBUGLOG
#  ifdef _WIN32
#   define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#  else
#   define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#  endif // _WIN32
#  define xDebug(fmt, ...) printf("[%s %s:%i] ", __FUNCTION__, __FILENAME__, __LINE__); printf(fmt, ##__VA_ARGS__)
# else
#  ifdef _WIN32
#   pragma warning(push)
#   pragma warning(disable : 4390)
#  endif
#  define xDebug(fmt, ...)
# endif
#endif

oem7::Receiver::Receiver(SERIALPORT& serial) : _serial(serial)
{
}

void oem7::Receiver::begin()
{
	setCommand("UNLOGALL TRUE");
	// Version
	setCommand("LOG COM1 VERSIONB ONCE");
	waitAvailable(100);
	if (getData() != MSG_VERSION) {
		_versionIdx = 0;
		xDebug("#VERSION Read Error!\n");
	}
	// Log Messages
	setCommand("LOG COM1 HWMONITORB ONTIME 1");
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
	// Get Data Flag
	enum {
		GET_HWMONITOR	= 0x01,
		GET_RXSTATUS	= 0x02,
		GET_TIME		= 0x04,
		GET_BESTPOS		= 0x08,
		GET_HEADING		= 0x10
	};
	_valid = false;
	uint8_t data = 0;
	// Get Data
	while (_serial.available() > 0) {
		switch (getData()) {
		case MSG_HWMONITOR:
			data |= GET_HWMONITOR;
			break;
		case MSG_RXSTATUS:
			data |= GET_RXSTATUS;
			break;
		case MSG_TIME:
			data |= GET_TIME;
			break;
		case MSG_BESTPOS:
			data |= GET_BESTPOS;
			break;
		case MSG_DUALANTHEADING:
			data |= GET_HEADING;
			break;
		}		
	}
	if (data == 0) return;
	// Monitor
	if ((data & GET_HWMONITOR)) {
		for (uint32_t i = 0; i < _measurement; ++i) 
			hardwareInfo(_monitor[i].boundary, _monitor[i].type, _monitor[i].value);
	}
	// Status
	if ((data & GET_RXSTATUS)) {
		if (_rxstatus.error != 0) {
			statusInfo(WORD_ERROR, _rxstatus.error);
			return;
		}
		statusInfo(WORD_STATUS, _rxstatus.rxstat);
		statusInfo(WORD_AUX1, _rxstatus.aux1stat);
		statusInfo(WORD_AUX2, _rxstatus.aux2stat);
		statusInfo(WORD_AUX3, _rxstatus.aux3stat);
		statusInfo(WORD_AUX4, _rxstatus.aux4stat);
	}
	// Check Antenna 1
	if (_rxstatus.rxstat & 0x00000008) return;		// Primary antenna power
	if (_rxstatus.rxstat & 0x00000010) return;		// LNA Failure
	if (_rxstatus.rxstat & 0x00000020) return;		// Primary antenna open circuit (disconnected)
	if (_rxstatus.rxstat & 0x00000040) return;		// Primary antenna short circuit
	if (_rxstatus.rxstat & 0x00004000) return;		// Antenna gain out of range
	if (_rxstatus.aux3stat & 0x30) return;			// Antenna 1 gain out of range
	// Check Antenna 2
	if (_rxstatus.aux2stat & 0x10000000) return;	// Secondary antenna not powered
	if (_rxstatus.aux2stat & 0x20000000) return;	// Secondary antenna open circuit (disconnected)
	if (_rxstatus.aux2stat & 0x40000000) return;	// Secondary antenna short circuit
	if (_rxstatus.aux3stat & 0xC0) return;			// Antenna 2 gain out of range
	// Check RTK
	if (_rxstatus.rxstat & 0x00040000) return;		// GPS almanac flag/UTC known
	if (_rxstatus.rxstat & 0x00080000) return;		// Position solution invalid
	if (_rxstatus.rxstat & 0x00400000) return;		// Clock model invalid
	// Time
	if ((data & GET_TIME) && _time.clock_status == CLOCK_VALID && _time.utc_status == UTC_VALID) {
		xDebug("#TIME[Status: %u, %04u-%02u-%02u %02u:%02u:%02u UTC]\n",
			_time.clock_status, _time.utc_year, _time.utc_month, _time.utc_day,
			_time.utc_hour, _time.utc_min, _time.utc_ms / 1000
		);
	}
	// BestPos
	if (data & GET_BESTPOS) {
		xDebug("#BESTPOS[Status: %u, PosType: %u, lat: %0.9f, lon: %0.9f, alt: %.02f, SatView: %u, SatUsed: %u]\n",
			_bestpos.solutionStatus, _bestpos.positionType,
			_bestpos.lat, _bestpos.lon, _bestpos.alt,
			_bestpos.satellitesTracked, _bestpos.satellitesUsed
		);		
		if (_bestpos.solutionStatus != SOL_COMPUTED) data &= ~GET_BESTPOS;
	}
	// Heading
	if (data & GET_HEADING) {
		xDebug("#HEADING[Status: %u, StatusEx: %02X, PosType: %u, Lenght: %.02f, Heading: %.02f, HeadingDev: %.02f Pitch: %.02f PitchDev: %.02f, SatView: %u, SatUsed: %u]\n",
			_heading.solutionStatus, _heading.solutionStatusEx, _heading.positionType,
			_heading.length, _heading.heading, _heading.hdgStdDev, _heading.pitch, _heading.ptchStdDev,
			_heading.satellitesTracked, _heading.satellitesUsed
		);
		if (_heading.solutionStatus != SOL_COMPUTED) data &= ~GET_HEADING;
	}
	// Validation
	if ((data & GET_BESTPOS) && (data & GET_HEADING)) {
		_valid = (_heading.positionType == POS_NARROW_INT || _heading.positionType == POS_WIDE_INT || _heading.positionType == POS_NARROW_FLOAT);
		//_valid = (_heading.positionType == POS_NARROW_INT);
	}
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

void oem7::Receiver::config()
{
	setCommand("UNLOGALL TRUE");
	// Antenna config (Talysman TW3972XF)
	setCommand("ANTENNATYPE REMOVE USER_ANTENNA_1");
	setCommand("ANTENNATYPE REMOVE USER_ANTENNA_2");
	setCommand("ANTENNATYPE REMOVE USER_ANTENNA_3");
	setCommand("ANTENNATYPE REMOVE USER_ANTENNA_4");
	setCommand("ANTENNATYPE REMOVE USER_ANTENNA_5");
	// TW3972XF
	setCommand("ANTENNATYPE ADD USER_ANTENNA_1 TW3972XF 13"
		" GPSL1 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GPSL2 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GPSL5 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GLONASSL1 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GLONASSL2 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GALILEOE1 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GALILEOE5A 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" GALILEOE5B 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" BEIDOUB1 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" BEIDOUB2 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" QZSSL1 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" QZSSL2 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
		" QZSSL5 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0"
	);
	setCommand("BASEANTENNATYPE USER_ANTENNA_1 NONE AUTO ANY");
	setCommand("THISANTENNATYPE USER_ANTENNA_1");
	setCommand("ANTENNAPOWER ON");
	setCommand("DUALANTENNAALIGN ENABLE 5 5");
	// Assigns all channels of a satellite system
	setCommand("ASSIGNALL ALL AUTO");
	// Priority no event
	setCommand("STATUSCONFIG PRIORITY STATUS 0");
	setCommand("STATUSCONFIG PRIORITY AUX1 0");
	setCommand("STATUSCONFIG PRIORITY AUX2 0");
	setCommand("STATUSCONFIG PRIORITY AUX3 0");
	setCommand("STATUSCONFIG PRIORITY AUX4 0");
	// Set no event
	setCommand("STATUSCONFIG SET STATUS 0");
	setCommand("STATUSCONFIG SET AUX1 0");
	setCommand("STATUSCONFIG SET AUX2 0");
	setCommand("STATUSCONFIG SET AUX3 0");
	setCommand("STATUSCONFIG SET AUX4 0");
	// Clear no event
	setCommand("STATUSCONFIG CLEAR STATUS 0");
	setCommand("STATUSCONFIG CLEAR AUX1 0");
	setCommand("STATUSCONFIG CLEAR AUX2 0");
	setCommand("STATUSCONFIG CLEAR AUX3 0");
	setCommand("STATUSCONFIG CLEAR AUX4 0");
	// Configure the sensitivity level for the Jammer Detected bit: 0 = most sensitive, 3 = least sensitive
	setCommand("ITWARNINGCONFIG 1");
	// Save Configuration
	setCommand("SAVECONFIG");
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
		xDebug("Message Read Error\n");
		return 0;
	}
	// Read CRC
	if (_serial.readBytes(&crcbuf[0], sizeof(uint32_t)) != sizeof(uint32_t)) {
		xDebug("CRC Read Error\n");
		return 0;
	}
	uint32_t crc = 0;
	memcpy(&crc, &crcbuf[0], sizeof(crc));
	// Check CRC
	const uint32_t chk = crc32(&buffer[0], size + HEAD_LENGHT);
	if (crc != chk) {
		xDebug("CRC Error! Contain: %u Computed: %u\n", crc, chk);
		return 0;
	}
	// to Data
	switch (head.msgId) {
	case MSG_VERSION:
		if (size < sizeof(uint32_t)) {
			xDebug("OEM7Version Wrong Size\n");
			return 0;
		}
		memcpy(&_versionIdx, &buffer[HEAD_LENGHT], sizeof(uint32_t));
		//xDebug("OEM7Version Number: %u\n", _versionIdx);
		if (size - sizeof(uint32_t) != _versionIdx * sizeof(Version)) {
			xDebug("OEM7Version Wrong Size\n");
			return 0;
		}
		memcpy(&_version[0], &buffer[HEAD_LENGHT + sizeof(uint32_t)], size - sizeof(uint32_t));
		break;
	case MSG_HWMONITOR:
		if (size < sizeof(uint32_t)) {
			xDebug("OEM7HWMonitor Wrong Size\n");
			return 0;
		}
		memcpy(&_measurement, &buffer[HEAD_LENGHT], sizeof(uint32_t));
		//xDebug("OEM7HWMonitor Number: %u\n", _measurement);
		if (size - sizeof(uint32_t) != _measurement * sizeof(HWMonitor)) {
			xDebug("OEM7HWMonitor Wrong Size\n");
			return 0;
		}
		memcpy(&_monitor[0], &buffer[HEAD_LENGHT + sizeof(uint32_t)], size - sizeof(uint32_t));
		break;
	case MSG_RXSTATUS:
		if (size != sizeof(oem7::RxStatus)) {
			xDebug("OEM7RxStatus Wrong Size\n");
			return 0;
		}
		memcpy(&_rxstatus, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_RXSTATUSEVENT:
		if (size != sizeof(oem7::RxStatusEvent)) {
			xDebug("OEM7RxStatusEvent Wrong Size\n");
			return 0;
		}
		memcpy(&_event, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_TIME:
		if (size != sizeof(oem7::Time)) {
			xDebug("OEM7Time Wrong Size\n");
			return 0;
		}
		memcpy(&_time, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_BESTPOS:
		if (size != sizeof(oem7::BestPos)) {
			xDebug("OEM7BestPos Wrong Size\n");
			return 0;
		}
		memcpy(&_bestpos, &buffer[HEAD_LENGHT], size);
		break;
	case MSG_DUALANTHEADING:
		if (size != sizeof(oem7::DualAntHeading)) {
			xDebug("OEM7DualAntHeading Wrong Size\n");
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
	auto yield = []() { std::this_thread::yield(); };
#endif
	unsigned long ms = millis();
	if (static_cast<unsigned long>(ms + timeout) == 0) ms = 0;
	while (_serial.available() == 0) {
		if (millis() - ms > timeout) return false;
		yield();
	}
	return true;
}

void oem7::Receiver::hardwareInfo(const uint8_t boundary, const uint8_t type, const float value)
{
	auto limit = [&]() {
		switch (boundary) {
		case BOUNDARY_ACCEPT: return "Ok";
		case BOUNDARY_LOW_WARNING: return "under the lower warning limit";
		case BOUNDARY_LOW_ERROR: return "under the lower error limit";
		case BOUNDARY_UP_WARNING: return "over the upper warning limit";
		case BOUNDARY_UP_ERROR: return "over the upper error limit";
		}
		return "unknown";
	};
	switch (type) {
	case HW_RESERVED: return;
	case HW_TEMPERATURE1:
		xLog("#HW Temperature: %f - %s\n", value, limit());
		break;
	case HW_A1_AMPERAGE:
		xLog("#HW Antenna Current: %f - %s\n", value, limit());
		break;
	case HW_CORE_3V3:
		xLog("#HW Digital Core 3V3 Voltage: %f - %s\n", value, limit());
		break;
	case HW_A1_VOLTAGE:
		xLog("#HW Antenna Voltage: %f - %s\n", value, limit());
		break;
	case HW_CORE_1V2:
		xLog("#HW Digital 1V2 Core Voltage: %f - %s\n", value, limit());
		break;
	case HW_SUPPLY_VOLTAGE:
		xLog("#HW Regulated Supply Voltage: %f - %s\n", value, limit());
		break;
	case HW_CORE_1V8:
		xLog("#HW Digital 1V8 Core Voltage: %f - %s\n", value, limit());
		break;
	case HW_CORE_5V:
		xLog("#HW 5V Voltage: %f - %s\n", value, limit());
		break;
	case HW_TEMPERATURE2:
		xLog("#HW Secondary Temperature: %f - %s\n", value, limit());
		break;
	case HW_PERIPHERAL:
		xLog("#HW Peripheral Core Voltage: %f - %s\n", value, limit());
		break;
	case HW_A2_AMPERAGE:
		xLog("#HW Secondary Antenna Current: %f - %s\n", value, limit());
		break;
	case HW_A2_VOLTAGE:
		xLog("#HW Secondary Antenna Voltage: %f - %s\n", value, limit());
		break;
	}
}

void oem7::Receiver::statusInfo(const uint8_t word, const uint32_t bitmask)
{
	const char** msg = nullptr;
	switch (word) {
	case WORD_ERROR:
		msg = txterror;
		break;
	case WORD_STATUS:
		msg = txtstatus;
		// Version Bits
		switch (bitmask & 0x06000000) {
		case 0x00:
			xLog("#STATUS: Interpret Status/Error Bits as OEM6 or earlier format\n");
			break;
		case 0x02000000:
			xLog("#STATUS: Interpret Status/Error Bits as OEM7 format\n");
			break;
		case 0x04000000:
		case 0x06000000:
			xLog("#STATUS: Reserved for a future version\n");
			break;
		}
		break;
	case WORD_AUX1:
		msg = txtaux1;
		break;
	case WORD_AUX2:
		msg = txtaux2;
		break;
	case WORD_AUX3:
		msg = txtaux3;
		// Antenna 1 Gain State
		switch (bitmask & 0x30) {
		case 0x00:
			xLog("#AUX3: Antenna 1 gain in range\n");
			break;
		case 0x10:
			xLog("#AUX3: Antenna 1 gain high\n");
			break;
		case 0x20:
			xLog("#AUX3: Antenna 1 gain low\n");
			break;
		case 0x30:
			xLog("#AUX3: Antenna 1 gain anomaly\n");
			break;
		default:
			xLog("#AUX3: Antenna 1 gain %02X\n", (bitmask & 0x30));
			break;
		}
		// Antenna 2 Gain State
		switch (bitmask & 0xC0) {
		case 0x00:
			xLog("#AUX3: Antenna 2 gain in range\n");
			break;
		case 0x40:
			xLog("#AUX3: Antenna 2 gain high\n");
			break;
		case 0x80:
			xLog("#AUX3: Antenna 2 gain low\n");
			break;
		case 0xC0:
			xLog("#AUX3: Antenna 2 gain anomaly\n");
			break;
		default:
			xLog("#AUX3: Antenna 2 gain %02X\n", (bitmask & 0xC0));
			break;
		}
		break;
	case WORD_AUX4:
		msg = txtaux4;
		break;
	default:
		return;
	}
	for (uint8_t i = 0; i < sizeof(bitmask) * 8; ++i) {
		if (bitmask & (1 << i)) {
			if (strlen(msg[i]) != 0) xLog("%s: %s\n", msg[32], msg[i]);
		}
	}
}

uint32_t oem7::Receiver::crc32(const uint8_t* buffer, size_t size)
{
	auto crc32value = [](uint32_t val) {
	    for (uint8_t i = 8; i > 0; i--) {
	        if (val & 1)
	            val = (val >> 1) ^ 0xEDB88320L;
	        else
	            val >>= 1;
	    }
	    return val;
	};
	uint32_t t1, t2;
	uint32_t crc = 0;
	while (size-- != 0) {
	    t1 = (crc >> 8) & 0x00FFFFFFL;
	    t2 = crc32value((crc ^ *buffer++) & 0xFF);
	    crc = t1 ^ t2;
	}
	return crc;
}

#ifndef DEBUGLOG
# ifdef _WIN32
#  pragma warning(pop)
# endif
#endif
