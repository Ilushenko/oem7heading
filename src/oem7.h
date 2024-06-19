/// \file       oem7.h
/// \brief      This file is part of OEM7 Heading
///	\copyright  &copy; https://github.com/Ilushenko Oleksandr Ilushenko 
///	\author     Oleksandr Ilushenko
/// \date       2024
#ifndef __OEM7_H__
#define __OEM7_H__

#if defined(ESP8266) || defined(ESP32)
#include "Arduino.h"
#else
#include <stdint.h>
#endif

/// \defgroup oem7data OEM7 Protocol
/// \brief Provide OEM7 Protocol structures and data
/// \details Classes, structures and enumerators that ensure the operation of NovAtel OEM7 dual-antenna receiver
/// \anchor strualign
/// \note All structures must defined with 1-byte alignment
/// \li Use \c __attribute__((__packed__)) by \b GCC
/// \li Use \c #pragma \c pack(push,1) and \c #pragma \c pack(pop) directives by \b Win32

#ifdef WIN32
#define ATTR_PACKED
#else
#define ATTR_PACKED __attribute__((__packed__))
#endif

/// \namespace oem7 NovAtel OEM7 Protocol
/// \brief Provides working OEM7 receiver
/// \details Classes, structures and enumerators that ensure the operation of the OEM7 receiver
/// \details See documentation of protocol message structures: https://docs.novatel.com/OEM7/Content/Logs/Core_Logs.htm
namespace oem7 {
    /// \brief Header Sync Bytes and Lenght
    /// \details https://docs.novatel.com/OEM7/Content/Messages/Binary.htm
    /// \ingroup oem7data
    enum {
        HEAD_SYNC_1 	        = 0xAA, ///< Sync Byte 1
        HEAD_SYNC_2 	        = 0x44, ///< Sync Byte 2
        HEAD_SYNC_3 	        = 0x12, ///< Sync Byte 3
        HEAD_LENGHT 	        = 0x1C 	///< Lenght
    };
    /// \brief Message ID
    /// \details Messages for using in this library
    /// \ingroup oem7data
    enum {
        MSG_BESTPOS		        = 42,	///< Best position
        MSG_RXSTATUS	        = 93,	///< Receiver status
        MSG_TIME		        = 101,	///< Time data
        MSG_HEADING2            = 1335,	///< Heading information with multiple rovers
        MSG_DUALANTHEADING      = 2042  ///< Synchronous heading information for dual antenna product
    };
    /// \brief Receiver Error
    /// \details https://docs.novatel.com/OEM7/Content/Logs/RXSTATUS.htm#ReceiverError
    /// \ingroup oem7data
    enum {
        ERR_DRAM		        = 0x00000001,	///< RAM failure on an OEM7 card may also be indicated by a flashing red LED
        ERR_FIRMWARE	        = 0x00000002,	///< Invalid firmware
        ERR_ROM			        = 0x00000004,	///< ROM status
        ERR_ESN			        = 0x00000010,	///< Electronic Serial Number (ESN) access status
        ERR_AUTH		        = 0x00000020,	///< Authorization code status
        ERR_VOLTAGE		        = 0x00000080,	///< Supply voltage status
        ERR_TEMPERATURE         = 0x00000200,	///< Temperature status (as compared against acceptable limits)
        ERR_MINOS		        = 0x00000400,	///< MINOS status
        ERR_PLLRF		        = 0x00000800,	///< PLL RF status. Error with an RF PLL. See AUX2 status bits
        ERR_NVM			        = 0x00008000,	///< NVM status
        ERR_SOFT_LIMIT	        = 0x00010000,	///< Software resource limit exceeded
        ERR_MODEL		        = 0x00020000,	///< Model invalid for this receiver
        ERR_REMOTE		        = 0x00100000,	///< Remote loading has begun
        ERR_EXPORT		        = 0x00200000,	///< Export restriction
        ERR_SAFEMODE	        = 0x00400000,	///< Safe Mode
        ERR_HARDWARE	        = 0x80000000	///< Component hardware failure
    };
    /// \brief Clock Model Status
    /// \details https://docs.novatel.com/OEM7/Content/Logs/CLOCKMODEL.htm#ClockModelStatus
    /// \ingroup oem7data
    enum {
        CLOCK_VALID			    = 0, ///< The clock model is valid
        CLOCK_CONVERGING        = 1, ///< The clock model is near validity
        CLOCK_ITERATING		    = 2, ///< The clock model is iterating towards validity
        CLOCK_INVALID		    = 3  ///< The clock model is not valid
    };
    /// \brief UTC Status
    /// \ingroup oem7data
    enum {
        UTC_INVALID	            = 0,    ///< Invalid 
        UTC_VALID	            = 1,    ///< Valid
        UTC_WARNING	            = 2     ///< Warning (Indicates that the leap second value is used as a default due to the lack of an almanac)
    };
    /// \brief Solution Status
    /// \details https://docs.novatel.com/OEM7/Content/Logs/BESTPOS.htm#SolutionStatus
    /// \ingroup oem7data
    enum {
        SOL_COMPUTED			= 0,	///< Solution computed
        SOL_INSUFFICIENT_OBS	= 1,	///< Insufficient observations
        SOL_NO_CONVERGENCE		= 2,	///< No convergence
        SOL_SINGULARITY			= 3,	///< Singularity at parameters matrix
        SOL_COV_TRACE			= 4,	///< Covariance trace exceeds maximum (trace > 1000 m)
        SOL_TEST_DIST			= 5,	///< Test distance exceeded (maximum of 3 rejections if distance >10 km)
        SOL_COLD_START			= 6,	///< Not yet converged from cold start
        SOL_V_H_LIMIT			= 7,	///< Height or velocity limits exceeded (in accordance with export licensing restrictions)
        SOL_VARIANCE			= 8,	///< Variance exceeds limits
        SOL_RESIDUALS			= 9,	///< Residuals are too large
        SOL_INTEGRITY_WARNING   = 13,	///< Large residuals make position unreliable
        SOL_PENDING				= 18,	///< When a FIX position command is entered, the receiver computes its own position and determines if the fixed position is valid
        SOL_INVALID_FIX			= 19,	///< The fixed position, entered using the FIX position command, is not valid
        SOL_UNAUTHORIZED		= 20,	///< Position type is unauthorized
        SOL_INVALID_RATE		= 22	///< The selected logging rate is not supported for this solution type.
    };
    /// \brief Position or Velocity Type
    /// \details https://docs.novatel.com/OEM7/Content/Logs/BESTPOS.htm#Position_VelocityType
    /// \ingroup oem7data
    enum {
        POS_NONE				= 0,  ///< No solution
        POS_FIXEDPOS			= 1,  ///< Position has been fixed by the FIX position command or by position averaging.
        POS_FIXEDHEIGHT			= 2,  ///< Position has been fixed by the FIX height or FIX auto command or by position averaging
        POS_DOPPLER_VELOCITY	= 8,  ///< Velocity computed using instantaneous Doppler
        POS_SINGLE				= 16, ///< Solution calculated using only data supplied by the GNSS satellites
        POS_PSRDIFF				= 17, ///< Solution calculated using pseudorange differential (DGPS, DGNSS) corrections
        POS_WAAS				= 18, ///< Solution calculated using corrections from an SBAS satellite
        POS_PROPAGATED			= 19, ///< Propagated by a Kalman filter without new observations
        POS_L1_FLOAT			= 32, ///< Single-frequency RTK solution with unresolved, float carrier phase ambiguities
        POS_NARROW_FLOAT		= 34, ///< Multi-frequency RTK solution with unresolved, float carrier phase ambiguities
        POS_L1_INT				= 48, ///< Single-frequency RTK solution with carrier phase ambiguities resolved to integers
        POS_WIDE_INT			= 49, ///< Multi-frequency RTK solution with carrier phase ambiguities resolved to widelane integers
        POS_NARROW_INT			= 50, ///< Multi-frequency RTK solution with carrier phase ambiguities resolved to narrow-lane integers
        POS_RTK_DIRECT_INS		= 51, ///< RTK status where the RTK filter is directly initialized from the INS filter
        POS_INS_SBAS			= 52, ///< INS position, where the last applied position update used a GNSS solution computed using corrections from an SBAS (WAAS) solution
        POS_INS_PSRSP			= 53, ///< INS position, where the last applied position update used a single point GNSS (SINGLE) solution
        POS_INS_PSRDIFF			= 54, ///< INS position, where the last applied position update used a pseudorange differential GNSS (PSRDIFF) solution
        POS_INS_RTKFLOAT		= 55, ///< INS position, where the last applied position update used a floating ambiguity RTK (L1_FLOAT or NARROW_FLOAT) solution
        POS_INS_RTKFIXED		= 56, ///< INS position, where the last applied position update used a fixed integer ambiguity RTK (L1_INT, WIDE_INT or NARROW_INT) solution
        POS_PPP_CONV			= 68, ///< Converging TerraStar-C, TerraStar-C PRO or TerraStar-X solution
        POS_PPP					= 69, ///< Converged TerraStar-C, TerraStar-C PRO or TerraStar-X solution
        POS_OPERATIONAL			= 70, ///< Solution accuracy is within UAL operational limit
        POS_WARNING				= 71, ///< Solution accuracy is outside UAL operational limit but within warning limit
        POS_OUT_OF_BOUNDS		= 72, ///< Solution accuracy is outside UAL limits
        POS_INS_PPP_CONV		= 73, ///< INS position, where the last applied position update used a converging TerraStar-C, TerraStar-C PRO or TerraStar-X PPP (PPP_CONVERGING) solution
        POS_INS_PPP				= 74, ///< INS position, where the last applied position update used a converged TerraStar-C, TerraStar-C PRO or TerraStar-X PPP (PPP) solution
        POS_PPP_BASIC_CONV		= 77, ///< Converging TerraStar-L solution
        POS_PPP_BASIC			= 78, ///< Converged TerraStar-L solution
        POS_INS_PPP_BASIC_CONV	= 79, ///< INS position, where the last applied position update used a converging TerraStar-L PPP (PPP_BASIC) solution
        POS_INS_PPP_BASIC		= 80, ///< INS position, where the last applied position update used a converged TerraStar-L PPP (PPP_BASIC) solution
    };
#ifdef WIN32
#pragma pack(push,1)
#endif
    /// \struct oem7::Head oem7.h
    /// \brief Binary message header structure
    /// \details Using for parsing OEM7 messages
    /// \details https://docs.novatel.com/OEM7/Content/Messages/Binary.htm
    /// \details \ref strualign "Structure alignment"
    /// \ingroup oem7data
    struct ATTR_PACKED Head {
        uint16_t msgId;				///< Message ID
        uint8_t msgType;			///< Message Type
        uint8_t portAddress;		///< Port Address
        uint16_t msgLenght;			///< The length in bytes of the body of the message, not including the header nor the CRC
        uint16_t sequence;			///< Sequence. Used for multiple related logs
        uint8_t idleTime;			///< Time the processor is idle, calculated once per second
        uint8_t timeStatus;			///< Indicates the quality of the GPS reference time
        uint16_t week;				///< GPS reference week number
        uint32_t ms;				///< Milliseconds from the beginning of the GPS reference week
        uint32_t receiverStatus;	///< 32-bits representing the status of various hardware and software components of the receiver
        uint16_t reserved;			///< Reserved for internal use
        uint16_t receiverVersion;	///< A value (0 - 65535) representing the receiver software build number
    };
    /// \struct oem7::RxStatus oem7.h
    /// \brief \c RXSTATUS Binary structure
    /// \details Receiver status
    /// \details https://docs.novatel.com/OEM7/Content/Logs/RXSTATUS.htm
    /// \details \ref strualign "Structure alignment"
    /// \ingroup oem7data
    struct ATTR_PACKED RxStatus {
        uint32_t error;				///< Receiver error
        uint32_t numStats;			///< Number of status codes (including Receiver Status).
        uint32_t rxstat;			///< Receiver status word
        uint32_t rxstatPri;			///< Receiver status priority mask,
        uint32_t rxstatSet;			///< Receiver status event set mask
        uint32_t rxstatClear;		///< Receiver status event clear mask
        uint32_t aux1stat;			///< Auxiliary 1 status word
        uint32_t aux1statPri;		///< Auxiliary 1 status priority mask
        uint32_t aux1statSet;		///< Auxiliary 1 status event set mask
        uint32_t aux1statClear;		///< Auxiliary 1 status event clear mask
        uint32_t aux2stat;			///< Auxiliary 2 status word
        uint32_t aux2statPri;		///< Auxiliary 2 status priority mask
        uint32_t aux2statSet;		///< Auxiliary 2 status event set mask
        uint32_t aux2statClear;		///< Auxiliary 2 status event clear mask
        uint32_t aux3stat;			///< Auxiliary 3 status word
        uint32_t aux3statPri;		///< Auxiliary 3 status priority mask
        uint32_t aux3statSet;		///< Auxiliary 3 status event set mask
        uint32_t aux3statClear;		///< Auxiliary 3 status event clear mask
        uint32_t aux4stat;			///< Auxiliary 4 status word
        uint32_t aux4statPri;		///< Auxiliary 4 status priority mask
        uint32_t aux4statSet;		///< Auxiliary 4 status event set mask
        uint32_t aux4statClear;		///< Auxiliary 4 status event clear mask
    };
    /// \struct oem7::Time oem7.h
    /// \brief \c TIME Binary structure
    /// \details Time data
    /// \details https://docs.novatel.com/OEM7/Content/Logs/TIME.htm
    /// \details \ref strualign "Structure alignment"
    /// \ingroup oem7data
    struct ATTR_PACKED Time {
        uint32_t clock_status;		///< Clock model status
        double offset;				///< Receiver clock offset in seconds from GPS system time
        double offset_std;			///< Receiver clock offset standard deviation (s)
        double utc_offset;			///< The offset of GPS system time from UTC time, computed using almanac parameters.
        uint32_t utc_year;			///< UTC year
        uint8_t utc_month;			///< UTC month (0-12). If UTC time is unknown, the value for month is 0.
        uint8_t utc_day;			///< UTC day (0-31). If UTC time is unknown, the value for day is 0.
        uint8_t utc_hour;			///< UTC hour (0-23)
        uint8_t utc_min;			///< UTC minute (0-59)
        uint32_t utc_ms;			///< UTC millisecond (0-60999). Maximum of 60999 when leap second is applied.
        uint32_t utc_status;		///< UTC status
    };
    /// \struct oem7::BestPos oem7.h
    /// \brief \c BESTPOS Binary structure
    /// \details Best position
    /// \details https://docs.novatel.com/OEM7/Content/Logs/BESTPOS.htm
    /// \details \ref strualign "Structure alignment"
    /// \ingroup oem7data
    struct ATTR_PACKED BestPos {
        uint32_t solutionStatus;	///< Solution status
        uint32_t positionType;		///< Position type
        double lat;					///< Latitude (degrees)
        double lon;					///< Longitude (degrees)
        double alt;					///< Height above mean sea level (metres)
        float undulation;			///< Undulation - the relationship between the geoid and the ellipsoid (m) of the chosen datum
        uint32_t datum_id;			///< Datum ID number
        float latStdDev;			///< Latitude standard deviation (m)
        float lonStdDev;			///< Longitude standard deviation (m)
        float altStdDev;			///< Height standard deviation (m)
        char baseID[4];				///< Base station ID
        float diff_age;				///< Differential age in seconds
        float sol_age;				///< Solution age in seconds
        uint8_t satellitesTracked;	///< Number of satellites tracked
        uint8_t satellitesUsed;		///< Number of satellites used in solution
        uint8_t satellitesL1;		///< Number of satellites with L1/E1/B1 signals used in solution
        uint8_t satellitesMulti;	///< Number of satellites with multi-frequency signals above the mask angle
        uint8_t reserved;			///< Reserved
        uint8_t solutionStatusEx;	///< Extended solution status
        uint8_t gbdMask;			///< Galileo and BeiDou signals u sed mask
        uint8_t gpsMask;			///< GPS and GLONASS signals used mask
    };
    /// \struct oem7::Heading2 oem7.h
    /// \brief \c HEADING2 Binary structure
    /// \details Heading information with multiple rovers
    /// \details https://docs.novatel.com/OEM7/Content/Logs/HEADING2.htm
    /// \details \ref strualign "Structure alignment"
    /// \ingroup oem7data
    struct ATTR_PACKED Heading2 {
        uint32_t solutionStatus;	///< Solution status
        uint32_t positionType;		///< Position type
        float length;				///< Baseline length in metres
        float heading;				///< Heading in degrees (0 to 359.999 degrees)
        float pitch;				///< Pitch (+/-90 degrees)
        float reserved;				///< Reserved
        float hdgStdDev;			///< Heading standard deviation in degrees
        float ptchStdDev;			///< Pitch standard deviation in degrees
        char roverID[4];			///< Rover Receiver ID
        char baseID[4];				///< Base Receiver ID
        uint8_t satellitesTracked;	///< Number of satellites tracked
        uint8_t satellitesUsed;		///< Number of satellites used in solution
        uint8_t satellitesObs;		///< Number of satellites above the elevation mask angle
        uint8_t satellitesMulti;	///< Number of satellites with multi-frequency signals above the mask angle
        uint8_t solutionSource;		///< Solution source
        uint8_t solutionStatusEx;	///< Extended solution status
        uint8_t gbdMask;			///< Galileo and BeiDou signals used mask
        uint8_t gpsMask;			///< GPS and GLONASS signals used mask
    };
    /// \struct oem7::DualAntHeading oem7.h
    /// \brief \c HEADING2 Binary structure
    /// \details Synchronous heading information for dual antenna product
    /// \details https://docs.novatel.com/OEM7/Content/Logs/DUALANTENNAHEADING.htm
    /// \details \ref strualign "Structure alignment"
    /// \ingroup oem7data
    struct ATTR_PACKED DualAntHeading {
        uint32_t solutionStatus;	///< Solution status
        uint32_t positionType;		///< Position type
        float length;				///< Baseline length in metres
        float heading;				///< Heading in degrees (0 to 359.999 degrees)
        float pitch;				///< Pitch (+/-90 degrees)
        float reserved;				///< Reserved
        float hdgStdDev;			///< Heading standard deviation in degrees
        float ptchStdDev;			///< Pitch standard deviation in degrees
        char stationID[4];          ///< Station ID
        uint8_t satellitesTracked;	///< Number of satellites tracked
        uint8_t satellitesUsed;		///< Number of satellites used in solution
        uint8_t satellitesObs;		///< Number of satellites above the elevation mask angle
        uint8_t satellitesMulti;	///< Number of satellites with multi-frequency signals above the mask angle
        uint8_t solutionSource;		///< Solution source
        uint8_t solutionStatusEx;	///< Extended solution status
        uint8_t gbdMask;			///< Galileo and BeiDou signals used mask
        uint8_t gpsMask;			///< GPS and GLONASS signals used mask
    };
#ifdef WIN32
#pragma pack(pop)
#endif
}

#endif // __OEM7_H__
