//
// Created by Fanchao Liu on 30/06/20.
//

#ifndef GAMEMATCHER_ADAPTER_H
#define GAMEMATCHER_ADAPTER_H

#include <QWidget>
#include <QLayout>

template <typename Entities, typename WidgetCreate, typename WidgetUpdate>
void setEntities(QLayout *layout, const Entities &entities, WidgetCreate create, WidgetUpdate update) {
    typedef decltype(create()) WidgetPointer;
    const size_t numViews = layout->count();
    const size_t numPlayers = entities.size();
    const size_t numReuse = qMin(numPlayers, numViews);
    int i = 0;
    for (; i < numReuse; i++) {
        update(qobject_cast<WidgetPointer>(layout->itemAt(i)->widget()), entities[i]);
    }

    if (numViews < numPlayers) {
        // Add missing
        for (; i < numPlayers; i++) {
            WidgetPointer widget = create();
            update(widget, entities[i]);
            layout->addWidget(widget);
        }
    } else if (numPlayers < numViews) {
        // Remove excessive
        while (layout->count() > numPlayers) {
            layout->removeItem(layout->itemAt(layout->count() - 1));
        }
    }
}

#endif //GAMEMATCHER_ADAPTER_H
