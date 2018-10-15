#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
class IconButton : public QLabel
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = 0);

signals:
    void iconClicked(void);
public slots:

protected:
        void mousePressEvent(QMouseEvent *ev);
};

#endif // ICONBUTTON_H
