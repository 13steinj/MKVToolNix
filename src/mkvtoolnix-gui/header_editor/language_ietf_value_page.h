#pragma once

#include "common/common_pch.h"

#include "mkvtoolnix-gui/header_editor/value_page.h"

namespace mtx::gui {

namespace Util {
class LanguageDisplayWidget;
}

namespace HeaderEditor {

class Tab;

class LanguageIETFValuePage: public ValuePage {
public:
  Util::LanguageDisplayWidget *m_ldwValue{};
  std::string m_originalValue;

public:
  LanguageIETFValuePage(Tab &parent, PageBase &topLevelPage, EbmlMaster &master, EbmlCallbacks const &callbacks, translatable_string_c const &title, translatable_string_c const &description);
  virtual ~LanguageIETFValuePage();

  virtual QWidget *createInputControl() override;
  virtual QString originalValueAsString() const override;
  virtual QString currentValueAsString() const override;
  virtual void resetValue() override;
  virtual bool validateValue() const override;
  virtual void copyValueToElement() override;
};

}}
