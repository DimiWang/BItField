#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTextStream>
#include "register.h"
#include <QFile>



int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
     QCoreApplication::setApplicationName("bitfield");
     QCoreApplication::setApplicationVersion("1.0");

         QCommandLineParser parser;
         parser.setApplicationDescription("BitField parser");
         parser.addHelpOption();
         QCommandLineOption verboseOption(QStringList() << "i"<<"input" << "Enable verbose output");
         parser.addOption(verboseOption);         
         parser.addPositionalArgument("input_file", "Source file to copy.");
         parser.addPositionalArgument("output_file", "Output file");
         parser.addPositionalArgument("format", "Output format.","to_string, to_hex, to_bits, to_u32, to_u8");
         parser.process(app);
         const QStringList args = parser.positionalArguments();
         if(args.count()==3 && QFile::exists(args.at(0))){


             QFile output(args[2]);
             QFile input(args[0]);
             const QString format = args[1];
             if(input.open(QIODevice::ReadOnly) && output.open(QIODevice::WriteOnly))
             {

                 Register reg(0,"temporary");
                 if(reg.loadJsonData(input.readAll(),0) ==true){

                 QRegExp regexp("to_string\\((.*)\\)");
                 if(regexp.exactMatch(format)){
                     QTextStream(&output)<<QString(reg.toString(regexp.cap(1)))
                                           .replace("<cr>","\n");
                 }
                 else if( format == "to_string"){
                    QTextStream(&output)<<QString(reg.toString(regexp.cap(1)))
                                          .replace("<cr>","\n");
                 }
                 else if(format == "to_u32"){
                    for(int i=0;(reg.size()/32)+(reg.size()%32 != 0);i++)
                    {
                        quint32 value = reg.value(i*32,(i+1)*32-1);
                        QTextStream(&output)<<QString("%1").arg(value,8,16,QChar('\0')).toUpper().toLatin1();
                    }
                 }
                 else if(format == "to_u8"){
                    for(int i=0;(reg.size()/8)+(reg.size()%8!=0);i++)
                    {
                        quint32 value = reg.value(i*8,(i+1)*8-1);
                        QTextStream(&output)<<QString("%1").arg(value,2,16,QChar('\0')).toUpper().toLatin1();
                    }
                 }
                 else if( format == "to_hex"){
                     QTextStream(&output)<<reg.toHex().toUpper();
                 }
                 else if( format == "to_bits"){
                     QTextStream(&output)<<reg.toBitString();
                 }

                 output.close();
                 input.close();
                 }
             }
         }
    return app.exec();
}
