#ifndef STRTABDATA_H
#define STRTABDATA_H
/*
  Copyright (C) 2010-2011 David Anderson.  

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 51
  Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.

*/


// strtabdata.h
// Creates a string table in a way consistent with
// elf string tables. The zero index is a null byte always.

class strtabdata {
public:
    strtabdata(): data_(new char[1000]),
        datalen_(1000), nexttouse_(0) { data_[0] = 0; nexttouse_ = 1;};
    ~strtabdata()  { delete[]data_; };
    unsigned addString(const std::string & newstr) {
        // The 1 is for the terminating null byte.
        unsigned nsz = newstr.size() +1;
        unsigned needed = nexttouse_ + nsz;
        if (needed >= datalen_) {
            unsigned baseincr = nsz;
            unsigned altincr = datalen_*2;
            if(altincr> baseincr) {
                baseincr = altincr;
            }
            unsigned newsize = datalen_ + baseincr;
            char *newdata = new char [newsize];
            memcpy(newdata, data_, nexttouse_);
            delete [] data_;
            data_ = newdata;
            datalen_ = newsize;
        }
        memcpy(data_ + nexttouse_, newstr.c_str(),nsz);
        unsigned newstrindex = nexttouse_;
        nexttouse_ += nsz;
        return newstrindex;
    };
    void *exposedata() {return (void *)data_;};
    unsigned exposelen() const {return nexttouse_;};
private:
    char *   data_;

    // datalen_ is the size in bytes pointed to by data_ .
    unsigned datalen_;

    // nexttouse_ is the index of the next (unused) byte in
    // data_ , so it is also the amount of space in data_ that
    // is in use.
    unsigned nexttouse_;
};
#endif /* STRTABDATA_H */
