/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2012 IMPOMEZIA <schat@impomezia.com>
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

#ifndef PLUGINSVIEW_H_
#define PLUGINSVIEW_H_

#include <QVariant>
#include <QWebView>

class PluginsView : public QWebView
{
  Q_OBJECT

public:
  PluginsView(QWidget *parent = 0);
  Q_INVOKABLE int state(const QString &id);
  Q_INVOKABLE QVariantList list() const;
  Q_INVOKABLE void enable(const QString &id, bool enable);
  Q_INVOKABLE void restart();

private slots:
  void boot();
  void populateJavaScriptWindowObject();

private:
  QString desc(const QVariantMap &data) const;
};

#endif /* PLUGINSVIEW_H_ */
