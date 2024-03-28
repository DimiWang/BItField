
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QDebug>
#include <QMessageBox>
#include <QElapsedTimer>
#include "lineedit.h"
#include <QFileDialog>
#include <QtMath>
#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QtEndian>
#include <QInputDialog>


static Register REG(0,"",Register::AllowSameName);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFont f = ui->teResult->font();
    f.setFamily("Monospace");
    ui->teResult->setFont(f);
    REG.blockSignals(1);//block forever    
    loadSettings();
    loadRecentFiles();
    m_dataFilePath = QDir::currentPath();
    m_jsonFilePath = QDir::currentPath();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::analyzeChanges()
{

}

void MainWindow::loadSettings()
{
    QSettings settings(QString("%2/%1.ini").arg(qApp->applicationName()).arg(qApp->applicationDirPath()));
    m_settings_ascii_windows = settings.value("General/ascii_windows_crlf",false).toBool();
    ui->cbUseWindowsCRLF->blockSignals(1);
    ui->cbUseWindowsCRLF->setChecked(m_settings_ascii_windows);
    ui->cbUseWindowsCRLF->blockSignals(0);
}

void MainWindow::saveSettings()
{
    QSettings settings(QString("%2/%1.ini").arg(qApp->applicationName()).arg(qApp->applicationDirPath()));
    settings.setValue("General/ascii_windows_crlf", m_settings_ascii_windows );
}

void MainWindow::loadRecentFiles(){
    QSettings settings(QString("%2/%1.ini").arg(qApp->applicationName()).arg(qApp->applicationDirPath()));

    const QVariantList files_list = settings.value("Files/RecentFiles",QVariant()).toList();
    bool save_again = false;
    int bak_index = ui->cmStructure->currentIndex();
    ui->cmStructure->blockSignals(1);
    ui->cmStructure->clear();
    ui->cmStructure->addItem("Load...");
    for(int i=0;i<files_list.count();i++){
        const QString file_path = files_list.at(i).toString();

        if(QFile::exists(file_path)){
            ui->cmStructure->addItem( QFileInfo(file_path).baseName(),file_path);
        }
        else save_again = true;
    }
    if(bak_index>=0)
        ui->cmStructure->setCurrentIndex(bak_index);
    ui->cmStructure->blockSignals(0);

    settings.beginGroup("Formats");

    foreach(const QString &key, settings.allKeys()){
        settings.value(key);
        ui->cmStructure->addItem(QString("-%1").arg(key), settings.value(key).toString());
    }
    settings.endGroup();

    if(save_again){
        saveRecentFiles();
    }


}

void MainWindow::on_pbApply_clicked()
{    
    REG.clear();

    QElapsedTimer t;
    t.start();
    quint32 load_options =0;
    if(ui->cmStructure->currentIndex()==1)
        load_options = Register::AbsoluteRange;

    if(!REG.loadJsonData(ui->teRegister->toPlainText().toLatin1(),load_options)){
        QMessageBox::critical(0,"Error","Error parsing file");
    }
    ui->lbDataSize->setText(QString("%1[%2h] bits %3[%4h] bytes")
                            .arg(REG.size())
                            .arg(REG.size(),4,16,QChar('0'))
                            .arg(REG.size()/8)
                            .arg(REG.size()/8,4,16,QChar('0'))
                            );
    ui->cmItems->blockSignals(1);
    ui->cmItems->clear();
    ui->cmItems->addItems(REG.fieldsList());
    ui->lbFieldInfo->setText("");
    ui->cmItems->blockSignals(0);
}

