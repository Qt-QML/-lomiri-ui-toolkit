#ifndef UCSLOTSLAYOUTPRIVATE_H
#define UCSLOTSLAYOUTPRIVATE_H

#include <QtQuick/private/qquickitem_p.h>
#include "ucslotslayout.h"

#define IMPLICIT_SLOTSLAYOUT_WIDTH_GU      40
#define IMPLICIT_SLOTSLAYOUT_HEIGHT_GU     7
#define IMPLICIT_SLOTSLAYOUT_MARGIN        2
#define SLOTSLAYOUT_LABELS_SPACING         1

class UCSlotsLayoutPrivate : QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(UCSlotsLayout)
public:
    UCSlotsLayoutPrivate();
    virtual ~UCSlotsLayoutPrivate();
    void init();

    static inline UCSlotsLayoutPrivate *get(UCSlotsLayout *that)
    {
        Q_ASSERT(that);
        return that->d_func();
    }

    void _q_relayout();
    void _q_updateSize();
    void _q_updateLabelsAnchors();

    bool ready;

    //used for vertical centering
    qreal labelsBoundingBoxHeight;

    QQuickText m_title;
    QQuickText m_subtitle;
    QQuickText m_subsubtitle;

    //in this variable we cache the current parent so that we can disconnect from the signals
    //when the parent changes. We need this because otherwise inside itemChange(..) we would
    //only have access to the new parent
    QQuickItem* m_parentItem;

private:
    void setDefaultLabelsProperties();

};

#endif // UCSLOTSLAYOUTPRIVATE_H
