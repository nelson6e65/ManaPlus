/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011  The ManaPlus Developers
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

#include "gui/equipmentwindow.h"

#include "gui/widgets/button.h"

#include "being.h"
#include "equipment.h"
#include "graphics.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"

#include "gui/itempopup.h"
#include "gui/theme.h"
#include "gui/setup.h"
#include "gui/viewport.h"

#include "gui/widgets/playerbox.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>

#include "debug.h"

static const int BOX_WIDTH = 36;
static const int BOX_HEIGHT = 36;

EquipmentWindow::EquipmentWindow(Equipment *equipment, Being *being,
                                 bool foring):
    Window(_("Equipment")),
    mEquipment(equipment),
    mSelected(-1),
    mForing(foring)
{
    fillBoxes();
    mBeing = being;
    mItemPopup = new ItemPopup;
    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    // Control that shows the Player
    mPlayerBox = new PlayerBox;
    mPlayerBox->setDimension(gcn::Rectangle(50, 80, 74, 168));
    mPlayerBox->setPlayer(being);

    if (foring)
        setWindowName("Being equipment");
    else
        setWindowName("Equipment");

    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(180, 345, ImageRect::CENTER);
    loadWindowState();

    mUnequip = new Button(_("Unequip"), "unequip", this);
    const gcn::Rectangle &area = getChildrenArea();
    mUnequip->setPosition(area.width  - mUnequip->getWidth() - 5,
                          area.height - mUnequip->getHeight() - 5);
    mUnequip->setEnabled(false);

    add(mPlayerBox);
    add(mUnequip);

}

EquipmentWindow::~EquipmentWindow()
{
    delete mItemPopup;
    mItemPopup = 0;
    if (this == beingEquipmentWindow)
    {
        if (mEquipment)
            delete mEquipment->getBackend();
        delete mEquipment;
        mEquipment = 0;
    }
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);

    Graphics *g = static_cast<Graphics*>(graphics);

    Window::drawChildren(graphics);

    int i = 0;
    const int fontHeight = getFont()->getHeight();

    std::vector<std::pair<int, int>*>::const_iterator it;
    std::vector<std::pair<int, int>*>::const_iterator it_end = mBoxes.end();

    for (it = mBoxes.begin(); it != it_end; ++ it, ++ i)
    {
        std::pair<int, int> *box = *it;
        if (i == mSelected)
        {
            const gcn::Color color = Theme::getThemeColor(Theme::HIGHLIGHT);

            // Set color to the highlight color
            g->setColor(gcn::Color(color.r, color.g, color.b, getGuiAlpha()));
            g->fillRectangle(gcn::Rectangle(box->first,
                box->second, BOX_WIDTH, BOX_HEIGHT));
        }

        // Set color black
        g->setColor(gcn::Color(0, 0, 0));
        // Draw box border
        g->drawRectangle(gcn::Rectangle(box->first, box->second,
            BOX_WIDTH, BOX_HEIGHT));

        if (!mEquipment)
            continue;

        Item *item = mEquipment->getEquipment(i);
        if (item)
        {
            // Draw Item.
            Image *image = item->getImage();
            if (image)
            {
                image->setAlpha(1.0f); // Ensure the image is drawn
                                       // with maximum opacity
                g->drawImage(image, box->first + 2, box->second + 2);
                if (i == EQUIP_PROJECTILE_SLOT)
                {
                    g->setColor(Theme::getThemeColor(Theme::TEXT));
                    graphics->drawText(toString(item->getQuantity()),
                        box->first + (BOX_WIDTH / 2),
                        box->second - fontHeight,
                        gcn::Graphics::CENTER);
                }
            }
        }
    }
}

void EquipmentWindow::action(const gcn::ActionEvent &event)
{
    if (!mEquipment)
        return;

    if (event.getId() == "unequip" && mSelected > -1)
    {
        Item *item = mEquipment->getEquipment(mSelected);
        Net::getInventoryHandler()->unequipItem(item);
        setSelected(-1);
    }
}

