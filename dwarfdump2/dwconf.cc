/*
  Copyright (C) 2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2009-2011 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 51
  Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan


$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/dwconf.c,v 1.4 2006/04/18 18:05:57 davea Exp $ */


#include "globals.h"
#include <vector>
#include <ctype.h>
#include "dwconf.h"
using std::string;
using std::cerr;
using std::cout;
using std::endl;

// The nesting level is arbitrary,  2 should suffice.
// But at least this prevents an infinite loop.
#define MAX_NEST_LEVEL 3 



struct token_s {
    token_s() {};
    ~token_s() {};
    const char *c_str() { return tk_data.c_str(); };
    string tk_data;
};
enum linetype_e {
    LT_ERROR,
    LT_COMMENT,
    LT_BLANK,
    LT_BEGINABI,
    LT_REG,
    LT_FRAME_INTERFACE,
    LT_CFA_REG,
    LT_INITIAL_REG_VALUE,
    LT_SAME_VAL_REG,
    LT_UNDEFINED_VAL_REG,
    LT_REG_TABLE_SIZE,
    LT_ADDRESS_SIZE,
    LT_INCLUDEABI,
    LT_ENDABI
};

struct comtable_s {
    enum linetype_e type;
    string name;
};

/* Count errors found in this scan of the configuration file. */
static int errcount = 0;        

static string name_begin_abi("beginabi:");
static string name_reg("reg:");
static string name_frame_interface("frame_interface:");
static string name_cfa_reg("cfa_reg:");
static string name_initial_reg_value("initial_reg_value:");
static string name_same_val_reg("same_val_reg:");
static string name_undefined_val_reg("undefined_val_reg:");
static string name_reg_table_size("reg_table_size:");
static string name_address_size("address_size:");
static string name_includeabi("includeabi:");
static string name_endabi("endabi:");

static struct comtable_s comtable[] = {
    {LT_BEGINABI, name_begin_abi},
    {LT_REG, name_reg},
    {LT_FRAME_INTERFACE, name_frame_interface},
    {LT_CFA_REG, name_cfa_reg},
    {LT_INITIAL_REG_VALUE, name_initial_reg_value},
    {LT_SAME_VAL_REG,name_same_val_reg},
    {LT_UNDEFINED_VAL_REG,name_undefined_val_reg},
    {LT_REG_TABLE_SIZE, name_reg_table_size},
    {LT_ADDRESS_SIZE, name_address_size},
    {LT_INCLUDEABI, name_includeabi},
    {LT_ENDABI, name_endabi},
};
static unsigned size_of_comtable = sizeof(comtable) / sizeof(comtable[0]);

struct conf_internal_s {
    conf_internal_s(struct dwconf_s *out): beginabi_lineno(0),
        frame_interface_lineno(0), initial_reg_value_lineno(0),
        reg_table_size_lineno(0),address_size_lineno(0),
        same_val_reg_lineno(0),
        undefined_val_reg_lineno(0), cfa_reg_lineno(0),
        regcount(0),
        conf_out(out),conf_defaults(0) {};
    ~conf_internal_s() {};
    unsigned long beginabi_lineno;
    unsigned long frame_interface_lineno;
    unsigned long initial_reg_value_lineno;
    unsigned long reg_table_size_lineno;
    unsigned long address_size_lineno;
    unsigned long same_val_reg_lineno;
    unsigned long undefined_val_reg_lineno;
    unsigned long cfa_reg_lineno;
    unsigned long regcount;
    std::string conf_name_used;
    struct dwconf_s *conf_out;
    std::string conf_named_used;
    const char ** conf_defaults;
private:
    // Do not use this, not implemented.
    conf_internal_s();
};



static FILE *find_a_file(const string &named_file, const char **defaults,
    string & name_used);
static bool find_abi_start(FILE * stream, const string &abi_name, long *offset,
    unsigned long *lineno_out);
static bool parse_abi(FILE * stream, const std::string &fname, 
    const std::string &abiname,
    struct conf_internal_s *out, unsigned long lineno,unsigned nestlevel);
static char * get_token(char *cp, token_s *tok);





