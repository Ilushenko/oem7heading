/// \file       Receiver.h
/// \brief      This file is part of OEM7 Heading
///	\copyright  &copy; https://github.com/Ilushenko Oleksandr Ilushenko 
///	\author     Oleksandr Ilushenko
/// \date       2024
#ifndef __OEM7_RECEIVER_H__
#define __OEM7_RECEIVER_H__

#include "oem7.h"
#if defined(ESP8266) || defined(ESP32)
#include "HardwareSerial.h"
#define SERIALPORT HardwareSerial
#else
#include <serialib.h>
#define SERIALPORT serialib
#endif

/// \defgroup oem7rec OEM7 Receiver
/// \brief OEM7 Receiver communication class
/// \details A class that provides communication with the GNSS module

namespace oem7 {
    /// \class oem7::Receiver Receiver.h
    /// \brief Provide time, position and heading by GNSS with multiple rovers
    /// \details Send OEM7 commands to GNSS module via serial
    /// \details Read OEM7 messages from GNSS module via serial
    /// \ingroup oem7rec
    class Receiver {
        Receiver() = delete;
        Receiver(const Receiver&) = delete;
        Receiver& operator = (const Receiver&) = delete;
    public:
        /// \brief Constructor
        /// \param serial Serial interface reference
        explicit Receiver(SERIALPORT& serial);
        /// \brief Destructor
        ~Receiver() {}
    public:
        /// \brief Starting working GNSS module
        /// \details Call this method at the beginning of the program
        void begin();
        /// \brief Stop working GNSS module
        /// \details Call this method at the end of the program
        void stop();
        /// \brief Update data of GNSS module
        /// \details To be called in the main program loop
        void update();
        /// \brief Factory reset
        /// \details Clears selected data from NVM and reset
        /// \details After reseting restore baud rate
        /// \details Arduino platform only
        /// \note Erase all user settings!
        void reset();
        /// \brief Configure device
        /// \details Change default device settings: antenna, status, jammer detection sensitivity etc
        /// \details Call thie method before \c Receiver::begin()
        void config();
    public:
        /// @{
        /// \name Getters

        /// \return Number of version components (cards, and so on)
        inline uint32_t versionComponent() const { return _versionIdx; }
        /// \brief Version getter
        /// \param idx Index of version component
        /// \return Version structure reference
        inline const Version& version(const uint32_t idx) const { return _version[idx > _versionIdx ? 0 : idx]; }
        /// \return Data valid flag
        inline bool isValid() const { return _valid; }
        /// \return Jamming detected
        inline bool isJamming() const { return (_rxstatus.rxstat & 0x00008000); }
        /// \return Spoofing detected
        inline bool isSpoofing() const { return _rxstatus.rxstat & 0x00000200; }
        /// \return Bestpos position type (See: \b Position \b or \b Velocity \b Type enumerator)
        inline uint8_t positionType() const { return _bestpos.positionType; }
        /// \return Heading position type (See: \b Position \b or \b Velocity \b Type enumerator)
        inline uint8_t headingType() const { return _heading.positionType; }
        /// \return Latitude (degrees)
        inline double lat() const { return _bestpos.lat; }
        /// \return Longitude (degrees)
        inline double lon() const { return _bestpos.lon; }
        /// \return Height above mean sea level (metres)
        inline double alt() const { return _bestpos.alt; }
        /// \return Latitude standard deviation (m)
        inline float latDev() const { return _bestpos.latStdDev; }
        /// \return Longitude standard deviation (m)
        inline float lonDev() const { return _bestpos.lonStdDev; }
        /// \return Height standard deviation (m)
        inline float altDev() const { return _bestpos.altStdDev; }
        /// \return Heading in degrees (0 to 359.999 degrees)
        inline double heading() const { return _heading.heading; }
        /// \return Heading standard deviation in degrees
        inline float  headingDev() const { return _heading.hdgStdDev; }
        /// \return Pitch (+/-90 degrees)
        inline double pitch() const { return _heading.pitch; }
        /// \return Pitch standard deviation in degrees
        inline float pitchDev() const { return _heading.ptchStdDev; }
        /// \return Number of Satellites in View
        inline uint8_t satellitesView() const { return _heading.satellitesTracked; }
        /// \return Number of Satellites in Used
        inline uint8_t satellitesUsed() const { return _heading.satellitesUsed; }
        /// \brief UTC-Time Getter
        /// \param year UTC year
        /// \param month UTC month (0-12). If UTC time is unknown, the value for month is 0.
        /// \param day UTC day (0-31). If UTC time is unknown, the value for day is 0.
        /// \param hour UTC hour (0-23)
        /// \param minute UTC minute (0-59)
        /// \param second UTC second (0-59)
        /// \return UTC-Time is Valid
        inline bool utcTime(uint32_t& year, uint8_t& month, uint8_t& day, uint8_t& hour, uint8_t& minute, uint8_t& second) const
        {
            year = _time.utc_year;
            month = _time.utc_month;
            day = _time.utc_day;
            hour = _time.utc_hour;
            minute = _time.utc_min;
            second = static_cast<uint8_t>(_time.utc_ms / 1000);
            return _time.clock_status == CLOCK_VALID && _time.utc_status == UTC_VALID;
        }
        /// @}
    private:
        /// \brief Send command to GNSS module
        /// \details Using abbreviated ASCII command
        /// \details See: https://docs.novatel.com/OEM7/Content/Commands/OEM7_Core_Commands.htm
        /// \param cmd Abbreviated ASCII command
        void setCommand(const char* cmd);
        /// \brief Read GNSS data
        /// \details Read and parse data from serial
        /// \details Check data CRC
        /// \details See: https://docs.novatel.com/OEM7/Content/Messages/Binary.htm
        uint16_t getData();
        /// \brief Wait for serial available
        /// \param timeout Wait timeout in ms
        /// \return \c true if serial available or \c false if timeout
        bool waitAvailable(const unsigned long timeout) const;
    private:
        /// \brief Print hardware monitor parameters
        /// \details Print hardware monitor temperature, antenna current and voltages
        /// \param boundary Boundary limit status of oem7::HWMonitor structure
        /// \param type Reading type of oem7::HWMonitor structure
        /// \param value Temperature, antenna current or voltage reading of oem7::HWMonitor structure
        static void hardwareInfo(const uint8_t boundary, const uint8_t type, const float value);
        /// \brief Print status info
        /// \details Print information about receiver and auxiliary bitmask
        /// \param word Status word of oem7::RxStatus structure
        /// \param bitmask receiver error, receiver status or auxiliary status of oem7::RxStatus structure
        static void statusInfo(const uint8_t word, const uint32_t bitmask);
	    /// \brief Calculates the CRC-32 of a block of data all at once
	    /// \param buffer Data block buffer
	    /// \param size Data block size (in bytes)
	    /// \return CRC for data verification
        static uint32_t crc32(const uint8_t* buffer, size_t size);
    private:
        SERIALPORT& _serial;
        bool _valid{ false };
        uint32_t _versionIdx{ 0 };
        uint32_t _measurement{ 0 };
        Version _version[8]{ 0 };
        HWMonitor _monitor[10]{ 0 };
	    RxStatus _rxstatus{ 0 };
        RxStatusEvent _event{ 0 };
	    Time _time{ 0 };
	    BestPos _bestpos{ 0 };
	    DualAntHeading _heading{ 0 };
    };
}

#endif // __OEM7_RECEIVER_H__
