/*
   Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
   Portions Copyright (C) 2007-2013 David Anderson. All Rights Reserved.
   Portions Copyright (C) 2010-2012 SN Systems Ltd. All Rights Reserved.
   Portions Copyright (C) 2015-2015 Google, Inc. All Rights Reserved.

   This program is free software; you can redistribute it and/or modify it
   under the terms of version 2.1 of the GNU Lesser General Public License
   as published by the Free Software Foundation.

   This program is distributed in the hope that it would be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

   Further, this software is distributed without any warranty that it is
   free of the rightful claim of any third person regarding infringement
   or the like.  Any license provided herein, whether implied or
   otherwise, applies only to this software file.  Patent licenses, if
   any, provided herein do not apply to combinations of this program with
   other software, or any other product whatsoever.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston MA 02110-1301,
   USA.

*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_line.h"

/* Line Register Set initial conditions. */
static struct Dwarf_Line_Registers_s _dwarf_line_table_regs_default_values = {
    /* Dwarf_Addr lr_address */ 0,
    /* Dwarf_Word lr_file */ 1,
    /* Dwarf_Word lr_line */  1,
    /* Dwarf_Word lr_column */  0,
    /* Dwarf_Bool lr_is_stmt */  false,
    /* Dwarf_Bool lr_basic_block */  false,
    /* Dwarf_Bool lr_end_sequence */  false,
    /* Dwarf_Bool lr_prologue_end */  false,
    /* Dwarf_Bool lr_epilogue_begin */  false,
    /* Dwarf_Small lr_isa */  0,
    /* Dwarf_Unsigned lr_op_index  */  0,
    /* Dwarf_Unsigned lr_discriminator */  0,
    /* Dwarf_Unsigned lr_call_context */  0,
    /* Dwarf_Unsigned lr_subprogram */  0,
};

void
_dwarf_set_line_table_regs_default_values(Dwarf_Line_Registers regs,
    Dwarf_Bool is_stmt)
{
    *regs = _dwarf_line_table_regs_default_values;
    regs->lr_is_stmt = is_stmt;
}


static int
is_path_separator(Dwarf_Small s)
{
    if (s == '/') {
        return 1;
    }
#ifdef HAVE_WINDOWS_PATH
    if (s == '\\') {
        return 1;
    }
#endif
    return 0;
}

/*  Return 0 if false, 1 if true.
    If HAVE_WINDOWS_PATH is defined we
    attempt to handle windows full paths:
    \\something   or  C:cwdpath.c
*/
static int
file_name_is_full_path(Dwarf_Small  *fname)
{
    Dwarf_Small firstc = *fname;
    if (is_path_separator(firstc)) {
        /* Full path. */
        return 1;
    }
    if (!firstc) {
        return 0;
    }
#ifdef HAVE_WINDOWS_PATH
    if ((firstc >= 'A' && firstc <= 'Z') ||
        (firstc >= 'a' && firstc <= 'z')) {

        Dwarf_Small secondc = fname[1];
        if (secondc == ':') {
            return 1;
        }
    }
#endif
    return 0;
}

/*  Although source files is supposed to return the
    source files in the compilation-unit, it does
    not look for any in the statement program.  In
    other words, it ignores those defined using the
    extended opcode DW_LNE_define_file.  */
