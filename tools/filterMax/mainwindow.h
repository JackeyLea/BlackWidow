#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "similarity.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonGet_clicked();

private:
    Ui::MainWindow *ui;
    Similarity *sim;
};
#endif // MAINWINDOW_H
