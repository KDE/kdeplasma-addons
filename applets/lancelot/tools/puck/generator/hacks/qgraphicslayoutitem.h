#ifndef LANCELOT_QGRAPHICS_LAYOUT_ITEM_H
#define LANCELOT_QGRAPHICS_LAYOUT_ITEM_H

class Q_GUI_EXPORT QGraphicsLayoutItem
{
    Q_PROPERTY ( QSize minimumSize READ minimumSize WRITE setMinimumSize )
    Q_PROPERTY ( qreal minimumWidth READ minimumWidth WRITE setMinimumWidth )
    Q_PROPERTY ( qreal minimumHeight READ minimumHeight WRITE setMinimumHeight )

    Q_PROPERTY ( QSize maximumSize READ maximumSize WRITE setMaximumSize )
    Q_PROPERTY ( qreal maximumWidth READ maximumWidth WRITE setMaximumWidth )
    Q_PROPERTY ( qreal maximumHeight READ maximumHeight WRITE setMaximumHeight )

    Q_PROPERTY ( QSize preferredSize READ preferredSize WRITE setPreferredSize )
    Q_PROPERTY ( qreal preferredWidth READ preferredWidth WRITE setPreferredWidth )
    Q_PROPERTY ( qreal preferredHeight READ preferredHeight WRITE setPreferredHeight )

};
#endif /* LANCELOT_QGRAPHICS_LAYOUT_ITEM_H */
