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
        /// \brief Factory reset
        /// \details Clears selected data from NVM and reset
        /// \details After reseting restore baud rate
        /// \note Erase all user settings!
        void reset();
        /// \brief Starting working GNSS module
        /// \details Call this method at the beginning of the program
        void begin();
        /// \brief Stop working GNSS module
        /// \details Call this method at the end of the program
        void stop();
        /// \brief Update data of GNSS module
        /// \details To be called in the main program loop
        void update();
    public:
        /// @{
        /// \name Getters

        /// \return Data valid flag
        inline bool isValid() const { return _valid; }
        /// \return Jamming detected
        inline bool isJamming() const { return _jamming; }
        /// \return Spoofing detected
        inline bool isSpoofing() const { return _spoofing; }
        /// \return Heading position type (See: \b Position \b or \b Velocity \b Type enumerator)
        inline uint8_t headingPositionType() const { return _heading.positionType; }
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
    private:
        /// \return Receiver errors or warnings detected
        bool checkDevice();
        /// \return Antenna primary and secondary antenna errors
        bool checkAntenna();
        /// \return RTK status
        bool checkRTK();
        /// \return Jamming detection
        bool checkJamming();
        /// \return Spoofing detection
        bool checkSpoofing();
    private:
	    /// \brief Calculates the CRC-32 of a block of data all at once
	    /// \param buffer Data block buffer
	    /// \param size Data block size (in bytes)
	    /// \return CRC for data verification
	    inline static uint32_t crc32block(const uint8_t* buffer, size_t size)
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
    private:
        SERIALPORT& _serial;
        bool _valid{ false };
        bool _jamming{ false };
        bool _spoofing{ false };
	    RxStatus _rxstatus{ 0 };
	    Time _time{ 0 };
	    BestPos _bestpos{ 0 };
	    DualAntHeading _heading{ 0 };
    };
}

#endif // __OEM7_RECEIVER_H__
