#ifndef VERTICALBUTTON_H
#define VERTICALBUTTON_H

#include <QPushButton>
#include <QStyleOptionButton>

class VerticalButton : public QPushButton
{
public:
    VerticalButton(QWidget* parent = 0);
    VerticalButton(const QString& text, QWidget* parent = 0);
    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);

private:
    Qt::Orientation _orientation;
};

#endif // VERTICALBUTTON_H
