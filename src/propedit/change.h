/*
   mkvpropedit -- utility for editing properties of existing Matroska files

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __PROPEDIT_CHANGE_H
#define __PROPEDIT_CHANGE_H

#include "common/os.h"

#include <string>
#include <vector>

#include "common/bitvalue.h"
#include "common/memory.h"
#include "common/property_element.h"
#include "common/smart_pointers.h"

class change_c {
public:
  enum change_type_e {
    ct_add,
    ct_set,
    ct_delete,
  };

  change_type_e m_type;
  std::string m_name, m_value;

  property_element_c m_property;

  std::string m_s_value;
  uint64_t m_ui_value;
  int64_t m_si_value;
  bool m_b_value;
  bitvalue_c m_x_value;
  double m_fp_value;

public:
  change_c(change_type_e type, const std::string &name, const std::string &value);

  void validate(std::vector<property_element_c> *property_table = NULL);
  void dump_info() const;

  bool lookup_property(std::vector<property_element_c> &table);

  std::string get_spec();

protected:
  void parse_ascii_string();
  void parse_binary();
  void parse_boolean();
  void parse_floating_point_number();
  void parse_signed_integer();
  void parse_unicode_string();
  void parse_unsigned_integer();
  void parse_value();
};
typedef counted_ptr<change_c> change_cptr;

#endif // __PROPEDIT_CHANGE_H
