#ifndef TFZIP2_H
#define TFZIP2_H

#include <QMainWindow>

namespace Ui {
class TFZip2;
}

class TFZip2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit TFZip2(QWidget *parent = 0);
    ~TFZip2();

private:
    Ui::TFZip2 *ui;
};

#endif // TFZIP2_H
