/*
  Copyright (C) 2017-2017 David Anderson. All Rights Reserved.

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
#include "dwarf_tsearch.h"
#include <stdlib.h>
#include <stdio.h>

#define TRUE  1
#define FALSE 0

#define HASHSEARCH

/*  Each section number can appear in at most one record in the hash
    because each section belongs in only one group.
    Each group number appears as often as appropriate. */
struct Dwarf_Group_Map_Entry_s {
  unsigned  gm_key;  /* section number */
  unsigned  gm_group_number; /* What group number is. */
};

static void *
grp_make_entry(unsigned section, unsigned group)
{
    struct Dwarf_Group_Map_Entry_s *e = 0;
    e = calloc(1,sizeof(struct Dwarf_Group_Map_Entry_s));
    if(e) {
        e->gm_key =    section;
        e->gm_group_number = group;
    }
    return e;
}


static DW_TSHASHTYPE
grp_data_hashfunc(const void *keyp)
{
    const struct Dwarf_Group_Map_Entry_s * enp = keyp;
    DW_TSHASHTYPE hashv = 0;

    hashv = enp->gm_key;
    return hashv;
}

static int
grp_compare_function(const void *l, const void *r)
{
    const struct Dwarf_Group_Map_Entry_s * lp = l;
    const struct Dwarf_Group_Map_Entry_s * rp = r;

    if (lp->gm_key < rp->gm_key) {
        return -1;
    }
    if (lp->gm_key > rp->gm_key) {
        return 1;
    }

    /* match. */
    return 0;
}

void
_dwarf_grp_destroy_free_node(void*nodep)
{
    struct Dwarf_Group_Map_Entry_s * enp = nodep;
    free(enp);
    return;
}

int
_dwarf_insert_in_group_map(Dwarf_Debug dbg,
    unsigned groupnum,
    unsigned section_index,
    Dwarf_Error * error)
{
    struct Dwarf_Group_Data_s *grp = &dbg->de_groupnumbers;

    void *entry2 = 0;
    struct Dwarf_Group_Map_Entry_s * entry3 = 0;

    if (!grp->gd_map) {
        /*  Number of sections is a kind of decent guess
            as to how much space would be useful. */
        dwarf_initialize_search_hash(&grp->gd_map,
            grp_data_hashfunc,grp->gd_number_of_sections);
        if (!grp->gd_map) {
            /*  It's really an error I suppose. */
            return DW_DLV_NO_ENTRY;
        }
    }
    entry3 = grp_make_entry(section_index,groupnum);
    if (!entry3) {
        _dwarf_error(dbg, error, DW_DLE_GROUP_MAP_ALLOC);
        return DW_DLV_ERROR;
    }
    entry2 = dwarf_tsearch(entry3,&grp->gd_map,grp_compare_function);
    if (!entry2) {
        free(entry3);
        _dwarf_error(dbg, error, DW_DLE_GROUP_MAP_ALLOC);
        return DW_DLV_ERROR;
    } else {
        struct Dwarf_Group_Map_Entry_s *re = 0;
        re = *(struct Dwarf_Group_Map_Entry_s **)entry2;
        if (re != entry3) {
            free(entry3);
            _dwarf_error(dbg, error, DW_DLE_GROUP_MAP_DUPLICATE);
            return DW_DLV_ERROR;
        } else {
            ++grp->gd_map_entry_count;
            /* OK. Added. Fall thru */
        }
    }
    return DW_DLV_OK;
}

int
_dwarf_section_get_target_group(Dwarf_Debug dbg,
    unsigned   obj_section_index,
    unsigned * groupnumber_out,
    UNUSEDARG Dwarf_Error    * error)
{
    struct Dwarf_Group_Map_Entry_s entry;
    struct Dwarf_Group_Map_Entry_s *entry2;
    struct Dwarf_Group_Data_s *grp = &dbg->de_groupnumbers;

    if (!grp->gd_map) {
        return DW_DLV_NO_ENTRY;
    }
    entry.gm_key = obj_section_index;
    entry.gm_group_number = 0; /* FAKE */

    entry2 = dwarf_tfind(&entry, &grp->gd_map,grp_compare_function);
    if (entry2) {
        struct Dwarf_Group_Map_Entry_s *e2 =
            *(struct Dwarf_Group_Map_Entry_s **)entry2;;
        *groupnumber_out = e2->gm_group_number;
        return DW_DLV_OK;
    }
    return DW_DLV_NO_ENTRY;
}




