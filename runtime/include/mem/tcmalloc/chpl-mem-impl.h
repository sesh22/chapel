/*
 * Copyright 2004-2016 Cray Inc.
 * Other additional copyright holders may be indicated within.
 * 
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 * 
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* tcmalloc memory function implementation */
#ifndef _chpl_mem_impl_H_
#define _chpl_mem_impl_H_


#include "tcmalloc.h"
#include "malloc_extension_c.h"

static inline void* chpl_calloc(size_t n, size_t size) {
  return tc_calloc(n,size);
}

static inline void* chpl_malloc(size_t size) {
  return tc_malloc(size);
}

static inline void* chpl_memalign(size_t boundary, size_t size) {
  return tc_memalign(boundary, size);
}

static inline void* chpl_realloc(void* ptr, size_t size) {
  return tc_realloc(ptr, size);
}

static inline void chpl_free(void* ptr) {
  tc_free(ptr);
}

static inline size_t chpl_goodAllocSize(size_t minSize) {
  return MallocExtension_GetEstimatedAllocatedSize(minSize);
}

#endif