static bool loadData(const QString &file_name, int format){

    QFile f;
    if(file_name.isEmpty()) return false;

    f.setFileName(file_name);
    if(QFile::exists(file_name) && f.open(QFile::ReadOnly)){
        switch(format){

        case 0:{
            quint32 addr=0;
            QByteArray file_text = f.readAll();
            QByteArrayList lines;
            if(file_text.contains(QByteArray("\r\n"))){
                lines = file_text.replace("\r\n","\n").split('\n');
            }else{
                lines = file_text.split('\n');
            }
            REG.fill(1);
            for(int i=0;i<lines.count();i++){
                if(addr<(quint32)REG.size()){
                    const quint32 value = lines[i].toUInt(0,16);
                    REG.setValue(addr,addr+32-1,value);
                }
                addr+=32;
            }
        }
            break;

        case 1:{

        }
            break;

        case 2:{
            QByteArray file_data = f.readAll();
            REG.fromByteArray(file_data);
        }
            break;


        }
    }else{
        QMessageBox::critical(0,"reading",QString("Can't open file"));
        return false;
    }
    return true;
}
void MainWindow::on_pbLoadFile_clicked()
{
    QFile f;
    if(REG.isEmpty()){
        QMessageBox::critical(0,"reading",QString("Reg is empty"));
        return ;
    }

    if(ui->cmFormat->currentIndex()==0){
        const QString filename = QFileDialog::getOpenFileName(0,"Load",m_dataFilePath,"Text(*.txt *.hex)");
        if(loadData(filename, 0)){
            QFileInfo fi(filename);
            ui->lbResultFile->setText(QString("<b>%1</b>").arg(fi.baseName()));
            m_dataFilePath = fi.filePath();
            update_content();
        }
    }else if(ui->cmFormat->currentIndex()==1){

    }
    else{
        const QString filename = QFileDialog::getOpenFileName(0,"Load",m_dataFilePath,"Binary (*.bin);;Sb3 files(*.sb3);;Other binary(*.*)");
        if( loadData(filename, 2) ){
            QFileInfo fi(filename);
            ui->lbResultFile->setText(QString("<b>%1</b>").arg(fi.baseName()));
            m_dataFilePath = fi.filePath();
            update_content();
        }
    }
}

void MainWindow::saveRecentFiles(){
    QSettings settings(QString("%2/%1.ini").arg(qApp->applicationName()).arg(qApp->applicationDirPath()));

    QStringList recent_files;
    for(int i=1;i<ui->cmStructure->count();i++){
        if(ui->cmStructure->itemText(i)== "Load..."){

        }
        else if(ui->cmStructure->itemText(i).startsWith("-")){

        }
        else{
            recent_files.append(ui->cmStructure->itemData(i).toString());
        }
        settings.setValue("Files/RecentFiles",recent_files);
    }
}

void MainWindow::on_pbSetValue_clicked()
{
    REG.setFieldValue(ui->cmItems->currentText(), ui->leValue->value());
    update_content();
}


void MainWindow::on_cmStructure_activated(int index)
{
    QFile f;
    if(ui->cmStructure->currentText()=="Load..."){
        f.setFileName(QFileDialog::getOpenFileName(0,"",m_jsonFilePath,"JSON (*.json)",0));
        if(QFile::exists(f.fileName())){
            QFileInfo fi(f.fileName());
            m_jsonFilePath = fi.filePath();
            const QString item_name = fi.baseName();
            ui->cmStructure->addItem(item_name, f.fileName());
            if(f.open(QFile::ReadOnly))
                ui->teRegister->setPlainText(f.readAll());
            saveRecentFiles();

            // set current item
            ui->cmStructure->blockSignals(1);
            ui->cmStructure->setCurrentText(item_name);
            ui->cmStructure->blockSignals(0);
        }
    }
    if(ui->cmStructure->currentText().startsWith("-")){
        ui->teRegister->setPlainText(ui->cmStructure->itemData(index).toString());
        return;
    }
    else{
        f.setFileName(ui->cmStructure->itemData(index).toString());
    }

    if(f.open(QIODevice::ReadOnly)){
        ui->teRegister->setPlainText(f.readAll());
        f.close();
    }
}



