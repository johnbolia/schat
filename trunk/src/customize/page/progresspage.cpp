/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008 IMPOMEZIA <schat@impomezia.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui>

#include "progresspage.h"
#include "wizardsettings.h"

/*!
 * \brief Конструктор класса ProgressPage.
 */
ProgressPage::ProgressPage(QWidget *parent)
  : QWizardPage(parent), m_process(0)
{
  m_settings = settings;
  setTitle(tr("Идёт создание дистрибутива"));
  setSubTitle(tr("Подождите идёт создание дистрибутива"));

  m_label = new QLabel(this);
  m_progress = new QProgressBar(this);
  m_log = new QTextEdit(this);
  m_log->setReadOnly(true);

  QVBoxLayout *mainLay = new QVBoxLayout(this);
  mainLay->addWidget(m_label);
  mainLay->addWidget(m_progress);
  mainLay->addWidget(m_log);
  mainLay->setMargin(0);
}


/*!
 * Инициализация страницы.
 * Создаёт очередь заданий и по нулевому таймеру запускает следующие задание.
 */
void ProgressPage::initializePage()
{
  qDebug() << "ProgressPage::initializePage()";

  m_mirror       = m_settings->getBool("Mirror");
  m_mirrorCore   = m_settings->getBool("MirrorCore");
  m_mirrorQt     = m_settings->getBool("MirrorQt");
  m_suffix       = m_settings->getString("Suffix");
  m_version      = m_settings->getString("Version");
  m_makensisFile = m_settings->getString("MakensisFile");
  if (!m_suffix.isEmpty())
    m_suffix = "-" + m_suffix;

  m_queue.enqueue(CreateNSI);
  m_queue.enqueue(CreateEXE);

  QTimer::singleShot(0, this, SLOT(nextJob()));
}


/*!
 * Выполняет следующее задание из очереди \a m_queue.
 */
void ProgressPage::nextJob()
{
  qDebug() << "ProgressPage::nextJob()";

  if (m_queue.isEmpty())
    return;

  Jobs job = m_queue.dequeue();

  if (job == CreateNSI) {
    m_settings->write();

    if (!createNsi())
      return;

    QTimer::singleShot(0, this, SLOT(nextJob()));
  }
  else if (job == CreateEXE) {
    if (!createExe())
      return;
  }
}


/*!
 * Уведомление об ошибке запуска процесса.
 */
void ProgressPage::processError()
{
  m_log->append(tr("<span style='color:#900;'>Произошла ошибка при запуске <b>%1</b> [%2]</span>")
      .arg(m_makensisFile)
      .arg(m_process->errorString()));
}


/*!
 * Завершение процесса компиляции, в случае успешного завершения если
 * очередь \a m_nsi пуста, запускается выполнение следующей задачи, если
 * не пуста, то запускается следующая компиляция.
 */
void ProgressPage::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::CrashExit)
    m_log->append(tr("<span style='color:#900;'>Произошёл сбой при запуске <b>%1</b> [%2]</span>")
          .arg(m_makensisFile)
          .arg(m_process->errorString()));
  else if (exitCode != 0)
    m_log->append(tr("<span style='color:#900;'>Произошла ошибка при выполнении <b>%1</b></span>")
          .arg(m_makensisFile));
  else {
    m_log->append(tr("Файл <b>%1</b> создан").arg(m_currentExe));
    if (m_nsi.isEmpty())
      QTimer::singleShot(0, this, SLOT(nextJob()));
    else
      compile();
  }
}


/*!
 * Отображение текста ошибки при выполнении компиляции.
 */
void ProgressPage::processStandardOutput()
{
  m_log->append(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
}


/*!
 * Запускает создание EXE файлов.
 *
 * \return Возвращает \a true, если запуск первого процесса в очереди был успешно
 * инициирован, \a false если произошла ошибка на раннем этапе.
 */
bool ProgressPage::createExe()
{
  qDebug() << "ProgressPage::createExe()";

  if (m_makensisFile.isEmpty()) {
    m_log->append(tr("<span style='color:#900;'>Не удалось определить путь к <b>makensis.exe</b></span>"));
    return false;
  }

  if (!QFile::exists(m_makensisFile)) {
    m_log->append(tr("<span style='color:#900;'>Не удалось найти файл <b>%1</b></span>").arg(m_makensisFile));
    return false;
  }

  if (!m_process) {
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(QApplication::applicationDirPath() + "/custom");
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(processStandardOutput()));
  }

  if (!m_nsi.isEmpty())
    compile();
  else
    return false;

  return true;
}


