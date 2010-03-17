/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   helper functions that need libebml/libmatroska

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __MTX_COMMON_EBML_H
#define __MTX_COMMON_EBML_H

#include "common/os.h"

#include <ebml/EbmlMaster.h>
#include <ebml/EbmlUnicodeString.h>

#include <matroska/KaxTracks.h>

using namespace libebml;
using namespace libmatroska;

#define can_be_cast(c, e) (dynamic_cast<c *>(e) != NULL)

bool MTX_DLL_API is_valid_utf8_string(const std::string &c);
UTFstring MTX_DLL_API cstrutf8_to_UTFstring(const std::string &c);
std::string MTX_DLL_API UTFstring_to_cstrutf8(const UTFstring &u);

int64_t MTX_DLL_API kt_get_default_duration(KaxTrackEntry &track);
int64_t MTX_DLL_API kt_get_number(KaxTrackEntry &track);
int64_t MTX_DLL_API kt_get_uid(KaxTrackEntry &track);
std::string MTX_DLL_API kt_get_codec_id(KaxTrackEntry &track);
int MTX_DLL_API kt_get_max_blockadd_id(KaxTrackEntry &track);
std::string MTX_DLL_API kt_get_language(KaxTrackEntry &track);

int MTX_DLL_API kt_get_a_channels(KaxTrackEntry &track);
float MTX_DLL_API kt_get_a_sfreq(KaxTrackEntry &track);
float MTX_DLL_API kt_get_a_osfreq(KaxTrackEntry &track);
int MTX_DLL_API kt_get_a_bps(KaxTrackEntry &track);

int MTX_DLL_API kt_get_v_pixel_width(KaxTrackEntry &track);
int MTX_DLL_API kt_get_v_pixel_height(KaxTrackEntry &track);

#define is_id(e, ref) (EbmlId(*e) == EBML_ID(ref))
#if !defined(EBML_INFO)
#define EBML_INFO(ref)  ref::ClassInfos
#endif
#if !defined(EBML_ID)
#define EBML_ID(ref)  ref::ClassInfos.GlobalId
#endif
#if !defined(EBML_ID_VALUE)
#define EBML_ID_VALUE(id)  id.Value
#endif
#if !defined(EBML_ID_LENGTH)
#define EBML_ID_LENGTH(id)  id.Length
#endif
#if !defined(EBML_CONTEXT)
#define EBML_CONTEXT(e)  e->Generic().Context
#endif
#if !defined(EBML_NAME)
#define EBML_NAME(e)  e->Generic().DebugName
#endif
#if !defined(EBML_INFO_ID)
#define EBML_INFO_ID(cb)      (cb).GlobalId
#endif
#if !defined(EBML_INFO_NAME)
#define EBML_INFO_NAME(cb)    (cb).DebugName
#endif
#if !defined(EBML_INFO_CREATE)
#define EBML_INFO_CREATE(cb)  (cb).Create()
#endif
#if !defined(EBML_INFO_CONTEXT)
#define EBML_INFO_CONTEXT(cb) (cb).Context
#endif
#if !defined(EBML_SEM_UNIQUE)
#define EBML_SEM_UNIQUE(s)  (s).Unique
#endif
#if !defined(EBML_SEM_INFO)
#define EBML_SEM_INFO(s)  (s).GetCallbacks
#endif
#if !defined(EBML_SEM_ID)
#define EBML_SEM_ID(s)    (s).GetCallbacks.GlobalId
#endif
#if !defined(EBML_SEM_CONTEXT)
#define EBML_SEM_CONTEXT(s) (s).GetCallbacks.Context
#endif
#if !defined(EBML_CTX_SIZE)
#define EBML_CTX_SIZE(c) (c).Size
#endif

#define FINDFIRST(p, c)   (static_cast<c *>(((EbmlMaster *)p)->FindFirstElt(EBML_INFO(c), false)))
#define FINDNEXT(p, c, e) (static_cast<c *>(((EbmlMaster *)p)->FindNextElt(*e, false)))

template <typename type>type &
GetEmptyChild(EbmlMaster &master) {
  EbmlElement *e;
  EbmlMaster *m;

  e = master.FindFirstElt(EBML_INFO(type), true);
  if ((m = dynamic_cast<EbmlMaster *>(e)) != NULL) {
    while (m->ListSize() > 0) {
      delete (*m)[0];
      m->Remove(0);
    }
  }

  return *(static_cast<type *>(e));
}

template <typename type>type &
GetNextEmptyChild(EbmlMaster &master,
                  const type &past_elt) {
  EbmlMaster *m;
  EbmlElement *e = master.FindNextElt(past_elt, true);
  if ((m = dynamic_cast<EbmlMaster *>(e)) != NULL) {
    while (m->ListSize() > 0) {
      delete (*m)[0];
      m->Remove(0);
    }
  }

  return *(static_cast<type *>(e));
}

template <typename type>type &
AddEmptyChild(EbmlMaster &master) {
  EbmlMaster *m;
  EbmlElement *e = new type;
  if ((m = dynamic_cast<EbmlMaster *>(e)) != NULL) {
    while (m->ListSize() > 0) {
      delete (*m)[0];
      m->Remove(0);
    }
  }
  master.PushElement(*e);

  return *(static_cast<type *>(e));
}

template<typename A> A &
GetChild(EbmlMaster *m) {
  return GetChild<A>(*m);
}

template<typename A, typename B> B &
GetChildAs(EbmlMaster &m) {
  return GetChild<A>(m);
}

template<typename A, typename B> B &
GetChildAs(EbmlMaster *m) {
  return GetChild<A>(*m);
}

template <typename A>A &
GetFirstOrNextChild(EbmlMaster &master,
                    A *previous_child) {
  return NULL == previous_child ? GetChild<A>(master) : GetNextChild<A>(master, *previous_child);
}

template <typename A>A &
GetFirstOrNextChild(EbmlMaster *master,
                    A *previous_child) {
  return NULL == previous_child ? GetChild<A>(*master) : GetNextChild<A>(*master, *previous_child);
}

EbmlElement *MTX_DLL_API empty_ebml_master(EbmlElement *e);
EbmlElement *MTX_DLL_API create_ebml_element(const EbmlCallbacks &callbacks, const EbmlId &id);
EbmlMaster *MTX_DLL_API sort_ebml_master(EbmlMaster *e);

const EbmlCallbacks *MTX_DLL_API find_ebml_callbacks(const EbmlCallbacks &base, const EbmlId &id);
const EbmlCallbacks *MTX_DLL_API find_ebml_callbacks(const EbmlCallbacks &base, const char *debug_name);
const EbmlCallbacks *MTX_DLL_API find_ebml_parent_callbacks(const EbmlCallbacks &base, const EbmlId &id);
const EbmlSemantic *MTX_DLL_API find_ebml_semantic(const EbmlCallbacks &base, const EbmlId &id);

EbmlElement *MTX_DLL_API find_ebml_element_by_id(EbmlMaster *master, const EbmlId &id);

void MTX_DLL_API fix_mandatory_elements(EbmlElement *master);

template<typename A> void
provide_default_for_child(EbmlMaster &master,
                          const UTFstring &default_value) {
  EbmlUnicodeString &value = GetChildAs<A, EbmlUnicodeString>(master);
  if (!static_cast<const UTFstring &>(value).length())
    value = default_value;
}

template<typename A> void
provide_default_for_child(EbmlMaster *master,
                          const UTFstring &default_value) {
  provide_default_for_child<A>(*master, default_value);
}

#endif // __MTX_COMMON_EBML_H