/*  New May 2017.  So users can find out what groups (dwo or COMDAT)
    are in the object and how much to allocate so one can get the
    group-section map data. */
int dwarf_sec_group_sizes(Dwarf_Debug dbg,
    Dwarf_Unsigned * section_count_out,
    Dwarf_Unsigned * group_count_out,
    Dwarf_Unsigned * selected_group_out,
    Dwarf_Unsigned * map_entry_count_out,
    UNUSEDARG Dwarf_Error    * error)
{
    struct Dwarf_Group_Data_s *grp = &dbg->de_groupnumbers;

    *section_count_out   = grp->gd_number_of_sections;
    *group_count_out     = grp->gd_number_of_groups;
    *selected_group_out  = dbg->de_groupnumber;
    *map_entry_count_out = grp->gd_map_entry_count;
    return DW_DLV_OK;
}


static Dwarf_Unsigned map_reccount = 0;
static struct temp_map_struc_s {
    Dwarf_Unsigned section;
    Dwarf_Unsigned group;
} *temp_map_data;


static void
grp_walk_map(const void *nodep,
    const DW_VISIT which,
    UNUSEDARG const int depth)
{
    struct Dwarf_Group_Map_Entry_s *re = 0;

    re = *(struct Dwarf_Group_Map_Entry_s **)nodep;
    if (which == dwarf_postorder || which == dwarf_endorder) {
        return;
    }
    temp_map_data[map_reccount].group   = re->gm_group_number;
    temp_map_data[map_reccount].section = re->gm_key;
    map_reccount += 1;
}

/* Looks better sorted by group then sec num. */
static int
map_sort_compar(const void*l, const void*r)
{
    struct temp_map_struc_s *lv = (struct temp_map_struc_s *)l;
    struct temp_map_struc_s *rv = (struct temp_map_struc_s *)r;

    if (lv->group < rv->group) {
        return -1;
    }
    if (lv->group > rv->group) {
        return 1;
    }
    if (lv->section < rv->section) {
        return -1;
    }
    if (lv->section > rv->section) {
        return 1;
    }
    /* Should never get here! */
    return 0;

}

/*  New May 2017. Reveals the map between group numbers
    and section numbers.
    Caller must allocate the arrays with space for 'map_entry_count'
    values and this function fills in the array entries.
    Output ordered by group number and section number.
    */
int dwarf_sec_group_map(Dwarf_Debug dbg,
    Dwarf_Unsigned   map_entry_count,
    Dwarf_Unsigned * group_numbers_array,
    Dwarf_Unsigned * sec_numbers_array,
    Dwarf_Error    * error)
{
    Dwarf_Unsigned i = 0;
    struct Dwarf_Group_Data_s *grp = 0;

    if(temp_map_data) {
        _dwarf_error(dbg,error,DW_DLE_GROUP_INTERNAL_ERROR);
        return DW_DLV_ERROR;
    }
    map_reccount = 0;
    grp = &dbg->de_groupnumbers;
    if (map_entry_count < grp->gd_map_entry_count) {
        _dwarf_error(dbg,error,DW_DLE_GROUP_COUNT_ERROR);
        return DW_DLV_ERROR;
    }
    temp_map_data = calloc(map_entry_count,sizeof(struct temp_map_struc_s));
    if(!temp_map_data) {
        _dwarf_error(dbg,error,DW_DLE_GROUP_MAP_ALLOC);
        return DW_DLV_ERROR;
    }
    dwarf_twalk(grp->gd_map,grp_walk_map);
    if (map_reccount != grp->gd_map_entry_count) {
        /*  Impossible. */
        _dwarf_error(dbg,error,DW_DLE_GROUP_INTERNAL_ERROR);
        return DW_DLV_ERROR;
    }

    qsort(temp_map_data,map_reccount,sizeof(struct temp_map_struc_s),
        map_sort_compar);
    for (i =0 ; i < map_reccount; ++i) {
        sec_numbers_array[i] = temp_map_data[i].section;
        group_numbers_array[i] = temp_map_data[i].group;
    }
    free(temp_map_data);
    map_reccount = 0;
    temp_map_data = 0;
    return DW_DLV_OK;
}





