/*
 * Copyright © 2007,2008,2009,2010  Red Hat, Inc.
 * Copyright © 2012,2018  Google, Inc.
 * Copyright © 2019  Facebook, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 * Facebook Author(s): Behdad Esfahbod
 */

#ifndef HB_SERIALIZE_HH
#define HB_SERIALIZE_HH

#include "hb.hh"
#include "hb-blob.hh"
#include "hb-map.hh"
#include "hb-pool.hh"

#ifdef HB_EXPERIMENTAL_API
#include "hb-subset-repacker.h"
#endif

/*
 * Serialize
 */

enum hb_serialize_error_t {
  HB_SERIALIZE_ERROR_NONE =            0x00000000u,
  HB_SERIALIZE_ERROR_OTHER =           0x00000001u,
  HB_SERIALIZE_ERROR_OFFSET_OVERFLOW = 0x00000002u,
  HB_SERIALIZE_ERROR_OUT_OF_ROOM =     0x00000004u,
  HB_SERIALIZE_ERROR_INT_OVERFLOW =    0x00000008u,
  HB_SERIALIZE_ERROR_ARRAY_OVERFLOW =  0x00000010u
};
HB_MARK_AS_FLAG_T (hb_serialize_error_t);

struct hb_serialize_context_t
{
  typedef unsigned objidx_t;

  enum whence_t {
     Head,	/* Relative to the current object head (default). */
     Tail,	/* Relative to the current object tail after packed. */
     Absolute	/* Absolute: from the start of the serialize buffer. */
   };



  struct object_t
  {
    void fini () {
      real_links.fini ();
      virtual_links.fini ();
    }

    object_t () = default;

#ifdef HB_EXPERIMENTAL_API
    object_t (const hb_object_t &o)
    {
      head = o.head;
      tail = o.tail;
      next = nullptr;
      real_links.alloc (o.num_real_links);
      for (unsigned i = 0 ; i < o.num_real_links; i++)
        real_links.push (o.real_links[i]);

      virtual_links.alloc (o.num_virtual_links);
      for (unsigned i = 0; i < o.num_virtual_links; i++)
        virtual_links.push (o.virtual_links[i]);
    }
#endif

    friend void swap (object_t& a, object_t& b)
    {
      hb_swap (a.head, b.head);
      hb_swap (a.tail, b.tail);
      hb_swap (a.next, b.next);
      hb_swap (a.real_links, b.real_links);
      hb_swap (a.virtual_links, b.virtual_links);
    }

    bool operator == (const object_t &o) const
    {
      // Virtual links aren't considered for equality since they don't affect the functionality
      // of the object.
      return (tail - head == o.tail - o.head)
	  && (real_links.length == o.real_links.length)
	  && 0 == hb_memcmp (head, o.head, (unsigned int)(tail - head))
	  && real_links.as_bytes () == o.real_links.as_bytes ();
    }
    uint32_t hash () const
    {
      // Virtual links aren't considered for equality since they don't affect the functionality
      // of the object.
      return hb_bytes_t (head, (unsigned int)(tail - head)).hash () ^
          real_links.as_bytes ().hash ();
    }

    struct link_t
    {
      unsigned width: 3;
      unsigned is_signed: 1;
      unsigned whence: 2;
      unsigned bias : 26;
      unsigned position;
      objidx_t objidx;

      link_t () = default;

#ifdef HB_EXPERIMENTAL_API
      link_t (const hb_link_t &o)
      {
        width = o.width;
        is_signed = 0;
        whence = 0;
        position = o.position;
        bias = 0;
        objidx = o.objidx;
      }
#endif
    };

    char *head;
    char *tail;
    hb_vector_t<link_t> real_links;
    hb_vector_t<link_t> virtual_links;
    object_t *next;

    auto all_links () const HB_AUTO_RETURN
        (( hb_concat (this->real_links, this->virtual_links) ));
    auto all_links_writer () HB_AUTO_RETURN
        (( hb_concat (this->real_links.writer (), this->virtual_links.writer ()) ));
  };

  struct snapshot_t
  {
    char *head;
    char *tail;
    object_t *current; // Just for sanity check
    unsigned num_real_links;
    unsigned num_virtual_links;
    hb_serialize_error_t errors;
  };

  snapshot_t snapshot ()
  { return snapshot_t {
      head, tail, current, current->real_links.length, current->virtual_links.length, errors }; }

