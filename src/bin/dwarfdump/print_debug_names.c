/*
Copyright 2017-2018 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of version 2 of the GNU General
  Public License as published by the Free Software Foundation.

  This program is distributed in the hope that it would be
  useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  Further, this software is distributed without any warranty
  that it is free of the rightful claim of any third person
  regarding infringement or the like.  Any license provided
  herein, whether implied or otherwise, applies only to this
  software file.  Patent licenses, if any, provided herein
  do not apply to combinations of this program with other
  software, or any other product whatsoever.

  You should have received a copy of the GNU General Public
  License along with this program; if not, write the Free
  Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
  Boston MA 02110-1301, USA.

*/

#include <config.h>

#include <string.h>
#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_globals.h"
#include "dd_naming.h"
#include "dd_sanitized.h"
#include "dd_esb.h"
#include "dd_esb_using_functions.h"

#define ATTR_ARRAY_SIZE 10

#if 0
static void
dump_table(const char *msg,unsigned long count,
    Dwarf_Half *idx, Dwarf_Half *form)
{
    unsigned long i = 0;
    for ( ; i < count;    ++i) {
        printf("[%lu] %x %x\n",i,idx[i],form[i]);
    }
}
#endif

static void
printindent(unsigned int l)
{
    unsigned int i = 0;
    for ( ; i < l; ++i) {
        printf(" ");
    }
}

static const Dwarf_Sig8 zerosig;
/*   The table entries here are indexed starting at 1 */
static int
print_cu_table(unsigned int indent,Dwarf_Dnames_Head dn,
    const char *type,
    Dwarf_Unsigned offset_count,
    Dwarf_Unsigned sig_count,
    Dwarf_Error *error)
{
    Dwarf_Unsigned i = 1;
    Dwarf_Unsigned totalcount = offset_count+sig_count;
    int res = 0;
    Dwarf_Bool formtu = FALSE;

    if (type[0] == 't' && type[1] == 'u' &&
        type[2] == 0) {
        formtu = TRUE;
    } else if (type[0] == 'c' && type[1] == 'u' &&
        type[2] == 0) {
        formtu = FALSE;
    } else {
        printf("ERROR: Calling print_cu_table with type"
            "%s is invalid. Must be tu or cu ."
            "Not printing this cu table set\n",
            type);
        glflags.gf_count_major_errors++;
        return DW_DLV_NO_ENTRY;
    }
    if (formtu) {
        printindent(indent);
        printf("%s List. Entry count: %" DW_PR_DUu
        " (local tu count %" DW_PR_DUu
        ",foreign tu count %" DW_PR_DUu
        ")\n",type,totalcount,offset_count,sig_count);
    } else {
        printindent(indent);
        printf("%s List. Entry count: %" DW_PR_DUu "\n",
            type,totalcount);
    }
    for (i = 1 ; i <= totalcount; ++i) {
        Dwarf_Unsigned offset = 0;
        Dwarf_Sig8     signature;

        signature = zerosig;
        res = dwarf_dnames_cu_table(dn,type,i,
            &offset, &signature,error);
        if (res == DW_DLV_NO_ENTRY) {
            break;
        }
        if (res == DW_DLV_ERROR) {
            return res;
        }
        /*  Could equally test for non-zero offset here. */
        if (i <= offset_count) {
            printindent(indent);
            printf("[%4" DW_PR_DUu "] ",i);
            printf("CU-offset:  0x%"
                DW_PR_XZEROS DW_PR_DUx "\n",
                offset);
            continue;
        }

        {
            static char sigarea[32];
            struct esb_s sig8str;

            esb_constructor_fixed(&sig8str,sigarea,sizeof(sigarea));
            format_sig8_string(&signature,&sig8str);
            printindent(indent);
            printf("[%4" DW_PR_DUu "] " ,i);
            printf("Signature:  %s\n",esb_get_string(&sig8str));
            esb_destructor(&sig8str);
        }
    }
    return DW_DLV_OK;
}

