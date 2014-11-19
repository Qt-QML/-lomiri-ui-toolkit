/*
 * Copyright 2014 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ucunits.h"
#include "uctheme.h"
#include "uclistitem.h"
#include "uclistitem_p.h"
#include <QtQml/QQmlInfo>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickflickable_p.h>
#include <QtQuick/private/qquickpositioners_p.h>
#include <QtQuick/private/qquickanimation_p.h>
#include "ucubuntuanimation.h"

QColor getPaletteColor(const char *profile, const char *color)
{
    QColor result;
    QObject *palette = UCTheme::instance().palette();
    if (palette) {
        QObject *paletteProfile = palette->property(profile).value<QObject*>();
        if (paletteProfile) {
            result = paletteProfile->property(color).value<QColor>();
        }
    }
    return result;
}
/******************************************************************************
 * SnapAnimator
 *
 * The class handles the animation executed when the ListItemAction panel is
 * swiped. The animation is executed from the swipe position the mouse/touch is
 * released to the desired position given in snap(). The action panel is assumed
 * to be anchored to the ListItem.contentItem left or right, depending on which
 * action list is swiped in. Therefore the animator only changes the ListItem.contentItem
 * x coordinate.
 * The animation is defined by the style.
 */
UCListItemSnapAnimator::UCListItemSnapAnimator(UCListItem *item)
    : QObject(item)
    , item(item)
{
}
UCListItemSnapAnimator::~UCListItemSnapAnimator()
{
    // make sure we cannot animate anymore, for safety
    item = 0;
}

/*
 * Snap the ListItem.contentItem in or out, depending on the position specified
 * in "to" parameter. If the position is 0, a snap out will be executed - see
 * snapOut(). In any other cases a snap in action will be performed - see snapIn().
 */
bool UCListItemSnapAnimator::snap(qreal to)
{
    if (!item) {
        return false;
    }
    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
    QQuickPropertyAnimation *snap = getDefaultAnimation();
    if (!snap) {
        // no animation, so we simply position the component
        listItem->contentItem->setX(to);
        return false;
    }
    snap->setTargetObject(listItem->contentItem);
    if (to == 0.0) {
        QObject::connect(snap, &QQuickAbstractAnimation::stopped,
                         this, &UCListItemSnapAnimator::snapOut);
    } else {
        QObject::connect(snap, &QQuickAbstractAnimation::stopped,
                         this, &UCListItemSnapAnimator::snapIn);
    }
    if (snap->properties().isEmpty() && snap->property().isEmpty()) {
        snap->setProperty("x");
    }
    snap->setFrom(listItem->contentItem->property(snap->property().toLocal8Bit().constData()));
    snap->setTo(to);
    snap->setAlwaysRunToEnd(true);
    // FIXME - this will be used later
//    listItem->setContentMoved(true);
    snap->start();
    return true;
}

/*
 * The function completes a running snap animation.
 */
void UCListItemSnapAnimator::complete()
{
    QQuickPropertyAnimation *snap = getDefaultAnimation();
    if (snap && snap->isRunning()) {
        snap->complete();
    }
}

/*
 * Snap out is performed when the ListItem.contentItem returns back to its original
 * X coordinates (0). At this point both leading and trailing action panels will
 * be disconnected, ascending Flickables will get unlocked (interactive value restored
 * to the state before they were locked) and ListItem.contentMoving will be reset.
 */
void UCListItemSnapAnimator::snapOut()
{
    if (senderSignalIndex() >= 0) {
        // disconnect animation, otherwise snapping will disconnect the panel
        QQuickAbstractAnimation *snap = getDefaultAnimation();
        QObject::disconnect(snap, 0, 0, 0);
    }
    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
    if (listItem->attachedProperties) {
        // restore flickable's interactive and cleanup
        listItem->attachedProperties->disableInteractive(item, false);
        // no need to listen flickables any longer
        listItem->attachedProperties->listenToRebind(item, false);
    }
    // disconnect actions - FIXME this will be used later
//    listItem->grabPanel(listItem->leadingActions, false);
//    listItem->grabPanel(listItem->trailingActions, false);
    // set contentMoving to false - FIXME used later
//    listItem->setContentMoving(false);
}

