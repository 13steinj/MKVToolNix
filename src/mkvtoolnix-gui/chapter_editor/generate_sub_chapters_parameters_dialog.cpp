#include "common/common_pch.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

#include "common/qt.h"
#include "common/strings/formatting.h"
#include "common/strings/parsing.h"
#include "mkvtoolnix-gui/forms/chapter_editor/generate_sub_chapters_parameters_dialog.h"
#include "mkvtoolnix-gui/chapter_editor/generate_sub_chapters_parameters_dialog.h"
#include "mkvtoolnix-gui/util/settings.h"
#include "mkvtoolnix-gui/util/util.h"

namespace mtx { namespace gui { namespace ChapterEditor {

using namespace mtx::gui;

GenerateSubChaptersParametersDialog::GenerateSubChaptersParametersDialog(QWidget *parent,
                                                                         int firstChapterNumber,
                                                                         uint64_t startTimecode)
  : QDialog{parent}
  , m_ui{new Ui::GenerateSubChaptersParametersDialog}
{
  setupUi(firstChapterNumber, startTimecode);
  retranslateUi();
}

GenerateSubChaptersParametersDialog::~GenerateSubChaptersParametersDialog() {
}

void
GenerateSubChaptersParametersDialog::setupUi(int firstChapterNumber,
                                             uint64_t startTimecode) {
  auto &cfg = Util::Settings::get();

  m_ui->setupUi(this);

  m_ui->sbFirstChapterNumber->setValue(firstChapterNumber);
  m_ui->leStartTimecode->setText(Q(format_timecode(startTimecode)));
  m_ui->leNameTemplate->setText(cfg.m_chapterNameTemplate);

  m_ui->cbLanguage->setup().setCurrentByData(cfg.m_defaultChapterLanguage);
  m_ui->cbCountry->setup(true, QY("– set to none –")).setCurrentByData(cfg.m_defaultChapterCountry);

  m_ui->sbNumberOfEntries->setFocus();

  connect(m_ui->leStartTimecode, &QLineEdit::textChanged, this, &GenerateSubChaptersParametersDialog::verifyStartTimecode);
}

void
GenerateSubChaptersParametersDialog::retranslateUi() {
  Util::setToolTip(m_ui->leStartTimecode, QY("The format is either the form 'HH:MM:SS.nnnnnnnnn' or a number followed by one of the units 's', 'ms' or 'us'."));
  Util::setToolTip(m_ui->leNameTemplate,
                   Q("%1 %2")
                   .arg(QY("This template will be used for new chapter entries."))
                   .arg(QY("The string '<NUM>' will be replaced by the chapter number.")));
}

int
GenerateSubChaptersParametersDialog::numberOfEntries()
  const {
  return m_ui->sbNumberOfEntries->value();
}

uint64_t
GenerateSubChaptersParametersDialog::durationInNs()
  const {
  return static_cast<uint64_t>(m_ui->sbDuration->value()) * 1000000000ull;
}

int
GenerateSubChaptersParametersDialog::firstChapterNumber()
  const {
  return m_ui->sbFirstChapterNumber->value();
}

uint64_t
GenerateSubChaptersParametersDialog::startTimecode()
  const {
  int64_t timecode = 0;
  parse_timecode(to_utf8(m_ui->leStartTimecode->text()), timecode);

  return timecode;
}

QString
GenerateSubChaptersParametersDialog::nameTemplate()
  const {
  return m_ui->leNameTemplate->text();
}

QString
GenerateSubChaptersParametersDialog::language()
  const {
  return m_ui->cbLanguage->currentData().toString();
}

OptQString
GenerateSubChaptersParametersDialog::country()
  const {
  auto countryStr = m_ui->cbCountry->currentData().toString();
  return countryStr.isEmpty() ? OptQString{} : OptQString{ countryStr };
}

void
GenerateSubChaptersParametersDialog::verifyStartTimecode() {
  int64_t dummy = 0;
  Util::buttonForRole(m_ui->buttonBox)->setEnabled(parse_timecode(to_utf8(m_ui->leStartTimecode->text()), dummy));
}

}}}
