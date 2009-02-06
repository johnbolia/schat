/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2009 IMPOMEZIA <schat@impomezia.com>
 *
 * This file based on "kopetechatwindowstyle.cpp" - A Chat Window Style.
 * Copyright © 2005      by Michaël Larouche      <larouche@kde.org>
 * Kopete    © 2002-2008 by the Kopete developers <kopete-devel@kde.org>
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

#include "chatwindowstyle.h"

#include <QtGui>

class ChatWindowStyle::Private
{
public:
  bool defaultStyle;
  QString baseHref;
  QString currentVariantPath;
  QString styleName;
  StyleVariants variantsList;

  QString templateHtml;
  QString headerHtml;
  QString footerHtml;
  QString incomingHtml;
  QString nextIncomingHtml;
  QString outgoingHtml;
  QString nextOutgoingHtml;
  QString statusHtml;
  QString actionIncomingHtml;
  QString actionOutgoingHtml;
  QString fileTransferIncomingHtml;
  QString outgoingStateSendingHtml;
  QString outgoingStateErrorHtml;
  QString outgoingStateSentHtml;
  QString outgoingStateUnknownHtml;
//  QString mainCSS;

  QHash<QString, bool> compactVariants;
};


/*!
 * \brief Build a single chat window style.
 *
 */
ChatWindowStyle::ChatWindowStyle(const QString &styleName, StyleBuildMode styleBuildMode)
  : d(new Private)
{
  init(styleName, styleBuildMode);
}


ChatWindowStyle::ChatWindowStyle(const QString &styleName, const QString &variantPath, StyleBuildMode styleBuildMode)
  : d(new Private)
{
  d->currentVariantPath = variantPath;
  init(styleName, styleBuildMode);
}


ChatWindowStyle::~ChatWindowStyle()
{
  delete d;
}


/*!
 * Check if the style has the support for Kopete Action template (Kopete extension).
 *
 * \return true if the style has Action template.
 */
bool ChatWindowStyle::hasActionTemplate() const
{
  return (!d->actionIncomingHtml.isEmpty() && !d->actionOutgoingHtml.isEmpty());
}


/*!
 * Check if the supplied variant has a compact form.
 */
bool ChatWindowStyle::hasCompact(const QString & styleVariant) const
{
  if (d->compactVariants.contains(styleVariant))
    return d->compactVariants.value(styleVariant);

  return false;
}


/*!
 * Return the compact version of the given style variant.
 * For the unmodified style, this returns "Variants/_compact_.css".
 */
QString ChatWindowStyle::compact( const QString & styleVariant ) const
{
  QString compacted = styleVariant;
  if ( styleVariant.isEmpty() ) {
    return QLatin1String( "Variants/_compact_.css" );
  } else {
    return compacted.insert( compacted.lastIndexOf('/') + 1, QString("_compact_") );
  }
}


/*!
 * Get the style resource directory.
 * Resources directory is the base where all CSS, HTML and images are located.
 *
 * Adium(and now Kopete too) style directories are disposed like this:
 * StyleName/
 *          Contents/
 *            Resources/
 *
 * \return the path to the resource directory.
 */
QString ChatWindowStyle::styleBaseHref() const
{
  if(d->defaultStyle)
    return "qrc" + d->baseHref;

  return QUrl::fromLocalFile(d->baseHref).toString();
}


/*!
 * Get the style path.
 * The style path points to the directory where the style is located.
 * ex: ~/.kde/share/apps/kopete/styles/StyleName/
 *
 * \return the style path based.
 */
QString ChatWindowStyle::styleName() const { return d->styleName; }


bool ChatWindowStyle::isValid(const QString &style)
{
  Q_UNUSED(style);

  return true;
}


