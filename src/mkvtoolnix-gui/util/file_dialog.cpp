#include "common/common_pch.h"

#include <QStandardPaths>
#include <QString>

#include "common/qt.h"
#include "mkvtoolnix-gui/util/file_dialog.h"

namespace mtx { namespace gui { namespace Util {

QString
dirPath(QDir const &dir) {
  return dirPath(dir.path());
}

QString
dirPath(QString const &dir) {
  auto path = dir;

  if (path.isEmpty() || (path == Q(".")))
    path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);

  if (path.isEmpty() || (path == Q(".")))
    path = QDir::currentPath();

  if (!QDir::toNativeSeparators(path).endsWith(QDir::separator()))
    path += Q("/");

  return QDir::fromNativeSeparators(path);
}

QString
sanitizeDirectory(QString const &directory,
                  bool withFileName) {
  auto dir     = to_utf8(directory.isEmpty() || (directory == Q(".")) ? QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) : directory);
  auto oldPath = bfs::absolute(bfs::path{dir});
  auto newPath = oldPath;
  auto ec      = boost::system::error_code{};

  while (   !(bfs::exists(newPath, ec) && bfs::is_directory(newPath, ec))
         && !newPath.parent_path().empty())
    newPath = newPath.parent_path();

  if (withFileName && (oldPath.filename() != "."))
    newPath /= oldPath.filename();

  // if (withFileName && (oldPath != newPath) && (oldPath.filename() != "."))
  //   newPath /= oldPath.filename();

  return Q(newPath.string());
}

QString
getOpenFileName(QWidget *parent,
                QString const &caption,
                QString const &dir,
                QString const &filter,
                QString *selectedFilter,
                QFileDialog::Options options) {
  return QFileDialog::getOpenFileName(parent, caption, sanitizeDirectory(dir, false), filter, selectedFilter, options & QFileDialog::DontUseCustomDirectoryIcons);
}

QStringList
getOpenFileNames(QWidget *parent,
                 QString const &caption,
                 QString const &dir,
                 QString const &filter,
                 QString *selectedFilter,
                 QFileDialog::Options options) {
  return QFileDialog::getOpenFileNames(parent, caption, sanitizeDirectory(dir, false), filter, selectedFilter, options & QFileDialog::DontUseCustomDirectoryIcons);
}

QString
getSaveFileName(QWidget *parent,
                QString const &caption,
                QString const &dir,
                QString const &filter,
                QString *selectedFilter,
                QFileDialog::Options options) {
  return QFileDialog::getSaveFileName(parent, caption, sanitizeDirectory(dir, true), filter, selectedFilter, options & QFileDialog::DontUseCustomDirectoryIcons);
}

QString
getExistingDirectory(QWidget *parent,
                     QString const &caption,
                     QString const &dir,
                     QFileDialog::Options options) {
  return QFileDialog::getExistingDirectory(parent, caption, sanitizeDirectory(dir, false), options & QFileDialog::DontUseCustomDirectoryIcons);
}

}}}
