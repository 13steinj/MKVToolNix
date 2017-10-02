/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definition input exceptions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

namespace mtx {
  namespace input {
    class exception: public mtx::exception {
    public:
      virtual const char *what() const throw() {
        return "unspecified reader error";
      }
    };

    class open_x: public exception {
    public:
      virtual const char *what() const throw() {
        return "open error";
      }
    };

    class invalid_format_x: public exception {
    public:
      virtual const char *what() const throw() {
        return "invalid format";
      }
    };

    class header_parsing_x: public exception {
    public:
      virtual const char *what() const throw() {
        return "headers could not be parsed or were incomplete";
      }
    };

    class extended_x: public exception {
    protected:
      std::string m_message;
    public:
      extended_x(const std::string &message)  : m_message(message)       { }
      extended_x(const boost::format &message): m_message(message.str()) { }
      virtual ~extended_x() throw() { }

      virtual const char *what() const throw() {
        return m_message.c_str();
      }
    };
  }
}
