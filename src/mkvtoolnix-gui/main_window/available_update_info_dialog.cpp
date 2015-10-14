#include "common/common_pch.h"

#if defined(HAVE_CURL_EASY_H)

#include <QDesktopServices>

#include "common/qt.h"
#include "mkvtoolnix-gui/forms/main_window/available_update_info_dialog.h"
#include "mkvtoolnix-gui/main_window/available_update_info_dialog.h"

namespace mtx { namespace gui {

AvailableUpdateInfoDialog::AvailableUpdateInfoDialog(QWidget *parent)
  : QDialog{parent, Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint}
  , ui{new Ui::AvailableUpdateInfoDialog}
{
  // Setup UI controls.
  ui->setupUi(this);

  setWindowTitle(QY("Online check for updates"));
  setChangeLogContent(Q(""));
  ui->status->setText(QY("Downloading release information"));

  auto thread = new UpdateCheckThread(parent);

  connect(thread, &UpdateCheckThread::checkFinished,               this, &AvailableUpdateInfoDialog::updateCheckFinished);
  connect(thread, &UpdateCheckThread::releaseInformationRetrieved, this, &AvailableUpdateInfoDialog::setReleaseInformation);

  thread->start();
}

AvailableUpdateInfoDialog::~AvailableUpdateInfoDialog() {
}

void
AvailableUpdateInfoDialog::setReleaseInformation(std::shared_ptr<pugi::xml_document> releasesInfo) {
  m_releasesInfo = releasesInfo;
}

void
AvailableUpdateInfoDialog::setChangeLogContent(QString const &content) {
  auto html = QStringList{};
  html << Q("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
            "<html><head><meta name=\"qrichtext\" content=\"1\" />"
            "<style type=\"text/css\">"
            "p, li { white-space: pre-wrap; }\n"
            "</style>"
            "</head><body>");

  html << Q("<h1><a href=\"%1\">MKVToolNix ChangeLog</a></h1>").arg(to_qs(MTX_CHANGELOG_URL).toHtmlEscaped());

  html << content;

  html << Q("</body></html>");

  ui->changeLog->setHtml(html.join(Q("")));
}

void
AvailableUpdateInfoDialog::updateCheckFinished(UpdateCheckStatus status,
                                               mtx_release_version_t releaseVersion) {
  auto statusStr = UpdateCheckStatus::NoNewReleaseAvailable == status ? QY("Currently no newer version is available online.")
                 : UpdateCheckStatus::NewReleaseAvailable   == status ? QY("There is a new version available online.")
                 :                                                      QY("There was an error querying the update status.");
  ui->status->setText(statusStr);

  if (UpdateCheckStatus::Failed == status)
    return;

  ui->currentVersion->setText(to_qs(releaseVersion.current_version.to_string()));
  ui->availableVersion->setText(to_qs(releaseVersion.latest_source.to_string()));
  ui->close->setText(QY("&Close"));

  auto url = releaseVersion.urls.find("general");
  if ((url != releaseVersion.urls.end()) && !url->second.empty()) {
    m_downloadURL = to_qs(url->second);
    ui->downloadURL->setText(Q("<html><body><a href=\"%1\">%1</a></body></html>").arg(m_downloadURL.toHtmlEscaped()));
    ui->download->setEnabled(true);
  }

  if (!m_releasesInfo)
    return;

  auto html              = QStringList{};
  auto numReleasesOutput = 0;
  auto releases          = m_releasesInfo->select_nodes("/mkvtoolnix-releases/release[not(@version='HEAD')]");
  auto re_released       = boost::regex{"^released\\s+v?[\\d\\.]+", boost::regex::perl | boost::regex::icase};
  auto re_bug            = boost::regex{"(#\\d+)", boost::regex::perl | boost::regex::icase};
  auto bug_formatter     = [](boost::smatch const &matches) -> std::string {
    auto number_str = matches[1].str().substr(1);
    return (boost::format("<a href=\"https://github.com/mbunkus/mkvtoolnix/issues/%1%\">#%1%</a>") % number_str).str();
  };

  releases.sort();

  for (auto &release : releases) {
    auto version_str   = std::string{release.node().attribute("version").value()};
    auto version_str_q = to_qs(version_str).toHtmlEscaped();
    auto codename_q    = to_qs(release.node().attribute("codename").value()).toHtmlEscaped();
    auto heading       = !codename_q.isEmpty() ? QY("Version %1 \"%2\"").arg(version_str_q).arg(codename_q) : QY("Version %1").arg(version_str_q);

    html << Q("<h2>%1</h2>").arg(heading)
         << Q("<p><ul>");

    for (auto change = release.node().child("changes").first_child() ; change ; change = change.next_sibling()) {
      if (   (std::string{change.name()} != "change")
          || boost::regex_search(change.child_value(), re_released))
        continue;

      auto text = boost::regex_replace(to_utf8(to_qs(change.child_value()).toHtmlEscaped()), re_bug, bug_formatter);
      html     << Q("<li>%1</li>").arg(to_qs(text));
    }

    html << Q("</ul></p>");

    numReleasesOutput++;
    if ((10 < numReleasesOutput) && (version_number_t{version_str} < releaseVersion.current_version))
      break;
  }

  html << Q("<p><a href=\"%1\">%2</a></h1>").arg(MTX_CHANGELOG_URL).arg(QY("Read full ChangeLog online"));

  setChangeLogContent(html.join(Q("\n")));
}

void
AvailableUpdateInfoDialog::visitDownloadLocation() {
  QDesktopServices::openUrl(m_downloadURL);
}

}}

#endif  // HAVE_CURL_EASY_H
