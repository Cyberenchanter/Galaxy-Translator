#include "mainwindow.h"
#include <QtWidgets>
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCurrentFile(QString());
    ui->actionNew_File->setShortcuts(QKeySequence::New);
    ui->actionOpen->setShortcuts(QKeySequence::Open);
    ui->actionSave->setShortcuts(QKeySequence::Save);
    ui->actionSave_As->setShortcuts(QKeySequence::SaveAs);
    ui->actionClose->setShortcuts(QKeySequence::Close);
    ui->actionCut->setShortcuts(QKeySequence::Cut);
    ui->actionCopy->setShortcuts(QKeySequence::Copy);
    ui->actionPaste->setShortcuts(QKeySequence::Paste);
    //connect(ui->actionCut, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::cut);
    //connect(ui->actionCopy, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::copy);
    //connect(ui->actionPaste, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::paste);
    //disconnect(ui->actionCut, &QAction::triggered, ui->PlainEdit, &QPlainTextEdit::cut);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::maybeSave()
{
    if (!ui->maintable->isWindowModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return on_actionSave_triggered();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::inittable()
{
    rowcount=0;
    ui->maintable->setRowCount(0);
}

void MainWindow::updatestat()
{
    qint64 len=mlist.size();
    for(int i=0;i<len;i++){
        if(!mlist[i].lang[lang_ori].isEmpty()){
            if(mlist[i].version[lang_ori]>mlist[i].version[lang_tar])
                mlist_stat[i]=1;
            if(mlist[i].version[lang_ori]==mlist[i].version[lang_tar])
                mlist_stat[i]=2;
        }else{
            if(!mlist[i].lang[lang_tar].isEmpty()){
                if(mlist[i].version[lang_ori]>mlist[i].version[lang_tar])
                    mlist_stat[i]=3;
            }
        }
    }
}

void MainWindow::parsefile(const QString &buf)
{
    mydata ans;
    bool stat=0;
    qint64 len=buf.length(),index=0;
    QString version;
    for(qint64 i=0;i<len;i++){
        if(buf[i]=='}'){
            stat=!stat;
            if(index>MAXLANGUAGE&&index<=MAXLANGUAGE*2){
                ans.version[index-MAXLANGUAGE-1]=version.toLongLong();
                version.clear();
            }
            index++;
        }
        if(stat){
            if(index==0)
                ans.id+=buf[i];
            if(index>0&&index<=MAXLANGUAGE)
                ans.lang[index-1]+=buf[i];
            if(index>MAXLANGUAGE&&index<=MAXLANGUAGE*2){
                version+=buf[i];
            }

        }
        if(buf[i]=='{'){
            stat=!stat;
        }
    }
    ans.no=mlist.size()+1;
    mlist.push_back(ans);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    QString buf;
    while (in.readLineInto(&buf)){
        parsefile(buf);
    }
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    ui->statusbar->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
//! [44] //! [45]
{
    QString errorMessage;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        qint64 len=mlist.size();
        for(qint64 i=0;i<len;i++){
            for(int j=0;j<=MAXLANGUAGE*2;j++){
                if(j==0)
                    out<<"{"<<mlist[i].id<<"}";
                if(j>0&&j<=MAXLANGUAGE)
                    out<<"{"<<mlist[i].lang[j-1]<<"}";
                if(j>MAXLANGUAGE&&j<=MAXLANGUAGE*2){
                    out<<"{"<<mlist[i].version[j-1-MAXLANGUAGE]<<"}";
                }
            }
            Qt::endl(out);
        }
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    ui->statusbar->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
//! [3] //! [4]
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::on_actionNew_File_triggered()
{
    if(maybeSave()){
        setCurrentFile(QString());
    }
}


void MainWindow::on_plainTextEdit_textChanged()
{

}


void MainWindow::on_actionOpen_triggered()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}


bool MainWindow::on_actionSave_triggered()
{
    if (curFile.isEmpty()) {
        return on_actionSave_As_triggered();
    } else {
        return saveFile(curFile);
    }
}


bool MainWindow::on_actionSave_As_triggered()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}


void MainWindow::on_actionClose_triggered()
{
    close();
}
