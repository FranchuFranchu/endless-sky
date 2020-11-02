/* CustomLink.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "CustomLink.h"

#include "DataNode.h"
#include "GameData.h"
#include "Outfit.h"
#include "System.h"


using namespace std;

class DataNode;

// For sorting in a set<>
bool CustomLink::operator<(const CustomLink& link) const {

    return (link.system->Name() + link.linkType)
    < (this->system->Name() + this->linkType);
}

// Shorthand for CustomLinkType::CanTravel
bool CustomLink::CanTravel(const Ship &ship) const{
    const CustomLinkType * linkTypeData = GameData::CustomLinkTypes().Get(linkType);
    return linkTypeData->CanTravel(ship);
}

bool CustomLink::CanTravel(const Outfit &outfit) const{
    const CustomLinkType * linkTypeData = GameData::CustomLinkTypes().Get(linkType);
    return linkTypeData->CanTravel(outfit);
}



// Load this link
void CustomLinkType::Load(const DataNode &node) {
    for(const DataNode &child : node)
    {
        if (child.Token(0) == "requires")
            requirement = child.Token(1);
        else if(child.Token(0) == "color" && child.Size() >= 4)
            closeColor = Color(child.Value(1), child.Value(2), child.Value(3));
        else if(child.Token(0) == "farcolor" && child.Size() >= 4)
            farColor = Color(child.Value(1), child.Value(2), child.Value(3), 0.5f);

    }
}

// Checks if a certain ship can travel through this link type
bool CustomLinkType::CanTravel(const Ship &ship) const{
    return ship.Attributes().Get(requirement);
}



// Same as above, byt overloaded with outfits
bool CustomLinkType::CanTravel(const Outfit &outfit) const{
    return outfit.Get(requirement);
}