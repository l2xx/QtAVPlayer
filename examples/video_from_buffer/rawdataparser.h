#ifndef RAWDATAPARSER_H
#define RAWDATAPARSER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QSharedPointer>
#include "file1.h"

class RawDataParser : public QObject
{
    Q_OBJECT
public:
    explicit RawDataParser(const QString &fileName, QObject *parent = nullptr);

    void setFile1(const QSharedPointer<File1> &file1);
public slots:
    bool start();
    bool stop();
private:
    void doParse();

private:
    QString m_fileName;
    QFile m_file;
    QSharedPointer<File1> m_file1;
};

#endif // RAWDATAPARSER_H
