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
#include <hidpp20/UnsupportedFeature.h>
#include <cstdio>
#include <iostream>
#include <memory>

#include "common/common.h"
#include "common/Option.h"
#include "common/CommonOptions.h"
#include "common/EventQueue.h"

extern "C" {
#include <unistd.h>
#include <signal.h>
#include <string.h>
}

using namespace HIDPP20;

class EventHandler
{
public:
	virtual const HIDPP20::FeatureInterface *feature () const = 0;
	virtual void handleEvent (const HIDPP::Report &event) = 0;
};

class IlluminationEventHandler: public EventHandler
{
	HIDPP20::IIllumination _ill;
	bool _state;
	uint16_t _brightness;
	uint16_t _temperature;
	uint16_t _eff_max;
public:
	IlluminationEventHandler (HIDPP20::Device *dev):
		_ill (dev),
		_state (_ill.getIllumination ()),
		_brightness (_ill.getBrightness()),
		_temperature (_ill.getColorTemperature()),
		_eff_max (_ill.getBrightnessEffectiveMax())
	{
		printf ("Light is %s\n", _state ? "on" : "off");
	}

	const HIDPP20::FeatureInterface *feature () const
	{
		return &_ill;
	}

	void handleEvent (const HIDPP::Report &event)
	{
		bool new_state;
		uint16_t value;

		switch (event.function ()) {
		case IIllumination::IlluminationChangeEvent:
			new_state = IIllumination::illuminationChangeEvent (event);
			if (new_state != _state) {
				printf ("Light turned %s\n", new_state ? "on" : "off");
				_state = new_state;
			}
			break;
		case IIllumination::BrightnessChangeEvent:
			value = IIllumination::brightnessChangeEvent (event);
			if (value != _brightness) {
				printf ("Brightness changed from %u to %u\n", _brightness, value);
				_brightness = value;
			}
			break;
		case IIllumination::ColorTemperatureChangeEvent:
			value = IIllumination::colorTemperatureChangeEvent (event);
			if (value != _temperature) {
				printf ("Color temperature changed from %u to %u\n", _temperature, value);
				_temperature = value;
			}
			break;
		case IIllumination::BrightnessEffectiveMaxChangeEvent:
			value = IIllumination::brightnessEffectiveMaxChangeEvent (event);
			if (value != _eff_max) {
				printf ("Effective max brightness changed from %u to %u\n", _eff_max, value);
				_eff_max = value;
			}
			break;
		case IIllumination::BrightnessClampedEvent:
			value = IIllumination::brightnessClampedEvent (event);
			printf ("Brightness clamped to %u\n", value);
			break;
		}
	}
};

class EventListener
{
	HIDPP::Dispatcher *dispatcher;
	HIDPP::DeviceIndex index;
	std::map<uint8_t, std::unique_ptr<EventHandler>> handlers;
	std::map<uint8_t, HIDPP::Dispatcher::listener_iterator> iterators;
public:
	EventListener (HIDPP::Dispatcher *dispatcher, HIDPP::DeviceIndex index):
		dispatcher (dispatcher),
		index (index)
	{
	}

	virtual ~EventListener ()
	{
		removeEventHandlers ();
	}

	virtual void addEventHandler (std::unique_ptr<EventHandler> &&handler)
	{
		uint8_t feature = handler->feature ()->index ();
		EventHandler *ptr = handler.get ();
		handlers.emplace (feature, std::move (handler));
		auto it = dispatcher->registerEventHandler (index, feature, [ptr] (const HIDPP::Report &report) {
			ptr->handleEvent (report);
			return true;
		});
		iterators.emplace (feature, it);
	}

	virtual void removeEventHandlers ()
	{
		for (const auto &p: iterators)
			dispatcher->unregisterEventHandler (p.second);
		handlers.clear ();
		iterators.clear ();
	}

	virtual void start () = 0;
	virtual void stop () = 0;

protected:
	virtual bool event (EventHandler *handler, const HIDPP::Report &report) = 0;
};

class SimpleListener: public EventListener
{
	HIDPP::SimpleDispatcher *dispatcher;
public:
	SimpleListener (HIDPP::SimpleDispatcher *dispatcher, HIDPP::DeviceIndex index):
		EventListener (dispatcher, index),
		dispatcher (dispatcher)
	{
	}

	virtual void start ()
	{
		dispatcher->listen ();
	}

	virtual void stop ()
	{
		dispatcher->stop ();
	}

protected:
	virtual bool event (EventHandler *handler, const HIDPP::Report &report)
	{
		handler->handleEvent (report);
		return true;
	}
};

EventListener *listener;

void sigint (int)
{
	listener->stop ();
}



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

	std::unique_ptr<HIDPP::SimpleDispatcher> dispatcher;
	try {
		dispatcher = std::make_unique<HIDPP::SimpleDispatcher> (path);
	}
	catch (std::exception &e) {
		std::cerr << "Failed to open device: " << e.what () << "." << std::endl;
		return EXIT_FAILURE;
	}
	HIDPP20::Device dev (dispatcher.get (), device_index);
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
		else if (op == "monitor") {
			struct sigaction sa, oldsa;
			memset (&sa, 0, sizeof (struct sigaction));
			sa.sa_handler = sigint;
			sigaction (SIGINT, &sa, &oldsa);

			listener = new SimpleListener (dispatcher.get (), device_index);
			try {
				listener->addEventHandler (std::make_unique<IlluminationEventHandler> (&dev));
			}
			catch (HIDPP20::UnsupportedFeature &e) {
				printf ("%s\n", e.what ());
			}

			listener->start();
			listener->removeEventHandlers ();
			sigaction (SIGINT, &oldsa, nullptr);
			delete listener;
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