/*  This finds a dwarfdump.conf file and
    then parses it.  It updates
    conf_out as appropriate.

    This finds the first file (looking in a set of places)
    with that name.  It then looks for the right  ABI entry.
    If the first file it finds does not have that ABI entry it
    gives up.
  
    It would also be reasonable to search every 'dwarfdump.conf'
    it finds for the abi. But we stop at the first dwarfdump.conf
    we find.

    This is the internal call to read the configure file.
    Implements a crude 'includeabi' feature.

    Returns  the number of errors found.
*/
int
find_conf_file_and_read_config_inner(const string &named_file,
    const string &named_abi, 
    struct conf_internal_s *conf_internal,
    unsigned nest_level)
{

    errcount = 0;

    string name_used;
    FILE *conf_stream = find_a_file(named_file, 
        conf_internal->conf_defaults, name_used);
    if (!conf_stream) {
        ++errcount;
        cout << "dwarfdump found no file \"" <<
            named_file << "\"!"<< endl;
        cout << "(add options -v -v to see what file names tried)"<<
            endl;
        return errcount;
    }
    if (verbose > 1) {
        cout << "dwarfdump using configuration file " <<
            name_used << endl;
    }
    conf_internal->conf_name_used = name_used;

    long offset = 0;
    unsigned long lineno = 0;
    bool res = find_abi_start(conf_stream, named_abi, &offset, &lineno);
    if (!res) {
        ++errcount;
        cout << "dwarfdump found no ABI " <<
            named_abi << " in file " <<
            name_used << "." <<endl;
        return errcount;
    }
    int seekres = fseek(conf_stream, offset, SEEK_SET);
    if (seekres != 0) {
        ++errcount;
        cout << "dwarfdump seek to " <<
            offset << " offset in " <<
            name_used << 
            " failed!" << endl;
        return errcount;
    }
    parse_abi(conf_stream, name_used, named_abi, conf_internal, lineno,
        nest_level);
    fclose(conf_stream);
    return errcount;
}

// This is the exteral-facing call to read the configure file.
int
find_conf_file_and_read_config(const string &named_file,
    const string &named_abi, 
    const char **defaults,
    struct dwconf_s *conf_out)
{
    // The cf_regs are to be set below, not from
    // the default set, so clear the exsting data out.
    conf_out->cf_regs.clear();
    // Ensure a reasonable minimum vector size.
    conf_out->cf_regs.resize(100);
    conf_internal_s conf_internal(conf_out);
    conf_internal.conf_defaults = defaults;

    int res = find_conf_file_and_read_config_inner(named_file,
        named_abi, 
        &conf_internal,0);
    return res;
}

/*  Given path strings, attempt to make a canonical file name:
    that is, avoid superfluous '/' so that no
    '//' (or worse) is created in the output. The path components
    are to be separated so at least one '/'
    is to appear between the two 'input strings' when
    creating the output.  */
static bool
canonical_append(string &target,
    const string &first_string, const string &second_string)
{
    // Do not take any ending /
    size_t firstlen = first_string.size();
    for (; firstlen > 0 && first_string[firstlen - 1] == '/';
        --firstlen) {
    }

    // Do not take any leading /
    unsigned secondskipto = 0;
    for (; second_string[secondskipto] == '/'; ++secondskipto) {
    }

    target = first_string.substr(0,firstlen) + string("/") +
        second_string.substr(secondskipto);
    return true;
}

