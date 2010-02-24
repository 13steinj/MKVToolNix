/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   translation, locale handling

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/os.h"

#include <algorithm>
#include <boost/regex.hpp>
#if HAVE_NL_LANGINFO
# include <langinfo.h>
#elif HAVE_LOCALE_CHARSET
# include <libcharset.h>
#endif
#if defined(HAVE_LIBINTL_H)
# include <libintl.h>
#endif
#include <locale.h>
#include <stdlib.h>

#include "common/common.h"
#include "common/locale_string.h"
#include "common/strings/editing.h"
#include "common/translation.h"

#if defined(SYS_WINDOWS)
# include <windows.h>
# include "common/fs_sys_helpers.h"
# include "common/memory.h"
#endif

std::vector<translation_c> translation_c::ms_available_translations;
int translation_c::ms_active_translation_idx = 0;

translation_c::translation_c(const std::string &unix_locale,
                             const std::string &windows_locale,
                             const std::string &windows_locale_sysname,
                             const std::string &english_name,
                             const std::string &translated_name,
                             bool line_breaks_anywhere)
  : m_unix_locale(unix_locale)
  , m_windows_locale(windows_locale)
  , m_windows_locale_sysname(windows_locale_sysname)
  , m_english_name(english_name)
  , m_translated_name(translated_name)
  , m_line_breaks_anywhere(line_breaks_anywhere)
{
}

void
translation_c::initialize_available_translations() {
  ms_available_translations.clear();
  ms_available_translations.push_back(translation_c("en_US", "en",    "english",  "English",             "English",  false));
#if defined(HAVE_LIBINTL_H)
  ms_available_translations.push_back(translation_c("de_DE", "de",    "german",   "German",              "Deutsch",  false));
  ms_available_translations.push_back(translation_c("ja_JP", "ja",    "japanese", "Japanese",            "日本語",   true));
  ms_available_translations.push_back(translation_c("zh_CN", "zh_CN", "chinese",  "Chinese Simplified",  "简体中文", true));
  ms_available_translations.push_back(translation_c("zh_TW", "zh_TW", "chinese",  "Chinese Traditional", "繁體中文", true));
#endif

  ms_active_translation_idx = 0;
}

int
translation_c::look_up_translation(const std::string &locale) {
  try {
    std::string locale_lower               = downcase(locale_string_c(locale).str(locale_string_c::half));
    std::vector<translation_c>::iterator i = ms_available_translations.begin();
    int idx                                = 0;

    while (i != ms_available_translations.end()) {
      if (   (downcase(i->get_locale()) == locale_lower)
#if defined(SYS_WINDOWS)
          || (downcase(i->m_windows_locale_sysname) == locale_lower)
#endif
           )
        return idx;
      ++i;
      ++idx;
    }

  } catch (locale_string_format_error_c &error) {
  }

  return -1;
}

std::string
translation_c::get_default_ui_locale() {
  std::string locale;

#if defined(HAVE_LIBINTL_H)
# if defined(SYS_WINDOWS)
  std::string env_var = get_environment_variable("LC_MESSAGES");
  if (!env_var.empty() && (-1 != look_up_translation(env_var)))
    return env_var;

  env_var = get_environment_variable("LANG");
  if (!env_var.empty() && (-1 != look_up_translation(env_var)))
    return env_var;

  char *data;
  int len = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, NULL, 0);
  if (0 < len) {
    data = (char *)safemalloc(len);
    memset(data, 0, len);
    if (0 != GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, data, len))
      locale = data;
    safefree(data);

    int idx = translation_c::look_up_translation(locale);
    if (-1 != idx)
      locale = ms_available_translations[idx].get_locale();
  }

# else  // SYS_WINDOWS

  char *data = setlocale(LC_MESSAGES, NULL);
  if (NULL != data) {
    std::string previous_locale = data;
    if (debugging_requested("locale"))
      mxinfo(boost::format("[get_default_ui_locale previous %1%]\n") % previous_locale);
    setlocale(LC_MESSAGES, "");
    data = setlocale(LC_MESSAGES, NULL);

    if (NULL != data)
      locale = data;

    if (debugging_requested("locale"))
      mxinfo(boost::format("[get_default_ui_locale new %1%]\n") % locale);

    setlocale(LC_MESSAGES, previous_locale.c_str());
  } else if (debugging_requested("locale"))
    mxinfo(boost::format("[get_default_ui_locale get previous failed]\n"));

# endif // SYS_WINDOWS
#endif  // HAVE_LIBINTL_H

  return locale;
}

std::string
translation_c::get_locale() {
#if defined(SYS_WINDOWS)
  return m_windows_locale;
#else
  return m_unix_locale;
#endif
}

translation_c &
translation_c::get_active_translation() {
  return ms_available_translations[ms_active_translation_idx];
}