  hb_serialize_context_t (void *start_, unsigned int size) :
    start ((char *) start_),
    end (start + size),
    current (nullptr)
  { reset (); }
  ~hb_serialize_context_t () { fini (); }

  void fini ()
  {
    for (object_t *_ : ++hb_iter (packed)) _->fini ();
    packed.fini ();
    this->packed_map.fini ();

    while (current)
    {
      auto *_ = current;
      current = current->next;
      _->fini ();
    }
    object_pool.fini ();
  }

  bool in_error () const { return bool (errors); }

  bool successful () const { return !bool (errors); }

  HB_NODISCARD bool ran_out_of_room () const { return errors & HB_SERIALIZE_ERROR_OUT_OF_ROOM; }
  HB_NODISCARD bool offset_overflow () const { return errors & HB_SERIALIZE_ERROR_OFFSET_OVERFLOW; }
  HB_NODISCARD bool only_offset_overflow () const { return errors == HB_SERIALIZE_ERROR_OFFSET_OVERFLOW; }
  HB_NODISCARD bool only_overflow () const
  {
    return errors == HB_SERIALIZE_ERROR_OFFSET_OVERFLOW
        || errors == HB_SERIALIZE_ERROR_INT_OVERFLOW
        || errors == HB_SERIALIZE_ERROR_ARRAY_OVERFLOW;
  }

  void reset (void *start_, unsigned int size)
  {
    start = (char*) start_;
    end = start + size;
    reset ();
    current = nullptr;
  }

  void reset ()
  {
    this->errors = HB_SERIALIZE_ERROR_NONE;
    this->head = this->start;
    this->tail = this->end;
    this->debug_depth = 0;

    fini ();
    this->packed.push (nullptr);
    this->packed_map.init ();
  }

  bool check_success (bool success,
                      hb_serialize_error_t err_type = HB_SERIALIZE_ERROR_OTHER)
  {
    return successful ()
        && (success || err (err_type));
  }

  template <typename T1, typename T2>
  bool check_equal (T1 &&v1, T2 &&v2, hb_serialize_error_t err_type)
  {
    if ((long long) v1 != (long long) v2)
    {
      return err (err_type);
    }
    return true;
  }

  template <typename T1, typename T2>
  bool check_assign (T1 &v1, T2 &&v2, hb_serialize_error_t err_type)
  { return check_equal (v1 = v2, v2, err_type); }

  template <typename T> bool propagate_error (T &&obj)
  { return check_success (!hb_deref (obj).in_error ()); }

  template <typename T1, typename... Ts> bool propagate_error (T1 &&o1, Ts&&... os)
  { return propagate_error (std::forward<T1> (o1)) &&
	   propagate_error (std::forward<Ts> (os)...); }

  /* To be called around main operation. */
  template <typename Type>
  Type *start_serialize ()
  {
    DEBUG_MSG_LEVEL (SERIALIZE, this->start, 0, +1,
		     "start [%p..%p] (%lu bytes)",
		     this->start, this->end,
		     (unsigned long) (this->end - this->start));

    assert (!current);
    return push<Type> ();
  }
  void end_serialize ()
  {
    DEBUG_MSG_LEVEL (SERIALIZE, this->start, 0, -1,
		     "end [%p..%p] serialized %u bytes; %s",
		     this->start, this->end,
		     (unsigned) (this->head - this->start),
		     successful () ? "successful" : "UNSUCCESSFUL");

    propagate_error (packed, packed_map);

    if (unlikely (!current)) return;
    if (unlikely (in_error()))
    {
      // Offset overflows that occur before link resolution cannot be handled
      // by repacking, so set a more general error.
      if (offset_overflow ()) err (HB_SERIALIZE_ERROR_OTHER);
      return;
    }

    assert (!current->next);

    /* Only "pack" if there exist other objects... Otherwise, don't bother.
     * Saves a move. */
    if (packed.length <= 1)
      return;

    pop_pack (false);

    resolve_links ();
  }

