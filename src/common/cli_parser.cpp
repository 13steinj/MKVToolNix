/** \brief command line parsing

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   \file

   \author Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/cli_parser.h"
#include "common/command_line.h"
#include "common/container.h"
#include "common/strings/editing.h"
#include "common/strings/formatting.h"
#include "common/translation.h"

#define INDENT_COLUMN_OPTION_NAME         2
#define INDENT_COLUMN_OPTION_DESCRIPTION 30
#define INDENT_COLUMN_SECTION_HEADER      1

cli_parser_c::option_t::option_t()
  : m_needs_arg{}
{
}

cli_parser_c::option_t::option_t(cli_parser_c::option_t::option_type_e type,
                                 translatable_string_c const &description,
                                 int indent)
  : m_type{type}
  , m_description{description}
  , m_needs_arg{}
  , m_indent{indent}
{
}

cli_parser_c::option_t::option_t(std::string const &name,
                                 translatable_string_c const &description)
  : m_type{cli_parser_c::option_t::ot_informational_option}
  , m_name{name}
  , m_description{description}
  , m_needs_arg{}
  , m_indent{INDENT_DEFAULT}
{
}

cli_parser_c::option_t::option_t(std::string const &spec,
                                 translatable_string_c const &description,
                                 cli_parser_cb_t const &callback,
                                 bool needs_arg)
  : m_type{cli_parser_c::option_t::ot_option}
  , m_spec{spec}
  , m_description{description}
  , m_callback{callback}
  , m_needs_arg{needs_arg}
  , m_indent{INDENT_DEFAULT}
{
}

std::string
cli_parser_c::option_t::format_text() {
  auto description = m_description.get_translated();

  if ((cli_parser_c::option_t::ot_option == m_type) || (cli_parser_c::option_t::ot_informational_option == m_type))
    return format_paragraph(description, INDENT_DEFAULT == m_indent ? INDENT_COLUMN_OPTION_DESCRIPTION : m_indent, std::string(INDENT_COLUMN_OPTION_NAME, ' ') + m_name);

  else if (cli_parser_c::option_t::ot_section_header == m_type)
    return std::string("\n") + format_paragraph(description + ":", INDENT_DEFAULT == m_indent ? INDENT_COLUMN_SECTION_HEADER : m_indent);

  return format_paragraph(description, INDENT_DEFAULT == m_indent ? 0 : m_indent);
}

// ------------------------------------------------------------

cli_parser_c::cli_parser_c(std::vector<std::string> const &args)
  : m_args{args}
{
  m_hooks[cli_parser_c::ht_common_options_parsed] = std::vector<cli_parser_cb_t>();
  m_hooks[cli_parser_c::ht_unknown_option]        = std::vector<cli_parser_cb_t>();
}

void
cli_parser_c::parse_args() {
  set_usage();
  while (handle_common_cli_args(m_args, ""))
    set_usage();

  run_hooks(cli_parser_c::ht_common_options_parsed);

  std::vector<std::string>::const_iterator sit;
  mxforeach(sit, m_args) {
    bool no_next_arg = (sit + 1) == m_args.end();
    m_current_arg    = *sit;
    m_next_arg       = no_next_arg ? "" : *(sit + 1);

    auto option_it = m_option_map.find(m_current_arg);
    if (option_it != m_option_map.end()) {
      auto &option = option_it->second;

      if (option.m_needs_arg) {
        if (no_next_arg)
          mxerror(boost::format(Y("Missing argument to '%1%'.\n")) % m_current_arg);
        ++sit;
      }

      option.m_callback();

    } else if (!run_hooks(cli_parser_c::ht_unknown_option))
      mxerror(boost::format(Y("Unknown option '%1%'.\n")) % m_current_arg);
  }
}

void
cli_parser_c::add_informational_option(std::string const &name,
                                       translatable_string_c const &description) {
  m_options.emplace_back(name, description);
}

void
cli_parser_c::add_option(std::string const &spec,
                         cli_parser_cb_t const &callback,
                         translatable_string_c const &description) {
  auto parts     = split(spec, "=", 2);
  auto needs_arg = parts.size() == 2;
  auto option    = cli_parser_c::option_t{spec, description, callback, needs_arg};
  auto names     = split(parts[0], "|");

  for (auto &name : names) {
    auto full_name = '@' == name[0]       ? name
                   : 1   == name.length() ? std::string( "-") + name
                   :                        std::string("--") + name;

    if (mtx::includes(m_option_map, full_name))
      mxerror(boost::format("cli_parser_c::add_option(): Programming error: option '%1%' is already used for spec '%2%' "
                            "and cannot be used for spec '%3%'.\n") % full_name % m_option_map[full_name].m_spec % spec);

    m_option_map[full_name] = option;

    if (!option.m_name.empty())
      option.m_name += ", ";
    option.m_name += full_name;
  }

  if (needs_arg)
    option.m_name += " " + parts[1];

  m_options.push_back(option);
}

void
cli_parser_c::add_section_header(translatable_string_c const &title,
                                 int indent) {
  m_options.emplace_back(cli_parser_c::option_t::ot_section_header, title, indent);
}

void
cli_parser_c::add_information(translatable_string_c const &information,
                              int indent) {
  m_options.emplace_back(cli_parser_c::option_t::ot_information, information, indent);
}

void
cli_parser_c::add_separator() {
  m_options.emplace_back(cli_parser_c::option_t::ot_information, translatable_string_c(""));
}

#define OPT(name, description) add_option(name, std::bind(&cli_parser_c::dummy_callback, this), description)

void
cli_parser_c::add_common_options() {
  OPT("v|verbose",                      YT("Increase verbosity."));
  OPT("q|quiet",                        YT("Suppress status output."));
  OPT("ui-language=<code>",             YT("Force the translations for 'code' to be used."));
  OPT("command-line-charset=<charset>", YT("Charset for strings on the command line"));
  OPT("output-charset=<cset>",          YT("Output messages in this charset"));
  OPT("r|redirect-output=<file>",       YT("Redirects all messages into this file."));
  OPT("@file",                          YT("Reads additional command line options from the specified file (see man page)."));
  OPT("h|help",                         YT("Show this help."));
  OPT("V|version",                      YT("Show version information."));
#if defined(HAVE_CURL_EASY_H)
  OPT("check-for-updates",              YT("Check online for the latest release."));
#endif  // defined(HAVE_CURL_EASY_H)
}

#undef OPT

void
cli_parser_c::set_usage() {
  usage_text = "";
  for (auto &option : m_options)
    usage_text += option.format_text();
}

void
cli_parser_c::dummy_callback() {
}

void
cli_parser_c::add_hook(cli_parser_c::hook_type_e hook_type,
                       const cli_parser_cb_t &callback) {
  m_hooks[hook_type].push_back(callback);
}

bool
cli_parser_c::run_hooks(cli_parser_c::hook_type_e hook_type) {
  if (m_hooks[hook_type].empty())
    return false;

  for (auto &hook : m_hooks[hook_type])
    if (hook)
      (hook)();

  return true;
}