QString MainWindow::itemToString(BitField *pfield, Represent represent){
    QString result;
    QString format;

    if(pfield->extras().contains("color")){
    result += QString("<span style=\"background-color:%1\">").arg(pfield->extra("color").toString());
    }
    const bool custom_repr = pfield->extras().contains("repr");
    const QString offset = QString("%1").arg(REG.indexOf(pfield->first()),4,16,QChar('0'));


    switch(represent){
    case AS_CHANGED:
        format = "<b>%4:&nbsp;&nbsp;%1[%3]=<font color=red>%2</font></b>";
        break;

    case AS_UNCHANGED:
        format = "<b>%4:&nbsp;&nbsp;%1[%3]=<font color=blue>%2</font></b>";
        break;

    case AS_IGNORESTYLE:
        format = "<b>%4:&nbsp;&nbsp;%1[%3]=%2</b>";
        break;

    }


    if(custom_repr){
        Register tmp(0,"tmp",true);
        tmp.addField(pfield);
        result += QString("<b>%2:&nbsp;&nbsp;%1</b>")
                .arg(tmp.toString(pfield->extra("repr").toString()))
                .arg(offset);
    }

    //auto
    else{
        if(pfield->size()==1){
            result += QString(format)
                    .arg(pfield->name())
                    .arg(pfield->value())
                    .arg(pfield->size())
                    .arg(offset);

        }
        else if(pfield->size()<=8){
            result = QString(format)
                    .arg(pfield->name())
                    .arg(pfield->toUInt(),0,16)
                    .arg(pfield->size())
                    .arg(offset);

        }
        else if(pfield->size()<=16){
            result = QString(format)
                    .arg(pfield->name())
                    .arg(pfield->toUInt(),4,16,QChar('0'))
                    .arg(pfield->size())
                    .arg(offset);

        }
        else if(pfield->size()<=32){
            result = QString(format)
                    .arg(pfield->name())
                    .arg(pfield->toUInt(),8,16,QChar('0'))
                    .arg(pfield->size())
                    .arg(offset);

        }else{
            QString hex_part = pfield->toHex();

            if(ui->cbTrim->isChecked() &&hex_part.size()>10){
                hex_part = hex_part.mid(0,10)+"...";
            }
            result = QString(format)
                    .arg(pfield->name())
                    .arg(hex_part)
                    .arg(pfield->size())
                    .arg(offset);

        }
    }
    if(pfield->extras().contains("color")){
        result += "</span>";
    }

    if(ui->cbDescr->isChecked() && !pfield->description().isEmpty()){
        result += QString("<br><font color=grey>%1</font>").arg(pfield->description());
    }

    return result;
}
void MainWindow::on_pbFill1_clicked()
{
    REG.fill(1);
    update_content();
}

void MainWindow::update_content()
{    
    QElapsedTimer t;
    t.start();
    analyzeChanges();
    const int bak_scroll_bar = ui->teResult->verticalScrollBar()->value();
    ui->teResult->clear();
    Register REG2 = REG;
    REG2.fill(1);

    foreach(const QString &item, REG.fieldsList()){
        if(REG.field(item)->toByteArray() == REG2.field(item)->toByteArray()){
            ui->teResult->appendHtml(itemToString(REG.field(item),AS_UNCHANGED));
        }
        else{
            ui->teResult->appendHtml(itemToString(REG.field(item),AS_CHANGED));
        }
    }
    ui->teResult->verticalScrollBar()->setValue(bak_scroll_bar);
}


void MainWindow::on_pbSaveToFile_clicked()
{
    QFile f;


    if(ui->cmFormat->currentIndex()==0){
        f.setFileName(QFileDialog::getSaveFileName(0,"",m_dataFilePath,"Text (*.txt *.hex)",0));
        if(!f.fileName().isEmpty() ){
            m_dataFilePath = QFileInfo(f).filePath();
            const QByteArray data_to_save = REG.toByteArray(Register::LSB);
            if(f.open(QIODevice::WriteOnly)){
                qint32 i=0;
                while(i<(quint32)REG.size()/32){

                    const qint32 addr_from =(qint32)i*32;
                    const qint32 addr_to =(qint32)(i+1)*32 -1;
                    const uint value = REG.sub(addr_from,addr_to)->toUInt();
                    if(m_settings_ascii_windows){
                        f.write(QString("%1\r\n").arg(value,8,16,QChar('0')).toLatin1());
                    }
                    else f.write(QString("%1\n").arg(value,8,16,QChar('0')).toLatin1());
                    i++;

                    // trim lines in ascii file
                    if(ui->cmTrimLines->isEnabled() && i>=ui->cmTrimLines->currentText().toUInt())
                        break;
                    }
                }
            }
            else{
                QMessageBox::critical(0,"writing",QString("Can't open file"));
            }
    }
    // 8 bit
    else if(ui->cmFormat->currentIndex()==1){

    }
    // binary
    else {
        f.setFileName(QFileDialog::getSaveFileName(0,"",m_dataFilePath,"Binary (*.bin);;Sb3 files(*.sb3);;Other binary(*.*)",0));
        if(!f.fileName().isEmpty()){
            m_dataFilePath = QFileInfo(f).filePath();
            if(f.open(QIODevice::WriteOnly)){
                QByteArray data = REG.toByteArray(Register::LSB);
                f.write(data);
            }
        }
    }
    f.close();
}