/*
 * Snap in only resets the ListItem.contentMoving property, but will keep leading/trailing
 * actions connected as well as all ascendant Flickables locked (interactive = false).
 */
void UCListItemSnapAnimator::snapIn()
{
    if (senderSignalIndex() >= 0) {
        // disconnect animation
        QQuickAbstractAnimation *snap = getDefaultAnimation();
        QObject::disconnect(snap, 0, 0, 0);
    }
    // turn content moving off - FIXME used later
//    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
//    listItem->setContentMoving(false);
}

/*
 * Returns the animation specified by the style.
 */
QQuickPropertyAnimation *UCListItemSnapAnimator::getDefaultAnimation()
{
    // FIXME - return the animation from the style
    return 0;
}

/******************************************************************************
 * Divider
 */
UCListItemDivider::UCListItemDivider(QObject *parent)
    : QObject(parent)
    , m_visible(true)
    , m_leftMarginChanged(false)
    , m_rightMarginChanged(false)
    , m_colorFromChanged(false)
    , m_colorToChanged(false)
    , m_thickness(0)
    , m_leftMargin(0)
    , m_rightMargin(0)
    , m_listItem(0)
{
    connect(&UCUnits::instance(), &UCUnits::gridUnitChanged, this, &UCListItemDivider::unitsChanged);
    connect(&UCTheme::instance(), &UCTheme::paletteChanged, this, &UCListItemDivider::paletteChanged);
    unitsChanged();
    paletteChanged();
}
UCListItemDivider::~UCListItemDivider()
{
}

void UCListItemDivider::init(UCListItem *listItem)
{
    QQml_setParent_noEvent(this, listItem);
    m_listItem = UCListItemPrivate::get(listItem);
}

void UCListItemDivider::unitsChanged()
{
    m_thickness = UCUnits::instance().dp(2);
    if (!m_leftMarginChanged) {
        m_leftMargin = UCUnits::instance().dp(2);
    }
    if (!m_rightMarginChanged) {
        m_rightMargin = UCUnits::instance().dp(2);
    }
    if (m_listItem) {
        m_listItem->update();
    }
}

void UCListItemDivider::paletteChanged()
{
    QColor background = getPaletteColor("normal", "background");
    if (!background.isValid()) {
        return;
    }
    // FIXME: we need a palette value for divider colors, till then base on the background
    // luminance
    if (!m_colorFromChanged || !m_colorToChanged) {
        qreal luminance = (background.red()*212 + background.green()*715 + background.blue()*73)/1000.0/255.0;
        bool lightBackground = (luminance > 0.85);
        if (!m_colorFromChanged) {
            m_colorFrom = lightBackground ? QColor("#26000000") : QColor("#26FFFFFF");
        }
        if (!m_colorToChanged) {
            m_colorTo = lightBackground ? QColor("#14FFFFFF") : QColor("#14000000");
        }
        updateGradient();
    }
}

void UCListItemDivider::updateGradient()
{
    m_gradient.clear();
    m_gradient.append(QGradientStop(0.0, m_colorFrom));
    m_gradient.append(QGradientStop(0.49, m_colorFrom));
    m_gradient.append(QGradientStop(0.5, m_colorTo));
    m_gradient.append(QGradientStop(1.0, m_colorTo));
    if (m_listItem) {
        m_listItem->update();
    }
}

QSGNode *UCListItemDivider::paint(const QRectF &rect)
{
    if (m_visible && (m_gradient.size() > 0)) {
        // the parent always recreates the node, so no worries for the existing child node
        QSGRectangleNode *rectNode = m_listItem->sceneGraphContext()->createRectangleNode();
        // margins are only applied when the ListItem is in normal state, when pressed,
        // the divider is painted from edge to edge
        qreal left = (m_listItem && m_listItem->pressed) ? 0 : m_leftMargin;
        qreal right = (m_listItem && m_listItem->pressed) ? rect.width() : rect.width() - m_leftMargin - m_rightMargin;
        rectNode->setRect(QRectF(left, rect.height() - m_thickness, right, m_thickness));
        rectNode->setGradientStops(m_gradient);
        rectNode->update();
        return rectNode;
    } else {
        return 0;
    }
}