int
dwarf_srcfiles(Dwarf_Die die,
    char ***srcfiles,
    Dwarf_Signed * srcfilecount, Dwarf_Error * error)
{
    /*  This pointer is used to scan the portion of the .debug_line
        section for the current cu. */
    Dwarf_Small *line_ptr;

    /*  Pointer to a DW_AT_stmt_list attribute in case it exists in the
        die. */
    Dwarf_Attribute stmt_list_attr;

    /*  Pointer to DW_AT_comp_dir attribute in die. */
    Dwarf_Attribute comp_dir_attr;

    /*  Pointer to name of compilation directory. */
    Dwarf_Small *comp_dir = 0;

    /*  Offset into .debug_line specified by a DW_AT_stmt_list
        attribute. */
    Dwarf_Unsigned line_offset = 0;

    /*  This points to a block of char *'s, each of which points to a
        file name. */
    char **ret_files = 0;

    /*  The Dwarf_Debug this die belongs to. */
    Dwarf_Debug dbg = 0;
    Dwarf_CU_Context context = 0;

    /*  Used to chain the file names. */
    Dwarf_Chain curr_chain = NULL;
    Dwarf_Chain prev_chain = NULL;
    Dwarf_Chain head_chain = NULL;
    Dwarf_Half attrform = 0;
    int resattr = DW_DLV_ERROR;
    int lres = DW_DLV_ERROR;
    struct Line_Table_Prefix_s line_prefix;
    unsigned i = 0;
    int res = DW_DLV_ERROR;

    /*  ***** BEGIN CODE ***** */
    /*  Reset error. */
    if (error != NULL) {
        *error = NULL;
    }

    CHECK_DIE(die, DW_DLV_ERROR);
    context = die->di_cu_context;
    dbg = context->cc_dbg;

    resattr = dwarf_attr(die, DW_AT_stmt_list, &stmt_list_attr, error);
    if (resattr != DW_DLV_OK) {
        return resattr;
    }

    if (dbg->de_debug_line.dss_index == 0) {
        _dwarf_error(dbg, error, DW_DLE_DEBUG_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    res = _dwarf_load_section(dbg, &dbg->de_debug_line,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    if (!dbg->de_debug_line.dss_size) {
        return (DW_DLV_NO_ENTRY);
    }


    lres = dwarf_whatform(stmt_list_attr,&attrform,error);
    if (lres != DW_DLV_OK) {
        return lres;
    }
    if (attrform != DW_FORM_data4 && attrform != DW_FORM_data8 &&
        attrform != DW_FORM_sec_offset  &&
        attrform != DW_FORM_GNU_ref_alt) {
        _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    lres = dwarf_global_formref(stmt_list_attr, &line_offset, error);
    if (lres != DW_DLV_OK) {
        return lres;
    }
    if (line_offset >= dbg->de_debug_line.dss_size) {
        _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    line_ptr = dbg->de_debug_line.dss_data + line_offset;
    {
        Dwarf_Unsigned fission_offset = 0;
        Dwarf_Unsigned fission_size = 0;
        int res = _dwarf_get_fission_addition_die(die, DW_SECT_LINE,
            &fission_offset,&fission_size,error);
        if(res != DW_DLV_OK) {
            return res;
        }
        line_ptr += fission_offset;
    }
    dwarf_dealloc(dbg, stmt_list_attr, DW_DLA_ATTR);

    /*  If die has DW_AT_comp_dir attribute, get the string that names
        the compilation directory. */
    resattr = dwarf_attr(die, DW_AT_comp_dir, &comp_dir_attr, error);
    if (resattr == DW_DLV_ERROR) {
        return resattr;
    }
    if (resattr == DW_DLV_OK) {
        int cres = DW_DLV_ERROR;
        char *cdir = 0;

        cres = dwarf_formstring(comp_dir_attr, &cdir, error);
        if (cres == DW_DLV_ERROR) {
            return cres;
        } else if (cres == DW_DLV_OK) {
            comp_dir = (Dwarf_Small *) cdir;
        }
    }
    if (resattr == DW_DLV_OK) {
        dwarf_dealloc(dbg, comp_dir_attr, DW_DLA_ATTR);
    }
    /*  We are in dwarf_srcfiles() */
    dwarf_init_line_table_prefix(&line_prefix);
    {
        Dwarf_Small *line_ptr_out = 0;
        int dres = _dwarf_read_line_table_prefix(dbg,
            context,
            line_ptr,
            dbg->de_debug_line.dss_size,
            &line_ptr_out,
            &line_prefix,
            NULL, NULL,error,
            0);

        if (dres == DW_DLV_ERROR) {
            dwarf_free_line_table_prefix(&line_prefix);
            return dres;
        }
        if (dres == DW_DLV_NO_ENTRY) {
            dwarf_free_line_table_prefix(&line_prefix);
            return dres;
        }

        line_ptr = line_ptr_out;
    }

    for (i = 0; i < line_prefix.pf_files_count; ++i) {
        struct Line_Table_File_Entry_s *fe =
            line_prefix.pf_line_table_file_entries + i;
        char *file_name = (char *) fe->lte_filename;
        char *dir_name = 0;
        char *full_name = 0;
        Dwarf_Unsigned dir_index = fe->lte_directory_index;

        if (dir_index == 0) {
            dir_name = (char *) comp_dir;
        } else {
            dir_name =
                (char *) line_prefix.pf_include_directories[
                    fe->lte_directory_index - 1];
        }

        /*  dir_name can be NULL if there is no DW_AT_comp_dir.
            file_name == fe->lte_filename aside from char signedness.
        */
        if (dir_name == 0 || file_name_is_full_path(fe->lte_filename)) {
            /*  This is safe because dwarf_dealloc is careful to not
                dealloc strings which are part of the raw .debug_* data.
            */
            full_name = file_name;
        } else {
            full_name = (char *) _dwarf_get_alloc(dbg, DW_DLA_STRING,
                strlen(dir_name) + 1 +
                strlen(file_name) +
                1);
            if (full_name == NULL) {
                dwarf_free_line_table_prefix(&line_prefix);
                _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                return (DW_DLV_ERROR);
            }

            /*  This is not careful to avoid // in the output, Nothing
                forces a 'canonical' name format here. Unclear if this
                needs to be fixed. */
#if defined (HAVE_WINDOWS_PATH)
            /*  Always '/' instead of '\\', this is a Windows -> Unix
                issue. */
            {
                int index = 0;
                int len = strlen(dir_name);
                for (index = 0; index < len; ++index) {
                    full_name[index] = dir_name[index];
                    if (full_name[index] == '\\') {
                        full_name[index] = '/';
                    }
                }
            }
#else
            strcpy(full_name, dir_name);
#endif /* HAVE_WINDOWS_PATH */
            strcat(full_name, "/");
            strcat(full_name, file_name);
        }
        curr_chain =
            (Dwarf_Chain) _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
        if (curr_chain == NULL) {
            dwarf_free_line_table_prefix(&line_prefix);
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }
        curr_chain->ch_item = full_name;
        if (head_chain == NULL)
            head_chain = prev_chain = curr_chain;
        else {
            prev_chain->ch_next = curr_chain;
            prev_chain = curr_chain;
        }
    }

    if (line_prefix.pf_files_count == 0) {
        *srcfiles = NULL;
        *srcfilecount = 0;
        dwarf_free_line_table_prefix(&line_prefix);
        return (DW_DLV_NO_ENTRY);
    }

    ret_files = (char **)
        _dwarf_get_alloc(dbg, DW_DLA_LIST, line_prefix.pf_files_count);
    if (ret_files == NULL) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        dwarf_free_line_table_prefix(&line_prefix);
        return (DW_DLV_ERROR);
    }

    curr_chain = head_chain;
    for (i = 0; i < line_prefix.pf_files_count; i++) {
        *(ret_files + i) = curr_chain->ch_item;
        prev_chain = curr_chain;
        curr_chain = curr_chain->ch_next;
        dwarf_dealloc(dbg, prev_chain, DW_DLA_CHAIN);
    }

    *srcfiles = ret_files;
    *srcfilecount = line_prefix.pf_files_count;
    dwarf_free_line_table_prefix(&line_prefix);
    return (DW_DLV_OK);
}


#include "dwarf_line_table_reader_common.c"

/*  Return DW_DLV_OK if ok. else DW_DLV_NO_ENTRY or DW_DLV_ERROR
    doaddrs is true iff this is being called for SGI IRIX rqs processing
    (ie, not a normal libdwarf dwarf_srclines or two-level  user call at all).
    dolines is true iff this is called by a dwarf_srclines call.
*/
int
_dwarf_internal_srclines(Dwarf_Die die,
    Dwarf_Unsigned * version,
    Dwarf_Line ** linebuf,
    Dwarf_Signed * count,
    Dwarf_Line ** linebuf_actuals,
    Dwarf_Signed * count_actuals,
    Dwarf_Bool doaddrs,
    Dwarf_Bool dolines,
    Dwarf_Error * error)
{
    /*  This pointer is used to scan the portion of the .debug_line
        section for the current cu. */
    Dwarf_Small *line_ptr = 0;

    /*  This points to the last byte of the .debug_line portion for the
        current cu. */
    Dwarf_Small *line_ptr_end = 0;

    /*  For two-level line tables, this points to the first byte of the
        actuals table (and the end of the logicals table) for the current
        cu. */
    Dwarf_Small *line_ptr_actuals = 0;
    Dwarf_Small *section_start = 0;

    /*  Pointer to a DW_AT_stmt_list attribute in case it exists in the
        die. */
    Dwarf_Attribute stmt_list_attr = 0;

    /*  Pointer to DW_AT_comp_dir attribute in die. */
    Dwarf_Attribute comp_dir_attr = 0;

    /*  Pointer to name of compilation directory. */
    Dwarf_Small *comp_dir = NULL;

    /*  Offset into .debug_line specified by a DW_AT_stmt_list
        attribute. */
    Dwarf_Unsigned line_offset = 0;

    Dwarf_File_Entry file_entries = 0;

    /*  These pointers are used to build the list of files names by this
        cu.  cur_file_entry points to the file name being added, and
        prev_file_entry to the previous one. */
    Dwarf_File_Entry cur_file_entry = 0;
    Dwarf_File_Entry prev_file_entry = 0;

    Dwarf_Word u = 0;
    Dwarf_Sword file_entry_count = 0;

    /*  Pointer to a Dwarf_Line_Context_s structure that contains the
        context such as file names and include directories for the set
        of lines being generated.
        This is always recorded on an
        DW_LNS_end_sequence operator,
        on  all special opcodes, and on DW_LNS_copy.
        */
    Dwarf_Line_Context line_context = 0;
    Dwarf_CU_Context   cu_context = 0;

    struct Line_Table_Prefix_s prefix;

    /*  The Dwarf_Debug this die belongs to. */
    Dwarf_Debug dbg = 0;
    int resattr = DW_DLV_ERROR;
    int lres = DW_DLV_ERROR;
    Dwarf_Half address_size = 0;
    Dwarf_Small * orig_line_ptr = 0;

    int res = DW_DLV_ERROR;

    /*  ***** BEGIN CODE ***** */
    if (error != NULL) {
        *error = NULL;
    }

    CHECK_DIE(die, DW_DLV_ERROR);
    cu_context = die->di_cu_context;
    dbg = cu_context->cc_dbg;

    res = _dwarf_load_section(dbg, &dbg->de_debug_line,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    if (!dbg->de_debug_line.dss_size) {
        return (DW_DLV_NO_ENTRY);
    }

    address_size = _dwarf_get_address_size(dbg, die);
    resattr = dwarf_attr(die, DW_AT_stmt_list, &stmt_list_attr, error);
    if (resattr != DW_DLV_OK) {
        return resattr;
    }
    lres = dwarf_global_formref(stmt_list_attr, &line_offset, error);
    if (lres != DW_DLV_OK) {
        return lres;
    }

    if (line_offset >= dbg->de_debug_line.dss_size) {
        _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    section_start = dbg->de_debug_line.dss_data;
    orig_line_ptr = section_start + line_offset;
    line_ptr = orig_line_ptr;
    dwarf_dealloc(dbg, stmt_list_attr, DW_DLA_ATTR);

    /*  If die has DW_AT_comp_dir attribute, get the string that names
        the compilation directory. */
    resattr = dwarf_attr(die, DW_AT_comp_dir, &comp_dir_attr, error);
    if (resattr == DW_DLV_ERROR) {
        return resattr;
    }
    if (resattr == DW_DLV_OK) {
        int cres = DW_DLV_ERROR;
        char *cdir = 0;

        cres = dwarf_formstring(comp_dir_attr, &cdir, error);
        if (cres == DW_DLV_ERROR) {
            return cres;
        } else if (cres == DW_DLV_OK) {
            comp_dir = (Dwarf_Small *) cdir;
        }
    }
    if (resattr == DW_DLV_OK) {
        dwarf_dealloc(dbg, comp_dir_attr, DW_DLA_ATTR);
    }
#ifdef dadebug
printf("dadebug dwarf_internal line  base 0x%llx line %d\n",(unsigned long long)dbg->de_debug_line.dss_data,__LINE__);
printf("dadebug dwarf_internal line  len 0x%llx \n",(unsigned long long)dbg->de_debug_line.dss_size);
printf("dadebug dwarf_internal line  offset 0x%llx \n",(unsigned long long)line_offset);
printf("dadebug dwarf_internal line  origlineptr 0x%llx\n",(unsigned long long)orig_line_ptr);
#endif
    /*  We are in dwarf_internal_srclines() */
    dwarf_init_line_table_prefix(&prefix);

    {
        Dwarf_Small *newlinep = 0;
        int resp = _dwarf_read_line_table_prefix(dbg,
            cu_context,
            line_ptr, dbg->de_debug_line.dss_size,
            &newlinep,
            &prefix,
            NULL,NULL,
            error,
            0);

        if (resp == DW_DLV_ERROR) {
            dwarf_free_line_table_prefix(&prefix);
            return resp;
        }
        if (resp == DW_DLV_NO_ENTRY) {
            dwarf_free_line_table_prefix(&prefix);
            return resp;
        }
        line_ptr_end = prefix.pf_line_ptr_end;
        line_ptr = newlinep;
        if (prefix.pf_actuals_table_offset > 0) {
            line_ptr_actuals = prefix.pf_line_prologue_start +
                prefix.pf_actuals_table_offset;
        }
    }

    /* Set up context structure for this set of lines. */
    line_context = (Dwarf_Line_Context)
        _dwarf_get_alloc(dbg, DW_DLA_LINE_CONTEXT, 1);
    if (line_context == NULL) {
        dwarf_free_line_table_prefix(&prefix);
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }

    /*  Fill out a Dwarf_File_Entry list as we use that to implement the
        define_file operation. */
    file_entries = prev_file_entry = NULL;
    for (u = 0; u < prefix.pf_files_count; ++u) {
        struct Line_Table_File_Entry_s *pfxfile =
            prefix.pf_line_table_file_entries + u;

        cur_file_entry = (Dwarf_File_Entry)
            _dwarf_get_alloc(dbg, DW_DLA_FILE_ENTRY, 1);
        if (cur_file_entry == NULL) {
            dwarf_free_line_table_prefix(&prefix);
            dwarf_dealloc(dbg,line_context,DW_DLA_LINE_CONTEXT);
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }

        cur_file_entry->fi_file_name = pfxfile->lte_filename;
        cur_file_entry->fi_dir_index = pfxfile->lte_directory_index;
        cur_file_entry->fi_time_last_mod =
            pfxfile->lte_last_modification_time;

        cur_file_entry->fi_file_length = pfxfile->lte_length_of_file;

        _dwarf_update_file_entry(cur_file_entry,&file_entries,
            &prev_file_entry,&file_entry_count);
    }

    if (line_ptr_actuals == 0) {
        int err_count_out = 0;
        /* Normal style (single level) line table. */
        Dwarf_Bool is_actuals_table = false;
        Dwarf_Bool is_single_table = true;
        res = read_line_table_program(dbg,
            line_ptr, line_ptr_end, orig_line_ptr,
            section_start,
            &prefix, line_context,
            linebuf, count,
            &file_entries, prev_file_entry, &file_entry_count,
            address_size, doaddrs, dolines,
            is_single_table,
            is_actuals_table,
            NULL,0, error,
            &err_count_out);
        if (res != DW_DLV_OK) {
            dwarf_free_line_table_prefix(&prefix);
            return res;
        }
        if (linebuf_actuals != NULL)
            *linebuf_actuals = NULL;
        if (count_actuals != NULL)
            *count_actuals = 0;
    } else {
        Dwarf_Bool is_actuals_table = false;
        Dwarf_Bool is_single_table = false;
        int err_count_out = 0;

        /*  Two-level line table.
            First read the logicals table. */
        res = read_line_table_program(dbg,
            line_ptr, line_ptr_actuals, orig_line_ptr,
            section_start,
            &prefix, line_context, linebuf, count,
            &file_entries, prev_file_entry, &file_entry_count,
            address_size, doaddrs, dolines,
            is_single_table,
            is_actuals_table, NULL, 0, error,
            &err_count_out);
        if (res != DW_DLV_OK) {
            dwarf_free_line_table_prefix(&prefix);
            return res;
        }
        if (linebuf_actuals != NULL) {
            Dwarf_Signed tmpcount = 0;
            is_actuals_table = true;
            /* The call requested an actuals table
                and one is present. So now read that one. */
            res = read_line_table_program(dbg,
                line_ptr_actuals, line_ptr_end, orig_line_ptr,
                section_start,
                &prefix, line_context, linebuf_actuals, &tmpcount,
                NULL, NULL, NULL,
                address_size, doaddrs, dolines,
                is_single_table,
                is_actuals_table, *linebuf, *count, error,
                &err_count_out);
            if (res != DW_DLV_OK) {
                dwarf_free_line_table_prefix(&prefix);
                return res;
            }
            if (count_actuals != NULL) {
                *count_actuals = tmpcount;
            }
        }
    }

    if (*count == 0 && (count_actuals == NULL || *count_actuals == 0)) {
        /*  Here we have no actual lines of any kind. In other words,
            it looks like a debugfission line table skeleton.
            In that case there are no line entries so the context
            had nowhere to be recorded. Hence we have to delete it
            else we would leak the context.  */
        Dwarf_File_Entry fe = file_entries;

        while (fe) {
            Dwarf_File_Entry fenext = fe->fi_next;

            dwarf_dealloc(dbg, fe, DW_DLA_FILE_ENTRY);
            fe = fenext;
        }
        dwarf_dealloc(dbg, line_context, DW_DLA_LINE_CONTEXT);
    } else {
        line_context->lc_file_entries = file_entries;
        line_context->lc_file_entry_count = file_entry_count;
        line_context->lc_include_directories_count =
            prefix.pf_include_directories_count;
        if (prefix.pf_include_directories_count > 0) {
            /*  This moves the pointer to the list of include directories
                from the prefix structure to the line_context. We do not
                want this array deallocated when the prefix structure is
                deallocated. */
            line_context->lc_include_directories =
                prefix.pf_include_directories;
            prefix.pf_include_directories_count = 0;
            prefix.pf_include_directories = NULL;
        }

        line_context->lc_subprogs_count = prefix.pf_subprogs_count;
        if (prefix.pf_subprogs_count > 0) {
            /*  Likewise, we move the pointer to the subprogram entries
                to the line_context structure. */
            line_context->lc_subprogs = prefix.pf_subprog_entries;
            prefix.pf_subprogs_count = 0;
            prefix.pf_subprog_entries = NULL;
        }

        line_context->lc_line_count = *count;
        line_context->lc_compilation_directory = comp_dir;
        line_context->lc_version_number = prefix.pf_version;
        line_context->lc_dbg = dbg;
    }

    if (version != NULL) {
        *version = prefix.pf_version;
    }

    dwarf_free_line_table_prefix(&prefix);
    return (DW_DLV_OK);
}

int
dwarf_srclines(Dwarf_Die die,
    Dwarf_Line ** linebuf,
    Dwarf_Signed * linecount, Dwarf_Error * error)
{
    Dwarf_Signed count = 0;
    int res  = _dwarf_internal_srclines(die,
        /* version= */ NULL,
        linebuf,
        &count,
        /* linebuf_actuals= */ NULL,
        /* linecount_actuals= */ NULL,
        /* addrlist= */ false,
        /* linelist= */ true,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    *linecount = count;
    return res;
}

int
dwarf_srclines_two_level(Dwarf_Die die,
    Dwarf_Unsigned * version,
    Dwarf_Line ** linebuf,
    Dwarf_Signed * linecount,
    Dwarf_Line ** linebuf_actuals,
    Dwarf_Signed * linecount_actuals,
    Dwarf_Error * error)
{
    Dwarf_Signed count = 0;
    Dwarf_Signed count_actuals = 0;
    int res  = _dwarf_internal_srclines(die,
        version,
        linebuf,
        &count,
        linebuf_actuals,
        &count_actuals,
        /* addrlist= */ false,
        /* linelist= */ true,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    *linecount = count;
    *linecount_actuals = count_actuals;
    return res;
}



/*  Every line table entry (except DW_DLE_end_sequence,
    which is returned using dwarf_lineendsequence())
    potentially has the begin-statement
    flag marked 'on'.   This returns thru *return_bool,
    the begin-statement flag.  */

int
dwarf_linebeginstatement(Dwarf_Line line,
    Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    if (line == NULL || return_bool == 0) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    *return_bool = (line->li_addr_line.li_l_data.li_is_stmt);
    return DW_DLV_OK;
}

/*  At the end of any contiguous line-table there may be
    a DW_LNE_end_sequence operator.
    This returns non-zero thru *return_bool
    if and only if this 'line' entry was a DW_LNE_end_sequence.

    Within a compilation unit or function there may be multiple
    line tables, each ending with a DW_LNE_end_sequence.
    Each table describes a contiguous region.
    Because compilers may split function code up in arbitrary ways
    compilers may need to emit multiple contigous regions (ie
    line tables) for a single function.
    See the DWARF3 spec section 6.2.  */
int
dwarf_lineendsequence(Dwarf_Line line,
    Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    *return_bool = (line->li_addr_line.li_l_data.li_end_sequence);
    return DW_DLV_OK;
}


/*  Each 'line' entry has a line-number.
    If the entry is a DW_LNE_end_sequence the line-number is
    meaningless (see dwarf_lineendsequence(), just above).  */
int
dwarf_lineno(Dwarf_Line line,
    Dwarf_Unsigned * ret_lineno, Dwarf_Error * error)
{
    if (line == NULL || ret_lineno == 0) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    *ret_lineno = (line->li_addr_line.li_l_data.li_line);
    return DW_DLV_OK;
}

/*  Each 'line' entry has a file-number, and index into the file table.
    If the entry is a DW_LNE_end_sequence the index is
    meaningless (see dwarf_lineendsequence(), just above).
    The file number returned is an index into the file table
    produced by dwarf_srcfiles(), but care is required: the
    li_file begins with 1 for real files, so that the li_file returned here
    is 1 greater than its index into the dwarf_srcfiles() output array.
    And entries from DW_LNE_define_file don't appear in
    the dwarf_srcfiles() output so file indexes from here may exceed
    the size of the dwarf_srcfiles() output array size.
*/
int
dwarf_line_srcfileno(Dwarf_Line line,
    Dwarf_Unsigned * ret_fileno, Dwarf_Error * error)
{
    if (line == NULL || ret_fileno == 0) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    /*  li_file must be <= line->li_context->lc_file_entry_count else it
        is trash. li_file 0 means not attributable to any source file
        per dwarf2/3 spec. */

    *ret_fileno = (line->li_addr_line.li_l_data.li_file);
    return DW_DLV_OK;
}

/*  Each 'line' entry has an is_addr_set attribute.
    If the entry is a DW_LNE_set_address, return TRUE through
    the *is_addr_set pointer.  */
int
dwarf_line_is_addr_set(Dwarf_Line line,
    Dwarf_Bool *is_addr_set, Dwarf_Error * error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    *is_addr_set = (line->li_addr_line.li_l_data.li_is_addr_set);
    return DW_DLV_OK;
}

/*  Each 'line' entry has a line-address.
    If the entry is a DW_LNE_end_sequence the adddress
    is one-beyond the last address this contigous region
    covers, so the address is not inside the region,
    but is just outside it.  */
int
dwarf_lineaddr(Dwarf_Line line,
    Dwarf_Addr * ret_lineaddr, Dwarf_Error * error)
{
    if (line == NULL || ret_lineaddr == 0) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    *ret_lineaddr = (line->li_address);
    return DW_DLV_OK;
}


/*  Obsolete: do not use this function.
    December 2011: For reasons lost in the mists of history,
    this returned -1, not zero (through the pointer
    ret_lineoff), if the column was zero.
    That was always bogus, even in DWARF2.
    It is also bogus that the column value is signed, but
    it is painful to change the argument type in 2011, so leave it.
    */
int
dwarf_lineoff(Dwarf_Line line,
    Dwarf_Signed * ret_lineoff, Dwarf_Error * error)
{
    if (line == NULL || ret_lineoff == 0) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    *ret_lineoff = (
        (line->li_addr_line.li_l_data.li_column == 0) ?
            -1 : line->li_addr_line.li_l_data.li_column);
    return DW_DLV_OK;
}
/*  Each 'line' entry has a column-within-line (offset
    within the line) where the
    source text begins.
    If the entry is a DW_LNE_end_sequence the line-number is
    meaningless (see dwarf_lineendsequence(), just above).
    Lines of text begin at column 1.  The value 0
    means the line begins at the left edge of the line.
    (See the DWARF3 spec, section 6.2.2).
    So 0 and 1 mean essentially the same thing.
    dwarf_lineoff_b() is new in December 2011.
    */
int
dwarf_lineoff_b(Dwarf_Line line,
    Dwarf_Unsigned * ret_lineoff, Dwarf_Error * error)
{
    if (line == NULL || ret_lineoff == 0) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    *ret_lineoff = line->li_addr_line.li_l_data.li_column;
    return DW_DLV_OK;
}


static int
dwarf_filename(Dwarf_Line_Context context, Dwarf_Sword fileno,
    char **ret_filename, Dwarf_Error *error)
{
    Dwarf_Signed i = 0;
    Dwarf_File_Entry file_entry;
    Dwarf_Small *name_buffer = 0;
    Dwarf_Small *include_directory = 0;
    Dwarf_Small include_direc_full_path = 0;
    Dwarf_Small file_name_full_path = 0;
    unsigned int comp_dir_len = 0;
    Dwarf_Debug dbg = context->lc_dbg;

    if (fileno > context->lc_file_entry_count) {
        _dwarf_error(dbg, error, DW_DLE_LINE_FILE_NUM_BAD);
        return (DW_DLV_ERROR);
    }

    if (fileno == 0) {
        /*  No file name known: see dwarf2/3 spec. */
        _dwarf_error(dbg, error, DW_DLE_NO_FILE_NAME);
        return (DW_DLV_ERROR);
    }
    file_entry = context->lc_file_entries;
    /*  ASSERT: li_file > 0, dwarf correctness issue, see line table
        definition of dwarf2/3 spec. */
    /*  Example: if li_file is 2 and lc_file_entry_count is 3,
        file_entry is file 3 (1 based), aka 2( 0 based) file_entry->next
        is file 2 (1 based), aka 1( 0 based) file_entry->next->next is
        file 1 (1 based), aka 0( 0 based) file_entry->next->next->next
        is NULL.

        and this loop finds the file_entry we need (2 (1 based) in this
        case). Because lc_file_entries are in reverse order and
        effectively zero based as a count whereas li_file is 1 based. */
    for (i = fileno - 1; i > 0; i--) {
        file_entry = file_entry->fi_next;
    }

    if (file_entry->fi_file_name == NULL) {
        _dwarf_error(dbg, error, DW_DLE_NO_FILE_NAME);
        return (DW_DLV_ERROR);
    }

    file_name_full_path = file_name_is_full_path(file_entry->fi_file_name);
    if (file_name_full_path) {
        *ret_filename = ((char *) file_entry->fi_file_name);
        return DW_DLV_OK;
    }

    if (file_entry->fi_dir_index == 0) {

        /*  dir_index of 0 means that the compilation was in the
            'current directory of compilation' */
        if (context->lc_compilation_directory == NULL) {
            /*  We don't actually *have* a current directory of
                compilation: DW_AT_comp_dir was not present Rather than
                emitting DW_DLE_NO_COMP_DIR lets just make an empty name
                here. In other words, do the best we can with what we do
                have instead of reporting an error. _dwarf_error(dbg,
                error, DW_DLE_NO_COMP_DIR); return(DW_DLV_ERROR); */
            comp_dir_len = 0;
        } else {
            comp_dir_len = strlen((char *)
                (context->lc_compilation_directory));
        }

        name_buffer = (Dwarf_Small *)
            _dwarf_get_alloc(context->lc_dbg, DW_DLA_STRING,
                comp_dir_len + 1 +
                strlen((char *) file_entry->fi_file_name) + 1);
        if (name_buffer == NULL) {
            _dwarf_error(context->lc_dbg, error,
                DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }

        if (comp_dir_len > 0) {
            /*  If comp_dir_len is 0 we do not want to put a / in front
                of the fi_file_name as we just don't know anything. */
            strcpy((char *) name_buffer,
                (char *) (context->lc_compilation_directory));
            strcat((char *) name_buffer, "/");
        }
        strcat((char *) name_buffer, (char *) file_entry->fi_file_name);
        *ret_filename = ((char *) name_buffer);
        return DW_DLV_OK;
    }

    if (file_entry->fi_dir_index > context->lc_include_directories_count) {
        _dwarf_error(dbg, error, DW_DLE_INCL_DIR_NUM_BAD);
        return (DW_DLV_ERROR);
    }

    include_directory =
        context->lc_include_directories[file_entry->fi_dir_index - 1];

    if (context->lc_compilation_directory) {
        comp_dir_len = strlen((char *) (context->lc_compilation_directory));
    } else {
        /*  No DW_AT_comp_dir present. Do the best we can without it. */
        comp_dir_len = 0;
    }

    include_direc_full_path = file_name_is_full_path(include_directory);
    name_buffer = (Dwarf_Small *)_dwarf_get_alloc(dbg, DW_DLA_STRING,
        (include_direc_full_path ?  0 : comp_dir_len + 1) +
            strlen((char *)include_directory) + 1 +
            strlen((char *)file_entry->fi_file_name) + 1);
    if (name_buffer == NULL) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }

    if (!include_direc_full_path) {
        if (comp_dir_len > 0) {
            strcpy((char *)name_buffer,
                (char *)context->lc_compilation_directory);
            /*  Who provides the / needed after the compilation
                directory? */
            if (!is_path_separator(name_buffer[comp_dir_len - 1])) {
                /*  Here we provide the / separator. It
                    should work ok for Windows */
                /*  Overwrite previous nul terminator with needed / */
                name_buffer[comp_dir_len] = '/';
                name_buffer[comp_dir_len + 1] = 0;
            }
        }
    } else {
        strcpy((char *) name_buffer, "");
    }
    strcat((char *) name_buffer, (char *) include_directory);
    strcat((char *) name_buffer, "/");
    strcat((char *) name_buffer, (char *) file_entry->fi_file_name);
    *ret_filename = ((char *) name_buffer);
    return DW_DLV_OK;
}

int
dwarf_linesrc(Dwarf_Line line, char **ret_linesrc, Dwarf_Error * error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }

    if (line->li_context == NULL) {
        _dwarf_error(NULL, error, DW_DLE_LINE_CONTEXT_NULL);
        return (DW_DLV_ERROR);
    }

    return dwarf_filename(line->li_context,
        line->li_addr_line.li_l_data.li_file, ret_linesrc, error);

}

/*  Every line table entry potentially has the basic-block-start
    flag marked 'on'.   This returns thru *return_bool,
    the basic-block-start flag.
*/
int
dwarf_lineblock(Dwarf_Line line,
    Dwarf_Bool * return_bool, Dwarf_Error * error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    *return_bool = (line->li_addr_line.li_l_data.li_basic_block);
    return DW_DLV_OK;
}

/* We gather these into one call as it's likely one
   will want all or none of them.  */
int dwarf_prologue_end_etc(Dwarf_Line  line,
    Dwarf_Bool  *    prologue_end,
    Dwarf_Bool  *    epilogue_begin,
    Dwarf_Unsigned * isa,
    Dwarf_Unsigned * discriminator,
    Dwarf_Error *    error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    *prologue_end = (line->li_addr_line.li_l_data.li_prologue_end);
    *epilogue_begin = (line->li_addr_line.li_l_data.li_epilogue_begin);
    *isa = (line->li_addr_line.li_l_data.li_isa);
    *discriminator = (line->li_addr_line.li_l_data.li_discriminator);
    return DW_DLV_OK;
}

int
dwarf_linelogical(Dwarf_Line line,
    Dwarf_Unsigned * logical,
    Dwarf_Error*     error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    *logical = (line->li_addr_line.li_l_data.li_line);
    return DW_DLV_OK;
}

int
dwarf_linecontext(Dwarf_Line line,
    Dwarf_Unsigned * context,
    Dwarf_Error*     error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    *context = (line->li_addr_line.li_l_data.li_call_context);
    return DW_DLV_OK;
}

int
dwarf_line_subprogno(Dwarf_Line line,
    Dwarf_Unsigned * subprog,
    Dwarf_Error *    error)
{
    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return (DW_DLV_ERROR);
    }
    *subprog = (line->li_addr_line.li_l_data.li_subprogram);
    return DW_DLV_OK;
}

int
dwarf_line_subprog(Dwarf_Line line,
    char   **        subprog_name,
    char   **        decl_filename,
    Dwarf_Unsigned * decl_line,
    Dwarf_Error *    error)
{
    Dwarf_Unsigned subprog_no;
    Dwarf_Subprog_Entry subprog;
    Dwarf_Debug dbg;
    int res;

    if (line == NULL) {
        _dwarf_error(NULL, error, DW_DLE_DWARF_LINE_NULL);
        return DW_DLV_ERROR;
    }

    if (line->li_context == NULL) {
        _dwarf_error(NULL, error, DW_DLE_LINE_CONTEXT_NULL);
        return DW_DLV_ERROR;
    }

    dbg = line->li_context->lc_dbg;

    subprog_no = line->li_addr_line.li_l_data.li_subprogram;
    if (subprog_no == 0) {
        *subprog_name = NULL;
        *decl_filename = NULL;
        *decl_line = 0;
        return DW_DLV_OK;
    }

    if (subprog_no > line->li_context->lc_subprogs_count) {
        _dwarf_error(dbg, error, DW_DLE_NO_FILE_NAME);
        return DW_DLV_ERROR;
    }

    subprog = &line->li_context->lc_subprogs[subprog_no - 1];

    *subprog_name = (char *)subprog->ds_subprog_name;
    *decl_line = subprog->ds_decl_line;

    res = dwarf_filename(line->li_context, subprog->ds_decl_file,
        decl_filename, error);
    if (res != DW_DLV_OK) {
        *decl_filename = NULL;
    }

    return DW_DLV_OK;
}


#if 0  /* Ignore this.  This needs major re-work. */
/*  This routine works by looking for exact matches between
    the current line address and pc, and crossovers from
    from less than pc value to greater than.  At each line
    that satisfies the above, it records a pointer to the
    line, and the difference between the address and pc.
    It then scans these pointers and picks out those with
    the smallest difference between pc and address.
*/
int
dwarf_pclines(Dwarf_Debug dbg,
    Dwarf_Addr pc,
    Dwarf_Line ** linebuf,
    Dwarf_Signed slide,
    Dwarf_Signed * linecount, Dwarf_Error * error)
{
    /*  Scans the line matrix for the current cu to which a pointer
        exists in dbg. */
    Dwarf_Line line;
    Dwarf_Line prev_line;

    /*  These flags are for efficiency reasons. Check_line is true
        initially, but set false when the address of the current line is
        greater than pc.  It is set true only when the address of the
        current line falls below pc.  This assumes that addresses within
        the same segment increase, and we are only interested in the
        switch from a less than pc address to a greater than. First_line
        is set true initially, but set false after the first line is
        scanned.  This is to prevent looking at the address of previous
        line when slide is DW_DLS_BACKWARD, and the first line is being
        scanned. */
    Dwarf_Bool check_line, first_line;

    /*  Diff tracks the smallest difference a line address and the input
        pc value. */
    Dwarf_Signed diff, i;

    /*  For the slide = DW_DLS_BACKWARD case, pc_less is the value of
        the address of the line immediately preceding the first line
        that has value greater than pc. For the slide = DW_DLS_FORWARD
        case, pc_more is the values of address for the first line that
        is greater than pc. Diff is the difference between either of the
        these values and pc. */
    Dwarf_Addr pc_less, pc_more;

    /*  Pc_line_buf points to a chain of pointers to lines of which
        those with a diff equal to the smallest difference will be
        returned. */
    Dwarf_Line *pc_line_buf, *pc_line;

    /*  Chain_count counts the number of lines in the above chain for
        which the diff is equal to the smallest difference This is the
        number returned by this routine. */
    Dwarf_Signed chain_count;

    chain_head = NULL;

    check_line = true;
    first_line = true;
    diff = MAX_LINE_DIFF;

    for (i = 0; i < dbg->de_cu_line_count; i++) {

        line = *(dbg->de_cu_line_ptr + i);
        prev_line = first_line ? NULL : *(dbg->de_cu_line_ptr + i - 1);

        if (line->li_address == pc) {
            chain_ptr = (struct chain *)
                _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
            if (chain_ptr == NULL) {
                _dwarf_error(NULL, error, DW_DLE_ALLOC_FAIL);
                return (DW_DLV_ERROR);
            }

            chain_ptr->line = line;
            chain_ptr->diff = diff = 0;
            chain_ptr->next = chain_head;
            chain_head = chain_ptr;
        } else {
            /*  Look for crossover from less than pc address to greater
                than. */
            if (check_line && line->li_address > pc &&
                (first_line ? 0 : prev_line->li_address) < pc) {
                if (slide == DW_DLS_BACKWARD && !first_line) {
                    pc_less = prev_line->li_address;
                    if (pc - pc_less <= diff) {
                        chain_ptr = (struct chain *)
                            _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
                        if (chain_ptr == NULL) {
                            _dwarf_error(NULL, error, DW_DLE_ALLOC_FAIL);
                            return (DW_DLV_ERROR);
                        }
                        chain_ptr->line = prev_line;
                        chain_ptr->diff = diff = pc - pc_less;
                        chain_ptr->next = chain_head;
                        chain_head = chain_ptr;
                    }
                    check_line = false;
                } else if (slide == DW_DLS_FORWARD) {
                    pc_more = line->li_address;
                    if (pc_more - pc <= diff) {
                        chain_ptr = (struct chain *)
                            _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
                        if (chain_ptr == NULL) {
                            _dwarf_error(NULL, error, DW_DLE_ALLOC_FAIL);
                            return (DW_DLV_ERROR);
                        }
                        chain_ptr->line = line;
                        chain_ptr->diff = diff = pc_more - pc;
                        chain_ptr->next = chain_head;
                        chain_head = chain_ptr;
                    }
                    check_line = false;
                } else {
                    /* Check addresses only when they go */
                    /* below pc.  */
                    if (line->li_address < pc) {
                        check_line = true;
                    }
                }
            }
        }
        first_line = false;
    }
    chain_count = 0;
    for (chain_ptr = chain_head; chain_ptr != NULL;
        chain_ptr = chain_ptr->next) {
        if (chain_ptr->diff == diff) {
            chain_count++;
        }
    }
    pc_line_buf = pc_line = (Dwarf_Line)
        _dwarf_get_alloc(dbg, DW_DLA_LIST, chain_count);
    for (chain_ptr = chain_head; chain_ptr != NULL;
        chain_ptr = chain_ptr->next) {
        if (chain_ptr->diff == diff) {
            *pc_line = chain_ptr->line;
            pc_line++;
        }
    }
    for (chain_ptr = chain_head; chain_ptr != NULL;) {
        chain_head = chain_ptr;
        chain_ptr = chain_ptr->next;
        dwarf_dealloc(dbg, chain_head, DW_DLA_CHAIN);
    }
    *linebuf = pc_line_buf;
    return (chain_count);
}
#endif



/*
   It's impossible for callers of dwarf_srclines() to get to and
   free all the resources (in particular, the li_context and its
   lc_file_entries).
   So this function, new July 2005, does it.

   As of September 2015 this will now delete either
   table of a two-level line table.
   In the two-level case one calls it once each on
   both the logicals and actuals tables.
   (in either order, the order is not critical).
   Once  the  logicals table is dealloced any
   use of the actuals table will surely result in chaos.
   Just do the two calls one after the other.

   In the standard case one calls it just once on the
   linebuf.
*/

void
dwarf_srclines_dealloc(Dwarf_Debug dbg, Dwarf_Line * linebuf,
    Dwarf_Signed count)
{

    Dwarf_Signed i = 0;
    struct Dwarf_Line_Context_s *context = 0;

    if (count > 0 && !linebuf[0]->li_is_actuals_table) {
        /*  All these entries share a single context, and
            for two-levels tables each table gets it too.
            Hence we will dealloc ONLY if !is_actuals_table
            so for single and two-level tables the space
            is deallocated. */
        context = linebuf[0]->li_context;
    }
    for (i = 0; i < count; ++i) {
        dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
    }
    dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);

    if (context) {
        /*  Only called when is_actuals_table is false.
            Because  logicals table entries also point
            to the context we defer to the logicals
            dwarf_srclines_dealloc() to free the chain.. */
        Dwarf_File_Entry fe = context->lc_file_entries;

        while (fe) {
            Dwarf_File_Entry fenext = fe->fi_next;

            dwarf_dealloc(dbg, fe, DW_DLA_FILE_ENTRY);
            fe = fenext;
        }
        if (context->lc_subprogs) {
            free(context->lc_subprogs);
        }
        if (context->lc_include_directories) {
            free(context->lc_include_directories);
        }
        dwarf_dealloc(dbg, context, DW_DLA_LINE_CONTEXT);
    }

    return;
}

/* There is an error, so count it. If we are printing
   errors by command line option, print the details.  */
void
_dwarf_print_header_issue(Dwarf_Debug dbg,
    const char *specific_msg,
    Dwarf_Small *data_start,
    int *err_count_out)
{
    if (!err_count_out) {
        return;
    }
    /* Are we in verbose mode */
    if (dwarf_cmdline_options.check_verbose_mode){
        dwarf_printf(dbg,
            "\n*** DWARF CHECK: "
            ".debug_line: %s", specific_msg);

        if (data_start >= dbg->de_debug_line.dss_data &&
            (data_start < (dbg->de_debug_line.dss_data +
            dbg->de_debug_line.dss_size))) {
            Dwarf_Unsigned off = data_start - dbg->de_debug_line.dss_data;
            dwarf_printf(dbg,
                " at offset 0x%" DW_PR_XZEROS DW_PR_DUx
                "  ( %" DW_PR_DUu " ) ",
                off,off);
        } else {
            dwarf_printf(dbg,
                " (unknown section location) ");
        }
        dwarf_printf(dbg,"***\n");
    }
    *err_count_out += 1;
}


int
_dwarf_decode_line_string_form(Dwarf_Debug dbg,
    Dwarf_Unsigned form,
    Dwarf_Unsigned offset_size,
    Dwarf_Small **line_ptr,
    Dwarf_Small *line_ptr_end,
    char **return_str,
    Dwarf_Error * error)
{
    int res = 0;

    switch (form) {
    case DW_FORM_line_strp: {
        Dwarf_Small *secstart = 0;
        Dwarf_Small *secend = 0;
        Dwarf_Small *strptr = 0;
        Dwarf_Unsigned offset = 0;
        Dwarf_Small *offsetptr = *line_ptr;

        res = _dwarf_load_section(dbg, &dbg->de_debug_line_str,error);
        if (res != DW_DLV_OK) {
            return res;
        }

        secstart = dbg->de_debug_line_str.dss_data;
        secend = secstart + dbg->de_debug_line_str.dss_size;

        READ_UNALIGNED(dbg, offset, Dwarf_Unsigned,offsetptr, offset_size);
        *line_ptr += offset_size;
        strptr = secstart + offset;
        res = _dwarf_check_string_valid(dbg,
            secstart,strptr,secend,error);
        if (res != DW_DLV_OK) {
            return res;
        }
        *return_str = (char *) strptr;
        return DW_DLV_OK;
        }
    case DW_FORM_string: {
        Dwarf_Small *secend = line_ptr_end;;
        Dwarf_Small *strptr = *line_ptr;
        res = _dwarf_check_string_valid(dbg,
            strptr ,strptr,secend,error);
        if (res != DW_DLV_OK) {
            return res;
        }
        *return_str = (char *)strptr;
        *line_ptr += strlen((const char *)strptr) + 1;
        return DW_DLV_OK;
        }
    default:
        _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
        return DW_DLV_ERROR;
    }
}

int
_dwarf_decode_line_udata_form(Dwarf_Debug dbg,
    Dwarf_Unsigned form,
    Dwarf_Small **line_ptr,
    Dwarf_Unsigned *return_val,
    Dwarf_Error * error)
{
    Dwarf_Unsigned val = 0;
    Dwarf_Small * lp = *line_ptr;

    switch (form) {

    case DW_FORM_udata:
        DECODE_LEB128_UWORD(lp, val);
        *return_val = val;
        *line_ptr = lp;
        return DW_DLV_OK;

    default:
        _dwarf_error(dbg, error, DW_DLE_ATTR_FORM_BAD);
        return DW_DLV_ERROR;
    }
}


void
_dwarf_update_file_entry(Dwarf_File_Entry  cur_file_entry,
    Dwarf_File_Entry *file_entries,
    Dwarf_File_Entry *prev_file_entry,
    Dwarf_Sword      *file_entry_count)
{
    if (*file_entries == NULL) {
        *file_entries = cur_file_entry;
    } else {
        (*prev_file_entry)->fi_next = cur_file_entry;
    }
    *prev_file_entry = cur_file_entry;
    (*file_entry_count)++;
}

void
_dwarf_update_chain_list( Dwarf_Chain chain_line,
    Dwarf_Chain *head_chain, Dwarf_Chain *curr_chain)
{
    if (*head_chain == NULL) {
        *head_chain = chain_line;
    } else {
        (*curr_chain)->ch_next = chain_line;
    }
    *curr_chain = chain_line;
}

void
_dwarf_free_chain_entries(Dwarf_Debug dbg,Dwarf_Chain head,int count)
{
    int i = 0;
    Dwarf_Chain curr_chain = head;
    for (i = 0; i < count; i++) {
        Dwarf_Chain t = curr_chain;
        curr_chain = curr_chain->ch_next;
        dwarf_dealloc(dbg, t, DW_DLA_CHAIN);
    }
}



/* Initialize the Line_Table_Prefix_s struct.
   memset is not guaranteed a portable initializer, but works
   fine for current architectures.   AFAIK.
*/
void
dwarf_init_line_table_prefix(struct Line_Table_Prefix_s *pf)
{
    memset(pf, 0, sizeof(*pf));
}

/* Free any malloc'd area.  of the Line_Table_Prefix_s struct. */
void
dwarf_free_line_table_prefix(struct Line_Table_Prefix_s *pf)
{
    if (pf->pf_include_directories) {
        free(pf->pf_include_directories);
        pf->pf_include_directories = 0;
    }
    if (pf->pf_line_table_file_entries) {
        free(pf->pf_line_table_file_entries);
        pf->pf_line_table_file_entries = 0;
    }
    if (pf->pf_subprog_entries) {
        free(pf->pf_subprog_entries);
        pf->pf_subprog_entries = 0;
    }
    return;
}

