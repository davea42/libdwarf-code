
/*  The following are taken from libdwarf2.1.mm
    to verify there are no silly tographical errors
    in that document.

    These should not be built routinely nor
    should it every be executed.

    The code here is what user code should be,
    hence the code typed here is
    PUBLIC DOMAIN.
    This file is not part of libdwarf, though
    it appears in the same directory as libdwarf.
*/


#include "dwarf.h"
#include "libdwarf.h"

#define NULL 0

void example1(Dwarf_Die somedie)
{
    Dwarf_Debug dbg = 0;
    Dwarf_Signed atcount;
    Dwarf_Attribute *atlist;
    Dwarf_Error error = 0;
    Dwarf_Signed i = 0;
    int errv;

    errv = dwarf_attrlist(somedie, &atlist,&atcount, &error);
    if (errv == DW_DLV_OK) {
        for (i = 0; i < atcount; ++i) {
            /* use atlist[i] */
            dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
        }
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }
}

void example2(Dwarf_Debug dbg, Dwarf_Debug tieddbg)
{
    Dwarf_Error error = 0;
    int res = 0;

    /*  Do the dwarf_init() or dwarf_elf_init
        calls to set
        dbg, tieddbg at this point. Then: */
    res = dwarf_set_tied_dbg(dbg,tieddbg,&error);
    if (res != DW_DLV_OK) {
        /* Something went wrong*/
    }
}

void example3(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    int res = 0;
    res = dwarf_set_tied_dbg(dbg,NULL,&error);
    if (res != DW_DLV_OK) {
        /* Something went wrong*/
    }
}

void example4(Dwarf_Debug dbg,Dwarf_Die in_die,Dwarf_Bool is_info)
{
    Dwarf_Die return_sib = 0;
    Dwarf_Error error = 0;
    int res = 0;

    /* in_die might be NULL or a valid Dwarf_Die */
    res = dwarf_siblingof_b(dbg,in_die,is_info,&return_sib, &error);
    if (res == DW_DLV_OK) {
        /* Use return_sib here. */
        dwarf_dealloc(dbg, return_sib, DW_DLA_DIE);
        /*  return_sib is no longer usable for anything, we
            ensure we do not use it accidentally with: */
        return_sib = 0;
    }
}


void example5(Dwarf_Debug dbg,Dwarf_Die in_die)
{
    Dwarf_Die return_kid = 0;
    Dwarf_Error error = 0;
    int res = 0;

    res = dwarf_child(in_die,&return_kid, &error);
    if (res == DW_DLV_OK) {
        /* Use return_kid here. */
        dwarf_dealloc(dbg, return_kid, DW_DLA_DIE);
        /*  return_die is no longer usable for anything, we
            ensure we do not use it accidentally with: */
        return_kid = 0;
    }
}

void example6(Dwarf_Debug dbg,Dwarf_Off die_offset,Dwarf_Bool is_info)
{
    Dwarf_Error error = 0;
    Dwarf_Die return_die = 0;
    int res = 0;

    res = dwarf_offdie_b(dbg,die_offset,is_info,&return_die, &error);
    if (res == DW_DLV_OK) {
        /* Use return_die here. */
        dwarf_dealloc(dbg, return_die, DW_DLA_DIE);
        /*  return_die is no longer usable for anything, we
            ensure we do not use it accidentally with: */
        return_die = 0;
    } else {
        /*  res could be NO ENTRY or ERROR, so no
            dealloc necessary.  */
    }
}

void example7(Dwarf_Debug dbg, Dwarf_Die in_die,Dwarf_Bool is_info)
{
    int res = 0;
    Dwarf_Off cudieoff = 0;
    Dwarf_Die cudie = 0;
    Dwarf_Error error = 0;

    print_die_data(dbg,in_die);
    res = dwarf_CU_dieoffset_given_die(in_die,&cudieoff,&error);
    if(res != DW_DLV_OK) {
        /*  FAIL */
        return;
    }
    res = dwarf_offdie_b(dbg,cudieoff,is_info,&cudie,&error);
    if(res != DW_DLV_OK) {
        /* FAIL */
        return;
    }
    print_die_data(dbg,cudie);
    dwarf_dealloc(dbg,cudie, DW_DLA_DIE);
}


void example8(Dwarf_Debug dbg, Dwarf_Die somedie)
{
    Dwarf_Signed atcount = 0;
    Dwarf_Attribute *atlist = 0;
    Dwarf_Error error = 0;
    int errv = 0;

    errv = dwarf_attrlist(somedie, &atlist,&atcount, &error);
    if (errv == DW_DLV_OK) {
        Dwarf_Signed i = 0;

        for (i = 0; i < atcount; ++i) {
            /* use atlist[i] */
            dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
        }
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }
}

