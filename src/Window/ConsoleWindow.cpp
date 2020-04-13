#include "ConsoleWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QIcon>
#include <QProcess>

void ConsoleWindow::onProcessDead()
{
	close();
	deleteLater();
}

ConsoleWindow::ConsoleWindow(QProcess* process)
{
	setGeometry(10, 40,800, 400);
	setWindowIcon(QIcon(":/hck/logo_console_256.png"));
	setWindowTitle(tr("Console"));
	
	setLayout(new QVBoxLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	auto text = new QPlainTextEdit;
	text->setReadOnly(true);
	QFont f(text->font().family(), 10, QFont::Black);
	f.setStyleHint(QFont::Monospace);
	text->setFont(f);
	
	text->setStyleSheet("QPlainTextEdit {background: #041404; color: #ded; border: none;}");
	text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	layout()->addWidget(text);

	connect(process, &QProcess::started, this, [text, process]() {
		text->appendHtml("<span style='color: #4f4'>" + process->program() + ' ' + process->arguments().join(' ') + "</span>");
	});
	connect(process, &QProcess::readyReadStandardOutput, this, [text, process]()
	{
		auto sout = QString::fromLocal8Bit(process->readAllStandardOutput());
		if (!sout.isEmpty())
			text->appendHtml(sout);
		auto serr = QString::fromLocal8Bit(process->readAllStandardError());
		
		if (!serr.isEmpty()) {
			text->appendHtml("<span style='color: #c00;'>" + serr + "</span>");
		}
	});
	
	show();
}
