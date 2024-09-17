#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sim(new Similarity())
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonGet_clicked()
{
    ui->lineEditMD5->clear();
    ui->lineEditResult->clear();

    QString source = ui->textSource->toPlainText();
    QTextEdit a(source,this);
    ui->textSource->setPlainText(a.toPlainText());
    source.clear();
    source = ui->textSource->toPlainText();

    if(source.isEmpty()) {
        ui->lineEditMD5->setText(QString("Empty"));
        ui->lineEditResult->setText(QString("Empty"));
        return;
    }

    ui->lineEditMD5->setText(sim->checkout(source));
    ui->lineEditResult->setText(sim->maxStr);
}