void
example_loclistc(Dwarf_Debug dbg,Dwarf_Attribute someattr)
{
    Dwarf_Unsigned lcount = 0;
    Dwarf_Loc_Head_c loclist_head = 0;
    Dwarf_Error error = 0;
    int lres = 0;

    lres = dwarf_get_loclist_c(someattr,&loclist_head,&lcount,&error);
    if (lres == DW_DLV_OK) {
        Dwarf_Unsigned i = 0;
        Dwarf_Locdesc_c locentry = 0;

        /*  Before any return remember to call
            dwarf_loc_head_c_dealloc(loclist_head); */
        for (i = 0; i < lcount; ++i) {
            Dwarf_Small loclist_source = 0;
            Dwarf_Small lle_value = 0; /* DWARF5 */
            Dwarf_Addr lopc = 0;
            Dwarf_Addr hipc = 0;
            Dwarf_Unsigned ulocentry_count = 0;
            Dwarf_Locdesc_c locentry = 0;

            /*  section_offset is the section offset of the expression, not
                the location description prefix. */
            Dwarf_Unsigned section_offset = 0;

            /*  locdesc_offset is the section offset of the
                location description prefix. */
            Dwarf_Unsigned locdesc_offset = 0;

            lres = dwarf_get_locdesc_entry_c(loclist_head,
                i,
                &lle_value,&lopc,&hipc,
                &ulocentry_count,
                &locentry,
                &loclist_source,
                &section_offset,
                &locdesc_offset,
                &error);
            if (lres == DW_DLV_OK) {
                /*  Here, use loclist_source and
                    lle_value to determine what
                    sort of loclist it is and what to do with
                    the values. locentry_count will only be
                    more than zero if there is a set of location
                    operators.
                    One must use lle_value to determine how
                    to interpret lopc,hipc as sometimes they
                    are a target address and sometimes an
                    index into .debug_addr or even a length. */
                Dwarf_Unsigned j = 0;
                int opres = 0;
                Dwarf_Small op = 0;

                for (j = 0; i < ulocentry_count; ++i) {
                    Dwarf_Unsigned opd1 = 0;
                    Dwarf_Unsigned opd2 = 0;
                    Dwarf_Unsigned opd3 = 0;
                    Dwarf_Unsigned offsetforbranch = 0;

                    opres = dwarf_get_location_op_value_c(locentry,
                        j,&op,&opd1, &opd2,&opd3,&offsetforbranch,
                        &error);
                    if (opres == DW_DLV_OK) {
                        /* Do something with the operators. */
                    } else {
                        /*Something is wrong. */
                    }
                }
            } else {
                /* Something is wrong. Do something. */
            }
        }
        /* In case of error or any other situation where one
            is giving up one can call dwarf_loc_head_c_dealloc()
            to free all the memory associated with loclist_head.  */
        dwarf_loc_head_c_dealloc(loclist_head);
        loclist_head = 0;
    }
}
void
example_locexprc(Dwarf_Debug dbg,Dwarf_Ptr expr_bytes,
   Dwarf_Unsigned expr_len,
   Dwarf_Half addr_size,
   Dwarf_Half offset_size,
   Dwarf_Half version)
{
    Dwarf_Loc_Head_c head = 0;
    Dwarf_Locdesc_c locentry = 0;
    int res2 = 0;
    Dwarf_Unsigned lopc = 0;
    Dwarf_Unsigned hipc = 0;
    Dwarf_Unsigned ulistlen = 0;
    Dwarf_Unsigned ulocentry_count = 0;
    Dwarf_Unsigned section_offset = 0;
    Dwarf_Unsigned locdesc_offset = 0;
    Dwarf_Small lle_value = 0;
    Dwarf_Small loclist_source = 0;
    Dwarf_Unsigned i = 0;
    Dwarf_Error error = 0;

    res2 = dwarf_loclist_from_expr_c(dbg,
        expr_bytes,expr_len,
        addr_size,
        offset_size,
        version,
        &head,
        &ulistlen,
        &error);
    if(res2 == DW_DLV_NO_ENTRY) {
        return;
    }
    if(res2 == DW_DLV_ERROR) {
        return;
    }
    /*  These are a location expression, not loclist.
        So we just need the 0th entry. */
    res2 = dwarf_get_locdesc_entry_c(head,
        0, /* Data from 0th LocDesc */
        &lle_value,
        &lopc, &hipc,
        &ulocentry_count,
        &locentry,
        &loclist_source,
        &section_offset,
        &locdesc_offset,
        &error);
    if (res2 == DW_DLV_ERROR) {
        dwarf_loc_head_c_dealloc(head);
        return;
    } else if (res2 == DW_DLV_NO_ENTRY) {
        dwarf_loc_head_c_dealloc(head);
        return;
    }
    /*  ASSERT: ulistlen == 1 */
    for (i = 0; i < ulocentry_count;++i) {
        Dwarf_Small op = 0;
        Dwarf_Unsigned opd1 = 0;
        Dwarf_Unsigned opd2 = 0;
        Dwarf_Unsigned opd3 = 0;
        Dwarf_Unsigned offsetforbranch = 0;

        res2 = dwarf_get_location_op_value_c(locentry,
            i, &op,&opd1,&opd2,&opd3,&offsetforbranch,
            &error);
        /* Do something with the expression operator and operands */
        if (res2 != DW_DLV_OK) {
            dwarf_loc_head_c_dealloc(head);
            return;
        }
    }
    dwarf_loc_head_c_dealloc(head);
}

