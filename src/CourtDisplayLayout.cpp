//
// Created by Fanchao Liu on 28/07/20.
//

#include "CourtDisplayLayout.h"

#include <vector>
#include <memory>
#include <cmath>

struct CourtDisplayLayout::Impl {
    std::vector<std::unique_ptr<QLayoutItem>> children;

    size_t minWidth = 300;
};

void CourtDisplayLayout::addItem(QLayoutItem *item) {
    d->children.emplace_back(item);
}

QSize CourtDisplayLayout::minimumSize() const {
    QSize size;
    for (const auto &child : d->children) {
        size = size.expandedTo(child->minimumSize());
    }
    return size;
}

void CourtDisplayLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);

    if (!d->children.empty()) {
        size_t desiredChildWidth = std::max(d->minWidth, rect.width() / d->children.size());
        const size_t numColumns = rect.width() / desiredChildWidth;
        const size_t childWidth = rect.width() / numColumns;
        const size_t numRows = std::ceil(d->children.size() / static_cast<double>(numColumns));
        const size_t childHeight = rect.height() / numRows;

        int x = rect.x(), y = rect.y();
        for (size_t i = 0, size = d->children.size(); i < size; i++) {
            d->children[i]->setGeometry(QRect(x, y, childWidth, childHeight));
            if ((i + 1) % numColumns == 0) {
                y += childHeight;
                x = 0;
            } else {
                x += childWidth;
            }
        }
    }
}

QLayoutItem *CourtDisplayLayout::itemAt(int index) const {
    if (index >= d->children.size() || index < 0) {
        return nullptr;
    }

    return d->children[index].get();
}

QLayoutItem *CourtDisplayLayout::takeAt(int index) {
    if (index >= d->children.size() || index < 0) {
        return nullptr;
    }

    auto item = std::move(d->children[index]);
    d->children.erase(d->children.begin() + index);
    return item.release();
}

CourtDisplayLayout::CourtDisplayLayout():d(new Impl) {}

CourtDisplayLayout::~CourtDisplayLayout() {
    delete d;
}

int CourtDisplayLayout::count() const {
    return d->children.size();
}

QSize CourtDisplayLayout::sizeHint() const {
    return QSize();
}
