/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

enum file_status_e {
  FILE_STATUS_DONE         = 0,
  FILE_STATUS_DONE_AND_DRY,
  FILE_STATUS_HOLDING,
  FILE_STATUS_MOREDATA
};