void
example9(Dwarf_Debug dbg,Dwarf_Attribute someattr)
{
    Dwarf_Signed lcount = 0;
    Dwarf_Locdesc **llbuf = 0;
    Dwarf_Error error = 0;
    int lres = 0;

    lres = dwarf_loclist_n(someattr, &llbuf,&lcount,&error);
    if (lres == DW_DLV_OK) {
        Dwarf_Signed i = 0;
        for (i = 0; i < lcount; ++i) {
            /*  Use llbuf[i]. Both Dwarf_Locdesc and the
                array of Dwarf_Loc it points to are
                defined in libdwarf.h: they are
                not opaque structs. */
            dwarf_dealloc(dbg, llbuf[i]->ld_s, DW_DLA_LOC_BLOCK);
            dwarf_dealloc(dbg,llbuf[i], DW_DLA_LOCDESC);
        }
        dwarf_dealloc(dbg, llbuf, DW_DLA_LIST);
    }
}


void examplea(Dwarf_Debug dbg,Dwarf_Attribute someattr)
{
    Dwarf_Signed lcount = 0;
    Dwarf_Locdesc *llbuf = 0;
    Dwarf_Error error = 0;
    int lres = 0;

    lres = dwarf_loclist(someattr, &llbuf,&lcount,&error);
    if (lres == DW_DLV_OK) {
        /* lcount is always 1, (and has always been 1) */
        /* Use llbuf here. */

        dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
    }
}

void exampleb(Dwarf_Debug dbg,Dwarf_Ptr data, Dwarf_Unsigned len)
{
    Dwarf_Signed lcount = 0;
    Dwarf_Locdesc *llbuf = 0;
    Dwarf_Error error = 0;
    int lres = 0;

    lres = dwarf_loclist_from_expr(dbg,data,len, &llbuf,&lcount, &error);
    if (lres == DW_DLV_OK) {
        /* lcount is always 1 */
        /* Use llbuf  here.*/

        dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
    }
}


void examplec(Dwarf_Die cu_die)
{
    /* EXAMPLE: DWARF5 style access.  */
    Dwarf_Line  *linebuf = 0;
    Dwarf_Signed linecount = 0;
    Dwarf_Line  *linebuf_actuals = 0;
    Dwarf_Signed linecount_actuals = 0;
    Dwarf_Line_Context line_context = 0;
    Dwarf_Signed linecount_total = 0;
    Dwarf_Small  table_count = 0;
    Dwarf_Unsigned lineversion = 0;
    Dwarf_Error err = 0;
    int sres = 0;
    /* ... */
    /*  we use 'return' here to signify we can do nothing more
        at this point in the code. */
    sres = dwarf_srclines_b(cu_die,&lineversion,
        &table_count,&line_context,&err);
    if (sres != DW_DLV_OK) {
        /*  Handle the DW_DLV_NO_ENTRY  or DW_DLV_ERROR
            No memory was allocated so there nothing
            to dealloc. */
        return;
    }
    if (table_count == 0) {
        /*  A line table with no actual lines.  */
        /*...do something, see dwarf_srclines_files_count()
            etc below. */

        dwarf_srclines_dealloc_b(line_context);
        /*  All the memory is released, the line_context
            and linebuf zeroed now
            as a reminder they are stale. */
        linebuf = 0;
        line_context = 0;
    } else if (table_count == 1) {
        Dwarf_Signed i = 0;
        /*  Standard dwarf 2,3,4, or 5 line table */
        /*  Do something. */
        /*  For this case where we have a line table we will likely
            wish to get the line details: */
        sres = dwarf_srclines_from_linecontext(line_context,
            &linebuf,&linecount,
            &err);
        if (sres != DW_DLV_OK) {
            /* Error. Clean up the context information. */
            dwarf_srclines_dealloc_b(line_context);
            return;
        }
        /* The lines are normal line table lines. */
        for (i = 0; i < linecount; ++i) {
            /* use linebuf[i] */
        }
        dwarf_srclines_dealloc_b(line_context);
        /*  All the memory is released, the line_context
            and linebuf zeroed now as a reminder they are stale */
        linebuf = 0;
        line_context = 0;
        linecount = 0;
    } else {
        Dwarf_Signed i = 0;
        /*  ASSERT: table_count == 2,
            Experimental two-level line table. Version 0xf006
            We do not define the meaning of this non-standard
            set of tables here. */

        /*  For 'something C' (two-level line tables)
            one codes something like this
            Note that we do not define the meaning or use of two-level line
            tables as these are experimental, not standard DWARF. */
        sres = dwarf_srclines_two_level_from_linecontext(line_context,
            &linebuf,&linecount,
            &linebuf_actuals,&linecount_actuals,
            &err);
        if (sres == DW_DLV_OK) {
            for (i = 0; i < linecount; ++i) {
                /* use linebuf[i], these are the 'logicals' entries. */
            }
            for (i = 0; i < linecount_actuals; ++i) {
                /* use linebuf_actuals[i], these are the actuals entries */
            }
            dwarf_srclines_dealloc_b(line_context);
            line_context = 0;
            linebuf = 0;
            linecount = 0;
            linebuf_actuals = 0;
            linecount_actuals = 0;
        } else if (sres == DW_DLV_NO_ENTRY) {
            /* This should be impossible, but do something.   */
            /* Then Free the line_context */
            dwarf_srclines_dealloc_b(line_context);
            line_context = 0;
            linebuf = 0;
            linecount = 0;
            linebuf_actuals = 0;
            linecount_actuals = 0;
        } else {
            /*  ERROR, show the error or something.
                Free the line_context. */
            dwarf_srclines_dealloc_b(line_context);
            line_context = 0;
            linebuf = 0;
            linecount = 0;
            linebuf_actuals = 0;
            linecount_actuals = 0;
        }
    }
}

