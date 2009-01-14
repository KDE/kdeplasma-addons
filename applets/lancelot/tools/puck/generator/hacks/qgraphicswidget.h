#ifndef LANCELOT_QGRAPHICS_WIDGET_H
#define LANCELOT_QGRAPHICS_WIDGET_H

class Q_GUI_EXPORT QGraphicsWidget : public QGraphicsLayoutItem
{
    Q_OBJECT
    Q_PROPERTY(QPalette palette READ palette WRITE setPalette)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection RESET unsetLayoutDirection)
    Q_PROPERTY(QSizeF size READ size WRITE resize)
    Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(Qt::WindowFlags windowFlags READ windowFlags WRITE setWindowFlags)
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle)
};
#endif /* LANCELOT_QGRAPHICS_WIDGET_H */