Item *EquipmentWindow::getItem(int x, int y) const
{
    if (!mEquipment)
        return 0;

    std::vector<std::pair<int, int>*>::const_iterator it;
    std::vector<std::pair<int, int>*>::const_iterator it_end = mBoxes.end();
    int i = 0;

    for (it = mBoxes.begin(); it != it_end; ++ it, ++ i)
    {
        std::pair<int, int> *box = *it;
        const gcn::Rectangle tRect(box->first, box->second,
            BOX_WIDTH, BOX_HEIGHT);

        if (tRect.isPointInRect(x, y))
            return mEquipment->getEquipment(i);
    }
    return NULL;
}

void EquipmentWindow::mousePressed(gcn::MouseEvent& mouseEvent)
{
    Window::mousePressed(mouseEvent);

    if (!mEquipment)
        return;

    const int x = mouseEvent.getX();
    const int y = mouseEvent.getY();

    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        if (mForing)
            return;
        // Checks if any of the presses were in the equip boxes.
        std::vector<std::pair<int, int>*>::const_iterator it;
        std::vector<std::pair<int, int>*>::const_iterator it_end = mBoxes.end();
        int i = 0;

        for (it = mBoxes.begin(); it != it_end; ++ it, ++ i)
        {
            std::pair<int, int> *box = *it;
            Item *item = mEquipment->getEquipment(i);
            const gcn::Rectangle tRect(box->first, box->second,
                BOX_WIDTH, BOX_HEIGHT);

            if (tRect.isPointInRect(x, y) && item)
                setSelected(i);
        }
    }
    else if (mouseEvent.getButton() == gcn::MouseEvent::RIGHT)
    {
        if (Item *item = getItem(x, y))
        {
            if (mItemPopup)
                mItemPopup->setVisible(false);

            /* Convert relative to the window coordinates to absolute screen
             * coordinates.
             */
            const int mx = x + getX();
            const int my = y + getY();
            if (viewport)
            {
                if (mForing)
                    viewport->showUndressPopup(mx, my, mBeing, item);
                else
                    viewport->showPopup(this, mx, my, item, true);
            }
        }
    }
}

// Show ItemTooltip
void EquipmentWindow::mouseMoved(gcn::MouseEvent &event)
{
    if (!mItemPopup)
        return;

    const int x = event.getX();
    const int y = event.getY();

    Item *item = getItem(x, y);

    if (item)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        mItemPopup->setItem(item);
        mItemPopup->position(x + getX(), y + getY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

// Hide ItemTooltip
void EquipmentWindow::mouseExited(gcn::MouseEvent &event A_UNUSED)
{
    if (mItemPopup)
        mItemPopup->setVisible(false);
}

void EquipmentWindow::setSelected(int index)
{
    mSelected = index;
    if (mUnequip)
        mUnequip->setEnabled(mSelected != -1);
}

void EquipmentWindow::setBeing(Being *being)
{
    mPlayerBox->setPlayer(being);
    mBeing = being;
    if (mEquipment)
        delete mEquipment->getBackend();
    delete mEquipment;
    if (!being)
    {
        mEquipment = 0;
        return;
    }
    mEquipment = being->getEquipment();
}

void EquipmentWindow::updateBeing(Being *being)
{
    if (being == mBeing)
        setBeing(being);
}

void EquipmentWindow::resetBeing(Being *being)
{
    if (being == mBeing)
        setBeing(0);
}

void EquipmentWindow::fillBoxes()
{
    addBox(90, 40);     // torso
    addBox(8, 78);      // gloves
    addBox(70, 0);      // hat
    addBox(50, 253);    // pants
    addBox(90, 253);    // boots
    addBox(8, 213);     // FREE
    addBox(129, 213);   // wings
    addBox(50, 40);     // scarf
    addBox(8, 168);     // weapon
    addBox(129, 168);   // shield
    addBox(129, 78);    // ammo
    addBox(8, 123);     // amulet
    addBox(129, 123);   // ring
}

void EquipmentWindow::addBox(int x, int y)
{
    mBoxes.push_back(new std::pair<int, int>(
        x + getPadding(), y + getTitleBarHeight()));
}
