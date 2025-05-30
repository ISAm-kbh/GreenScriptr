#ifndef GREENWINDOW_H
#define GREENWINDOW_H

#include "mainwindow.h"

class GreenWindow
{
public:
    static QList<MainWindow *> windows;

    GreenWindow() = delete;
};
#endif // GREENWINDOW_H
