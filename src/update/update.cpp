/* $Id$
 * Simple Chat
 * Copyright © 2008 IMPOMEZIA (http://impomezia.net.ru)
 */

#include <QtCore>

#include "download.h"
#include "update.h"


/** [public]
 * 
 */
Update::Update(const QUrl &url, QObject *parent)
  : QObject(parent)
{
  m_url        = url;
  m_appPath    = qApp->applicationDirPath();
  m_targetPath = m_appPath + "/updates";
  m_download   = new Download(m_targetPath, this);
  m_urlPath    = QFileInfo(m_url.toString()).path();
  
  connect(m_download, SIGNAL(saved(const QString &)), SLOT(saved(const QString &)));
  connect(m_download, SIGNAL(error()), SLOT(error()));
}


/** [public slots]
 * 
 */
void Update::execute()
{
  if (!QDir().exists(m_targetPath))
    if (!QDir().mkdir(m_targetPath))
      qApp->exit(400); // не удалось создать папку для обновлений
  
  m_state = GettingUpdateXml;
  m_download->get(m_url);
}


/** [private slots]
 * 
 */
void Update::error()
{
  qApp->exit(32); // ошибка закачки
}


/** [private slots]
 * 
 */
void Update::saved(const QString &filename)
{
  if (m_state == GettingUpdateXml) {
    
    if (!m_reader.readFile(filename)) {
      qApp->exit(16); // не удалось прочитать update.xml
      return;
    }
    
    if (!m_reader.isUpdateAvailable()) {
      qApp->exit(8); // нет доступных обновлений
      return;
    }
    
    m_state = GettingUpdates;
    m_queue = m_reader.queue();
    
    foreach (FileInfo fileInfo, m_queue)
      m_files << fileInfo.name;
    
    FileInfo fileInfo = m_queue.dequeue();    
    m_download->get(QUrl(m_urlPath + "/win32/" + fileInfo.name));
  }
  else if (m_state == GettingUpdates) {
    if (m_queue.isEmpty()) {
      QString newName = m_appPath + "/schat-install.exe";
      
      if (QFile::exists(newName))
        QFile::remove(newName);
      
      if (!QFile::copy(m_appPath + "/schat-update.exe", newName)) {
        qApp->exit(4); // Ошибка создания копии
        return;
      }
      
      writeSettings();
      
//      QProcess::startDetached(m_appPath + "/schat-install.exe", QStringList() << "-install", m_appPath);
         
      qApp->exit(0); // обновления успешно скачаны

    }
    else {
      FileInfo fileInfo = m_queue.dequeue();    
      m_download->get(QUrl(m_urlPath + "/win32/" + fileInfo.name));
    }
  }
}


/** [private]
 * 
 */
void Update::writeSettings()
{
  QSettings s(m_appPath + "/schat.conf", QSettings::IniFormat, this);
  s.beginGroup("Updates");
  s.setValue("Files", m_files);  
}
