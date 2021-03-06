#include "common/common_pch.h"

#include <QAbstractItemView>
#include <QComboBox>

#include "common/iso639.h"
#include "common/qt.h"
#include "mkvtoolnix-gui/header_editor/language_ietf_value_page.h"
#include "mkvtoolnix-gui/main_window/main_window.h"
#include "mkvtoolnix-gui/util/language_display_widget.h"
#include "mkvtoolnix-gui/util/widget.h"

namespace mtx::gui::HeaderEditor {

using namespace mtx::gui;

LanguageIETFValuePage::LanguageIETFValuePage(Tab &parent,
                                             PageBase &topLevelPage,
                                             EbmlMaster &master,
                                             EbmlCallbacks const &callbacks,
                                             translatable_string_c const &title,
                                             translatable_string_c const &description)
  : ValuePage{parent, topLevelPage, master, callbacks, ValueType::AsciiString, title, description}
{
}

LanguageIETFValuePage::~LanguageIETFValuePage() {
}

QWidget *
LanguageIETFValuePage::createInputControl() {
  m_originalValue     = m_element ? static_cast<EbmlString *>(m_element)->GetValue() : ""s;
  auto parsedLanguage = mtx::bcp47::language_c::parse(m_originalValue);
  auto currentValue   = parsedLanguage.is_valid() ? parsedLanguage.format() : m_originalValue;

  m_ldwValue          = new Util::LanguageDisplayWidget{this};

  m_ldwValue->setLanguage(parsedLanguage);

  return m_ldwValue;
}

QString
LanguageIETFValuePage::originalValueAsString()
  const {
  return Q(m_originalValue);
}

QString
LanguageIETFValuePage::currentValueAsString()
  const {
  return Q(m_ldwValue->language().format());
}

void
LanguageIETFValuePage::resetValue() {
  m_ldwValue->setLanguage(mtx::bcp47::language_c::parse(m_originalValue));
}

bool
LanguageIETFValuePage::validateValue()
  const {
  return m_ldwValue->language().is_valid();
}

void
LanguageIETFValuePage::copyValueToElement() {
  static_cast<EbmlString *>(m_element)->SetValue(to_utf8(currentValueAsString()));
}

}
