#include "oem7.h"

const char* oem7::txterror[33] {
	"DRAM failure",						// 0x00000001
	"Invalid firmware",					// 0x00000002
	"ROM",								// 0x00000004
	"",									// 0x00000008
	"ESN access",						// 0x00000010
	"Authorization code",				// 0x00000020
	"",									// 0x00000040
	"Supply voltage",					// 0x00000080
	"",									// 0x00000100
	"Temperature status",				// 0x00000200
	"MINOS status",						// 0x00000400
	"PLL RF status",					// 0x00000800
	"",									// 0x00001000
	"",									// 0x00002000
	"",									// 0x00004000
	"NVM status",						// 0x00008000
	"Software resource limit exceeded",	// 0x00010000
	"Model invalid for this receiver",	// 0x00020000
	"",									// 0x00040000
	"",									// 0x00080000
	"Remote loading has begun",			// 0x00100000
	"Export restriction",				// 0x00200000
	"Safe Mode",						// 0x00400000
	"",									// 0x00800000
	"",									// 0x01000000
	"",									// 0x02000000
	"",									// 0x04000000
	"",									// 0x08000000
	"",									// 0x10000000
	"",									// 0x20000000
	"",									// 0x40000000
	"Component hardware failure",		// 0x80000000
	"#ERROR"
};

const char* oem7::txtstatus[33] = {
	"Error",							// 0x00000001
	"Temperature warning",				// 0x00000002
	"Voltage supply warning",			// 0x00000004
	"Primary antenna not powered",		// 0x00000008
	"LNA Failure",						// 0x00000010
	"Primary antenna open circuit",		// 0x00000020
	"Primary antenna short circuit",	// 0x00000040
	"CPU overload",						// 0x00000080
	"COM buffer overrun",				// 0x00000100
	"Spoofing detected",				// 0x00000200
	"",									// 0x00000400
	"Link overrun",						// 0x00000800
	"Input overrun",					// 0x00001000
	"Aux transmit overrun",				// 0x00002000
	"Antenna gain out of range",		// 0x00004000
	"Jammer Detected",					// 0x00008000
	"INS reset",						// 0x00010000
	"IMU communication failure",		// 0x00020000
	"GPS almanac flag/UTC known",		// 0x00040000
	"Position solution invalid",		// 0x00080000
	"Position fixed",					// 0x00100000
	"Clock steering disabled",			// 0x00200000
	"Clock model invalid",				// 0x00400000
	"External oscillator locked",		// 0x00800000
	"Software resource warning",		// 0x01000000
	"",									// 0x06000000
	"",									// 0x06000000
	"Tracking mode: HDR",				// 0x08000000
	"Digital Filtering Enabled",		// 0x10000000
	"Auxiliary 3 event",				// 0x20000000
	"Auxiliary 2 event",				// 0x40000000
	"Auxiliary 1 event",				// 0x80000000
	"#STATUS"
};

const char* oem7::txtaux1[33] = {
	"Jammer detected on RF1",			// 0x00000001
    "Jammer detected on RF2",			// 0x00000002
    "Jammer detected on RF3",			// 0x00000004
    "Position averaging on",			// 0x00000008
    "Jammer detected on RF4",			// 0x00000010
    "Jammer detected on RF5",			// 0x00000020
    "Jammer detected on RF6",			// 0x00000040
    "USB not connected",				// 0x00000080
    "USB1 buffer overrun",				// 0x00000100
    "USB2 buffer overrun",				// 0x00000200
    "USB3 buffer overrun",				// 0x00000400
    "",									// 0x00000800
    "Profile activation error",			// 0x00001000
    "Throttled ethernet reception",		// 0x00002000
    "",									// 0x00004000
    "",									// 0x00008000
    "",									// 0x00010000
    "",									// 0x00020000
    "Ethernet not connected",			// 0x00040000
    "ICOM1 buffer overrun",				// 0x00080000
    "ICOM2 buffer overrun",				// 0x00100000
    "ICOM3 buffer overrun",				// 0x00200000
    "NCOM1 buffer overrun",				// 0x00400000
    "NCOM2 buffer overrun",				// 0x00800000
    "NCOM3 buffer overrun",				// 0x01000000
    "",									// 0x02000000
    "",									// 0x04000000
    "",									// 0x08000000
    "",									// 0x10000000
    "",									// 0x20000000
    "Status error reported by the IMU",	// 0x40000000
    "IMU measurement outlier detected",	// 0x80000000
	"#AUX1"
};

