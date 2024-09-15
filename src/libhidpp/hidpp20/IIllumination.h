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

	IIllumination (Device *dev);

	/**
	 * Get the current Illumination state.
	 */
	bool getIllumination(void);

	/**
	 * Set the current Illumination state.
	 */
	void setIllumination(bool state);
};

}

#endif