void UCListItemDivider::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    m_listItem->resize();
    m_listItem->update();
    Q_EMIT visibleChanged();
}

void UCListItemDivider::setLeftMargin(qreal leftMargin)
{
    if (m_leftMargin == leftMargin) {
        return;
    }
    m_leftMargin = leftMargin;
    m_leftMarginChanged = true;
    m_listItem->update();
    Q_EMIT leftMarginChanged();
}

void UCListItemDivider::setRightMargin(qreal rightMargin)
{
    if (m_rightMargin == rightMargin) {
        return;
    }
    m_rightMargin = rightMargin;
    m_rightMarginChanged = true;
    m_listItem->update();
    Q_EMIT rightMarginChanged();
}

void UCListItemDivider::setColorFrom(const QColor &color)
{
    if (m_colorFrom == color) {
        return;
    }
    m_colorFrom = color;
    m_colorFromChanged = true;
    updateGradient();
    Q_EMIT colorFromChanged();
}

void UCListItemDivider::setColorTo(const QColor &color)
{
    if (m_colorTo == color) {
        return;
    }
    m_colorTo = color;
    m_colorToChanged = true;
    updateGradient();
    Q_EMIT colorToChanged();
}

/******************************************************************************
 * ListItemPrivate
 */
UCListItemPrivate::UCListItemPrivate()
    : UCStyledItemBasePrivate()
    , pressed(false)
    , contentMoved(false)
    , highlightColorChanged(false)
    , ready(false)
    , overshoot(UCUnits::instance().gu(2))
    , color(Qt::transparent)
    , highlightColor(Qt::transparent)
    , attachedProperties(0)
    , contentItem(new QQuickItem)
    , divider(new UCListItemDivider)
    , leadingActions(0)
    , trailingActions(0)
    , animator(0)
{
}
UCListItemPrivate::~UCListItemPrivate()
{
}

void UCListItemPrivate::init()
{
    Q_Q(UCListItem);
    contentItem->setObjectName("ListItemHolder");
    QQml_setParent_noEvent(contentItem, q);
    contentItem->setParentItem(q);
    divider->init(q);
    // content will be redirected to the contentItem, therefore we must report
    // children changes as it would come from the main component
    QObject::connect(contentItem, &QQuickItem::childrenChanged,
                     q, &UCListItem::childrenChanged);
    q->setFlag(QQuickItem::ItemHasContents);
    // turn activeFocusOnPress on
    q->setActiveFocusOnPress(true);

    // catch theme palette changes
    QObject::connect(&UCTheme::instance(), SIGNAL(paletteChanged()), q, SLOT(_q_updateColors()));
    _q_updateColors();

    // watch size change and set implicit size;
    QObject::connect(&UCUnits::instance(), SIGNAL(gridUnitChanged()), q, SLOT(_q_updateSize()));
    _q_updateSize();
}

void UCListItemPrivate::_q_updateColors()
{
    Q_Q(UCListItem);
    highlightColor = getPaletteColor("selected", "background");
    q->update();
}

void UCListItemPrivate::_q_rebound()
{
    setPressed(false);
    // disconnect the flickable
    listenToRebind(false);
}

// rebound without animation
void UCListItemPrivate::promptRebound()
{
    setPressed(false);
}

// called when units size changes
void UCListItemPrivate::_q_updateSize()
{
    Q_Q(UCListItem);
    QQuickItem *owner = flickable ? flickable : parentItem;
    q->setImplicitWidth(owner ? owner->width() : UCUnits::instance().gu(40));
    q->setImplicitHeight(UCUnits::instance().gu(7));
    // update overshoot value
    overshoot = UCUnits::instance().gu(2);
}

