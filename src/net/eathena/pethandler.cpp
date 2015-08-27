/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2015  The ManaPlus Developers
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

#include "net/eathena/pethandler.h"

#include "actormanager.h"
#include "notifymanager.h"

#include "being/localplayer.h"
#include "being/petinfo.h"
#include "being/playerinfo.h"

#include "enums/resources/notifytypes.h"

#include "gui/windows/eggselectiondialog.h"

#include "gui/widgets/createwidget.h"

#include "gui/widgets/tabs/chat/chattab.h"

#include "net/inventoryhandler.h"
#include "net/serverfeatures.h"

#include "net/ea/eaprotocol.h"

#include "net/eathena/menu.h"
#include "net/eathena/messageout.h"
#include "net/eathena/petrecv.h"
#include "net/eathena/protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "debug.h"

extern Net::PetHandler *petHandler;

namespace EAthena
{

PetHandler::PetHandler() :
    MessageHandler()
{
    static const uint16_t _messages[] =
    {
        SMSG_PET_MESSAGE,
        SMSG_PET_ROULETTE,
        SMSG_PET_EGGS_LIST,
        SMSG_PET_DATA,
        SMSG_PET_STATUS,
        SMSG_PET_FOOD,
        SMSG_PET_CATCH_PROCESS,
        0
    };
    handledMessages = _messages;
    petHandler = this;
}

void PetHandler::handleMessage(Net::MessageIn &msg)
{
    BLOCK_START("PetHandler::handleMessage")
    switch (msg.getId())
    {
        case SMSG_PET_MESSAGE:
            PetRecv::processPetMessage(msg);
            break;

        case SMSG_PET_ROULETTE:
            PetRecv::processPetRoulette(msg);
            break;

        case SMSG_PET_EGGS_LIST:
            PetRecv::processEggsList(msg);
            break;

        case SMSG_PET_DATA:
            PetRecv::processPetData(msg);
            break;

        case SMSG_PET_STATUS:
            PetRecv::processPetStatus(msg);
            break;

        case SMSG_PET_FOOD:
            PetRecv::processPetFood(msg);
            break;

        case SMSG_PET_CATCH_PROCESS:
            PetRecv::processPetCatchProcess(msg);
            break;

        default:
            break;
    }
    BLOCK_END("PetHandler::handleMessage")
}

void PetHandler::move(const int petId A_UNUSED,
                      const int x, const int y) const
{
    if (!serverFeatures->haveMovePet())
        return;
    createOutPacket(CMSG_PET_MOVE_TO);
    outMsg.writeInt32(0, "pet id");
    outMsg.writeInt16(static_cast<int16_t>(x), "x");
    outMsg.writeInt16(static_cast<int16_t>(y), "y");
}

void PetHandler::spawn(const Being *const being A_UNUSED,
                       const int petId A_UNUSED,
                       const int x A_UNUSED, const int y A_UNUSED) const
{
}

void PetHandler::emote(const uint8_t emoteId, const int petId A_UNUSED)
{
    createOutPacket(CMSG_PET_EMOTE);
    outMsg.writeInt8(emoteId, "emote id");
}

void PetHandler::catchPet(const Being *const being) const
{
    if (!being)
        return;

    createOutPacket(CMSG_PET_CATCH);
    outMsg.writeBeingId(being->getId(), "monster id");
}

void PetHandler::sendPetMessage(const int data) const
{
    createOutPacket(CMSG_PET_SEND_MESSAGE);
    outMsg.writeInt32(data, "param");
}

void PetHandler::setName(const std::string &name) const
{
    createOutPacket(CMSG_PET_SET_NAME);
    outMsg.writeString(name, 24, "name");
}

void PetHandler::requestStatus() const
{
    createOutPacket(CMSG_PET_MENU_ACTION);
    outMsg.writeInt8(0, "action");
}

void PetHandler::feed() const
{
    createOutPacket(CMSG_PET_MENU_ACTION);
    outMsg.writeInt8(1, "action");
}

void PetHandler::dropLoot() const
{
    createOutPacket(CMSG_PET_MENU_ACTION);
    outMsg.writeInt8(2, "action");  // performance
}

void PetHandler::returnToEgg() const
{
    createOutPacket(CMSG_PET_MENU_ACTION);
    outMsg.writeInt8(3, "action");
    PlayerInfo::setPet(nullptr);
}

void PetHandler::unequip() const
{
    createOutPacket(CMSG_PET_MENU_ACTION);
    outMsg.writeInt8(4, "action");
}

void PetHandler::setDirection(const unsigned char type) const
{
    if (!serverFeatures->haveMovePet())
        return;
    createOutPacket(CMSG_PET_DIRECTION);
    outMsg.writeInt32(0, "pet id");
    outMsg.writeInt8(0, "head direction");
    outMsg.writeInt8(0, "unused");
    outMsg.writeInt8(MessageOut::toServerDirection(type),
        "pet direction");
}

void PetHandler::startAi(const bool start A_UNUSED) const
{
}

}  // namespace EAthena
