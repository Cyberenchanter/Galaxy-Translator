#include "iodialog.h"
#include "mainwindow.h"
#include "./ui_iodialog.h"

IODialog::IODialog(QWidget *parent,bool is_out) :
    QDialog(parent),
    ui(new Ui::IODialog)
{
    ui->setupUi(this);
    ui->comp_select->setIcon(QIcon(":/images/folder.png"));
    combo[0]=ui->combo_de;
    combo[1]=ui->combo_en;
    combo[2]=ui->combo_es;
    combo[3]=ui->combo_esmx;
    combo[4]=ui->combo_fr;
    combo[5]=ui->combo_it;
    combo[6]=ui->combo_ko;
    combo[7]=ui->combo_pl;
    combo[8]=ui->combo_pt;
    combo[9]=ui->combo_ru;
    combo[10]=ui->combo_zhcn;
    combo[11]=ui->combo_zhtw;
    for(int i=0;i<MAXLANGUAGE;i++){
        combo[i]->setEnabled(false);
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if(is_out){
        ui->misc->hide();
        for(int i=0;i<MAXLANGUAGE;i++){
            combo[i]->removeItem(1);
        }
        setWindowTitle(QString("Export to component folder"));
    }else{
        setWindowTitle(QString("Import from component folder"));
    }
    updatedir();
}

IODialog::~IODialog()
{
    delete ui;
}
void IODialog::updatedir(){
    QFileInfo fileinfo = QFileInfo(QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("ComponentList (*.SC2Components *.SC2Mod)")));
    if(fileinfo.suffix()=="SC2Components"){
        dir=fileinfo.absolutePath();
    }else{
        if(fileinfo.suffix()=="SC2Mod"){
            dir=fileinfo.absoluteFilePath();
        }
        else{
            return;
        }
    }
    dir+="/";
    ui->comp_path->setText(dir);
    QString path;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    for(int i=0;i<MAXLANGUAGE;i++){
        path=dir+lang_code[i]+".SC2Data/LocalizedData/GameStrings.txt";
        QFile file(path);
        if(file.exists()){
            combo[i]->setEnabled(true);
            combo[i]->setCurrentIndex(1);
        }else{
            combo[i]->setCurrentIndex(0);
            combo[i]->setEnabled(false);
        }
    }
}

void IODialog::on_comp_select_clicked()
{
    updatedir();
}
void IODialog::getoptions(MainWindow::iooptions &options){
    options.dir=dir;
    for(int i=0;i<MAXLANGUAGE;i++){
        options.lang_state[i]=combo[i]->currentIndex();
    }
    options.misc[0]=ui->is_reduct->checkState();
    options.misc[1]=ui->is_trigger_merge->checkState();
    return;
}

