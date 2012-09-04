#include "verticalButton.h"
#include <QStylePainter>

/*!
\brief Vychozi konsturktor
*/
VerticalButton::VerticalButton(QWidget* parent)
    : QPushButton(parent)
{
    _orientation = Qt::Vertical;
}

/*!
\brief Konstruktor s moznosti definice popisku tlacitka
*/
VerticalButton::VerticalButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    _orientation = Qt::Vertical;
}

/*!
\brief Nastaveni velikosti tlacitka
*/
QSize VerticalButton::minimumSizeHint() const
{
    QSize size = QPushButton::minimumSizeHint();
    size.transpose();
    return size;
}

/*!
\brief Vykresleni tlacitka
*/
void VerticalButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    QStyleOptionButton options;

    painter.rotate(-90);
    painter.translate(-height(), 0);

    options.initFrom(this);
    QSize size = options.rect.size();
    size.transpose();
    options.rect.setSize(size);
    options.features = QStyleOptionButton::None;
    options.text = text();

    painter.drawControl(QStyle::CE_PushButton, options);
}
