/*
 * Copyright (c) 2001, 2017, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "gc/g1/heapRegion.hpp"
#include "gc/g1/g1NumaSpace.hpp"
#include "gc/shared/collectedHeap.hpp"
#include "gc/shared/spaceDecorator.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/atomic.hpp"
#include "runtime/thread.inline.hpp"
#include "utilities/align.hpp"

G1NUMASpace::G1NUMASpace(uint hrm_index,
             G1BlockOffsetTable* bot,
             MemRegion mr)
        : HeapRegion(hrm_index, bot, mr)
{
}

void G1NUMASpace::initialize(MemRegion mr, bool clear_space, bool mangle_space)
{
    HeapRegion::initialize(mr, clear_space, mangle_space);
    
    Thread* thr = Thread::current();
    int lgrp_id = thr->lgrp_id();
    if (lgrp_id == -1 || !os::numa_has_group_homing()) 
    {
        lgrp_id = os::numa_get_group_id();
        thr->set_lgrp_id(lgrp_id);
    }
  
    set_page_size(os::vm_page_size());
    bias_region(mr, os::numa_get_group_id());
}

// Bias region towards the first-touching lgrp. Set the right page sizes.
void G1NUMASpace::bias_region(MemRegion mr, int lgrp_id) {
  HeapWord *start = align_up(mr.start(), GrainBytes);
  HeapWord *end = align_down(mr.end(), GrainBytes);
  if (end > start) {
    MemRegion aligned_region(start, end);
    assert((intptr_t)aligned_region.start()     % GrainBytes == 0 &&
           (intptr_t)aligned_region.byte_size() % GrainBytes == 0, "Bad alignment");
    assert(region().contains(aligned_region), "Sanity");
    // First we tell the OS which page size we want in the given range. The underlying
    // large page can be broken down if we require small pages.
    os::realign_memory((char*)aligned_region.start(), aligned_region.byte_size(), GrainBytes);
    // Then we uncommit the pages in the range.
    os::free_memory((char*)aligned_region.start(), aligned_region.byte_size(), GrainBytes);
    // And make them local/first-touch biased.
    os::numa_make_local((char*)aligned_region.start(), aligned_region.byte_size(), lgrp_id);
  }
}

// Free all pages in the region.
void G1NUMASpace::free_region(MemRegion mr) {
  HeapWord *start = align_up(mr.start(), GrainBytes);
  HeapWord *end = align_down(mr.end(), GrainBytes);
  if (end > start) {
    MemRegion aligned_region(start, end);
    assert((intptr_t)aligned_region.start()     % GrainBytes == 0 &&
           (intptr_t)aligned_region.byte_size() % GrainBytes == 0, "Bad alignment");
    assert(region().contains(aligned_region), "Sanity");
    os::free_memory((char*)aligned_region.start(), aligned_region.byte_size(), GrainBytes);
  }
}