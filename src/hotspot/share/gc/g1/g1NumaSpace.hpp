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

class G1BlockOffsetTable;

#include "gc/g1/heapRegion.hpp"
#include "gc/shared/gcUtil.hpp"
#include "utilities/macros.hpp"

// A NUMASpace is similar to a contiguous space, but allocates with respect
// to NUMA-wide context

class G1NUMASpace : public HeapRegion {

public:
    
  G1NUMASpace(uint hrm_index,
             G1BlockOffsetTable* bot,
             MemRegion mr);

  virtual void initialize(MemRegion mr, bool clear_space = false, bool mangle_space = SpaceDecorator::Mangle);
  
private:
    
    size_t _page_size;
    
    size_t page_size() const { return _page_size; }
    void set_page_size(size_t newSize) { _page_size = newSize; }
    
    void bias_region(MemRegion mr, int lgrp_id);
    void free_region(MemRegion mr);
};