/// \file       example_esp.cpp
/// \brief      This example how to use a oem7::Receiver class by Win32
///	\author     Oleksandr Ilushenko
/// \date       2024
#include "Receiver.h"

#include <cstdio>
#include <atomic>
#include <thread>
#include <iostream>

#define PORT "\\\\.\\COM3"

int main(int argc, char** argv)
{
	serialib serial;
    char err = serial.openDevice(PORT, 115200);
    if (err != 1) {
		printf("Error opening %s: %i\n", PORT, static_cast<int>(err));
        return -1;
    }
	printf("Open serial: %s\n", PORT);
	// Quit Ctrl
	std::atomic_bool run = true;
	std::thread([&] {
		std::cout << "\nENTER 'q' for Quit" << std::endl;
		std::string command;
		while (run.load() == true) {
			std::cin >> command;
			switch (command[0]) {
			case 'q':
				run.store(false);
				break;
			default:
				std::cout << "Unsupported Command \"" << command[0] << "\"" << std::endl;
				std::cout << "\nENTER 'q' for Quit" << std::endl;
				break;
			}
		}
	}).detach();
	// Setup
	oem7::Receiver gnss(serial);
	gnss.begin();
	// Print Version
	const uint32_t vx = gnss.versionComponent();
	printf("#VERSION [ Number: %u\n", vx);
	for (uint32_t i = 0; i < vx; ++i) {
		printf(" Type: %u, Model: %s, PSN: %s, HW: %s, SW: %s, Boot: %s, Date: %s, Time: %s\n",
			gnss.version(i).type,
			&gnss.version(i).model[0],
			&gnss.version(i).psn[0],
			&gnss.version(i).hw[0],
			&gnss.version(i).sw[0],
			&gnss.version(i).boot[0],
			&gnss.version(i).compdate[0],
			&gnss.version(i).comptime[0]
		);
	}
	printf("]\n");
	// Loop
	uint32_t year;
	uint8_t month, day, hour, minute, second;
	while (run.load() == true) {
		gnss.update();
		
		if (!gnss.isValid()) continue;

		if (gnss.utcTime(year, month, day, hour, minute, second)) {
			printf("UTC Time: %04u-%02u-%02u %02u:%02u:%02u\n", year, month, day, hour, minute, second);
		}

		switch (gnss.headingType()) {
		case oem7::POS_NARROW_FLOAT:
			printf("RTK: solution with unresolved, float carrier phase ambiguities\n");
			break;
		case oem7::POS_WIDE_INT:
			printf("RTK: solution with carrier phase ambiguities resolved to widelane integers\n");
			break;
		case oem7::POS_NARROW_INT:
			printf("RTK: solution with carrier phase ambiguities resolved to narrow-lane integers\n");
			break;
		default:
			printf("RTK: solution with no carrier: %u\n", gnss.headingType());
			continue;
		}

		printf("Jamming: %s, Spoofing: %s\n", (gnss.isJamming() ? "yes" : "no"), (gnss.isSpoofing() ? "yes" : "no"));
		
		printf("Position: lat = %.09f, lon = %.09f, alt = %.02f\n", gnss.lat(), gnss.lon(), gnss.alt());
		
		printf("Heading: heading = %.02f, heading accuracy = %.02f, pitch = %.02f, pitch accuracy = %.02f\n\n",
			gnss.heading(), gnss.headingDev(), gnss.pitch(), gnss.pitchDev()
		);		
	}
	// Exit
	gnss.stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	serial.closeDevice();
	printf("Close serial: %s\n", PORT);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	return 0;
}