/*!
 * Запускает создание NSI файлов.
 *
 * \return Возвращает \a true если все файлы успешно созданы, иначе \a false.
 */
bool ProgressPage::createNsi()
{
  m_label->setText(tr("Создание NSI файлов..."));

  if (!createNsi(Main))
    return false;

  if (m_mirror && m_mirrorCore) {
    if (!createNsi(Core))
      return false;

    if (m_mirrorQt)
      if (!createNsi(Runtime))
        return false;
  }

  return true;
}


/*!
 * Создаёт NSI файл с заданными настройками.
 * Папка \b custom должна существовать.
 *
 * \param type Тип файла, который нужно создать.
 * \return     Возвращает \a true в случае успешного создания файла, иначе \a false.
 */
bool ProgressPage::createNsi(Nsi type)
{
  qDebug() << "ProgressPage::createNsi()";

  QString fileName;
  if (type == Main)
    fileName = "setup.nsi";
  else if (type == Core)
    fileName = "setup-core.nsi";
  else if (type == Runtime)
    fileName = "setup-runtime.nsi";

  QFile file(QApplication::applicationDirPath() + "/custom/" + fileName);

  if (file.open(QIODevice::WriteOnly)) {
    m_nsi.enqueue(type);

    QTextStream stream(&file);
    stream.setCodec("CP-1251");

    stream << "# Generated by " << QApplication::applicationName() << " Customize " << QApplication::applicationVersion() << endl << endl;

    if (type == Main || type == Core) {
      stream << "!define Core" << endl
             << "!define Emoticons.Kolobok" << endl
             << "!define Emoticons.Simple" << endl
             << "!define Daemon" << endl;
    }

    if (type == Main || type == Runtime)
      stream << "!define Qt" << endl;

    if (type == Core)
      stream << "!define SCHAT_PREFIX \"core-\"" << endl;
    else if (type == Runtime)
      stream << "!define SCHAT_PREFIX \"runtime-\"" << endl;

    if (!m_suffix.isEmpty())
      stream << "!define SCHAT_SUFFIX \"" << m_suffix << '"' << endl;

    stream << "!define SCHAT_VERSION " << m_version << endl
           << "!define SCHAT_QTDIR \"..\"" << endl
           << "!define SCHAT_QT_BINDIR \"..\"" << endl
           << "!define SCHAT_BINDIR \"..\"" << endl
           << "!define SCHAT_DATADIR \"..\"" << endl
           << "!define VC90_REDIST_DIR \"..\"" << endl << endl
           << "!include \"engine\\core.nsh\"" << endl;

    m_log->append(tr("Файл <b>%1</b> создан").arg(fileName));
    return true;
  }
  else {
    m_log->append(tr("<span style='color:#900;'>Ошибка создания файла <b>%1</b> [%2]</span>").arg(fileName).arg(file.errorString()));
    return false;
  }
}


/*!
 * Запуск компилятора NSIS для создания exe файла(ов).
 */
void ProgressPage::compile()
{
  qDebug() << "ProgressPage::compile()";

  QStringList args;
  args << "/V1";
  Nsi nsi = m_nsi.dequeue();

  if (nsi == Main) {
    args << "setup.nsi";
    m_currentExe = "schat-";
  }
  else if (nsi == Core) {
    args << "setup-core.nsi";
    m_currentExe = "schat-core-";
  }
  else if (nsi == Runtime) {
    args << "setup-runtime.nsi";
    m_currentExe = "schat-runtime-";
  }
  m_currentExe += (m_version + m_suffix + ".exe");

  m_label->setText(tr("Создание %1...").arg(m_currentExe));
  m_process->start('"' + m_makensisFile + '"', args);
}
