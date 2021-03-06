/* CustomLink.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef CUSTOMLINK_H_
#define CUSTOMLINK_H_

#include "Color.h"
#include "Outfit.h"
#include "Ship.h"
#include "System.h"

class System;
class Color;
class Ship;
class Outfit;

class CustomLink
{
	public:

		// For sorting in set objects
		bool operator<(const CustomLink&) const;

		// Check if a ship/outfit can travel through this link. This function calls CustomLinkType::CanTravel
		bool CanTravel(const Ship &ship) const;
		bool CanTravel(const Outfit &outfit) const;

		// System this link links to
		const System *system = nullptr;
		// String representing link type
		std::string linkType = "some_link_type";
};

// Plugin-editable hyperlink type
class CustomLinkType
{
	public:
		// Load a linktype description.
		void Load(const DataNode &node);
		// Check if a ship/outfit can travel through this link type
		bool CanTravel(const Ship &ship) const;
		bool CanTravel(const Outfit &outfit) const;

		bool doHyperdriveMovement = false;
		Color farColor = Color(0.5f, 1.f); // Link color when it is far away from the player
		Color closeColor  = Color(1.f, 1.f); // Color when it is close to the player

		// Same as above, but used when the player can't travel through the links. Can be used to create "secret" links.
		Color unusableFarColor = Color(0.5f, 0.f);
		Color unusableCloseColor = Color(1.f, 0.f);

		std::string requirement; // Outfit needed to travel through this link
};
#endif // CUSTOMLINK_H
