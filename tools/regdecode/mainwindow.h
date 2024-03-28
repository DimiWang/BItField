#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "register.h"


typedef enum {
    AS_CHANGED,
    AS_UNCHANGED,
    AS_IGNORESTYLE
}Represent;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void analyzeChanges();
    void loadSettings();
    void saveSettings();
    void loadRecentFiles();
    void saveRecentFiles();

private slots:
    void on_pbApply_clicked();

    void on_pbLoadFile_clicked();

    void on_pbSetValue_clicked();

    void on_pbFill1_clicked();

    void update_content();

    void on_pbSaveToFile_clicked();

    void on_cbTrim_toggled(bool checked);

    void on_pbUpdate_clicked();

    void on_cmStructure_currentIndexChanged(int index);

    void on_cmStructure_activated(int index);

    void on_cbDescr_toggled(bool checked);

    void on_cmItems_activated(int index);

    void on_cmProcessAll_clicked();

    void on_pbStoreFormat_clicked();

    void on_pbRemoveFormat_clicked();

    void on_cmFormat_currentIndexChanged(int index);
    void on_cbUseWindowsCRLF_toggled(bool checked);

protected:
    void closeEvent(QCloseEvent *ev);

private:
    bool m_settings_ascii_windows;

    QString m_dataFilePath;
    QString m_jsonFilePath;
    Ui::MainWindow *ui;
    QString itemToString(BitField *f, Represent represent);
    QString parseRegChangedParams();
};
#endif // MAINWINDOW_H
