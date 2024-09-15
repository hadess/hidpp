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

#ifndef LIBHIDPP_HIDPP20_IILLUMINATION_H
#define LIBHIDPP_HIDPP20_IILLUMINATION_H

#include <hidpp20/FeatureInterface.h>

namespace HIDPP20
{

/**
 * Control non-RGB LED features.
 */
class IIllumination: public FeatureInterface
{
public:
	static constexpr uint16_t ID = 0x1990;

	enum Function {
		GetIllumination = 0,
		SetIllumination = 1,
		GetBrightnessInfo = 2,
		GetBrightness = 3,
		SetBrightness = 4,
		GetBrightnessLevels = 5,
		SetBrightnessLevels = 6,
		GetColorTemperatureInfo = 7,
		GetColorTemperature = 8,
		SetColorTemperature = 9,
		GetColorTemperatureLevels = 10,
		SetColorTemperatureLevels = 11,
		GetBrightnessEffectiveMax = 12,
	};

	enum Event {
		IlluminationChangeEvent = 0,
		BrightnessChangeEvent = 1,
		ColorTemperatureChangeEvent = 2,
		BrightnessEffectiveMaxChangeEvent = 3,
		BrightnessClampedEvent = 4,
	};

	enum Flags {
		hasEvents = 1 << 0,
		hasLinearLevels = 1 << 1,
		hasNonLinearLevels = 1 << 2,
		hasDynamicMaximum = 1 << 3,
	};

	struct Info {
		uint8_t flags;
		uint16_t min;
		uint16_t max;
		uint16_t res;
		unsigned int maxLevels : 4;
	};

	IIllumination (Device *dev);

	/**
	 * Get the current Illumination state.
	 */
	bool getIllumination(void);

	/**
	 * Set the current Illumination state.
	 */
	void setIllumination(bool state);

	/**
	 * Get information about brightness.
	 */
	Info getBrightnessInfo(void);

	/**
	 * Get the current Illumination brightness.
	 */
	uint16_t getBrightness(void);

	/**
	 * Get the maximum brightness based on hardware limits,
	 * 0 means the max value from getBrightnessInfo().
	 */
	uint16_t getBrightnessEffectiveMax(void);

	/**
	 * Set the Illumination brightness.
	 */
	void setBrightness(uint16_t value);

	/**
	 * Get information about color temperature.
	 */
	Info getColorTemperatureInfo(void);

	/**
	 * Get the current Illumination color temperature.
	 */
	uint16_t getColorTemperature(void);

	/**
	 * Set the Illumination color temperature.
	 */
	void setColorTemperature(uint16_t value);

	/**
	 * Parse an illumination change event.
	 */
	static bool illuminationChangeEvent (const HIDPP::Report &event);

	/**
	 * Parse a brightness change event.
	 */
	static uint16_t brightnessChangeEvent (const HIDPP::Report &event);

	/**
	 * Parse a color temperature change event.
	 */
	static uint16_t colorTemperatureChangeEvent (const HIDPP::Report &event);

	/**
	 * Parse a change in the effective maximum brightness.
	 */
	static uint16_t brightnessEffectiveMaxChangeEvent (const HIDPP::Report &event);

	/**
	 * Parse a notification of a recent request to set the
	 * brightness to a value larger than the current effective
	 * maximum brightness.
	 */
	static uint16_t brightnessClampedEvent (const HIDPP::Report &event);
};

}

#endif