typedef struct {
    QString filename;
    QString     data;
}HexFile;
QList<HexFile> m_hex_files;

QString MainWindow::parseRegChangedParams(){
    QString result;
    QStringList items = REG.fieldsList();
    Register REG2 = REG;
    REG2.fill(1);
    foreach(const QString &item,items){
        if(REG.sub(item)->toByteArray() != REG2.sub(item)->toByteArray()){
            result += itemToString(REG.field(item),AS_IGNORESTYLE)+"\n";
        }
    }
    return result;
}


void MainWindow::on_cbTrim_toggled(bool checked)
{
    update_content();
}


void MainWindow::on_pbUpdate_clicked()
{    
    update_content();
}


void MainWindow::on_cmStructure_currentIndexChanged(int index)
{

}

void MainWindow::on_cbDescr_toggled(bool checked)
{
    update_content();
}


void MainWindow::on_cmItems_activated(int index)
{
    if(REG.contains(ui->cmItems->itemText(index))){
        BitField *f = REG.field(ui->cmItems->itemText(index));
        ui->leValue->setValue(f->value());
        ui->lbFieldInfo->setText(QString("%1 bits").arg(f->size()));
    }
}


void MainWindow::on_cmProcessAll_clicked()
{
    QDir directory("../ifr0");
    const QStringList txt_files = directory.entryList(QStringList() << "*.txt",QDir::Files);
    foreach(const QString &csv_filename, txt_files) {
        //do whatever you need to do

        if(loadData(QString("../ifr0/%2").arg(csv_filename),0)){
            HexFile hex_file;
            hex_file.filename = csv_filename;
            hex_file.data = parseRegChangedParams();
            m_hex_files.append(hex_file);
        }
    }

    QFile f("output.csv");
    if(f.open(QIODevice::WriteOnly)){
        foreach(const HexFile &hex, m_hex_files){
            const QString line = QString("\"%1\",\"%2\"\n").arg(hex.filename).arg(hex.data);
            f.write(line.toLatin1());
        }
    }
    f.close();
}
//[
//{"name":"imageType[7:0]"},
//{"name":"reserved[12:8]"},
//{"name":"tzm_preset[1]"},
//{"name":"tzm_image_type[1]"},
//{"name":"reserved[1]"},
//{"name":"reserved[31:16]"}
//]

void MainWindow::on_pbStoreFormat_clicked()
{
    QSettings settings(QString("%2/%1.ini").arg(qApp->applicationName()).arg(qApp->applicationDirPath()));
    bool ok;
    const QString format_name = QInputDialog::getText(0, "Input dialog",
                                                "Format name", QLineEdit::Normal,
                                                "", &ok);
    ui->cmStructure->addItem(QString("-%1").arg(format_name),ui->teRegister->toPlainText());
    settings.setValue(QString("Formats/%2").arg(format_name),ui->teRegister->toPlainText());
}


void MainWindow::on_pbRemoveFormat_clicked()
{
    QSettings settings(QString("%2/%1.ini").arg(qApp->applicationName()).arg(qApp->applicationDirPath()));
    settings.remove(QString("Formats/%2").arg(ui->cmStructure->currentText().mid(1)));
    loadRecentFiles();
    ui->cmStructure->setCurrentIndex(0);
}


void MainWindow::on_cmFormat_currentIndexChanged(int index)
{
    ui->cmTrimLines->setEnabled(1);
    if(index>1){
        ui->cmTrimLines->setEnabled(0);
    }
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    saveRecentFiles();
    saveSettings();
    ev->accept();
}


void MainWindow::on_cbUseWindowsCRLF_toggled(bool checked)
{
    m_settings_ascii_windows = checked;
    saveSettings();
}