static int
print_buckets(unsigned int indent,Dwarf_Dnames_Head dn,
    Dwarf_Unsigned bucket_count,
    Dwarf_Error *error)
{
    int res = 0;
    Dwarf_Unsigned bn = 0;

    if (!bucket_count) {
        return DW_DLV_NO_ENTRY;
    }

    printf("\n");
    printindent(indent);
    printf("Bucket (hash) table entry count: "
        "%" DW_PR_DUu "\n",bucket_count);
    printindent(indent);
    printf("[ ]    nameindex collisioncount\n");
    for ( ; bn < bucket_count; ++bn) {
        Dwarf_Unsigned name_index = 0;
        Dwarf_Unsigned collision_count = 0;

        res = dwarf_dnames_bucket(dn,bn,&name_index,
            &collision_count,error);
        if (res == DW_DLV_ERROR) {
            return res;
        }
        if (res == DW_DLV_NO_ENTRY) {
            break;
        }
        printindent(indent);
        printf("[ %3" DW_PR_DUu "] ",bn);
        printf("%6" DW_PR_DUu " %6" DW_PR_DUu ,
            name_index,
            collision_count);
        printf("\n");
    }
    printf("\n");
    return DW_DLV_OK;
}

static void
P_Entry(const char * leader,Dwarf_Unsigned value)
{
    printf("   %13s  0x%" DW_PR_XZEROS DW_PR_DUx
        " (%8" DW_PR_DUu
        ")\n",
        leader,value,value);
}
static int
print_dnames_offsets(unsigned int indent,Dwarf_Dnames_Head dn,
    Dwarf_Error *error)
{
    Dwarf_Unsigned header_offset = 0;
    Dwarf_Unsigned cu_table_offset = 0;
    Dwarf_Unsigned tu_local_offset = 0;
    Dwarf_Unsigned foreign_tu_offset = 0;
    Dwarf_Unsigned buckets_offset = 0;
    Dwarf_Unsigned hashes_offset = 0;
    Dwarf_Unsigned stringoffsets_offset = 0;
    Dwarf_Unsigned entryoffsets_offset = 0;
    Dwarf_Unsigned abbrev_table_offset = 0;
    Dwarf_Unsigned entry_pool_offset = 0;
    int res = 0;

    printf("\n");
    printindent(indent);
    printf("Table Offsets \n");
    res = dwarf_dnames_offsets(dn,&header_offset,
        &cu_table_offset, &tu_local_offset, &foreign_tu_offset,
        &buckets_offset, &hashes_offset,
        &stringoffsets_offset,&entryoffsets_offset,
        &abbrev_table_offset,&entry_pool_offset,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    P_Entry("Header        :",header_offset);
    P_Entry("CU table      :",cu_table_offset);
    P_Entry("TU local      :",tu_local_offset);
    P_Entry("Foreign TU    :",foreign_tu_offset);
    P_Entry("Buckets       :",buckets_offset);
    P_Entry("Hashes        :",hashes_offset);
    P_Entry("String Offsets:",stringoffsets_offset);
    P_Entry("Entry Offsets :",entryoffsets_offset);
    P_Entry("Abbrev Table  :",abbrev_table_offset);
    P_Entry("Entry Pool:   :",entry_pool_offset);
    return DW_DLV_OK;
}

static int
print_dnames_abbrevtable(unsigned int indent,Dwarf_Dnames_Head dn,
    Dwarf_Unsigned table_length)
{
    int res = 0;
    Dwarf_Unsigned abbrev_offset     = 0;
    Dwarf_Unsigned abbrev_code       = 0;
    Dwarf_Unsigned abbrev_tag        = 0;
    Dwarf_Unsigned array_size        = ATTR_ARRAY_SIZE;
    static Dwarf_Half idxattr_array[ATTR_ARRAY_SIZE];
    static Dwarf_Half form_array[ATTR_ARRAY_SIZE];
    Dwarf_Unsigned actual_attr_count = 0;
    Dwarf_Unsigned i                 = 0;

    printf("\n");
    printindent(indent);
    printf("Debug Names abbreviation table: length %"
        DW_PR_DUu " bytes.\n", table_length);
    printindent(indent);
    printf("[] offset   code"
        "            count idxattr\n");
    res = DW_DLV_OK;
    for (  ; res == DW_DLV_OK; ++i) {
        Dwarf_Unsigned limit = 0;
        Dwarf_Unsigned k     = 0;
        const char *tagname = "<TAGunknown>";

        /*  Never returns DW_DLV_ERROR */
        res = dwarf_dnames_abbrevtable(dn,i,
            &abbrev_offset,
            &abbrev_code, &abbrev_tag,
            array_size,
            idxattr_array,form_array,
            &actual_attr_count);
        if (res == DW_DLV_NO_ENTRY) {
            break;
        }
        dwarf_get_TAG_name(abbrev_tag,&tagname);
        printindent(indent+2);
        printf("[%4" DW_PR_DUu "] ",i);
        printf("     0x%" DW_PR_XZEROS DW_PR_DUx " ",abbrev_offset);
        printf("     0x%05" DW_PR_DUx "\n",abbrev_code);
        printf("     %3" DW_PR_DUu " ",actual_attr_count);
        printindent(indent+12);
        printf("     0x%04" DW_PR_DUx " %-16s",abbrev_tag,tagname);
        printf("\n");
        limit = actual_attr_count;
        if (limit > ATTR_ARRAY_SIZE) {
            printf("   ERROR: allowed %" DW_PR_DUu " pairs,"
                " But we have %" DW_PR_DUu "pairs!\n",
                array_size, actual_attr_count);
            glflags.gf_count_major_errors++;
        }
        printindent(indent);
        printf("[]     idxattr  form \n");
        for (k = 0; k < limit; ++k) {
            const char *idname = "<unknownidx>";
            const char *formname = "<unknownform>";
            Dwarf_Half a = idxattr_array[k];
            Dwarf_Half f = form_array[k];

            printindent(indent);
            printf("[%4" DW_PR_DUu "] ",k);
            printf("0x%04x ",a);
            printf("0x%04x ",f);
            if (a || f) {
                dwarf_get_IDX_name(a,&idname);
                printf("%-19s",idname);
                dwarf_get_FORM_name(f,&formname);
                printf("%15s",formname);
                if (! (a && f)){
                    printf("ERROR: improper idx/form pair!");
                    glflags.gf_count_major_errors++;
                }
            } else {
                printf(" (end of list)");
            }
            printf("\n");
        }
    }
    return DW_DLV_OK;
}

static int
print_attr_array(unsigned int indent,
    Dwarf_Unsigned attr_count,
    Dwarf_Unsigned array_size,
    Dwarf_Half *   idxattr_array,
    Dwarf_Half *   form_array)
{
    Dwarf_Unsigned k = 0;
    Dwarf_Unsigned count = attr_count;
    if (!attr_count) {
        printindent(indent);
        printf("No idxattr/form content available\n");
        return DW_DLV_NO_ENTRY;
    }
    if (array_size < attr_count) {
        printindent(indent);
        printf("Array size %" DW_PR_DUu
            " but count is %" DW_PR_DUu
            " so some entries not available\n",
            array_size,attr_count);
        count = array_size;
    }
    printindent(indent);
    printf("[]    idxnum formnum    idxname            formname\n");
    for ( ; k < count; ++k) {
        const char *idname = 0;
        const char *formname = 0;
        Dwarf_Half a = idxattr_array[k];
        Dwarf_Half f = form_array[k];

        printindent(indent);
        printf("[%3" DW_PR_DUu "]", k);
        printf(" 0x%04u 0x%04u", a,f);
        if (k == (count-1)) {
            if (a || f) {
                printf(" ERROR: last entry should be 0,0"
                    "not 0x%x a 0x%xf \n",a,f);
                glflags.gf_count_major_errors++;
                break;
            } else {
                printf(" (end of list)\n");
                continue;
            }
        }
        dwarf_get_IDX_name(a,&idname);
        printf(" %-19s",idname);
        dwarf_get_FORM_name(f,&formname);
        printf("%15s",formname);
        printf("\n");
    }
    return DW_DLV_OK;
}

#define MAXPAIRS 7 /* The standard defines 5.*/
static Dwarf_Half     idx_array[MAXPAIRS];
static Dwarf_Half     form_array[MAXPAIRS];
static Dwarf_Unsigned offsets_array[MAXPAIRS];
static Dwarf_Sig8     signatures_array[MAXPAIRS];

static int
print_name_values(unsigned int indent,Dwarf_Dnames_Head dn ,
    Dwarf_Unsigned name_index ,
    Dwarf_Unsigned offset_in_entrypool ,
    Dwarf_Error * error)
{
    int res = 0;
    Dwarf_Unsigned abbrev_code = 0;
    Dwarf_Half     tag         = 0;
    Dwarf_Unsigned value_count = 0;
    Dwarf_Unsigned index_of_abbrev = 0;
    Dwarf_Unsigned offset_of_initial_value = 0;
    Dwarf_Bool     single_cu_case = 0;
    Dwarf_Unsigned single_cu_offset = 0;
    Dwarf_Unsigned offset_next_entry_pool = 0;
    const char    *idname = "<DW_IDX-unknown>";
    Dwarf_Unsigned i = 0;
    const char    *tagname = "<TAGunknown";

    res = dwarf_dnames_entrypool(dn,
        offset_in_entrypool,
        &abbrev_code,&tag,&value_count,&index_of_abbrev,
        &offset_of_initial_value,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    printindent(indent);
    dwarf_get_TAG_name(tag,&tagname);
    printf(
        "Nameindex %6" DW_PR_DUu
        " abbrevcode %4" DW_PR_DUu
        " abbrevindex %4" DW_PR_DUu
        "\n",
        name_index,
        abbrev_code,
        index_of_abbrev);
    printindent(indent);
    printf(
        "Tag 0x%04x %-16s\n",
        tag,tagname);
    printindent(indent);
    printf(
        "Valuecount %4" DW_PR_DUu
        " valuesoffset 0x%04" DW_PR_DUx
        "\n",
        value_count, offset_of_initial_value);
    if (value_count > MAXPAIRS) {
        printf("ERROR: The number of values in an entrypool entry is"
            " %" DW_PR_DUu
            " but  the max allowed is %d",value_count,MAXPAIRS);
        glflags.gf_count_major_errors++;
        return DW_DLV_OK;
    }
    res = dwarf_dnames_entrypool_values(dn,
        index_of_abbrev,
        offset_of_initial_value,
        value_count,
        idx_array,
        form_array,
        offsets_array,
        signatures_array,
        &single_cu_case,&single_cu_offset,
        &offset_next_entry_pool,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }

    indent += 2;
    printindent(indent);
    printf("Entrypool Values, count %" DW_PR_DUu "\n",value_count);
    if (single_cu_case) {
        printindent(indent);
        printf("Single CU case, CUoffset defaults to: 0x%"
            DW_PR_XZEROS DW_PR_DUx "\n",
            single_cu_offset);
    }
    printindent(indent);
    printf("[ ]  idxattr    idxname           value\n");
    for (i = 0; i < value_count; ++i) {
        Dwarf_Half idx = idx_array[i];

        printindent(indent);
        printf("[%2" DW_PR_DUu "] ",i);

        if (!idx) {
            if (i == (value_count-1)) {
                printf(" 0 (end of list)\n");
                continue;
            }
        }
        dwarf_get_IDX_name(idx,&idname);
        printf("     %2u %-19s ",idx,idname);
        switch(idx) {
        case DW_IDX_compile_unit:
            printf(" CUindex= %" DW_PR_DUu ,offsets_array[i]);
            break;
        case DW_IDX_type_unit:
            printf(" typeunitindex= %" DW_PR_DUu ,offsets_array[i]);
            break;
        case DW_IDX_die_offset:
            printf(" DIEoffset= 0x%" DW_PR_XZEROS DW_PR_DUu ,
                offsets_array[i]);
            break;
        case DW_IDX_parent:
            printf(" indexofparent= %" DW_PR_DUu ,offsets_array[i]);
            break;
        case DW_IDX_type_hash: {
            struct esb_s m;

            esb_constructor(&m);
            format_sig8_string((Dwarf_Sig8*)&signatures_array[i],
                &m);
            printf(" typehash= %s",esb_get_string(&m));
            esb_destructor(&m);
            break;
        }
        default: {
            printf("ERROR: idxattr %u is unknown!",idx);
            glflags.gf_count_major_errors++;
        }
        }
        printf("\n");
    }
    return DW_DLV_OK;
}

static int
print_names_table(unsigned int indent,Dwarf_Dnames_Head dn,
    Dwarf_Unsigned name_count,
    Dwarf_Unsigned bucket_count,
    Dwarf_Error * error)
{
    Dwarf_Unsigned i = 1;
    int res                  = 0;
    Dwarf_Unsigned bucketnum = 0;
    Dwarf_Unsigned hashval   = 0;
    Dwarf_Unsigned offset_to_debug_str = 0;
    char * ptrtostr          = 0;
    Dwarf_Unsigned offset_in_entrypool = 0;
    Dwarf_Unsigned abbrev_number = 0;
    Dwarf_Half abbrev_tag    = 0;
    Dwarf_Unsigned array_size = ATTR_ARRAY_SIZE;
    static Dwarf_Half nt_idxattr_array[ATTR_ARRAY_SIZE];
    static Dwarf_Half nt_form_array[ATTR_ARRAY_SIZE];
    Dwarf_Unsigned attr_count = 0;

    memset(nt_idxattr_array,0,sizeof(Dwarf_Half) * ATTR_ARRAY_SIZE);
    memset(nt_form_array,0,sizeof(Dwarf_Half) * ATTR_ARRAY_SIZE);
    printf("\n");
    printindent(indent);
    printf("Names Table, entry count %" DW_PR_DUu "\n",name_count);
    printindent(indent);
    printf("[] ");
    if (bucket_count) {
        printf("    Bucket Hash");
    } else {
    }
    printf("      StrOffset Name\n");
    for ( ; i <= name_count;++i) {
        const char *tagname = "<TAGunknown>";
        printindent(indent);
        res = dwarf_dnames_name(dn,i,
            &bucketnum, &hashval,
            &offset_to_debug_str,&ptrtostr,
            &offset_in_entrypool, &abbrev_number,
            &abbrev_tag,
            array_size, nt_idxattr_array,
            nt_form_array,
            &attr_count,error);
        if (res == DW_DLV_ERROR) {
            return res;
        }
        if (res == DW_DLV_NO_ENTRY) {
            printf("[%4" DW_PR_DUu "] ",i);
            printf("ERROR: NO ENTRY on name index "
                "%" DW_PR_DUu " is impossible ",i);
            glflags.gf_count_major_errors++;
            printf("\n");
            continue;
        }
        printf("[%4" DW_PR_DUu "] ",i);
        if (bucket_count) {
            printf("%5" DW_PR_DUu " ",bucketnum);
            printf("0x%" DW_PR_XZEROS DW_PR_DUx " ",hashval);
        }
        printf("0x%06" DW_PR_DUx , offset_to_debug_str);
        printf(" %s",ptrtostr?sanitized(ptrtostr):"<null>");
        printf("\n");
        printindent(indent+4);
        dwarf_get_TAG_name(abbrev_tag,&tagname);
        printf("entrypool= 0x%06" DW_PR_DUx ,
            offset_in_entrypool);
        printf(" abbrev#= %4" DW_PR_DUu,
            abbrev_number);
        printf(" attrcount= %4" DW_PR_DUu,
            attr_count);
        printf(" arraysz= %4" DW_PR_DUu "\n",
            array_size);
        printindent(indent+4);
        printf("tag= 0x%04x %-16s",
            abbrev_tag,tagname);
        printf("\n");
        if (glflags.verbose) {
            print_attr_array(indent+2,
                attr_count,array_size,
                nt_idxattr_array, nt_form_array);
        }
        res = print_name_values(indent+6,dn,i,offset_in_entrypool,
            error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    return DW_DLV_OK;
}

static int
print_dname_record(Dwarf_Dnames_Head dn,
    Dwarf_Unsigned offset,
    Dwarf_Unsigned new_offset,
    Dwarf_Error *error)
{
    int res = 0;
    Dwarf_Unsigned comp_unit_count = 0;
    Dwarf_Unsigned local_type_unit_count = 0;
    Dwarf_Unsigned foreign_type_unit_count = 0;
    Dwarf_Unsigned bucket_count = 0;
    Dwarf_Unsigned name_count = 0;
    Dwarf_Unsigned abbrev_table_size = 0;
    Dwarf_Unsigned entry_pool_size = 0;
    Dwarf_Unsigned augmentation_string_size = 0;
    Dwarf_Unsigned section_size = 0;
    Dwarf_Half table_version = 0;
    Dwarf_Half offset_size = 0;
    char * augstring = 0;
    unsigned int indent = 0;

    res = dwarf_dnames_sizes(dn,&comp_unit_count,
        &local_type_unit_count,
        &foreign_type_unit_count,
        &bucket_count,
        &name_count,&abbrev_table_size,
        &entry_pool_size,&augmentation_string_size,
        &augstring,
        &section_size,&table_version,
        &offset_size,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    printf("\n");
    printf("Name table offset       : 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n",
        offset);
    printf("Next name table offset  : 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n",
        new_offset);
    printf("Section size            : 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n",
        section_size);
    printf("Table version           : %u\n",
        table_version);
    printf("Comp unit count         : %" DW_PR_DUu "\n",
        comp_unit_count);
    printf("Type unit count         : %" DW_PR_DUu "\n",
        local_type_unit_count);
    printf("Foreign Type unit count : %" DW_PR_DUu "\n",
        foreign_type_unit_count);
    printf("Bucket count            : %" DW_PR_DUu "\n",
        bucket_count);
    printf("Name count              : %" DW_PR_DUu "\n",
        name_count);
    printf("Abbrev table length     : %" DW_PR_DUu "\n",
        abbrev_table_size);
    printf("Entry pool size         : %" DW_PR_DUu "\n",
        entry_pool_size);
    printf("Augmentation string size: %" DW_PR_DUu "\n",
        augmentation_string_size);
    if (augmentation_string_size > 0) {
        printf("Augmentation string     : %s\n",
            sanitized(augstring));
    }
    if (glflags.verbose) {
        res = print_dnames_offsets(indent+2,dn,error);
        if (res == DW_DLV_ERROR) {
            return res;
        }
    }
    if (glflags.verbose >1) {
        print_dnames_abbrevtable(indent+2,dn,
            abbrev_table_size);
    }
    res = print_cu_table(indent+2,dn,"cu",comp_unit_count,
        0,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    res = print_cu_table(indent+2,dn,"tu",local_type_unit_count,
        foreign_type_unit_count,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    res = print_buckets(indent+2,dn,bucket_count,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    res = print_names_table(indent+2,dn,name_count,
        bucket_count,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    return DW_DLV_OK;
}

int
print_debug_names(Dwarf_Debug dbg,Dwarf_Error *error)
{
    Dwarf_Dnames_Head dnhead = 0;
    Dwarf_Unsigned offset = 0;
    Dwarf_Unsigned new_offset = 0;
    int res = 0;
    const char * section_name = ".debug_names";
    struct esb_s truename;
    char buf[DWARF_SECNAME_BUFFER_SIZE];

    if (!dbg) {
        printf("ERROR: Cannot print .debug_names, "
            "no Dwarf_Debug passed in");
        return DW_DLV_NO_ENTRY;
    }
    glflags.current_section_id = DEBUG_NAMES;
    /* Do nothing if not printing. */
    if (!glflags.gf_do_print_dwarf) {
        return DW_DLV_OK;
    }

    /*  Only print anything if we know it has debug names
        present. And for now there is none. FIXME. */
    res = dwarf_dnames_header(dbg,offset,&dnhead,&new_offset,error);
    if (res == DW_DLV_NO_ENTRY) {
        return res;
    }
    esb_constructor_fixed(&truename,buf,sizeof(buf));
    get_true_section_name(dbg,section_name,
        &truename,TRUE);
    printf("\n%s\n",sanitized(esb_get_string(&truename)));
    esb_destructor(&truename);

    while (res == DW_DLV_OK) {
        res = print_dname_record(dnhead,offset,new_offset,error);
        if (res != DW_DLV_OK) {
            dwarf_dealloc_dnames(dnhead);
            dnhead = 0;
            return res;
        }
        offset = new_offset;
        dwarf_dealloc_dnames(dnhead);
        dnhead = 0;
        res = dwarf_dnames_header(dbg,offset,&dnhead,
            &new_offset,error);
    }
    return res;
}