const char* oem7::txtaux2[33] = {
    "SPI communication failure",		// 0x00000001
    "I2C communication failure",		// 0x00000002
    "COM4 buffer overrun",				// 0x00000004
    "COM5 buffer overrun",				// 0x00000008
    "",									// 0x00000010
    "",									// 0x00000020
    "",									// 0x00000040
    "",									// 0x00000080
    "",									// 0x00000100
    "COM1 buffer overrun",				// 0x00000200
    "COM2 buffer overrun",				// 0x00000400
    "COM3 buffer overrun",				// 0x00000800
    "PLL RF1 unlock",					// 0x00001000
    "PLL RF2 unlock",					// 0x00002000
    "PLL RF3 unlock",					// 0x00004000
    "PLL RF4 unlock",					// 0x00008000
    "PLL RF5 unlock",					// 0x00010000
    "PLL RF6 unlock",					// 0x00020000
    "CCOM1 buffer overrun",				// 0x00040000
    "CCOM2 buffer overrun",				// 0x00080000
    "CCOM3 buffer overrun",				// 0x00100000
    "CCOM4 buffer overrun",				// 0x00200000
    "CCOM5 buffer overrun",				// 0x00400000
    "CCOM6 buffer overrun",				// 0x00800000
    "ICOM4 buffer overrun",				// 0x01000000
    "ICOM5 buffer overrun",				// 0x02000000
    "ICOM6 buffer overrun",				// 0x04000000
    "ICOM7 buffer overrun",				// 0x08000000
    "Secondary antenna not powered",	// 0x10000000
    "Secondary antenna open circuit",	// 0x20000000
    "Secondary antenna short circuit",	// 0x40000000
    "Reset loop detected",				// 0x80000000
	"#AUX2"
};

const char* oem7::txtaux3[33] = {
	"SCOM buffer overrun",								// 0x00000001
	"WCOM1 buffer overrun",								// 0x00000002
	"FILE buffer overrun",								// 0x00000004
	"",													// 0x00000008
	"",													// 0x00000030
	"",													// 0x00000030
	"",													// 0x000000C0
	"",													// 0x000000C0
	"GPS reference time is incorrect",					// 0x00000100
	"",													// 0x00000200
	"",													// 0x00000400
	"",													// 0x00000800
	"",													// 0x00001000
	"",													// 0x00002000
	"",													// 0x00004000
	"",													// 0x00008000
	"DMI hardware failure", 							// 0x00010000
	"", 												// 0x00020000
	"", 												// 0x00040000
	"", 												// 0x00080000
	"", 												// 0x00100000
	"", 												// 0x00200000
	"", 												// 0x00400000
	"", 												// 0x00800000
	"Spoofing ñalibration failed", 						// 0x01000000
	"Spoofing ñalibration required", 					// 0x02000000
	"", 												// 0x04000000
	"",							 						// 0x08000000
	"", 												// 0x10000000
	"Web content is corrupt or does not exist", 		// 0x20000000
	"RF Calibration Data has an error", 				// 0x40000000
	"RF Calibration Data is exists and has no errors",	// 0x80000000
	"#AUX3"
};


const char* oem7::txtaux4[33] = {
	"< 60% of available satellites are tracked well", 	// 0x00000001
	"< 15% of available satellites are tracked well", 	// 0x00000002
	"",													// 0x00000004
	"", 												// 0x00000008
	"", 												// 0x00000010
	"", 												// 0x00000020
	"", 												// 0x00000040
	"", 												// 0x00000080
	"", 												// 0x00000100
	"", 												// 0x00000200
	"", 												// 0x00000400
	"", 												// 0x00000800
	"Clock freewheeling due to bad position integrity", // 0x00001000
	"", 												// 0x00002000
	"< 60% of expected corrections available", 			// 0x00004000
	"< 15% of expected corrections available", 			// 0x00008000
	"Bad RTK Geometry", 								// 0x00010000
	"", 												// 0x00020000
	"", 												// 0x00040000
	"Long RTK Baseline >50 km", 						// 0x00080000
	"Poor RTK COM Link corrections quality <= 60%", 	// 0x00100000
	"Poor ALIGN COM Link corrections quality <= 60%", 	// 0x00200000
	"GLIDE Not Active", 								// 0x00400000
	"Bad PDP Geometry", 								// 0x00800000
	"No TerraStar Subscription",						// 0x01000000
	"",													// 0x02000000
	"",													// 0x04000000
	"",													// 0x08000000
	"Bad PPP Geometry",									// 0x10000000
	"",													// 0x20000000
	"No INS Alignment",									// 0x40000000
	"INS not converged",								// 0x80000000
	"#AUX4"
};
