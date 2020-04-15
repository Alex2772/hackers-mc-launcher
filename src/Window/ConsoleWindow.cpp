#include "ConsoleWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QIcon>
#include <QProcess>

void ConsoleWindow::onClose()
{
	close();
	deleteLater();
}

ConsoleWindow::ConsoleWindow(QProcess* process)
{
	setGeometry(10, 40,1000, 500);
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
	connect(process, &QProcess::readyReadStandardOutput, this, [&, text, process]()
	{
		auto sout = QString::fromLocal8Bit(process->readAllStandardOutput());
		sout.remove('\r');
		if (!sout.isEmpty()) {			
			sout.replace("\n", "<br />");
			sout.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

			bool first = true;
			for (auto s : sout.split("\x1b[")) {
				if (first)
				{
					first = false;
				} else
				{
					if (!s.isEmpty()) {
						switch (s.at(0).toLatin1())
						{
						case 'm':
							mCurrentColor = -1;
							s = s.mid(1);
							break;
						case '0':
							mCurrentColor = -1;
							s = s.mid(2);
							break;
						case '3':
							mCurrentColor = s.at(1).toLatin1() - '0';
							s = s.mid(3);
						}
						
					}
				}
				if (s.isEmpty())
					continue;
				
				if (mCurrentColor < 0) {
					if (sout.endsWith("<br />"))
					{
						// will be appended automatically
						sout = sout.mid(0, sout.length() - 6);
					}
					text->appendHtml(sout);
				} else
				{
					QString colorHex;
					for (int i = 0; i < 3; ++i)
					{
						colorHex += (mCurrentColor & (1 << (i))) ? "AA" : "00";
					}
					if (s.endsWith("<br />"))
					{
						// will be appended automatically
						s = s.mid(0, s.length() - 6);
					}
					text->appendHtml("<span style='color:#" + colorHex + ";'>" + s + "</span>");
				}
			}
		}
		auto serr = QString::fromLocal8Bit(process->readAllStandardError());
		serr.replace("\n", "<br />");
		
		if (!serr.isEmpty()) {
			text->appendHtml("<span style='color: #c00;'>" + serr + "</span>");
		}
	});
		
	show();
}