#ifdef BUILD_FOR_TEST
struct canap_s {
    const char *res_exp;
    const char *first;
    const char *second;
} canap[] = {
    {
    "ab/c", "ab", "c"}, {
    "ab/c", "ab/", "c"}, {
    "ab/c", "ab", "/c"}, {
    "ab/c", "ab////", "/////c"}, {
    "ab/", "ab", ""}, {
    "ab/", "ab////", ""}, {
    "ab/", "ab////", ""}, {
    "/a", "", "a"}, {
    0, "/abcdefgbijkl", "pqrstuvwxyzabcd"}, {
    0, 0, 0}
};
static void
test_canonical_append(void)
{
    unsigned failcount = 0;

    cout <<"Entry test_canonical_append" << endl;
    for (unsigned i = 0;; ++i) {

        if (canap[i].first == 0 && canap[i].second == 0)
            break;

        string targ;
        bool ok = canonical_append(targ, canap[i].first,
            canap[i].second);
        if (ok) {
            if (canap[i].res_exp == 0) {
                /* GOOD */
                cout <<"PASS " << i <<endl;
            } else {
                ++failcount;
                cout << "FAIL: entry " << i <<
                    " wrong, expected " << canap[i].res_exp <<
                    " got NULL " << endl;
            }
        } else {
            // Impossible now.
            ++failcount;
            cout << "FAIL: entry " << i <<
                " wrong,  expected an ok result " << endl;
        }
    }
    cout << "FAIL count " <<  failcount << endl;

}
#endif /* BUILD_FOR_TEST */
/* Try to find a file as named and open for read.
   We treat each name as a full name, we are not
   combining separate name and path components.
   This is  an arbitrary choice...

    The defaults are listed in dwarfdump.c in the array 
    config_file_defaults[].
*/
static FILE *
find_a_file(const std::string &named_file, const char **defaults, 
   string & name_used_out)
{
    string lname = named_file;
    const char *type = "rw";

#ifdef BUILD_FOR_TEST
    test_canonical_append();
#endif /* BUILD_FOR_TEST */

    if (!lname.empty()) {
        /* Name given, just assume it is fully correct, try no other. */
        if (verbose > 1) {
            cout << "dwarfdump looking for configuration as " <<
                lname << endl;
        }
        FILE *fin = fopen(lname.c_str(), type);
        if (fin) {
            name_used_out = lname;
            return fin;
        }
        return 0;
    }
    /* No name given, find a default, if we can. */
    for (unsigned i = 0; defaults[i]; ++i) {
        string lname2 = defaults[i];
        if (strncmp(lname2.c_str(), "HOME/", 5) == 0) {
            char *homedir = getenv("HOME");
            if (homedir) {
                string tname;
                canonical_append(tname,homedir, lname2.substr(5));
                lname2 = tname;
            }
        }
        if (verbose > 1) {
            cout << "dwarfdump looking for configuration as " <<
                lname2 << endl;
        }
        FILE *fin = fopen(lname2.c_str(), type);
        if (fin) {
            name_used_out = lname2;
            return fin;
        }
    }
    return 0;
}

/* Start at a token begin, see how long it is,
   return length. */
unsigned
find_token_len(char *cp)
{
    unsigned len = 0;
    for (; *cp; ++cp) {
        if (isspace(*cp)) {
            return len;
        }
        if (*cp == '#') {
            return len;         /* begins comment */
        }
        ++len;
    }
    return len;
}

/*
   Skip past all whitespace: the only code that even knows
   what whitespace is.
*/
static char *
skipwhite(char *cp)
{
    for (; *cp; ++cp) {
        if (!isspace(*cp)) {
            return cp;
        }
    }
    return cp;
}

/*  Return TRUE if ok. FALSE if find more tokens.
    Emit error message if error.  */
static bool
ensure_has_no_more_tokens(char *cp, const string &fname, unsigned long lineno)
{
    token_s tok ;
    get_token(cp, &tok);
    if (!tok.tk_data.empty() ) {
        cout << "dwarfdump.conf error: " <<
            "extra characters after command operands, found " <<
            "\"" << tok.tk_data << "\" in " << fname <<
            " line " << lineno << endl;
        ++errcount;
        return false;
    }
    return true;
}


/*  There may be many  beginabi: lines in a dwarfdump.conf file,
    find the one we want and return its file offset.  */
static bool
find_abi_start(FILE * stream, const string &abi_name, 
    long *offset, unsigned long *lineno_out)
{
    char buf[100];
    unsigned long lineno = 0;

    for (; !feof(stream);) {

        long loffset = ftell(stream);

        char *line = fgets(buf, sizeof(buf), stream);
        ++lineno;
        if (!line) {
            ++errcount;
            return false;
        }

        token_s tok;
        line = get_token(buf, &tok);

        if (tok.tk_data !=  name_begin_abi ) {
            continue;
        }
        token_s tok2;
        get_token(line,&tok2);
        if (tok2.tk_data !=  abi_name) {
            continue;
        }

        *offset = loffset;
        *lineno_out = lineno;
        return true;
    }

    ++errcount;
    return false;
}

/* The tokenizer for our simple parser.  */
static char *
get_token(char *cp, token_s *outtok)
{
    char *lcp = skipwhite(cp);
    unsigned tlen = find_token_len(lcp);

    if (tlen > 0) {
        string s(lcp);
        outtok->tk_data = s.substr(0,tlen);
    } else {
        outtok->tk_data = "";
    }
    return lcp + tlen;

}

