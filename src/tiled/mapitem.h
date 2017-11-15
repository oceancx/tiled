/*
 * mapitem.h
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

#pragma once

#include <QGraphicsItem>
#include <QMap>
#include <QSet>

namespace Tiled {

class Layer;
class MapObject;

namespace Internal {

class LayerItem;
class MapDocument;
class MapObjectItem;

class MapItem : public QGraphicsItem
{
public:
    MapItem(MapDocument *mapDocument, QGraphicsItem *parent = nullptr);

    MapDocument *mapDocument() const;

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *,
               QWidget *widget = nullptr) override;

    /**
     * Returns the set of selected map object items.
     */
    const QSet<MapObjectItem*> &selectedObjectItems() const
    { return mSelectedObjectItems; }

    /**
     * Sets the set of selected map object items. This translates to a call to
     * MapDocument::setSelectedObjects.
     */
    void setSelectedObjectItems(const QSet<MapObjectItem*> &items);

    /**
     * Returns the MapObjectItem associated with the given \a mapObject.
     */
    MapObjectItem *itemForObject(MapObject *object) const
    { return mObjectItems.value(object); }

signals:
    void selectedObjectItemsChanged();

private:
    void createLayerItems(const QList<Layer *> &layers);
    LayerItem *createLayerItem(Layer *layer);

    void updateCurrentLayerHighlight();

    MapDocument *mMapDocument;
    QGraphicsRectItem *mDarkRectangle;
    QMap<Layer*, LayerItem*> mLayerItems;
    QMap<MapObject*, MapObjectItem*> mObjectItems;
    QSet<MapObjectItem*> mSelectedObjectItems;
};

inline MapDocument *MapItem::mapDocument() const
{
    return mMapDocument;
}

} // namespace Internal
} // namespace Tiled