void exampled(Dwarf_Debug dbg,Dwarf_Die somedie)
{
    Dwarf_Signed count = 0;
    Dwarf_Line *linebuf = 0;
    Dwarf_Signed i = 0;
    Dwarf_Error error = 0;
    int sres = 0;

    sres = dwarf_srclines(somedie, &linebuf,&count, &error);
    if (sres == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use linebuf[i] */
        }
        dwarf_srclines_dealloc(dbg, linebuf, count);
    }
}

void examplee(Dwarf_Debug dbg,Dwarf_Die somedie)
{
    Dwarf_Signed count = 0;
    char **srcfiles = 0;
    Dwarf_Signed i = 0;
    Dwarf_Error error = 0;
    int res = 0;

    res = dwarf_srcfiles(somedie, &srcfiles,&count,&error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use srcfiles[i] */
            dwarf_dealloc(dbg, srcfiles[i], DW_DLA_STRING);
        }
        dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
    }
}

void examplef(Dwarf_Debug dbg)
{
    Dwarf_Signed count = 0;
    Dwarf_Global *globs = 0;
    Dwarf_Signed i = 0;
    Dwarf_Error  error = 0;
    int res = 0;

    res = dwarf_get_globals(dbg, &globs,&count, &error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use globs[i] */
        }
        dwarf_globals_dealloc(dbg, globs, count);
    }
}



void exampleg(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Type *types = 0;
    Dwarf_Signed i = 0;
    int res = 0;

    res = dwarf_get_pubtypes(dbg, &types,&count, &error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use types[i] */
        }
        dwarf_types_dealloc(dbg, types, count);
    }
}

void exampleh(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Weak *weaks = 0;
    Dwarf_Signed i = 0;
    int res = 0;

    res = dwarf_get_weaks(dbg, &weaks, &count, &error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use weaks[i] */
        }
        dwarf_weaks_dealloc(dbg, weaks, count);
    }
}

void examplei(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Weak *weaks = 0;
    Dwarf_Signed i = 0;
    int res = 0;

    res = dwarf_get_weaks(dbg, &weaks, &count, &error);
    if (res == DW_DLV_OK) {
        /*  OBSOLETE: do not use dealloc for this.
            See above */
        for (i = 0; i < count; ++i) {
            /* use weaks[i] */
            dwarf_dealloc(dbg, weaks[i], DW_DLA_WEAK);
        }
        dwarf_dealloc(dbg, weaks, DW_DLA_LIST);
    }
}

void examplej(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Func *funcs = 0;
    Dwarf_Signed i = 0;
    int fres = 0;

    fres = dwarf_get_funcs(dbg, &funcs, &count, &error);
    if (fres == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use funcs[i] */
        }
        dwarf_funcs_dealloc(dbg, funcs, count);
    }
}

