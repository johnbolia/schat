/* $Id$
 * Simple Chat
 * Copyright © 2008 IMPOMEZIA (http://impomezia.net.ru)
 */

#include <QtCore>

#include "updatexmlreader.h"
#include "version.h"


/** [public]
 * 
 */
UpdateXmlReader::UpdateXmlReader()
{
  m_coreLevel = -1;
  m_qtLevel = -1;
}


/** [public]
 * 
 */
bool UpdateXmlReader::isUpdateAvailable() const
{
  if (m_queue.isEmpty())
    return false;
  
  if (m_qtLevel > UpdateLevelQt)
    return true;
  
  if (m_coreLevel > UpdateLevelCore)
    return true;
  
  return false;
}


/** [public]
 * 
 */
bool UpdateXmlReader::readFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text))
    return false;
  
  setDevice(&file);

  while (!atEnd()) {
    readNext();

    if (isStartElement()) {
      if (name() == "updates" && attributes().value("version") == "1.0")
        readUpdates();
      else
        raiseError(QObject::tr("BAD FILE FORMAT OR VERSION"));
    }
  }

  return !error();
}


/** [private]
 * 
 */
void UpdateXmlReader::readCumulative()
{
  while (!atEnd()) {
    readNext();

    if (isEndElement())
      break;

    if (isStartElement()) {
      if (name() == "file") {
        if ((attributes().value("type") == "qt") && (attributes().value("level").toString().toInt() == m_qtLevel))
          m_queue.enqueue(m_path + "/win32/" + readElementText());
        else if ((attributes().value("type") == "core") && (attributes().value("level").toString().toInt() == m_coreLevel))
          m_queue.enqueue(m_path + "/win32/" + readElementText());
      }
      else
        readUnknownElement();
    }
  } 
}


/** [private]
 * 
 */
void UpdateXmlReader::readFiles()
{
  while (!atEnd()) {
    readNext();

    if (isEndElement())
      break;

    if (isStartElement()) {
      if (name() == "cumulative")
        readCumulative();
      else
        readUnknownElement();
    }
  }
}


/** [private]
 * 
 */
void UpdateXmlReader::readMeta()
{
  while (!atEnd()) {
    readNext();

    if (isEndElement())
      break;

    if (isStartElement()) {
      if (name() == "core") {
        m_coreLevel = attributes().value("level").toString().toInt();
        m_core = readElementText();
      }
      else if (name() == "qt") {
        m_qtLevel = attributes().value("level").toString().toInt();
        m_qt = readElementText();
      }
      else
        readUnknownElement();
    }
  }
}


/** [private]
 * 
 */
void UpdateXmlReader::readUnknownElement()
{
  while (!atEnd()) {
    readNext();

    if (isEndElement())
      break;

    if (isStartElement())
      readUnknownElement();
  }
}


/** [private]
 * 
 */
void UpdateXmlReader::readUpdates()
{
  while (!atEnd()) {
    readNext();

    if (isEndElement())
      break;

    if (isStartElement()) {
      if (name() == "meta")
        readMeta();
      else if (name() == "files" && attributes().value("platform") == "win32") 
        readFiles();
      else
        readUnknownElement();
    }
  }  
}