/*  Given  a line of the table, determine if it is a command
    or not, and if a command, which one is it.
    Return LT_ERROR if it's not recognized.  */
static enum linetype_e
which_command(char *cp, struct comtable_s **tableentry)
{

    if (*cp == '#') {
        return LT_COMMENT;
    }
    if (!*cp) {
        return LT_BLANK;
    }
    token_s tok;
    get_token(cp, &tok);
    for (unsigned i = 0; i < size_of_comtable; ++i) {
        if (tok.tk_data == comtable[i].name) {
            *tableentry = &comtable[i];
            return comtable[i].type;
        }
    }
    return LT_ERROR;
}

/*  We are promised it's an abiname: command
    find the name on the line.  */
static bool
parsebeginabi(char *cp, const string &fname, const string &abiname,
    unsigned long lineno, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    size_t abinamelen = abiname.size();

    cp = cp + clen + 1;
    cp = skipwhite(cp);
    token_s tok;
    get_token(cp, &tok);
    if (tok.tk_data.size() != abinamelen ||
        strncmp(cp, abiname.c_str(), abinamelen) != 0) {
        ++errcount;
        cout << "dwarfdump internal error: mismatch " <<
            cp << " with " << tok.tk_data << "   " <<
            fname << " line " << lineno  <<endl;
        return false;
    }
    bool res = ensure_has_no_more_tokens(cp + tok.tk_data.size(), 
        fname, lineno);
    return res;
}

/*  This expands register names as required, but does not
    ensure no names duplicated.  */
static void
add_to_reg_table(struct dwconf_s *conf,
    const string &rname, unsigned long rval, 
    const string &fname,
    unsigned long lineno)
{
    if( rval >= conf->cf_regs.size()) {
        conf->cf_regs.resize(rval+1);
    }
    conf->cf_regs[rval] = rname;
    return;
}

/*  Our input is supposed to be a number.
    Determine the value (and return it) or generate an error message.  */
static int
make_a_number(const string &cmd, const string &filename, 
    unsigned long lineno, struct token_s *tok, unsigned long *val_out)
{
    char *endnum = 0;
    unsigned long val = 0;
    const char *begin = tok->tk_data.c_str();
    val = strtoul(begin, &endnum, 0);
    if (val == 0 && endnum == begin) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            cmd << " missing register number (\"" <<
            tok->tk_data << "\" not valid)  " <<
            filename << " line " << lineno <<endl;
        return false;
    }
    if (endnum != (begin + tok->tk_data.size())) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            cmd << " Missing register number (\"" <<
            tok->tk_data << "\" not valid)  " <<
            filename << " line " << lineno <<endl;
        return false;
    }
    *val_out = val;
    return true;



}

/*  We are guaranteed it's a reg: command, so parse that command
    and record the interesting data.  */
static bool
parsereg(char *cp, const string &fname, unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    token_s regnum;
    token_s tokreg;

    cp = cp + clen + 1;
    cp = get_token(cp, &tokreg);
    cp = get_token(cp, &regnum);
    if (tokreg.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: reg: missing register name  " <<
            fname << " line " << lineno  <<endl;
        return false;

    }
    if (regnum.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: reg: Missing register name  " <<
            fname << " line " << lineno  <<endl;
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &regnum, &val);

    if (!ok) {
        ++errcount;
        return false;
    }

    add_to_reg_table(conf->conf_out, tokreg.tk_data, val, fname, lineno);

    {
        bool res = ensure_has_no_more_tokens(cp, fname, lineno);

        return res;
    }
}

/*  We are guaranteed it's an frame_interface: command.
    Parse it and record the value data.  */
static bool
parseframe_interface(char *cp, const string &fname, unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    cp = cp + clen + 1;
    token_s tok;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing interface number " <<
            fname << " line " << lineno  << endl;
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    if (val != 2 && val != 3) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " only interface numbers 2 or 3 are allowed, not " <<
            val <<
            ". " <<
            fname << " line " << lineno  << endl;
        return false;
    }

    conf->conf_out->cf_interface_number = (int) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}

/*  We are guaranteed it's a cfa_reg: command. Parse it
    and record the important data.  */