void examplek(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Func *funcs = 0;
    Dwarf_Signed count = 0;
    Dwarf_Signed i = 0;
    int fres = 0;

    fres = dwarf_get_funcs(dbg, &funcs,&count, &error);
    if (fres == DW_DLV_OK) {
        /* OBSOLETE: see dwarf_funcs_dealloc() above */
        for (i = 0; i < count; ++i) {
            /* use funcs[i] */
            dwarf_dealloc(dbg, funcs[i], DW_DLA_FUNC);
        }
        dwarf_dealloc(dbg, funcs, DW_DLA_LIST);
    }
}

void examplel(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Type *types = 0;
    Dwarf_Signed i = 0;
    int res = 0;

    res = dwarf_get_types(dbg, &types,&count, &error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use types[i] */
        }
        dwarf_types_dealloc(dbg, types, count);
    }
}

void examplem(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Type *types = 0;
    Dwarf_Signed i = 0;
    int res = 0;

    /*  OBSOLETE: see dwarf_types_dealloc() above */
    res = dwarf_get_types(dbg, &types,&count, &error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use types[i] */
            dwarf_dealloc(dbg, types[i], DW_DLA_TYPENAME);
        }
        dwarf_dealloc(dbg, types, DW_DLA_LIST);
    }
}

void examplen(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Var *vars = 0;
    Dwarf_Signed i = 0;
    int res = 0;
    res = dwarf_get_vars(dbg, &vars,&count,&error);
    if (res == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use vars[i] */
        }
        dwarf_vars_dealloc(dbg, vars, count);
    }
}


void exampleo(Dwarf_Debug dbg)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Var *vars = 0;
    Dwarf_Signed i = 0;
    int res = 0;
    res = dwarf_get_vars(dbg, &vars,&count,&error);
    if (res == DW_DLV_OK) {
        /* DO NOT USE: see dwarf_vars_dealloc() above */
        for (i = 0; i < count; ++i) {
            /* use vars[i] */
            dwarf_dealloc(dbg, vars[i], DW_DLA_VAR);
        }
        dwarf_dealloc(dbg, vars, DW_DLA_LIST);
    }
}



void examplep(Dwarf_Debug dbg, Dwarf_Off cur_off)
{
    Dwarf_Error error = 0;
    Dwarf_Signed count = 0;
    Dwarf_Macro_Details *maclist = 0;
    Dwarf_Signed i = 0;
    Dwarf_Unsigned max = 500000; /* sanity limit */
    int errv = 0;

    /*  Given an offset from a compilation unit,
        start at that offset (from DW_AT_macroinfo)
        and get its macro details. */
    errv = dwarf_get_macro_details(dbg, cur_off,max,
        &count,&maclist,&error);
    if (errv == DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use maclist[i] */
        }
        dwarf_dealloc(dbg, maclist, DW_DLA_STRING);
    }
    /*  Loop through all the compilation units macro info from zero.
        This is not guaranteed to work because DWARF does not
        guarantee every byte in the section is meaningful:
        there can be garbage between the macro info
        for CUs.  But this loop will sometimes work.
    */
    cur_off = 0;
    while((errv = dwarf_get_macro_details(dbg, cur_off,max,
        &count,&maclist,&error))== DW_DLV_OK) {
        for (i = 0; i < count; ++i) {
            /* use maclist[i] */
        }
        cur_off = maclist[count-1].dmd_offset + 1;
        dwarf_dealloc(dbg, maclist, DW_DLA_STRING);
    }
}

void exampleq(Dwarf_Debug dbg)
{
    Dwarf_Cie *cie_data = 0;
    Dwarf_Signed cie_count = 0;
    Dwarf_Fde *fde_data = 0;
    Dwarf_Signed fde_count = 0;
    Dwarf_Error error = 0;
    int fres = 0;

    fres = dwarf_get_fde_list(dbg,&cie_data,&cie_count,
        &fde_data,&fde_count,&error);
    if (fres == DW_DLV_OK) {
        dwarf_fde_cie_list_dealloc(dbg, cie_data, cie_count,
            fde_data,fde_count);
    }
}

/* OBSOLETE EXAMPLE */
void exampleqb(Dwarf_Debug dbg)
{
    Dwarf_Signed count = 0;
    Dwarf_Cie *cie_data = 0;
    Dwarf_Signed cie_count = 0;
    Dwarf_Fde *fde_data = 0;
    Dwarf_Signed fde_count = 0;
    Dwarf_Error error = 0;
    Dwarf_Signed i = 0;
    int fres = 0;

    fres = dwarf_get_fde_list(dbg,&cie_data,&cie_count,
        &fde_data,&fde_count,&error);
    if (fres == DW_DLV_OK) {
        for (i = 0; i < cie_count; ++i) {
            /* use cie[i] */
            dwarf_dealloc(dbg, cie_data[i], DW_DLA_CIE);
        }
        for (i = 0; i < fde_count; ++i) {
            /* use fde[i] */
            dwarf_dealloc(dbg, fde_data[i], DW_DLA_FDE);
        }
        dwarf_dealloc(dbg, cie_data, DW_DLA_LIST);
        dwarf_dealloc(dbg, fde_data, DW_DLA_LIST);
    }
}