  template <typename Type = void>
  Type *push ()
  {
    if (unlikely (in_error ())) return start_embed<Type> ();

    object_t *obj = object_pool.alloc ();
    if (unlikely (!obj))
      check_success (false);
    else
    {
      obj->head = head;
      obj->tail = tail;
      obj->next = current;
      current = obj;
    }
    return start_embed<Type> ();
  }
  void pop_discard ()
  {
    object_t *obj = current;
    if (unlikely (!obj)) return;
    if (unlikely (in_error())) return;

    current = current->next;
    revert (obj->head, obj->tail);
    obj->fini ();
    object_pool.release (obj);
  }

  /* Set share to false when an object is unlikely shareable with others
   * so not worth an attempt, or a contiguous table is serialized as
   * multiple consecutive objects in the reverse order so can't be shared.
   */
  objidx_t pop_pack (bool share=true)
  {
    object_t *obj = current;
    if (unlikely (!obj)) return 0;
    if (unlikely (in_error())) return 0;

    current = current->next;
    obj->tail = head;
    obj->next = nullptr;
    unsigned len = (unsigned)(obj->tail - obj->head);
    head = obj->head; /* Rewind head. */

    if (!len)
    {
      assert (!obj->real_links.length);
      assert (!obj->virtual_links.length);
      return 0;
    }

    objidx_t objidx;
    if (share)
    {
      objidx = packed_map.get (obj);
      if (objidx)
      {
        merge_virtual_links (obj, objidx);
	obj->fini ();
	return objidx;
      }
    }

    tail -= len;
    memmove (tail, obj->head, len);

    obj->head = tail;
    obj->tail = tail + len;

    packed.push (obj);

    if (unlikely (!propagate_error (packed)))
    {
      /* Obj wasn't successfully added to packed, so clean it up otherwise its
       * links will be leaked. When we use constructor/destructors properly, we
       * can remove these. */
      obj->fini ();
      return 0;
    }

    objidx = packed.length - 1;

    if (share) packed_map.set (obj, objidx);
    propagate_error (packed_map);

    return objidx;
  }

  void revert (snapshot_t snap)
  {
    // Overflows that happened after the snapshot will be erased by the revert.
    if (unlikely (in_error () && !only_overflow ())) return;
    assert (snap.current == current);
    current->real_links.shrink (snap.num_real_links);
    current->virtual_links.shrink (snap.num_virtual_links);
    errors = snap.errors;
    revert (snap.head, snap.tail);
  }

  void revert (char *snap_head,
	       char *snap_tail)
  {
    if (unlikely (in_error ())) return;
    assert (snap_head <= head);
    assert (tail <= snap_tail);
    head = snap_head;
    tail = snap_tail;
    discard_stale_objects ();
  }

  void discard_stale_objects ()
  {
    if (unlikely (in_error ())) return;
    while (packed.length > 1 &&
	   packed.tail ()->head < tail)
    {
      packed_map.del (packed.tail ());
      assert (!packed.tail ()->next);
      packed.tail ()->fini ();
      packed.pop ();
    }
    if (packed.length > 1)
      assert (packed.tail ()->head == tail);
  }

  // Adds a virtual link from the current object to objidx. A virtual link is not associated with
  // an actual offset field. They are solely used to enforce ordering constraints between objects.
  // Adding a virtual link from object a to object b will ensure that object b is always packed after
  // object a in the final serialized order.
  //
  // This is useful in certain situations where there needs to be a specific ordering in the
  // final serialization. Such as when platform bugs require certain orderings, or to provide
  //  guidance to the repacker for better offset overflow resolution.
  void add_virtual_link (objidx_t objidx)
  {
    if (unlikely (in_error ())) return;

    if (!objidx)
      return;

    assert (current);

    auto& link = *current->virtual_links.push ();
    if (current->virtual_links.in_error ())
      err (HB_SERIALIZE_ERROR_OTHER);

    link.width = 0;
    link.objidx = objidx;
    link.is_signed = 0;
    link.whence = 0;
    link.position = 0;
    link.bias = 0;
  }

  template <typename T>
  void add_link (T &ofs, objidx_t objidx,
		 whence_t whence = Head,
		 unsigned bias = 0)
  {
    if (unlikely (in_error ())) return;

    if (!objidx)
      return;

    assert (current);
    assert (current->head <= (const char *) &ofs);

    auto& link = *current->real_links.push ();
    if (current->real_links.in_error ())
      err (HB_SERIALIZE_ERROR_OTHER);

    link.width = sizeof (T);
    link.objidx = objidx;
    if (unlikely (!sizeof (T)))
    {
      // This link is not associated with an actual offset and exists merely to enforce
      // an ordering constraint.
      link.is_signed = 0;
      link.whence = 0;
      link.position = 0;
      link.bias = 0;
      return;
    }

    link.is_signed = std::is_signed<hb_unwrap_type (T)>::value;
    link.whence = (unsigned) whence;
    link.position = (unsigned int)((const char *) &ofs - current->head);
    link.bias = bias;
  }

