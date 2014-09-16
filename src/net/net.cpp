/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#include "net/net.h"

#include "main.h"

#include "net/loginhandler.h"

#ifdef TMWA_SUPPORT
#include "net/tmwa/generalhandler.h"
#endif

#ifdef EATHENA_SUPPORT
#include "net/eathena/generalhandler.h"
#endif

#include "debug.h"

namespace Net
{
    class AdminHandler;
    class AuctionHandler;
    class BankHandler;
    class BeingHandler;
    class BuySellHandler;
    class CashShopHandler;
}

Net::AdminHandler *adminHandler = nullptr;
Net::CharServerHandler *charServerHandler = nullptr;
Net::ChatHandler *chatHandler = nullptr;
Net::GeneralHandler *generalHandler = nullptr;
Net::InventoryHandler *inventoryHandler = nullptr;
Net::LoginHandler *loginHandler = nullptr;
Net::GameHandler *gameHandler = nullptr;
Net::GuildHandler *guildHandler = nullptr;
Net::NpcHandler *npcHandler = nullptr;
Net::PartyHandler *partyHandler = nullptr;
Net::PetHandler *petHandler = nullptr;
Net::PlayerHandler *playerHandler = nullptr;
Net::SkillHandler *skillHandler = nullptr;
Net::TradeHandler *tradeHandler = nullptr;
Net::BeingHandler *beingHandler = nullptr;
Net::BuySellHandler *buySellHandler = nullptr;
Net::ServerFeatures *serverFeatures = nullptr;
Net::MailHandler *mailHandler = nullptr;
Net::AuctionHandler *auctionHandler = nullptr;
Net::CashShopHandler *cashShopHandler = nullptr;
Net::FamilyHandler *familyHandler = nullptr;
Net::BankHandler *bankHandler = nullptr;

Net::CharServerHandler *Net::getCharServerHandler()
{
    return charServerHandler;
}

Net::ChatHandler *Net::getChatHandler()
{
    return chatHandler;
}

Net::GameHandler *Net::getGameHandler()
{
    return gameHandler;
}

Net::GeneralHandler *Net::getGeneralHandler()
{
    return generalHandler;
}

Net::GuildHandler *Net::getGuildHandler()
{
    return guildHandler;
}

Net::InventoryHandler *Net::getInventoryHandler()
{
    return inventoryHandler;
}

Net::LoginHandler *Net::getLoginHandler()
{
    return loginHandler;
}

Net::NpcHandler *Net::getNpcHandler()
{
    return npcHandler;
}

Net::PartyHandler *Net::getPartyHandler()
{
    return partyHandler;
}

Net::PetHandler *Net::getPetHandler()
{
    return petHandler;
}

Net::PlayerHandler *Net::getPlayerHandler()
{
    return playerHandler;
}

Net::SkillHandler *Net::getSkillHandler()
{
    return skillHandler;
}

Net::TradeHandler *Net::getTradeHandler()
{
    return tradeHandler;
}

Net::ServerFeatures *Net::getServerFeatures()
{
    return serverFeatures;
}

Net::MailHandler *Net::getMailHandler()
{
    return mailHandler;
}

Net::FamilyHandler *Net::getFamilyHandler()
{
    return familyHandler;
}


namespace Net
{
ServerInfo::Type networkType = ServerInfo::UNKNOWN;

void connectToServer(const ServerInfo &server)
{
    BLOCK_START("Net::connectToServer")
    if (networkType == server.type && getGeneralHandler())
    {
        getGeneralHandler()->reload();
    }
    else
    {
        if (networkType != ServerInfo::UNKNOWN && getGeneralHandler())
            getGeneralHandler()->unload();

        switch (server.type)
        {
            case ServerInfo::EATHENA:
#ifdef EATHENA_SUPPORT
                new EAthena::GeneralHandler;
#else
                new TmwAthena::GeneralHandler;
#endif
                break;
            case ServerInfo::TMWATHENA:
            case ServerInfo::EVOL:
            case ServerInfo::UNKNOWN:
            default:
#ifdef TMWA_SUPPORT
                new TmwAthena::GeneralHandler;
#else
                new EAthena::GeneralHandler;
#endif
                break;
        }

        getGeneralHandler()->load();

        networkType = server.type;
    }

    if (getLoginHandler())
    {
        getLoginHandler()->setServer(server);
        getLoginHandler()->connect();
    }
    BLOCK_END("Net::connectToServer")
}

void unload()
{
    GeneralHandler *const handler = getGeneralHandler();
    if (handler)
        handler->unload();
}

ServerInfo::Type getNetworkType()
{
    return networkType;
}

}  // namespace Net