void exampler(Dwarf_Debug dbg,Dwarf_Addr mypcval)
{
    /*  Given a pc value
        for a function find the FDE and CIE data for
        the function.
        Example shows basic access to FDE/CIE plus
        one way to access details given a PC value.
        dwarf_get_fde_n() allows accessing all FDE/CIE
        data so one could build up an application-specific
        table of information if that is more useful.  */
    Dwarf_Signed count = 0;
    Dwarf_Cie *cie_data = 0;
    Dwarf_Signed cie_count = 0;
    Dwarf_Fde *fde_data = 0;
    Dwarf_Signed fde_count = 0;
    Dwarf_Error error = 0;
    int fres = 0;

    fres = dwarf_get_fde_list_eh(dbg,&cie_data,&cie_count,
        &fde_data,&fde_count,&error);
    if (fres == DW_DLV_OK) {
        Dwarf_Fde myfde = 0;
        Dwarf_Addr low_pc = 0;
        Dwarf_Addr high_pc = 0;
        fres = dwarf_get_fde_at_pc(fde_data,mypcval,
            &myfde,&low_pc,&high_pc,
            &error);
        if (fres == DW_DLV_OK) {
            Dwarf_Cie mycie = 0;
            fres = dwarf_get_cie_of_fde(myfde,&mycie,&error);
            if (fres == DW_DLV_OK) {
                /*  Now we can access a range of information
                    about the fde and cie applicable. */
            }
        }
        dwarf_fde_cie_list_dealloc(dbg, cie_data, cie_count,
            fde_data,fde_count);
    }
    /* ERROR or NO ENTRY. Do something */
}

void examples(Dwarf_Debug dbg,Dwarf_Cie cie,
  Dwarf_Ptr instruction,Dwarf_Unsigned len)
{
    Dwarf_Signed count = 0;
    Dwarf_Frame_Op *frameops = 0;
    Dwarf_Error error = 0;
    int res = 0;

    res = dwarf_expand_frame_instructions(cie,instruction,len,
        &frameops,&count, &error);
    if (res == DW_DLV_OK) {
        Dwarf_Signed i = 0;

        for (i = 0; i < count; ++i) {
            /* use frameops[i] */
        }
        dwarf_dealloc(dbg, frameops, DW_DLA_FRAME_BLOCK);
    }
}

void examplet(Dwarf_Debug dbg,Dwarf_Unsigned offset)
{
    /*  Looping through the dwarf_loc section finding loclists:
        an example.  */
    int res;
    Dwarf_Unsigned next_entry = 0;
    Dwarf_Addr hipc_off = 0;
    Dwarf_Addr lowpc_off = 0;
    Dwarf_Ptr data = 0;
    Dwarf_Unsigned entry_len = 0;
    Dwarf_Error err = 0;

    for(;;) {
        res = dwarf_get_loclist_entry(dbg,offset,&hipc_off,
            &lowpc_off, &data, &entry_len,&next_entry,&err);
        if (res == DW_DLV_OK) {
            /* A valid entry. */
            offset = next_entry;
            continue;
        } else if (res ==DW_DLV_NO_ENTRY) {
            /* Done! */
            break;
        } else {
            /* Error! */
            break;
        }
    }
}