static bool
parsecfa_reg(char *cp, const string &fname, unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    token_s tok;
    unsigned long val = 0;
    bool ok = false;

    cp = cp + clen + 1;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing cfa_reg number " << 
            fname << " line " << lineno  << endl;
        return false;
    }

    ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    conf->conf_out->cf_cfa_reg = (int) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}


/*  We are guaranteed it's an initial_reg_value: command,
    parse it and put the reg value where it will be remembered. 
*/
static bool
parseinitial_reg_value(char *cp, const string &fname,
    unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    cp = cp + clen + 1;
    struct token_s tok;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing initial reg value " <<
            fname << " line " << lineno << endl; 
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    conf->conf_out->cf_initial_rule_value = (int) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}
/*  We are guaranteed it's an initial_reg_value: command,
    parse it and put the reg value where it will be remembered.  */
static bool
parsesame_val_reg(char *cp, const string &fname,
    unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    cp = cp + clen + 1;
    struct token_s tok;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing same_val_reg value " <<
            fname << " line " << lineno << endl;
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    conf->conf_out->cf_same_val = (int) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}
/*  We are guaranteed it's an initial_reg_value: command,
    parse it and put the reg value where it will be remembered.  */
static bool
parseundefined_val_reg(char *cp, const string &fname,
    unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    cp = cp + clen + 1;
    struct token_s tok;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing undefined_val_reg value " <<
            fname << " line " << lineno << endl;
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    conf->conf_out->cf_undefined_val = (int) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}




/* We are guaranteed it's a table size command, parse it
    and record the table size.
*/
static bool
parsereg_table_size(char *cp, const string &fname, unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    token_s tok;

    cp = cp + clen + 1;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing reg table size value " <<
            fname << " line " << lineno << endl;
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    conf->conf_out->cf_table_entry_count = (unsigned long) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}
/* We are guaranteed it's an address size command, parse it
    and record the address size.
*/
static bool
parseaddress_size(char *cp, const string &fname, unsigned long lineno,
    struct conf_internal_s *conf, struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    token_s tok;

    cp = cp + clen + 1;
    cp = get_token(cp, &tok);
    if (tok.tk_data.empty()) {
        ++errcount;
        cout << "dwarfdump.conf error: " <<
            comtab->name <<
            " missing address size value " <<
            fname << " line " << lineno << endl;
        return false;
    }

    unsigned long val = 0;
    bool ok = make_a_number(comtab->name, fname, lineno, &tok, &val);

    if (!ok) {
        ++errcount;
        return false;
    }
    conf->conf_out->cf_address_size = (unsigned long) val;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}



/*  We are guaranteed it's an endabi: command, parse it and
    check we have the right abi.
*/
static bool
parseendabi(char *cp, const string &fname, 
    const string &abiname, unsigned long lineno,
    struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();


    cp = cp + clen + 1;
    struct token_s tok;
    cp = get_token(cp, &tok);
    if (abiname != tok.tk_data) {
        ++errcount;
        cout << comtab->name <<
            " error: mismatch abi name " <<
            tok.tk_data << " (here) vs. " <<
            abiname <<
            " (beginabi:)  " <<
            fname << " line " << lineno << endl;
        return false;
    }
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}

static int 
parseincludeabi(char *cp,const string &fname,unsigned long lineno,
    std::string &abiname_out,struct comtable_s *comtab)
{
    size_t clen = comtab->name.size();
    struct token_s tok;
    cp = cp + clen + 1;
    cp = get_token(cp,&tok);
    abiname_out = tok.tk_data;
    bool res = ensure_has_no_more_tokens(cp, fname, lineno);
    return res;
}

   



