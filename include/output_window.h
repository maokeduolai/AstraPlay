#ifndef OUTPUT_WINDOW_H
#define OUTPUT_WINDOW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QProcess>

class OutputWindow : public QWidget {
Q_OBJECT

public:
    explicit OutputWindow(QWidget *parent = nullptr);

    void startProcess(const QString &program, const QStringList &arguments);

signals:

    void errorOccurred(QProcess::ProcessError error);

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void errorMessageEmit(const QString &error);

private slots:

    void onReadyReadStandardOutput();

    void onReadyReadStandardError();

private:
    QPlainTextEdit *textEdit;

    QProcess *process;
};

#endif //OUTPUT_WINDOW_H