void exampleu(Dwarf_Debug dbg)
{
    Dwarf_Signed count = 0;
    Dwarf_Arange *arang = 0;
    int res = 0;
    Dwarf_Error error = 0;

    res = dwarf_get_aranges(dbg, &arang,&count, &error);
    if (res == DW_DLV_OK) {
        Dwarf_Signed i = 0;

        for (i = 0; i < count; ++i) {
            /* use arang[i] */
            dwarf_dealloc(dbg, arang[i], DW_DLA_ARANGE);
        }
        dwarf_dealloc(dbg, arang, DW_DLA_LIST);
    }
}
void examplev(Dwarf_Debug dbg,Dwarf_Unsigned offset,Dwarf_Die die)
{
    Dwarf_Signed count = 0;
    Dwarf_Ranges *ranges = 0;
    Dwarf_Unsigned bytes = 0;
    Dwarf_Error error = 0;
    int res = 0;
    res = dwarf_get_ranges_a(dbg,offset,die,
        &ranges,&count,&bytes,&error);
    if (res == DW_DLV_OK) {
        Dwarf_Signed i;
        for( i = 0; i < count; ++i ) {
            Dwarf_Ranges *cur = ranges+i;
            /* Use cur. */
        }
        dwarf_ranges_dealloc(dbg,ranges,count);
    }
}
void examplew(Dwarf_Debug dbg,Dwarf_Unsigned offset,Dwarf_Die die)
{
    Dwarf_Gdbindex gindexptr = 0;
    Dwarf_Unsigned version = 0;
    Dwarf_Unsigned cu_list_offset = 0;
    Dwarf_Unsigned types_cu_list_offset = 0;
    Dwarf_Unsigned address_area_offset = 0;
    Dwarf_Unsigned symbol_table_offset = 0;
    Dwarf_Unsigned constant_pool_offset = 0;
    Dwarf_Unsigned section_size = 0;
    Dwarf_Unsigned reserved = 0;
    Dwarf_Error error = 0;
    const char * section_name = 0;
    int res = 0;
    res = dwarf_gdbindex_header(dbg,&gindexptr,
        &version,&cu_list_offset, &types_cu_list_offset,
        &address_area_offset,&symbol_table_offset,
        &constant_pool_offset, &section_size,
        &reserved,&section_name,&error);
    if (res == DW_DLV_NO_ENTRY) {
        return;
    } else if (res == DW_DLV_ERROR) {
        return;
    }
    {
        /* do something with the data */
        Dwarf_Unsigned length = 0;
        Dwarf_Unsigned typeslength = 0;
        Dwarf_Unsigned i = 0;
        res = dwarf_gdbindex_culist_array(gindexptr,
            &length,&error);
        /* Example actions. */
        if (res == DW_DLV_OK) {
            for(i = 0; i < length; ++i) {
                Dwarf_Unsigned cuoffset = 0;
                Dwarf_Unsigned culength = 0;
                res = dwarf_gdbindex_culist_entry(gindexptr,
                    i,&cuoffset,&culength,&error);
                if (res == DW_DLV_OK) {
                    /* Do something with cuoffset, culength */
                }
            }
        }
        res = dwarf_gdbindex_types_culist_array(gindexptr,
            &typeslength,&error);
        if (res == DW_DLV_OK) {
            for(i = 0; i < typeslength; ++i) {
                Dwarf_Unsigned cuoffset = 0;
                Dwarf_Unsigned tuoffset = 0;
                Dwarf_Unsigned culength = 0;
                Dwarf_Unsigned type_signature  = 0;
                res = dwarf_gdbindex_types_culist_entry(gindexptr,
                    i,&cuoffset,&tuoffset,&type_signature,&error);
                if (res == DW_DLV_OK) {
                    /* Do something with cuoffset etc. */
                }
            }
        }
        dwarf_gdbindex_free(gindexptr);
    }
}
void examplewgdbindex(Dwarf_Gdbindex gdbindex)
{
    Dwarf_Unsigned list_len = 0;
    Dwarf_Unsigned i = 0;
    int res = 0;
    Dwarf_Error err = 0;

    res = dwarf_gdbindex_addressarea(gdbindex, &list_len,&err);
    if (res != DW_DLV_OK) {
        /* Something wrong, ignore the addressarea */
    }
    /* Iterate through the address area. */
    for( i  = 0; i < list_len; i++) {
        Dwarf_Unsigned lowpc = 0;
        Dwarf_Unsigned highpc = 0;
        Dwarf_Unsigned cu_index,
        res = dwarf_gdbindex_addressarea_entry(gdbindex,i,
            &lowpc,&highpc,
            &cu_index,
            &err);
        if (res != DW_DLV_OK) {
            /* Something wrong, ignore the addressarea */
            return;
        }
        /*  We have a valid address area entry, do something
            with it. */
    }
}
void examplex(Dwarf_Gdbindex gdbindex)
{
    Dwarf_Unsigned symtab_list_length = 0;
    Dwarf_Unsigned i = 0;
    Dwarf_Error err = 0;
    int res = 0;

    res = dwarf_gdbindex_symboltable_array(gdbindex,
        &symtab_list_length,&err);
    if (res != DW_DLV_OK) {
        return;
    }
    for( i  = 0; i < symtab_list_length; i++) {
        Dwarf_Unsigned symnameoffset = 0;
        Dwarf_Unsigned cuvecoffset = 0;
        Dwarf_Unsigned cuvec_len = 0;
        Dwarf_Unsigned ii = 0;
        const char *name = 0;
        res = dwarf_gdbindex_symboltable_entry(gdbindex,i,
            &symnameoffset,&cuvecoffset,
            &err);
        if (res != DW_DLV_OK) {
            return;
        }
        res = dwarf_gdbindex_string_by_offset(gdbindex,
            symnameoffset,&name,&err);
        if(res != DW_DLV_OK) {
            return;
        }
        res = dwarf_gdbindex_cuvector_length(gdbindex,
            cuvecoffset,&cuvec_len,&err);
        if( res != DW_DLV_OK) {
            return;
        }
        for(ii = 0; ii < cuvec_len; ++ii ) {
            Dwarf_Unsigned attributes = 0;
            Dwarf_Unsigned cu_index = 0;
            Dwarf_Unsigned reserved1 = 0;
            Dwarf_Unsigned symbol_kind = 0;
            Dwarf_Unsigned is_static = 0;

            res = dwarf_gdbindex_cuvector_inner_attributes(
                gdbindex,cuvecoffset,ii,
                &attributes,&err);
            if( res != DW_DLV_OK) {
                return;
            }
            /*  'attributes' is a value with various internal
                fields so we expand the fields. */
            res = dwarf_gdbindex_cuvector_instance_expand_value(gdbindex,
                attributes, &cu_index,&reserved1,&symbol_kind, &is_static,
                &err);
            if( res != DW_DLV_OK) {
                return;
            }
            /* Do something with the attributes. */
        }
    }
}

