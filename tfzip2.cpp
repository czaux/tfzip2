#include "tfzip2.h"
#include "ui_tfzip2.h"

TFZip2::TFZip2(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TFZip2)
{
    ui->setupUi(this);
}

TFZip2::~TFZip2()
{
    delete ui;
}
