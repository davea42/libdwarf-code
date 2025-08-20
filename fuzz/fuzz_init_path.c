/* Copyright 2021 Google LLC
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Libdwarf library callers can only use these headers.
 */
#include "dwarf.h"
#include "libdwarf.h"

/*
 * A fuzzer that simulates a small part of the simplereader.c example.
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  char filename[256];

#ifdef DWREGRESSIONTEMP
  /*  Under msys2, the /tmp/ results in an open fail,
      so we discard the /tmp/ here */
  sprintf(filename, "junklibfuzzer.%d", getpid());
#else
  sprintf(filename, "/tmp/libfuzzer.%d", getpid());
#endif
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    printf("FAIL libfuzzer cannot open temp as writeable %s\n",
        filename);
    return 0;
  }
  fwrite(data, size, 1, fp);
  fclose(fp);
  Dwarf_Ptr errarg = 0;
  Dwarf_Handler errhand = 0;
  Dwarf_Debug dbg = 0;
  Dwarf_Error *errp = NULL;
#define MACHO_PATH_LEN 2000
  char macho_real_path[2000];
  dwarf_init_path(filename, macho_real_path, MACHO_PATH_LEN, DW_GROUPNUMBER_ANY,
                  errhand, errarg, &dbg, errp);

  dwarf_finish(dbg);
  static const char *glpath[1] = {"/usr/include/c++/9/debug"};
  unsigned char path_source = 0;
  dwarf_init_path_dl(filename, NULL, 256, DW_GROUPNUMBER_ANY, errhand, errarg,
                     &dbg, (char **)glpath, 1, &path_source, errp);
  dwarf_finish(dbg);
  unlink(filename);
  return 0;
}