// returns the index of the list item when used in model driven views,
// and the child index in other cases
int UCListItemPrivate::index()
{
    Q_Q(UCListItem);
    // is there an index context property?
    QQmlContext *context = qmlContext(q);
    QVariant index = context->contextProperty("index");
    return index.isValid() ?
                index.toInt() :
                (parentItem ? QQuickItemPrivate::get(parentItem)->childItems.indexOf(q) : -1);
}

// set pressed flag and update contentItem
void UCListItemPrivate::setPressed(bool pressed)
{
    if (this->pressed != pressed) {
        this->pressed = pressed;
        Q_Q(UCListItem);
        q->update();
        Q_EMIT q->pressedChanged();
    }
}

// connects/disconnects from the Flickable anchestor to get notified when to do rebound
void UCListItemPrivate::listenToRebind(bool listen)
{
    if (attachedProperties) {
        Q_Q(UCListItem);
        attachedProperties->listenToRebind(q, listen);
    }
}

void UCListItemPrivate::resize()
{
    Q_Q(UCListItem);
    QRectF rect(q->boundingRect());
    if (divider && divider->m_visible) {
        rect.setHeight(rect.height() - divider->m_thickness);
    }
    contentItem->setSize(rect.size());
}

void UCListItemPrivate::update()
{
    if (!ready) {
        return;
    }
    Q_Q(UCListItem);
    q->update();
}

/*!
 * \qmltype ListItem
 * \instantiates UCListItem
 * \inqmlmodule Ubuntu.Components 1.2
 * \ingroup unstable-ubuntu-listitems
 * \since Ubuntu.Components 1.2
 * \brief The ListItem element provides Ubuntu design standards for list or grid
 * views.
 * The ListItem component was designed to be used in a list view. It does not
 * define any specific layout, but while its contents can be freely chosen by
 * the developer, care must be taken to keep the contents light in order to ensure
 * good performance when used in long list views.
 *
 * The component provides two color properties which configures the item's background
 * when normal or pressed. This can be configured through \l color and \l highlightColor
 * properties.
 *
 * \c contentItem holds all components and resources declared as child to ListItem.
 * Being an Item, all properties can be accessed or altered. However, make sure you
 * never change \c x, \c y, \c width, \c height or \c anchors properties as those are
 * controlled by the ListItem itself when leading or trailing actions are revealed
 * and thus might cause the component to misbehave.
 *
 * Each ListItem has a thin divider shown on the bottom of the component. This
 * divider can be configured through the \c divider grouped property, which can
 * configure its margins from the edges of the ListItem as well as its visibility.
 */

/*!
 * \qmlsignal ListItem::clicked()
 *
 * The signal is emitted when the component gets released while the \l pressed property
 * is set. The signal is not emitted if the ListItem content is swiped or when used in
 * Flickable (or ListView, GridView) and the Flickable gets moved.
 */
UCListItem::UCListItem(QQuickItem *parent)
    : UCStyledItemBase(*(new UCListItemPrivate), parent)
{
    Q_D(UCListItem);
    d->init();
}

UCListItem::~UCListItem()
{
}

UCListItemAttached *UCListItem::qmlAttachedProperties(QObject *owner)
{
    return new UCListItemAttached(owner);
}

void UCListItem::componentComplete()
{
    UCStyledItemBase::componentComplete();
    d_func()->ready = true;
}

void UCListItem::itemChange(ItemChange change, const ItemChangeData &data)
{
    UCStyledItemBase::itemChange(change, data);
    if (change == ItemParentHasChanged) {
        Q_D(UCListItem);
        // make sure we are not connected to the previous Flickable
        d->listenToRebind(false);
        // check if we are in a positioner, and if that positioner is in a Flickable
        QQuickBasePositioner *positioner = qobject_cast<QQuickBasePositioner*>(data.item);
        if (positioner && positioner->parentItem()) {
            d->flickable = qobject_cast<QQuickFlickable*>(positioner->parentItem()->parentItem());
        } else if (data.item && data.item->parentItem()){
            // check if we are in a Flickable then
            d->flickable = qobject_cast<QQuickFlickable*>(data.item->parentItem());
        }

        // attach ListItem properties to the flickable or to the parent item
        if (d->flickable) {
            d->attachedProperties = static_cast<UCListItemAttached*>(qmlAttachedPropertiesObject<UCListItem>(d->flickable));
        } else if (data.item) {
            d->attachedProperties = static_cast<UCListItemAttached*>(qmlAttachedPropertiesObject<UCListItem>(data.item));
        } else {
            // about to be deleted or reparented, disable attached
            d->attachedProperties = 0;
        }

        if (data.item) {
            QObject::connect(d->flickable ? d->flickable : data.item, SIGNAL(widthChanged()), this, SLOT(_q_updateSize()));
        }

        // update size
        d->_q_updateSize();
    }
}

void UCListItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    UCStyledItemBase::geometryChanged(newGeometry, oldGeometry);
    // resize background item
    Q_D(UCListItem);
    d->resize();
}

QSGNode *UCListItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    Q_D(UCListItem);
    QColor color = d->pressed ? d->highlightColor : d->color;

    delete oldNode;
    if (width() <= 0 || height() <= 0) {
        return 0;
    }

    QSGRectangleNode *rectNode = 0;
    if (color.alpha() > 0) {
        rectNode = QQuickItemPrivate::get(this)->sceneGraphContext()->createRectangleNode();
        rectNode->setColor(color);
        // cover only the area of the contentItem
        rectNode->setRect(d->contentItem->boundingRect());
        rectNode->update();
    }
    oldNode = rectNode;
    if (d->divider && d->divider->m_visible) {
        QSGNode * dividerNode = d->divider->paint(boundingRect());
        if (dividerNode && oldNode) {
            oldNode->appendChildNode(dividerNode);
        } else if (dividerNode) {
            oldNode = dividerNode;
        }
    }
    return oldNode;
}

void UCListItem::mousePressEvent(QMouseEvent *event)
{
    UCStyledItemBase::mousePressEvent(event);
    Q_D(UCListItem);
    if (d->attachedProperties && d->attachedProperties->isMoving()) {
        // while moving, we cannot select any items
        return;
    }
    if (event->button() == Qt::LeftButton) {
        d->setPressed(true);
        // connect the Flickable to know when to rebound
        d->listenToRebind(true);
    }
    // accept the event so we get the rest of the events as well
    event->setAccepted(true);
}

void UCListItem::mouseReleaseEvent(QMouseEvent *event)
{
    UCStyledItemBase::mouseReleaseEvent(event);
    Q_D(UCListItem);
    // set released
    if (d->pressed) {
        d->listenToRebind(false);
        Q_EMIT clicked();
    }
    d->setPressed(false);
}

/*!
 * \qmlproperty ListItemActions ListItem::leadingActions
 *
 * The property holds the actions and its configuration to be revealed when swiped
 * from left to right.
 */
UCListItemActions *UCListItem::leadingActions() const
{
    Q_D(const UCListItem);
    return d->leadingActions;
}
void UCListItem::setLeadingActions(UCListItemActions *actions)
{
    Q_D(UCListItem);
    if (d->leadingActions == actions) {
        return;
    }
    d->leadingActions = actions;
    Q_EMIT leadingActionsChanged();
}

/*!
 * \qmlproperty ListItemActions ListItem::trailingActions
 *
 * The property holds the actions and its configuration to be revealed when swiped
 * from right to left.
 */
UCListItemActions *UCListItem::trailingActions() const
{
    Q_D(const UCListItem);
    return d->trailingActions;
}
void UCListItem::setTrailingActions(UCListItemActions *actions)
{
    Q_D(UCListItem);
    if (d->trailingActions == actions) {
        return;
    }
    d->trailingActions = actions;
    Q_EMIT trailingActionsChanged();
}

/*!
 * \qmlproperty Item ListItem::contentItem
 *
 * contentItem holds the components placed on a ListItem.
 */
QQuickItem* UCListItem::contentItem() const
{
    Q_D(const UCListItem);
    return d->contentItem;
}

