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

#include <hidpp20/IIllumination.h>

#include <misc/Endian.h>

#include <cassert>

using namespace HIDPP20;

constexpr uint16_t IIllumination::ID;

IIllumination::IIllumination (Device *dev):
	FeatureInterface (dev, ID, "Illumination")
{
}

bool IIllumination::getIllumination(void)
{
	std::vector<uint8_t> params (16), results;
	results = call (GetIllumination, params);
	return readLE<uint8_t> (results, 0) != 0;
}

void IIllumination::setIllumination(bool state)
{
	std::vector<uint8_t> params (16);
	writeLE<uint8_t> (params, 0, state);
	call (SetIllumination, params);
}