/* Return TRUE if we succeeded and filed in *out.
   Return FALSE if we failed (and fill in nothing).
  beginabi:  <abiname>
  reg: <regname> <dwarf regnumber>
  frame_interface: <integer value 2 or 3>
  cfa_reg:  <number>
  initial_reg_value:  <number: normally 1034 or 1035 >
  same_value 'reg number' :  <number: normally 1034  >
  undefined_value 'reg number' :  <number: normally 1035 >
  reg_table_size: <size of table>
  endabi:  <abiname>
 
  We are positioned at the start of a beginabi: line when
  called.

*/
static bool
parse_abi(FILE * stream, const string &fname, const string &abiname,
    struct conf_internal_s *conf_internal, unsigned long lineno,
    unsigned int nest_level)
{
    struct dwconf_s *localconf = conf_internal->conf_out;

    if( nest_level > MAX_NEST_LEVEL) {
        ++errcount;
        cout <<"dwarfdump.conf: includeabi nest too deep  in " <<
            fname << " at line " << lineno<<endl;
        return false;
    }

    for (; !feof(stream);) {

        char *line = 0;
        char buf[1000];
        /* long loffset = ftell(stream); */
        line = fgets(buf, sizeof(buf), stream);
        if (!line) {
            ++errcount;
            cout << "dwarfdump: end of file or error before endabi: in "<<
                fname << ", line " << lineno << endl;
            return false;
        }
        ++lineno;
        line = skipwhite(line);
        // comtabp points to the table entry
        // of interest (identified by which_command()).
        struct comtable_s *comtabp = 0;
        int comtype = which_command(line, &comtabp);
        switch (comtype) {
        case LT_ERROR:
            ++errcount;
            cout << "dwarfdump: Unknown text in "<<
                fname << " is \"" <<
                line << "\" at line " <<  lineno << endl;
            break;
        case LT_COMMENT:
            break;
        case LT_BLANK:
            break;
        case LT_BEGINABI:
            if (conf_internal->beginabi_lineno > 0 &&
                nest_level == 0) {
                ++errcount;
                cout << "dwarfdump: Encountered beginabi: when not expected. "
                    << fname <<
                    " line " << lineno <<
                    " previous beginabi line " <<
                    conf_internal->beginabi_lineno << endl;
            }
            conf_internal->beginabi_lineno = lineno;
            parsebeginabi(line, fname, abiname, lineno, comtabp);
            break;

        case LT_REG:
            parsereg(line, fname, lineno, conf_internal, comtabp);
            conf_internal->regcount++;
            break;
        case LT_FRAME_INTERFACE:
            if (conf_internal->frame_interface_lineno > 0) {
                ++errcount;
                cout << "dwarfdump: Encountered duplicate frame_interface: " 
                    << fname <<
                    " line " << lineno <<
                    " previous frame_interface: line " <<
                    conf_internal->frame_interface_lineno << endl;
            }
            conf_internal->frame_interface_lineno = lineno;
            parseframe_interface(line, fname,
                lineno, conf_internal, comtabp);
            break;
        case LT_CFA_REG:
            if (conf_internal->cfa_reg_lineno > 0) {
                ++errcount;
                cout << "dwarfdump: Encountered duplicate cfa_reg: " 
                    << fname <<
                    " line " << lineno <<
                    " previous cfa_reg line " <<
                    conf_internal->cfa_reg_lineno << endl;
            }
            conf_internal->cfa_reg_lineno = lineno;
            parsecfa_reg(line, fname, lineno, conf_internal, comtabp);
            break;
        case LT_INITIAL_REG_VALUE:
            if (conf_internal->initial_reg_value_lineno > 0) {
                ++errcount;
                cout << 
                    "dwarfdump: Encountered duplicate " <<
                    "initial_reg_value: " <<
                    fname <<
                    " line " << lineno <<
                    " previous initial_reg_value: line " <<
                    conf_internal->initial_reg_value_lineno<< endl;
            }
            conf_internal->initial_reg_value_lineno = lineno;
            parseinitial_reg_value(line, fname,
                lineno,conf_internal, comtabp);
            break;
        case LT_SAME_VAL_REG:
            if (conf_internal->same_val_reg_lineno > 0) {
                ++errcount;
                cout << 
                    "dwarfdump: Encountered duplicate " <<
                    "same_val_reg: " <<
                    fname <<
                    " line " << lineno <<
                    " previous same_reg_value: line " <<
                    conf_internal->same_val_reg_lineno << endl;
            }
            conf_internal->same_val_reg_lineno = lineno;
            parsesame_val_reg(line, fname,
                lineno, conf_internal, comtabp);
            break;
        case LT_UNDEFINED_VAL_REG:
            if (conf_internal->undefined_val_reg_lineno > 0) {
                ++errcount;
                cout << 
                    "dwarfdump: Encountered duplicate " <<
                    "undefined_val_reg: " <<
                    fname <<
                    " line " << lineno <<
                    " previous same_val_reg: line " <<
                    conf_internal->same_val_reg_lineno << endl;
            }
            conf_internal->undefined_val_reg_lineno = lineno;
            parseundefined_val_reg(line, fname,
                lineno, conf_internal, comtabp);
            break;

        case LT_REG_TABLE_SIZE:
            if (conf_internal->reg_table_size_lineno > 0) {
                ++errcount;
                cout << "dwarfdump: duplicate reg_table_size: " 
                    << fname <<
                    " line " << lineno <<
                    " previous reg_table_size: line " <<
                    conf_internal->reg_table_size_lineno << endl;
            }
            conf_internal->reg_table_size_lineno = lineno;
            parsereg_table_size(line, fname,
                lineno, conf_internal, comtabp);
            break;
        case LT_ENDABI:
            parseendabi(line, fname, abiname, lineno, comtabp);

            if (conf_internal->regcount > localconf->cf_table_entry_count) {
                ++errcount;
                cout << "dwarfdump: more registers named than  in  "
                    << abiname <<
                    "  ( " << conf_internal->regcount <<
                    " named vs  " << name_reg_table_size <<
                    " " << localconf->cf_table_entry_count <<
                    ")  " <<
                    fname << " line " << lineno << endl;
            }

            return true;
        case LT_ADDRESS_SIZE:
            if (conf_internal->address_size_lineno > 0) {
                ++errcount;
                cout << "dwarfdump: duplicate address_size:: "
                    << fname <<
                    " line " << lineno <<
                    " previous address_size: line " <<
                    conf_internal->address_size_lineno << endl;
            }
            conf_internal->address_size_lineno = lineno;
            parseaddress_size(line, fname,
                lineno, conf_internal, comtabp);
            break;
        case LT_INCLUDEABI: {
            std::string abiname_inner;
            unsigned long abilno = conf_internal->beginabi_lineno;
            bool ires = parseincludeabi(line,fname,lineno,abiname_inner,
                comtabp);
            if (ires == false) {
                return ires;
            }
            // For the nested abi read, the abi line number must be
            // set as if not-yet-read, and then restored. 
            conf_internal->beginabi_lineno = 0;
            find_conf_file_and_read_config_inner(conf_internal->conf_name_used,
                abiname_inner,conf_internal,nest_level+1);
            conf_internal->beginabi_lineno = abilno;
            }
            break;
        default:
            cout << "dwarfdump internal error, impossible line type " <<
                comtype << " " << fname << " " << lineno <<endl;
            exit(1);

        }
    }
    ++errcount;
    cout << "End of file, no endabi: found.  " <<
        fname << ", line " << lineno << endl;
    return false;
}

