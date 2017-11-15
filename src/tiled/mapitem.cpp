/*
 * mapitem.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mapitem.h"

#include "tilelayer.h"
#include "grouplayer.h"

#include "grouplayeritem.h"
#include "imagelayeritem.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "objectgroupitem.h"
#include "objectselectionitem.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"

#include <QPen>

namespace Tiled {
namespace Internal {

static const qreal darkeningFactor = 0.6;
static const qreal opacityFactor = 0.4;

MapItem::MapItem(MapDocument *mapDocument, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mMapDocument(mapDocument)
    , mDarkRectangle(new QGraphicsRectItem(this))
{
    // Since we don't do any painting, we can spare us the call to paint()
    setFlag(QGraphicsItem::ItemHasNoContents);

    auto tileSelectionItem = new TileSelectionItem(mMapDocument, this);
    tileSelectionItem->setZValue(10000 - 2);

    auto objectSelectionItem = new ObjectSelectionItem(mMapDocument, this);
    objectSelectionItem->setZValue(10000 - 1);

    mDarkRectangle->setPen(Qt::NoPen);
    mDarkRectangle->setBrush(Qt::black);
    mDarkRectangle->setOpacity(darkeningFactor);
}

QRectF MapItem::boundingRect() const
{
    return QRectF();
}

void MapItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void MapItem::setSelectedObjectItems(const QSet<MapObjectItem *> &items)
{
    // Inform the map document about the newly selected objects
    QList<MapObject*> selectedObjects;
    selectedObjects.reserve(items.size());

    for (const MapObjectItem *item : items)
        selectedObjects.append(item->mapObject());

    mMapDocument->setSelectedObjects(selectedObjects);
}

void MapItem::createLayerItems(const QList<Layer *> &layers)
{
    int layerIndex = 0;

    for (Layer *layer : layers) {
        LayerItem *layerItem = createLayerItem(layer);
        layerItem->setZValue(layerIndex);
        ++layerIndex;
    }
}

LayerItem *MapItem::createLayerItem(Layer *layer)
{
    LayerItem *layerItem = nullptr;

    QGraphicsItem *parent = this;
    if (layer->parentLayer())
        parent = mLayerItems.value(layer->parentLayer());

    switch (layer->layerType()) {
    case Layer::TileLayerType:
        layerItem = new TileLayerItem(static_cast<TileLayer*>(layer), mMapDocument, parent);
        break;

    case Layer::ObjectGroupType: {
        auto og = static_cast<ObjectGroup*>(layer);
        const ObjectGroup::DrawOrder drawOrder = og->drawOrder();
        ObjectGroupItem *ogItem = new ObjectGroupItem(og, parent);
        int objectIndex = 0;
        for (MapObject *object : og->objects()) {
            MapObjectItem *item = new MapObjectItem(object, mMapDocument,
                                                    ogItem);
            if (drawOrder == ObjectGroup::TopDownOrder)
                item->setZValue(item->y());
            else
                item->setZValue(objectIndex);

            mObjectItems.insert(object, item);
            ++objectIndex;
        }
        layerItem = ogItem;
        break;
    }

    case Layer::ImageLayerType:
        layerItem = new ImageLayerItem(static_cast<ImageLayer*>(layer), mMapDocument, parent);
        break;

    case Layer::GroupLayerType:
        layerItem = new GroupLayerItem(static_cast<GroupLayer*>(layer), parent);
        break;
    }

    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());

    mLayerItems.insert(layer, layerItem);

    if (GroupLayer *groupLayer = layer->asGroupLayer())
        createLayerItems(groupLayer->layers());

    return layerItem;
}

void MapItem::updateCurrentLayerHighlight()
{
    const auto currentLayer = mMapDocument->currentLayer();

    if (!mHighlightCurrentLayer || !currentLayer) {
        if (mDarkRectangle->isVisible()) {
            mDarkRectangle->setVisible(false);

            // Restore opacity for all layers
            const auto layerItems = mLayerItems;
            for (auto layerItem : layerItems)
                layerItem->setOpacity(layerItem->layer()->opacity());
        }

        return;
    }

    // Darken layers below the current layer
    const int siblingIndex = currentLayer->siblingIndex();
    const auto parentLayer = currentLayer->parentLayer();
    const auto parentItem = mLayerItems.value(parentLayer);

    mDarkRectangle->setParentItem(parentItem);
    mDarkRectangle->setZValue(siblingIndex - 0.5);
    mDarkRectangle->setVisible(true);

    // Set layers above the current layer to reduced opacity
    LayerIterator iterator(mMapDocument->map());
    qreal multiplier = 1;

    while (Layer *layer = iterator.next()) {
        GroupLayer *groupLayer = layer->asGroupLayer();
        if (!groupLayer)
            mLayerItems.value(layer)->setOpacity(layer->opacity() * multiplier);

        if (layer == currentLayer)
            multiplier = opacityFactor;
    }
}

} // namespace Internal
} // namespace Tiled
