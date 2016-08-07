/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2016  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENUMS_RESOURCES_MAP_BLOCKTYPE_H
#define ENUMS_RESOURCES_MAP_BLOCKTYPE_H

#include "enums/simpletypes/enumdefines.h"

enumStart(BlockType)
{
    NONE       = -1,
    GROUND     = 0,
    WALL       = 1,
    AIR        = 2,
    WATER      = 3,
    GROUNDTOP  = 4,
    PLAYERWALL = 5,
    NB_BLOCKTYPES
}
enumEnd(BlockType);

#endif  // ENUMS_RESOURCES_MAP_BLOCKTYPE_H
