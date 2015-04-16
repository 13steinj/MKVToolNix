/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   exception definitions for AAC data

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef MTX_COMMON_AAC_X_H
#define MTX_COMMON_AAC_X_H

#include "common/common_pch.h"

#include "common/error.h"

namespace aac {

class unsupported_feature_x: public mtx::exception {
private:
  std::string const m_error;

public:
  unsupported_feature_x(char const *feature)
    : m_error{(boost::format("unsupported feature »%1%«") % feature).str()}
  {
  }

  virtual char const *what() const throw() override {
    return m_error.c_str();
  }

  virtual std::string error() const throw() override {
    return m_error;
  }
};


} // namespace aac

#endif // MTX_COMMON_AAC_X_H