  unsigned to_bias (const void *base) const
  {
    if (unlikely (in_error ())) return 0;
    if (!base) return 0;
    assert (current);
    assert (current->head <= (const char *) base);
    return (unsigned)((const char *)base - current->head);
  }

  void resolve_links ()
  {
    if (unlikely (in_error ())) return;

    assert (!current);
    assert (packed.length > 1);

    for (const object_t* parent : ++hb_iter (packed))
      for (const object_t::link_t &link : parent->real_links)
      {
	const object_t* child = packed[link.objidx];
	if (unlikely (!child)) { err (HB_SERIALIZE_ERROR_OTHER); return; }
	unsigned offset = 0;
	switch ((whence_t) link.whence) {
	case Head:     offset = (unsigned int)(child->head - parent->head); break;
	case Tail:     offset = (unsigned int)(child->head - parent->tail); break;
	case Absolute: offset = (unsigned int)((head - start) + (child->head - tail)); break;
	}

	assert (offset >= link.bias);
	offset -= link.bias;
	if (link.is_signed)
	{
	  assert (link.width == 2 || link.width == 4);
	  if (link.width == 4)
	    assign_offset<int32_t> (parent, link, offset);
	  else
	    assign_offset<int16_t> (parent, link, offset);
	}
	else
	{
	  assert (link.width == 2 || link.width == 3 || link.width == 4);
	  if (link.width == 4)
	    assign_offset<uint32_t> (parent, link, offset);
	  else if (link.width == 3)
	    assign_offset<uint32_t, 3> (parent, link, offset);
	  else
	    assign_offset<uint16_t> (parent, link, offset);
	}
      }
  }

  unsigned int length () const
  {
    if (unlikely (!current)) return 0;
    return (unsigned int)(this->head - current->head);
  }

  void align (unsigned int alignment)
  {
    unsigned int l = length () % alignment;
    if (l)
      allocate_size<void> ((size_t)(alignment - l));
  }

  template <typename Type = void>
  Type *start_embed (const Type *obj HB_UNUSED = nullptr) const
  { return reinterpret_cast<Type *> (this->head); }
  template <typename Type>
  Type *start_embed (const Type &obj) const
  { return start_embed (std::addressof (obj)); }

  bool err (hb_serialize_error_t err_type)
  {
    return !bool ((errors = (errors | err_type)));
  }

  template <typename Type>
  Type *allocate_size (size_t size)
  {
    if (unlikely (in_error ())) return nullptr;

    if (unlikely (size > INT_MAX || this->tail - this->head < ptrdiff_t (size)))
    {
      err (HB_SERIALIZE_ERROR_OUT_OF_ROOM);
      return nullptr;
    }
    hb_memset (this->head, 0, (unsigned int)size);
    char *ret = this->head;
    this->head += size;
    return reinterpret_cast<Type *> (ret);
  }

  template <typename Type>
  Type *allocate_min ()
  { return this->allocate_size<Type> (Type::min_size); }

  template <typename Type>
  Type *embed (const Type *obj)
  {
    unsigned int size = obj->get_size ();
    Type *ret = this->allocate_size<Type> (size);
    if (unlikely (!ret)) return nullptr;
    memcpy (ret, obj, size);
    return ret;
  }
  template <typename Type>
  Type *embed (const Type &obj)
  { return embed (std::addressof (obj)); }

  template <typename Type, typename ...Ts> auto
  _copy (const Type &src, hb_priority<1>, Ts&&... ds) HB_RETURN
  (Type *, src.copy (this, std::forward<Ts> (ds)...))

  template <typename Type> auto
  _copy (const Type &src, hb_priority<0>) -> decltype (&(hb_declval<Type> () = src))
  {
    Type *ret = this->allocate_size<Type> (sizeof (Type));
    if (unlikely (!ret)) return nullptr;
    *ret = src;
    return ret;
  }