void
translation_c::set_active_translation(const std::string &locale) {
  int idx                   = look_up_translation(locale);
  ms_active_translation_idx = std::max(idx, 0);

  if (debugging_requested("locale"))
    mxinfo(boost::format("[translation_c::set_active_translation() active_translation_idx %1% for locale %2%]\n") % ms_active_translation_idx % locale);
}

// ------------------------------------------------------------

translatable_string_c::translatable_string_c()
{
}

translatable_string_c::translatable_string_c(const std::string &untranslated_string)
  : m_untranslated_string(untranslated_string)
{
}

translatable_string_c::translatable_string_c(const char *untranslated_string)
  : m_untranslated_string(untranslated_string)
{
}

std::string
translatable_string_c::get_translated()
  const
{
  return m_untranslated_string.empty() ? "" : Y(m_untranslated_string.c_str());
}

std::string
translatable_string_c::get_untranslated()
  const
{
  return m_untranslated_string;
}

// ------------------------------------------------------------

#if defined(HAVE_LIBINTL_H)

void
init_locales(std::string locale) {
  translation_c::initialize_available_translations();

  if (debugging_requested("locale"))
    mxinfo(boost::format("[init_locales start: locale %1%]\n") % locale);

  std::string locale_dir;
  std::string default_locale = translation_c::get_default_ui_locale();

  if (-1 == translation_c::look_up_translation(locale)) {
    if (debugging_requested("locale"))
      mxinfo(boost::format("[init_locales lookup failed; clearing locale]\n"));
    locale = "";
  }

  if (locale.empty()) {
    locale = default_locale;
    if (debugging_requested("locale"))
      mxinfo(boost::format("[init_locales setting to default locale %1%]\n") % locale);
  }

# if defined(SYS_WINDOWS)
  set_environment_variable("LANGUAGE", "");

  if (!locale.empty()) {
    // The Windows system headers define LC_MESSAGES but
    // Windows itself doesn't know it and treats "set_locale(LC_MESSAGES, ...)"
    // as an error. gettext uses the LANG and LC_MESSAGE environment variables instead.

    // Windows knows two environments: the system environment that's
    // modified by SetEnvironmentVariable() and the C library's cache
    // of said environment which is modified via _putenv().

    set_environment_variable("LANG",        locale);
    set_environment_variable("LC_MESSAGES", locale);

    translation_c::set_active_translation(locale);
  }

  locale_dir = get_installation_path() + "\\locale";

# else  // SYS_WINDOWS
  std::string chosen_locale;

  try {
    locale_string_c loc_default(default_locale);
    std::string loc_req_with_default_codeset(locale_string_c(locale).set_codeset_and_modifier(loc_default).str());

    if (setlocale(LC_MESSAGES, loc_req_with_default_codeset.c_str()) != NULL)
      chosen_locale = loc_req_with_default_codeset;

    else if (setlocale(LC_MESSAGES, locale.c_str()) != NULL)
      chosen_locale = locale;

    else {
      std::string loc_req_with_utf8 = locale_string_c(locale).set_codeset_and_modifier(locale_string_c("dummy.UTF-8")).str();
      if (setlocale(LC_MESSAGES, loc_req_with_utf8.c_str()) != NULL)
        chosen_locale = loc_req_with_utf8;
    }

  } catch (locale_string_format_error_c &error) {
    if (debugging_requested("locale"))
      mxinfo(boost::format("[init_locales format error in %1%]\n") % error.m_format);
  }

  if (debugging_requested("locale"))
    mxinfo(boost::format("[init_locales chosen locale %1%]\n") % chosen_locale);

  // Hard fallback to "C" locale if no suitable locale was
  // selected. This can happen if the system has no locales for
  // "en_US" or "en_US.UTF-8" compiled.
  if (chosen_locale.empty() && (setlocale(LC_MESSAGES, "C") != NULL))
    chosen_locale = "C";

  if (chosen_locale.empty())
    mxerror(Y("The locale could not be set properly. Check the LANG, LC_ALL and LC_MESSAGES environment variables.\n"));

  translation_c::set_active_translation(chosen_locale);

  locale_dir = MTX_LOCALE_DIR;
# endif  // SYS_WINDOWS

# if defined(SYS_APPLE)
  int result = setenv("LC_MESSAGES", chosen_locale.c_str(), 1);
  if (debugging_requested("locale"))
    mxinfo(boost::format("[init_locales setenv() return code: %1%]\n") % result);
# endif

  bindtextdomain("mkvtoolnix", locale_dir.c_str());
  textdomain("mkvtoolnix");
  bind_textdomain_codeset("mkvtoolnix", "UTF-8");
}

#else  // HAVE_LIBINTL_H

void
init_locales(std::string) {
  translation_c::initialize_available_translations();
}

#endif  // HAVE_LIBINTL_H
