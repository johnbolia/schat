/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2009 IMPOMEZIA <schat@impomezia.com>
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

#include <QtCore>

#include "daemonsettings.h"
#include "serviceinstaller.h"

/*!
 * Конструктор класса ServiceInstaller.
 */
ServiceInstaller::ServiceInstaller(QObject *parent)
  : QObject(parent),
  m_process(0),
  m_state(Ready)
{
}


/*!
 * Проверка на наличие сервера с именем \p name в реестре.
 *
 * \return \a true если сервис найден.
 */
bool ServiceInstaller::exists(const QString &name)
{
  QSettings s("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services", QSettings::NativeFormat);

  if (s.value(name + "/Start", -1).toInt() != -1 || !s.value(name + "/ImagePath", "").toString().isEmpty())
    return true;

  return false;
}


/*!
 * Проверка сервиса на правильность: наличие, корректные пути к \b srvany.exe и \b schatd.exe.
 */
bool ServiceInstaller::isValid(const QString &name)
{
  if (!exists(name))
    return false;

  QSettings s("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\" + name, QSettings::NativeFormat);

  QString imagePath = s.value("ImagePath", "").toString();
  if (imagePath.isEmpty() || QFileInfo(imagePath) != QFileInfo(QCoreApplication::applicationDirPath() + "/srvany.exe"))
    return false;

  QString application = s.value("Parameters/Application", "").toString();
  if (application.isEmpty() || QFileInfo(application) != QFileInfo(QCoreApplication::applicationDirPath() + "/schatd.exe"))
    return false;

  return true;
}


/*!
 * Запуск процедуры установки сервиса.
 */
void ServiceInstaller::install(const QString &name)
{
  if (m_state != Ready) {
    emit done(true);
    return;
  }

  if (name.isEmpty() || exists(name)) {
    emit done(true);
    return;
  }

  m_state = Installing;
  m_name  = name;
  createProcess();

  m_process->start('"' + QCoreApplication::applicationDirPath() + "/instsrv.exe\" \"" + name + "\" \""
      + QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/srvany.exe") + '"');
}


/*!
 * Запуск процедуры удаления сервиса.
 */
void ServiceInstaller::uninstall(const QString &name)
{
  if (m_state != Ready) {
    emit done(true);
    return;
  }

  if (!isValid(name)) {
    emit done(true);
    return;
  }

  m_state = Stopping;
  m_name  = name;
  createProcess();

  m_process->start("net stop \"" + name + '"');
}


/*!
 * Обработка ошибки при запуске/работе процесса.
 */
void ServiceInstaller::error()
{
  if (m_state == Stopping)
    startRemove();
  else
    emit done(true);
}


/*!
 * Завершение работы процесса.
 * В зависимости от состояния и успешности завершения выполняются различные задачи.
 * - Если происходила остановка сервиса, то запускается удаление.
 * - При успешной установке сервиса, в реестр дописывается параметр для запуска сервера.
 */
void ServiceInstaller::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (m_state == Stopping) {
    startRemove();
    return;
  }

  if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
    if (m_state == Installing) {
      QSettings s("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\" + m_name, QSettings::NativeFormat);
      s.setValue("Parameters/Application", QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/schatd.exe"));
      if (isValid(m_name)) {
        DaemonSettingsInstance->setBool("Service/Installed", true);
        DaemonSettingsInstance->setString("Service/Name", m_name);
      }
      else {
        m_state = Ready;
        emit done(true);
        return;
      }
    }
    else {
      DaemonSettingsInstance->setBool("Service/Installed", false);
    }
    m_state = Ready;
    emit done(false);
  }
  else
    emit done(true);
}


/*!
 * Создание в случае необходимости процесса для совершения действий.
 */
void ServiceInstaller::createProcess()
{
  if (!m_process) {
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(error()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(finished(int, QProcess::ExitStatus)));
  }
}


/*!
 * Запуск процесса удаления сервиса.
 */
void ServiceInstaller::startRemove()
{
  m_state = UnInstalling;
  m_process->start('"' + QCoreApplication::applicationDirPath() + "/instsrv.exe\" \"" + m_name + "\" remove");
}