/*!
 * \qmlpropertygroup ::ListItem::divider
 * \qmlproperty bool ListItem::divider.visible
 * \qmlproperty real ListItem::divider.leftMargin
 * \qmlproperty real ListItem::divider.rightMargin
 * \qmlproperty real ListItem::divider.colorFrom
 * \qmlproperty real ListItem::divider.colorTo
 *
 * This grouped property configures the thin divider shown in the bottom of the
 * component. Configures the visibility and the margins from the left and right
 * of the ListItem. When swiped left or right to reveal the actions, it is not
 * moved together with the content. \c colorFrom and \c colorTo configure
 * the starting and ending colors of the divider.
 *
 * When \c visible is true, the ListItem's content size gets thinner with the
 * divider's \c thickness.
 *
 * The default values for the properties are:
 * \list
 * \li \c visible: true
 * \li \c leftMargin: 2 GU
 * \li \c rightMargin: 2 GU
 * \endlist
 */
UCListItemDivider* UCListItem::divider() const
{
    Q_D(const UCListItem);
    return d->divider;
}

/*!
 * \qmlproperty bool ListItem::pressed
 * True when the item is pressed. The items stays pressed when the mouse or touch
 * is moved horizontally. When in Flickable (or ListView), the item gets un-pressed
 * (false) when the mouse or touch is moved towards the vertical direction causing
 * the flickable to move.
 */
bool UCListItem::pressed() const
{
    Q_D(const UCListItem);
    return d->pressed;
}

/*!
 * \qmlproperty bool ListItem::contentMoving
 * \readonly
 * The property describes whether the content is moving or not. The content is
 * moved when swiped or when snapping in or out, and lasts till the snapping
 * animation completes.
 */

/*!
 * \qmlsignal ListItem::contentMovementStarted()
 * The signal is emitted when the content movement has started.
 */

/*!
 * \qmlsignal UCListItemPrivate::contentMovementEnded
 * The signal is emitted when the content movement has ended.
 */
bool UCListItemPrivate::contentMoving() const
{
    return contentMoved;
}
void UCListItemPrivate::setContentMoving(bool moved)
{
    if (contentMoved == moved) {
        return;
    }
    contentMoved = moved;
    Q_Q(UCListItem);
    if (contentMoved) {
        Q_EMIT q->contentMovementStarted();
    } else {
        Q_EMIT q->contentMovementEnded();
    }
    Q_EMIT q->contentMovingChanged();

}

/*!
 * \qmlproperty color ListItem::color
 * Configures the color of the normal background. The default value is transparent.
 */
QColor UCListItem::color() const
{
    Q_D(const UCListItem);
    return d->color;
}
void UCListItem::setColor(const QColor &color)
{
    Q_D(UCListItem);
    if (d->color == color) {
        return;
    }
    d->color = color;
    update();
    Q_EMIT colorChanged();
}

/*!
 * \qmlproperty color ListItem::highlightColor
 * Configures the color when pressed. Defaults to the theme palette's background color.
 */
QColor UCListItem::highlightColor() const
{
    Q_D(const UCListItem);
    return d->highlightColor;
}
void UCListItem::setHighlightColor(const QColor &color)
{
    Q_D(UCListItem);
    if (d->highlightColor == color) {
        return;
    }
    d->highlightColor = color;
    // no more theme change watch
    disconnect(&UCTheme::instance(), SIGNAL(paletteChanged()), this, SLOT(_q_updateColors()));
    update();
    Q_EMIT highlightColorChanged();
}

/*!
 * \qmlproperty list<Object> ListItem::data
 * \default
 * Overloaded default property containing all the children and resources.
 */
QQmlListProperty<QObject> UCListItem::data()
{
    Q_D(UCListItem);
    return QQuickItemPrivate::get(d->contentItem)->data();
}

/*!
 * \qmlproperty list<Item> ListItem::children
 * Overloaded default property containing all the visible children of the item.
 */
QQmlListProperty<QQuickItem> UCListItem::children()
{
    Q_D(UCListItem);
    return QQuickItemPrivate::get(d->contentItem)->children();
}

#include "moc_uclistitem.cpp"