ChatWindowStyle::StyleVariants ChatWindowStyle::variants(const QString &style)
{
  QString variantDirPath = style + "Contents/Resources/Variants/";
  QDir variantDir(variantDirPath);
  variantDir.setFilter(QDir::Files);
  variantDir.setSorting(QDir::Name);

  QStringList entryList = variantDir.entryList(QStringList("*.css"));
  StyleVariants out;

  foreach (QString variantName, entryList) {
    out.insert(QFileInfo(variantName).completeBaseName(), "Variants/" + variantName);
  }

  return out;
}


/*!
 * Get the list of all variants for this theme.
 * If the variant aren't listed, it call the lister
 * before returning the list of the Variants.
 * If the variant are listed, it just return the cached
 * variant list.
 * \return the StyleVariants QHash.
 */
ChatWindowStyle::StyleVariants ChatWindowStyle::variants()
{
  // If the variantList is empty, list available variants.
  if(d->variantsList.isEmpty())
    listVariants();

  return d->variantsList;
}


/*!
 * Reload style from disk.
 */
void ChatWindowStyle::reload()
{
  d->variantsList.clear();
  readStyleFiles();
  listVariants();
}


QString ChatWindowStyle::templateHtml() const             { return d->templateHtml; }
QString ChatWindowStyle::headerHtml() const               { return d->headerHtml; }
QString ChatWindowStyle::footerHtml() const               { return d->footerHtml; }
QString ChatWindowStyle::incomingHtml() const             { return d->incomingHtml; }
QString ChatWindowStyle::nextIncomingHtml() const         { return d->nextIncomingHtml; }
QString ChatWindowStyle::outgoingHtml() const             { return d->outgoingHtml; }
QString ChatWindowStyle::nextOutgoingHtml() const         { return d->nextOutgoingHtml; }
QString ChatWindowStyle::statusHtml() const               { return d->statusHtml; }
QString ChatWindowStyle::actionIncomingHtml() const       { return d->actionIncomingHtml; }
QString ChatWindowStyle::actionOutgoingHtml() const       { return d->actionOutgoingHtml; }
QString ChatWindowStyle::fileTransferIncomingHtml() const { return d->fileTransferIncomingHtml; }
QString ChatWindowStyle::outgoingStateSendingHtml() const { return d->outgoingStateSendingHtml; }
QString ChatWindowStyle::outgoingStateSentHtml() const    { return d->outgoingStateSentHtml; }
QString ChatWindowStyle::outgoingStateErrorHtml() const   { return d->outgoingStateErrorHtml; }
QString ChatWindowStyle::outgoingStateUnknownHtml() const { return d->outgoingStateUnknownHtml; }


bool ChatWindowStyle::readStyleFile(QString &out, const QString &fileName, bool failBack)
{
  static QString baseHref = d->baseHref;
  QString realFileName = baseHref + fileName;

  if (!QFile::exists(realFileName))
    if (failBack)
      realFileName = ":/webkit/Contents/Resources/" + fileName;
    else
      return false;

  QFile file(realFileName);

  if (!file.open(QIODevice::ReadOnly))
    return false;

  QTextStream stream(&file);
  stream.setCodec(QTextCodec::codecForName("UTF-8"));
  out = stream.readAll();
  file.close();

  return true;
}


/*!
 * Init this class.
 */
void ChatWindowStyle::init(const QString &styleName, StyleBuildMode styleBuildMode)
{
  QString basePath = QApplication::applicationDirPath() + "/styles/";

  bool valid = false;
  if (styleName != "Default") {
    if (isValid(basePath + styleName))
      valid = true;
  }

  if (!valid) {
    d->styleName = "Default";
    d->baseHref = ":/webkit/Contents/Resources/";
    d->defaultStyle = true;
  }
  else {
    d->styleName = styleName;
    d->baseHref = basePath + styleName + "/Contents/Resources/";
    d->defaultStyle = false;
  }

  readStyleFiles();
  if (styleBuildMode & StyleBuildNormal)
    listVariants();
}


/*!
 * List available variants for the current style.
 */
