/*
 *  The ManaPlus Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2013  The ManaPlus Developers
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

#include "gui/widgets/dropdown.h"

#include "client.h"
#include "configuration.h"
#include "keydata.h"
#include "keyevent.h"

#include "gui/sdlinput.h"
#include "gui/viewport.h"

#include "gui/widgets/extendedlistmodel.h"
#include "gui/widgets/listbox.h"
#include "gui/widgets/popuplist.h"
#include "gui/widgets/scrollarea.h"

#include "resources/image.h"

#include "utils/dtor.h"

#include <guichan/font.hpp>

#include <algorithm>

#include "debug.h"

int DropDown::instances = 0;
Image *DropDown::buttons[2][2];
ImageRect DropDown::skinRect;
float DropDown::mAlpha = 1.0;
Skin *DropDown::mSkin = nullptr;

static std::string const dropdownFiles[2] =
{
    "dropdown.xml",
    "dropdown_pressed.xml"
};

DropDown::DropDown(const Widget2 *const widget,
                   gcn::ListModel *const listModel,
                   bool extended,
                   bool modal,
                   gcn::ActionListener *const listener,
                   const std::string &eventId):
    gcn::ActionListener(),
    gcn::BasicContainer(),
    gcn::KeyListener(),
    gcn::MouseListener(),
    gcn::FocusListener(),
    gcn::SelectionListener(),
    Widget2(widget),
    mExtended(extended),
    mPopup(new PopupList(this, listModel, extended, modal)),
    mShadowColor(getThemeColor(Theme::DROPDOWN_SHADOW)),
    mHighlightColor(getThemeColor(Theme::HIGHLIGHT)),
    mPadding(1),
    mImagePadding(2),
    mDroppedDown(false),
    mPushed(false),
    mFoldedUpHeight(0),
    mIsDragged(false)
{
    mFrameSize = 2;

    mPopup->setHeight(100);

    // Initialize graphics
    if (instances == 0)
    {
        // Load the background skin
        for (int i = 0; i < 2; i ++)
        {
            Skin *skin = Theme::instance()->load(
                dropdownFiles[i], "dropdown.xml");
            if (skin)
            {
                if (!i)
                    mSkin = skin;
                const ImageRect &rect = skin->getBorder();
                for (int f = 0; f < 2; f ++)
                {
                    if (rect.grid[f])
                    {
                        rect.grid[f]->incRef();
                        buttons[f][i] = rect.grid[f];
                        buttons[f][i]->setAlpha(mAlpha);
                    }
                    else
                    {
                        buttons[f][i] = nullptr;
                    }
                }
                if (i)
                    Theme::instance()->unload(skin);
            }
            else
            {
                for (int f = 0; f < 2; f ++)
                    buttons[f][i] = nullptr;
            }
        }

        // get the border skin
        if (Theme::instance())
        {
            Theme::instance()->loadRect(skinRect,
                "dropdown_background.xml", "");
        }
    }

    instances++;

    setWidth(100);
    setFocusable(true);
    setListModel(listModel);

    if (mPopup->getSelected() < 0)
        mPopup->setSelected(0);

    addMouseListener(this);
    addKeyListener(this);
    addFocusListener(this);

    adjustHeight();
    mPopup->setForegroundColor(getThemeColor(Theme::DROPDOWN));
    setForegroundColor(getThemeColor(Theme::DROPDOWN));

    if (!eventId.empty())
        setActionEventId(eventId);

    if (listener)
        addActionListener(listener);

    mPopup->adjustSize();

    if (mSkin)
    {
        mFrameSize = mSkin->getOption("frameSize");
        mPadding = mSkin->getPadding();
        mImagePadding = mSkin->getOption("imagePadding");
        mSpacing = mSkin->getOption("spacing");
    }
    adjustHeight();
}

DropDown::~DropDown()
{
    if (gui)
        gui->removeDragged(this);

    instances--;
    if (instances == 0)
    {
        for (int f = 0; f < 2; f ++)
        {
            for (int i = 0; i < 2; i ++)
            {
                if (buttons[f][i])
                    buttons[f][i]->decRef();
            }
        }
        Theme *const theme = Theme::instance();
        if (theme)
        {
            theme->unload(mSkin);
            theme->unloadRect(skinRect);
        }
    }
}

void DropDown::updateAlpha()
{
    const float alpha = std::max(Client::getGuiAlpha(),
        Theme::instance()->getMinimumOpacity());

    if (mAlpha != alpha)
    {
        mAlpha = alpha;

        if (buttons[0][0])
            buttons[0][0]->setAlpha(mAlpha);
        if (buttons[0][1])
            buttons[0][1]->setAlpha(mAlpha);
        if (buttons[1][0])
            buttons[1][0]->setAlpha(mAlpha);
        if (buttons[1][1])
            buttons[1][1]->setAlpha(mAlpha);

        for (int a = 0; a < 9; a++)
        {
            if (skinRect.grid[a])
                skinRect.grid[a]->setAlpha(mAlpha);
        }
    }
}

void DropDown::draw(gcn::Graphics* graphics)
{
    BLOCK_START("DropDown::draw")
    int h;

    if (mDroppedDown)
        h = mFoldedUpHeight;
    else
        h = getHeight();

    updateAlpha();

    const int alpha = static_cast<int>(mAlpha * 255.0f);
    const int pad = 2 * mPadding;
    mHighlightColor.a = alpha;
    mShadowColor.a = alpha;

    gcn::ListModel *const model = mPopup->getListModel();
    if (model && mPopup->getSelected() >= 0)
    {
        gcn::Font *const font = getFont();
        graphics->setColor(mForegroundColor);
        if (mExtended)
        {
            const int sel = mPopup->getSelected();
            ExtendedListModel *const model2
                = static_cast<ExtendedListModel *const>(model);
            const Image *const image = model2->getImageAt(sel);
            if (!image)
            {
                font->drawString(graphics, model->getElementAt(sel),
                    mPadding, mPadding);
            }
            else
            {
                static_cast<Graphics*>(graphics)->drawImage(
                    image, mImagePadding, (getHeight()
                    - image->getHeight()) / 2 + mPadding);
                font->drawString(graphics, model->getElementAt(sel),
                    image->getWidth() + mImagePadding + mSpacing, mPadding);
            }
        }
        else
        {
            font->drawString(graphics, model->getElementAt(
                mPopup->getSelected()), mPadding, mPadding);
        }
    }

    if (isFocused())
    {
        graphics->setColor(mHighlightColor);
        graphics->drawRectangle(gcn::Rectangle(mPadding, mPadding,
            getWidth() - h - pad, h - pad));
    }

    drawButton(graphics);

    if (mDroppedDown)
    {
//        drawChildren(graphics);
        // Draw two lines separating the ListBox with selected
        // element view.
        graphics->setColor(mHighlightColor);
        graphics->drawLine(0, h, getWidth(), h);
        graphics->setColor(mShadowColor);
        graphics->drawLine(0, h + 1, getWidth(), h + 1);
    }
    BLOCK_END("DropDown::draw")
}

void DropDown::drawFrame(gcn::Graphics *graphics)
{
    BLOCK_START("DropDown::drawFrame")
    const int bs = getFrameSize();
    const int w = getWidth() + bs * 2;
    const int h = getHeight() + bs * 2;

    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, skinRect);
    BLOCK_END("DropDown::drawFrame")
}

void DropDown::drawButton(gcn::Graphics *graphics)
{
    const int height = mDroppedDown ? mFoldedUpHeight : getHeight();

    Image *image = buttons[mDroppedDown][mPushed];
    if (image)
    {
        static_cast<Graphics*>(graphics)->drawImage(image,
            getWidth() - image->getWidth() - mImagePadding,
            (height - image->getHeight()) / 2);
    }
}

void DropDown::keyPressed(gcn::KeyEvent& keyEvent)
{
    if (keyEvent.isConsumed())
        return;

    const int actionId = static_cast<KeyEvent*>(&keyEvent)->getActionId();

    switch (actionId)
    {
        case Input::KEY_GUI_SELECT:
        case Input::KEY_GUI_SELECT2:
            dropDown();
            break;

        case Input::KEY_GUI_UP:
            setSelected(getSelected() - 1);
            break;

        case Input::KEY_GUI_DOWN:
            setSelected(getSelected() + 1);
            break;

        case Input::KEY_GUI_HOME:
            setSelected(0);
            break;

        case Input::KEY_GUI_END:
            if (mPopup->getListModel())
            {
                setSelected(mPopup->getListModel()->
                    getNumberOfElements() - 1);
            }
            break;

        default:
            return;
    }

    keyEvent.consume();
}

void DropDown::hideDrop()
{
    distributeActionEvent();
    mPopup->setVisible(false);
}

void DropDown::mousePressed(gcn::MouseEvent& mouseEvent)
{
    // If we have a mouse press on the widget.
    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT
        && !mDroppedDown && mouseEvent.getSource() == this)
    {
        mPushed = true;
        dropDown();
    }
    else
    {
        mPushed = false;
        foldUp();
        hideDrop();
    }
}

void DropDown::mouseReleased(gcn::MouseEvent &mouseEvent)
{
    if (mIsDragged)
        mPushed = false;

    // Released outside of widget. Can happen when we have modal
    // input focus.
    if ((0 > mouseEvent.getY()
        || mouseEvent.getY() >= getHeight()
        || mouseEvent.getX() < 0
        || mouseEvent.getX() >= getWidth())
        && mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        if (mIsDragged)
            foldUp();
    }
    else if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        mPushed = false;
    }

    mIsDragged = false;
}

void DropDown::mouseDragged(gcn::MouseEvent &mouseEvent)
{
    mIsDragged = true;
    mouseEvent.consume();
}

void DropDown::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    setSelected(getSelected() - 1);
    mouseEvent.consume();
}

void DropDown::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    setSelected(getSelected() + 1);
    mouseEvent.consume();
}

void DropDown::setSelectedString(std::string str)
{
    gcn::ListModel *const listModel = mPopup->getListModel();
    if (!listModel)
        return;

    for (int f = 0; f < listModel->getNumberOfElements(); f ++)
    {
        if (listModel->getElementAt(f) == str)
        {
            setSelected(f);
            break;
        }
    }
}

std::string DropDown::getSelectedString() const
{
    gcn::ListModel *const listModel = mPopup->getListModel();
    if (!listModel)
        return "";

    return listModel->getElementAt(getSelected());
}

void DropDown::adjustHeight()
{
    setHeight(getFont()->getHeight() + 2 * mPadding);
}

void DropDown::dropDown()
{
    if (!mDroppedDown)
    {
        mDroppedDown = true;
        mFoldedUpHeight = getHeight();
        adjustHeight();

//        if (getParent())
//            getParent()->moveToTop(this);
        int x = 0;
        int y = 0;
        getAbsolutePosition(x, y);
        const int frame = getParent()->getFrameSize();
        const int pad = mPopup->getPadding();
        const int pad2 = pad * 2;

        // here width should be adjusted on some other parameters
        mPopup->setWidth(getWidth() - pad2 + 8);
        mPopup->show(x - mPadding - frame - 1, y + getHeight());
        mPopup->requestMoveToTop();
        mPopup->requestFocus();
    }
}

void DropDown::foldUp()
{
    if (mDroppedDown)
    {
        mDroppedDown = false;
        adjustHeight();
    }
}

int DropDown::getSelected() const
{
    return mPopup->getSelected();
}

void DropDown::setSelected(int selected)
{
    if (selected >= 0)
        mPopup->setSelected(selected);
}

void DropDown::setListModel(gcn::ListModel *listModel)
{
    mPopup->setListModel(listModel);

    if (mPopup->getSelected() < 0)
        mPopup->setSelected(0);

    adjustHeight();
}

gcn::ListModel *DropDown::getListModel()
{
    return mPopup->getListModel();
}

void DropDown::action(const gcn::ActionEvent &actionEvent A_UNUSED)
{
    foldUp();
    distributeActionEvent();
}

gcn::Rectangle DropDown::getChildrenArea()
{
    if (mDroppedDown)
    {
        // Calculate the children area (with the one pixel border in mind)
        return gcn::Rectangle(1, mFoldedUpHeight + 1,
            getWidth() - 2, getHeight() - mFoldedUpHeight - 2);
    }

    return gcn::Rectangle();
}

void DropDown::valueChanged(const gcn::SelectionEvent& event A_UNUSED)
{
}

void DropDown::updateSelection()
{
    mDroppedDown = false;
    mPushed = false;
    distributeActionEvent();
    distributeValueChangedEvent();
}

void DropDown::addSelectionListener(SelectionListener* selectionListener)
{
    mSelectionListeners.push_back(selectionListener);
}

void DropDown::removeSelectionListener(SelectionListener* listener)
{
    mSelectionListeners.remove(listener);
}

void DropDown::distributeValueChangedEvent()
{
    for (SelectionListenerIterator iter = mSelectionListeners.begin();
          iter != mSelectionListeners.end();
          ++iter)
    {
        gcn::SelectionEvent event(this);
        (*iter)->valueChanged(event);
    }
}