/*  MIPS/IRIX frame register names.
    For alternate name sets, use dwarfdump.conf or
    revise dwarf.h and libdwarf.h and this table.
*/
static const char *mipsregnames[] = {
    "cfa",
    "r1/at", "r2/v0", "r3/v1",
    "r4/a0", "r5/a1", "r6/a2", "r7/a3",
    "r8/t0", "r9/t1", "r10/t2", "r11/t3",
    "r12/t4", "r13/t5", "r14/t6", "r15/t7",
    "r16/s0", "r17/s1", "r18/s2", "r19/s3",
    "r20/s4", "r21/s5", "r22/s6", "r23/s7",
    "r24/t8", "r25/t9", "r26/k0", "r27/k1",
    "r28/gp", "r29/sp", "r30/s8", "r31",

    "$f0", "$f1",
    "$f2", "$f3",
    "$f4", "$f5",
    "$f6", "$f7",
    "$f8", "$f9",
    "$f10", "$f11",
    "$f12", "$f13",
    "$f14", "$f15",
    "$f16", "$f17",
    "$f18", "$f19",
    "$f20", "$f21",
    "$f22", "$f23",
    "$f24", "$f25",
    "$f26", "$f27",
    "$f28", "$f29",
    "$f30", "$f31",
    "ra", "slk",
};

/*  Naming a few registers makes printing these just
    a little bit faster.
*/
static const char *genericregnames[] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
    "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
    "r20"
};

