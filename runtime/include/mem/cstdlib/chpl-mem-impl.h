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

/* cstdlib memory function implementation */
#ifndef _chpl_mem_impl_H_
#define _chpl_mem_impl_H_

// Uses the built-in malloc, calloc, realloc and free.

#include <stdlib.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#endif

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef _chpl_mem_warning_macros_h_

#include <stdlib.h>

#ifdef __GLIBC__
// get memalign
#include <malloc.h>
#endif

static inline void* chpl_calloc(size_t n, size_t size) {
  return calloc(n,size);
}

static inline void* chpl_malloc(size_t size) {
  return malloc(size);
}

static inline void* chpl_memalign(size_t boundary, size_t size) {
#ifdef __GLIBC__
  return memalign(boundary, size);
#else
  void* ret = NULL;
  int rc;
  rc = posix_memalign(&ret, boundary, size);
  if( rc == 0 ) return ret;
  else return NULL;
#endif
}

static inline void* chpl_realloc(void* ptr, size_t size) {
  return realloc(ptr,size);
}

static inline void chpl_free(void* ptr) {
  free(ptr);
}

// malloc_good_size is OSX specifc unfortunately. On other platforms just
// return minSize.
static inline size_t chpl_goodAllocSize(size_t minSize) {
#if defined(__APPLE__)
  return malloc_good_size(minSize);
#else
  return minSize;
#endif
}

// Now that we've defined our functions, turn the warnings back on.
#include "chpl-mem-warning-macros.h"

#define CHPL_USING_CSTDLIB_MALLOC 1


#endif
