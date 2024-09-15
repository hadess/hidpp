/*
 * Copyright 2024 Bastien Nocera
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <hidpp/SimpleDispatcher.h>
#include <hidpp20/Device.h>
#include <hidpp20/Error.h>
#include <hidpp20/IIllumination.h>
#include <cstdio>
#include <iostream>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"

using namespace HIDPP20;

int main (int argc, char *argv[])
{
	static const char *args = "device_path state|toggle|brightness|temp [params...]";
	HIDPP::DeviceIndex device_index = HIDPP::DefaultDevice;

	std::vector<Option> options = {
		DeviceIndexOption (device_index),
		VerboseOption (),
	};
	Option help = HelpOption (argv[0], args, &options);
	options.push_back (help);

	int first_arg;
	if (!Option::processOptions (argc, argv, options, first_arg))
		return EXIT_FAILURE;

	if (argc-first_arg < 2) {
		std::cerr << "Too few arguments." << std::endl;
		std::cerr << getUsage (argv[0], args, &options) << std::endl;
		return EXIT_FAILURE;
	}

	const char *path = argv[first_arg];
	std::string op = argv[first_arg+1];
	first_arg += 2;

	std::unique_ptr<HIDPP::Dispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		std::cerr << "Failed to open device: " << e.what () << "." << std::endl;
		return EXIT_FAILURE;
	}
	Device dev (dispatcher.get (), device_index);
	try {
		IIllumination ill (&dev);
		if (op == "state") {
			if (argc-first_arg < 1) {
				bool state = ill.getIllumination();
				std::cout << "\tstate: " << state << std::endl;
			}
			else {
				char *endptr;
				bool new_state = strtol (argv[first_arg], &endptr, 0) != 0;
				ill.setIllumination(new_state);
			}
		}
		else if (op == "toggle") {
			bool state = ill.getIllumination();
			ill.setIllumination(!state);
		}
		else if (op == "brightness") {
			if (argc-first_arg < 1) {
				uint16_t value = ill.getBrightness();
				std::cout << "\tbrightness: " << value << std::endl;
				IIllumination::Info info = ill.getBrightnessInfo();
				std::cout << "\tmin: " << info.min << std::endl;
				std::cout << "\tmax: " << info.max << std::endl;
				std::cout << "\tres: " << info.res << std::endl;
				value = ill.getBrightnessEffectiveMax();
				std::cout << "\teffective max: " << (value != 0 ? value : info.max)  << std::endl;
			}
			else {
				char *endptr;
				long new_value = strtol (argv[first_arg], &endptr, 0);
				if (*endptr != '\0' || new_value < 0 || new_value > UINT16_MAX) {
					std::cerr << "Invalid brightness value." << std::endl;
					return EXIT_FAILURE;
				}
				ill.setBrightness(new_value);
			}
		}
		else if (op == "temp") {
			if (argc-first_arg < 1) {
				uint16_t value = ill.getColorTemperature();
				std::cout << "\ttemperature: " << value << std::endl;
				IIllumination::Info info = ill.getColorTemperatureInfo();
				std::cout << "\tmin: " << info.min << std::endl;
				std::cout << "\tmax: " << info.max << std::endl;
				std::cout << "\tres: " << info.res << std::endl;
			}
			else {
				char *endptr;
				long new_value = strtol (argv[first_arg], &endptr, 0);
				if (*endptr != '\0' || new_value < 0 || new_value > UINT16_MAX) {
					std::cerr << "Invalid color temperature value." << std::endl;
					return EXIT_FAILURE;
				}
				ill.setColorTemperature(new_value);
			}
		}
		else {
			std::cerr << "Invalid operation: " << op << "." << std::endl;
			return EXIT_FAILURE;
		}
	}
	catch (Error &e) {
		std::cerr << "Error code " << e.errorCode () << ": " << e.what () << std::endl;
		return e.errorCode ();
	}

	return EXIT_SUCCESS;
}