void ChatWindowStyle::listVariants()
{
  QString variantDirPath = d->baseHref + QString::fromUtf8("Variants/");
  QDir variantDir(variantDirPath);

  QStringList variantList = variantDir.entryList( QStringList("*.css") );
  QStringList::ConstIterator it, itEnd = variantList.constEnd();
  QLatin1String compactVersionPrefix("_compact_");
  for(it = variantList.constBegin(); it != itEnd; ++it)
  {
    QString variantName = *it, variantPath;
    // Retrieve only the file name.
    variantName = variantName.left(variantName.lastIndexOf("."));
    if ( variantName.startsWith( compactVersionPrefix ) ) {
      if ( variantName == compactVersionPrefix ) {
        d->compactVariants.insert( "", true );
      }
      continue;
    }
    QString compactVersionFilename = *it;
    QString compactVersionPath = variantDirPath + compactVersionFilename.prepend( compactVersionPrefix );
    if ( QFile::exists( compactVersionPath )) {
      d->compactVariants.insert( variantName, true );
    }
    // variantPath is relative to baseHref.
    variantPath = QString("Variants/%1").arg(*it);
//    qDebug() << variantName << variantPath;
    d->variantsList.insert(variantName, variantPath);
  }
}


/*!
 * Read style HTML files from disk.
 */
void ChatWindowStyle::readStyleFiles()
{
  readStyleFile(d->templateHtml,             "Template.html");
  readStyleFile(d->headerHtml,               "Header.html");
  readStyleFile(d->footerHtml,               "Footer.html");
  readStyleFile(d->incomingHtml,             "Incoming/Content.html");
  readStyleFile(d->nextIncomingHtml,         "Incoming/NextContent.html");
  readStyleFile(d->outgoingHtml,             "Outgoing/Content.html");
  readStyleFile(d->nextOutgoingHtml,         "Outgoing/NextContent.html");
  readStyleFile(d->statusHtml,               "Status.html");
  readStyleFile(d->actionIncomingHtml,       "Incoming/Action.html", false);
  readStyleFile(d->actionOutgoingHtml,       "Outgoing/Action.html", false);
  readStyleFile(d->fileTransferIncomingHtml, "Incoming/FileTransferRequest.html");
  readStyleFile(d->outgoingStateUnknownHtml, "Outgoing/StateUnknown.html");
  readStyleFile(d->outgoingStateSendingHtml, "Outgoing/StateSending.html");
  readStyleFile(d->outgoingStateSentHtml,    "Outgoing/StateSent.html");
  readStyleFile(d->outgoingStateErrorHtml,   "Outgoing/StateError.html");

  if ( d->fileTransferIncomingHtml.isEmpty() ||
       ( !d->fileTransferIncomingHtml.contains( "saveFileHandlerId" ) &&
         !d->fileTransferIncomingHtml.contains( "saveFileAsHandlerId" ) ) )
  {  // Create default html
    d->fileTransferIncomingHtml = d->incomingHtml;
    QString message = QString( "%message%\n"
                               "<div>\n"
                               " <div style=\"width:37px; float:left;\">\n"
                               "  <img src=\"%fileIconPath%\" style=\"width:32px; height:32px; vertical-align:middle;\" />\n"
                               " </div>\n"
                               " <div>\n"
                               "  <span><b>%fileName%</b> (%fileSize%)</span><br>\n"
                               "  <span>\n"
                               "   <input id=\"%saveFileAsHandlerId%\" type=\"button\" value=\"%1\">\n"
                               "   <input id=\"%cancelRequestHandlerId%\" type=\"button\" value=\"%2\">\n"
                               "  </span>\n"
                               " </div>\n"
                               "</div>" )
                               .arg( QObject::tr( "Download" ), QObject::tr( "Cancel" ) );
    d->fileTransferIncomingHtml.replace( QLatin1String("%message%"), message );
  }
}