void exampley(Dwarf_Debug dbg, const char *type)
{
    /* type is "tu" or "cu" */
    int res = 0;
    Dwarf_Xu_Index_Header xuhdr = 0;
    Dwarf_Unsigned version_number = 0;
    Dwarf_Unsigned offsets_count = 0; /*L */
    Dwarf_Unsigned units_count = 0; /* M */
    Dwarf_Unsigned hash_slots_count = 0; /* N */
    Dwarf_Error err = 0;
    const char * ret_type = 0;
    const char * section_name = 0;

    res = dwarf_get_xu_index_header(dbg,
        type,
        &xuhdr,
        &version_number,
        &offsets_count,
        &units_count,
        &hash_slots_count,
        &section_name,
        &err);
    if (res == DW_DLV_NO_ENTRY) {
        /* No such section. */
        return;
    }
    if (res == DW_DLV_ERROR) {
        /* Something wrong. */
        return;
    }
    /* Do something with the xuhdr here . */
    dwarf_xu_header_free(xuhdr);
}

void examplez( Dwarf_Xu_Index_Header xuhdr,
  Dwarf_Unsigned hash_slots_count)
{
    /*  hash_slots_count returned by
        dwarf_get_xu_index_header(), see above. */
    static Dwarf_Sig8 zerohashval;

    Dwarf_Error err = 0;
    Dwarf_Unsigned h = 0;

    for( h = 0; h < hash_slots_count; h++) {
        Dwarf_Sig8 hashval;
        Dwarf_Unsigned index = 0;
        Dwarf_Unsigned col = 0;
        int res = 0;

        res = dwarf_get_xu_hash_entry(xuhdr,h,
            &hashval,&index,&err);
        if (res == DW_DLV_ERROR) {
            /* Oops. hash_slots_count wrong. */
            return;
        } else if (res == DW_DLV_NO_ENTRY) {
            /* Impossible */
            return;
        } else if (!memcmp(&hashval,&zerohashval,sizeof(Dwarf_Sig8))
            && index == 0 ) {
            /* An unused hash slot */
            continue;
        }
        /*  Here, hashval and index (a row index into offsets and lengths)
            are valid. */
    }
}

void exampleza(Dwarf_Xu_Index_Header xuhdr,
  Dwarf_Unsigned offsets_count, Dwarf_Unsigned index )
{
    Dwarf_Error err = 0;
    Dwarf_Unsigned col = 0;
    /*  We use  'offsets_count' returned by
        a dwarf_get_xu_index_header() call.
        We use 'index' returned by a
        dwarf_get_xu_hash_entry() call. */
    for (col = 0; col < offsets_count; col++) {
        Dwarf_Unsigned off = 0;
        Dwarf_Unsigned len = 0;
        const char * name = 0;
        Dwarf_Unsigned num = 0;
        int res = 0;

        res = dwarf_get_xu_section_names(xuhdr,
            col,&num,&name,&err);
        if (res != DW_DLV_OK) {
            break;
        }
        res = dwarf_get_xu_section_offset(xuhdr,
            index,col,&off,&len,&err);
        if (res != DW_DLV_OK) {
            break;
        }
        /*  Here we have the DW_SECT_ name and number
            and the base offset and length of the
            section data applicable to the hash
            that got us here.
            Use the values.*/
    }
}

void examplezb(void)
{
    const char * out = 0;
    int res = 0;

    /* The following is wrong, do not do it! */
    res = dwarf_get_ACCESS_name(DW_TAG_entry_point,&out);
    /*  Nothing one does here with 'res' or 'out'
        is meaningful. */

    /* The following is meaningful.*/
    res = dwarf_get_TAG_name(DW_TAG_entry_point,&out);
    if( res == DW_DLV_OK) {
        /*  Here 'out' is a pointer one can use which
            points to the string "DW_TAG_entry_point". */
    } else {
        /*  Here 'out' has not been touched, it is
            uninitialized.  Do not use it. */
    }
}