/*  This is a simple generic set of registers.  The 
    table entry count is pretty arbitrary.
*/
void
init_conf_file_data(struct dwconf_s *config_file_data) 
{
    config_file_data->cf_interface_number = 3;
    config_file_data->cf_table_entry_count = 100;
    config_file_data->cf_initial_rule_value = DW_FRAME_UNDEFINED_VAL;
    config_file_data->cf_cfa_reg =  DW_FRAME_CFA_COL3;
    config_file_data->cf_address_size = 0;
    config_file_data->cf_same_val = DW_FRAME_SAME_VAL;
    config_file_data->cf_undefined_val = DW_FRAME_UNDEFINED_VAL;
    unsigned generic_table_count =
        sizeof(genericregnames) / sizeof(genericregnames[0]);
    config_file_data->cf_regs.clear();
    config_file_data->cf_regs.reserve(generic_table_count);
    for(unsigned i = 0; i < generic_table_count; ++i) {
        config_file_data->cf_regs.push_back(genericregnames[i]);
    }
}

/*  These defaults match MIPS/IRIX ABI defaults, but this
    function is not actually used.
    For a 'generic' ABI, see -R or init_conf_file_data().
    To really get the old MIPS, use '-x abi=mips'. 
    For other ABIs, see -x abi=<whatever>
    to configure dwarfdump (and libdwarf) frame 
    data reporting at runtime.
*/
void
init_mips_conf_file_data(struct dwconf_s *config_file_data)
{
    /*  Interface 2 is deprecated, but for testing purposes
        is acceptable. */
    config_file_data->cf_interface_number = 2;
    config_file_data->cf_table_entry_count = DW_REG_TABLE_SIZE;
    config_file_data->cf_initial_rule_value =
        DW_FRAME_REG_INITIAL_VALUE;
    config_file_data->cf_cfa_reg = DW_FRAME_CFA_COL;
    config_file_data->cf_address_size = 0;
    config_file_data->cf_same_val = DW_FRAME_SAME_VAL;
    config_file_data->cf_undefined_val = DW_FRAME_UNDEFINED_VAL;
    unsigned mips_table_count =
        sizeof(mipsregnames) / sizeof(mipsregnames[0]);
    config_file_data->cf_regs.clear();
    config_file_data->cf_regs.reserve(mips_table_count);
    for(unsigned i = 0; i < mips_table_count; ++i) {
        config_file_data->cf_regs.push_back(mipsregnames[i]);
    }
    return;
}


/* A 'generic' ABI. For up to 1200 registers. 
*/
void
init_generic_config_1200_regs(struct dwconf_s *config_file_data)
{
    config_file_data->cf_interface_number = 3;
    config_file_data->cf_table_entry_count = 1200;
    /*  There is no defined name for cf_initial_rule_value,
        cf_same_val, or cf_undefined_val in libdwarf.h, 
        these must just be high enough to be higher than 
        any real register number. 
        DW_FRAME_CFA_COL3 must also be higher than any
        real register number. */
    config_file_data->cf_initial_rule_value = 1235; /* SAME VALUE */
    config_file_data->cf_cfa_reg =  DW_FRAME_CFA_COL3;
    config_file_data->cf_address_size = 0;
    config_file_data->cf_same_val = 1235;
    config_file_data->cf_undefined_val = 1234;
    unsigned generic_table_count =
        sizeof(genericregnames) / sizeof(genericregnames[0]);
    config_file_data->cf_regs.clear();
    config_file_data->cf_regs.reserve(generic_table_count);
    for(unsigned i = 0; i < generic_table_count; ++i) {
        config_file_data->cf_regs.push_back(genericregnames[i]);
    }
}

/*  Print the 'right' string for the register we are given.
    Deal sensibly with the special regs as well as numbers
    we know and those we have not been told about.
*/
void
print_reg_from_config_data(Dwarf_Signed reg,
    struct dwconf_s *config_data)
{

    if (reg == config_data->cf_cfa_reg) {
        cout << "cfa";
        return;
    }
    if (reg == config_data->cf_undefined_val) {
        cout << "u";
        return;
    }
    if (reg == config_data->cf_same_val) {
        cout << "s";
        return;
    }
    if ( reg < 0 || 
        reg >= config_data->cf_regs.size()) {
        cout << "r" << reg;
        return;
    }
    const string &name = config_data->cf_regs[reg];
    if (name.empty()) {
        /* Can happen, the reg names table can be sparse. */
        cout << "r" << reg;
        return;
    }
    cout << name;
    return;
}