  /* Like embed, but active: calls obj.operator=() or obj.copy() to transfer data
   * instead of memcpy(). */
  template <typename Type, typename ...Ts>
  Type *copy (const Type &src, Ts&&... ds)
  { return _copy (src, hb_prioritize, std::forward<Ts> (ds)...); }
  template <typename Type, typename ...Ts>
  Type *copy (const Type *src, Ts&&... ds)
  { return copy (*src, std::forward<Ts> (ds)...); }

  template<typename Iterator,
	   hb_requires (hb_is_iterator (Iterator)),
	   typename ...Ts>
  void copy_all (Iterator it, Ts&&... ds)
  { for (decltype (*it) _ : it) copy (_, std::forward<Ts> (ds)...); }

  template <typename Type>
  hb_serialize_context_t& operator << (const Type &obj) & { embed (obj); return *this; }

  template <typename Type>
  Type *extend_size (Type *obj, size_t size)
  {
    if (unlikely (in_error ())) return nullptr;

    assert (this->start <= (char *) obj);
    assert ((char *) obj <= this->head);
    assert ((size_t) (this->head - (char *) obj) <= size);
    if (unlikely (((char *) obj + size < (char *) obj) ||
		  !this->allocate_size<Type> (((char *) obj) + size - this->head))) return nullptr;
    return reinterpret_cast<Type *> (obj);
  }
  template <typename Type>
  Type *extend_size (Type &obj, size_t size)
  { return extend_size (std::addressof (obj), size); }

  template <typename Type>
  Type *extend_min (Type *obj) { return extend_size (obj, obj->min_size); }
  template <typename Type>
  Type *extend_min (Type &obj) { return extend_min (std::addressof (obj)); }

  template <typename Type, typename ...Ts>
  Type *extend (Type *obj, Ts&&... ds)
  { return extend_size (obj, obj->get_size (std::forward<Ts> (ds)...)); }
  template <typename Type, typename ...Ts>
  Type *extend (Type &obj, Ts&&... ds)
  { return extend (std::addressof (obj), std::forward<Ts> (ds)...); }

  /* Output routines. */
  hb_bytes_t copy_bytes () const
  {
    assert (successful ());
    /* Copy both items from head side and tail side... */
    unsigned int len = (unsigned int)((this->head - this->start)
		     + (this->end  - this->tail));

    // If len is zero don't hb_malloc as the memory won't get properly
    // cleaned up later.
    if (!len) return hb_bytes_t ();

    char *p = (char *) hb_malloc (len);
    if (unlikely (!p)) return hb_bytes_t ();

    memcpy (p, this->start, this->head - this->start);
    memcpy (p + (this->head - this->start), this->tail, this->end - this->tail);
    return hb_bytes_t (p, len);
  }
  template <typename Type>
  Type *copy () const
  { return reinterpret_cast<Type *> ((char *) copy_bytes ().arrayZ); }
  hb_blob_t *copy_blob () const
  {
    hb_bytes_t b = copy_bytes ();
    return hb_blob_create (b.arrayZ, b.length,
			   HB_MEMORY_MODE_WRITABLE,
			   (char *) b.arrayZ, hb_free);
  }

  const hb_vector_t<object_t *>& object_graph() const
  { return packed; }

  private:
  template <typename T, unsigned Size = sizeof (T)>
  void assign_offset (const object_t* parent, const object_t::link_t &link, unsigned offset)
  {
    auto &off = * ((BEInt<T, Size> *) (parent->head + link.position));
    assert (0 == off);
    check_assign (off, offset, HB_SERIALIZE_ERROR_OFFSET_OVERFLOW);
  }

  public:
  char *start, *head, *tail, *end;
  unsigned int debug_depth;
  hb_serialize_error_t errors;

  private:

  void merge_virtual_links (const object_t* from, objidx_t to_idx) {
    object_t* to = packed[to_idx];
    for (const auto& l : from->virtual_links) {
      to->virtual_links.push (l);
    }
  }

  /* Object memory pool. */
  hb_pool_t<object_t> object_pool;

  /* Stack of currently under construction objects. */
  object_t *current;

  /* Stack of packed objects.  Object 0 is always nil object. */
  hb_vector_t<object_t *> packed;

  /* Map view of packed objects. */
  hb_hashmap_t<const object_t *, objidx_t> packed_map;
};

#endif /* HB_SERIALIZE_HH */
