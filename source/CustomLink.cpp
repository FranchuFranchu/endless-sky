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

#include <string>

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
    string color_names[] = {"color", "far color", "unusable color", "unusable far color"};
    Color* color_pointers[] = {&closeColor, &farColor, &unusableCloseColor, &unusableFarColor};
    bool was_defined[] = {false, false, false, false};
    *(color_pointers[0]) = Color(0.0,0.0);
    for(const DataNode &child : node)
    {
        if(child.Size() < 2)
        {
            child.PrintTrace("Skipping " + child.Token(0) + " with no key given:");
            continue;
        }
        else if (child.Token(0) == "requires") 
        {
            requirement = child.Token(1);
            continue;
        }

        // I have to do this to avoid using a goto
        bool found_color = false;
        for (uint i = 0; i < (sizeof(color_names) / sizeof(string)); ++i) 
        {
            bool is_far = i % 2;
            bool is_unusable = i > 1;
            if (child.Token(0) == color_names[i]) 
            {
                if (child.Size() == 4) 
                {
                    if (is_unusable) {
                        child.PrintTrace(string("Warning: Custom link color when unusable \"") + color_names[i] + "\"did not specify an alpha value, so 0.0 (transparent) was assumed.");
                        *color_pointers[i] = Color(child.Value(1), child.Value(2), child.Value(3), 0);
                    } else {
                        *color_pointers[i] = Color(child.Value(1), child.Value(2), child.Value(3), 0.5 ? is_far : 1);
                    }
                }
                if (child.Size() == 5) 
                {
                    *color_pointers[i] = Color(child.Value(1), child.Value(2), child.Value(3), child.Value(4));
                }
                was_defined[i] = true;
                found_color = true;
                break;
            }
        }
        if (!found_color) {
            child.PrintTrace("Unrecognized key: " + child.Token(0) + ". Skipping it.");    
        }
        
    }

    // Adjust everything's colors with the alpha value
    for (uint i = 0; i < (sizeof(color_pointers) / sizeof(Color*)); ++i) 
    {
        *color_pointers[i] = color_pointers[i]->Transparent(color_pointers[i]->Get()[3]);
    }



    if (!was_defined[0]) // Close color 
    {
        node.PrintTrace("Warning: The attribute \"color\" was not specified for this custom link, gray was assumed.");
    }
    if (!was_defined[1]) // Far color
    {
        // Copy close color, but reduce alpha value
        farColor = closeColor.Transparent(0.5);
    }
    if (!was_defined[3]) // Unusable far color
    {
        // Copy close color, but reduce alpha value
        unusableFarColor = unusableCloseColor.Transparent(0.5);
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