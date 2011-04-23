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


// 
// irepform.h
//
//
class IRCUdata;
class IRAttr;
class IRFormInterface;

// An Abstract class.
class IRForm {
public:
    //virtual void emitvalue() = 0;
    //IRForm & operator=(const IRForm &r);
    virtual IRForm * clone() const  =0;
    virtual ~IRForm() {};
    IRForm() {};
    virtual enum Dwarf_Form_Class getFormClass() const = 0;
private:
};

class IRFormUnknown : public IRForm {
public: 
    IRFormUnknown():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_UNKNOWN)
        {}
    ~IRFormUnknown() {};
    IRFormUnknown(IRFormInterface *);
    IRFormUnknown(const IRFormUnknown &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
    }
    virtual IRFormUnknown * clone() const {
        return new IRFormUnknown(*this);
    }
    IRFormUnknown & operator=(const IRFormUnknown &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
    };
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
};  
// An address class entry refers to some part
// (normally a loadable) section of the object file.
// Not to DWARF info. Typically into .text or .data for example.
// We therefore want a section number and offset (generally useless for us)
// or preferably  an elf symbol as that has a value
// and an elf section number.
// We often/usually know neither here so we do not even try.
// Later we will make one up if we have to.
class IRFormAddress : public IRForm {
public: 
    IRFormAddress():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_ADDRESS),
        address_(0)
        {};
    IRFormAddress(IRFormInterface *);
    ~IRFormAddress() {};
    IRFormAddress & operator=(const IRFormAddress &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        address_ = r.address_;
    };
    IRFormAddress(const IRFormAddress &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        address_ = r.address_;
    }
    virtual IRFormAddress * clone() const {
        return new IRFormAddress(*this);
    };
    Dwarf_Addr  getAddress() { return address_;};
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    void setAddress(Dwarf_Addr addr) { address_ = addr; };
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Addr address_;
};  
class IRFormBlock : public IRForm {
public: 
    IRFormBlock():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_BLOCK),
        fromloclist_(0),sectionoffset_(0)
        {}
    IRFormBlock(IRFormInterface *);
    ~IRFormBlock() {};
    IRFormBlock & operator=(const IRFormBlock &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        blockdata_ = r.blockdata_;
        fromloclist_ = r.fromloclist_;
        sectionoffset_ = r.sectionoffset_;
    };
    IRFormBlock(const IRFormBlock &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        blockdata_ = r.blockdata_;
        fromloclist_ = r.fromloclist_;
        sectionoffset_ = r.sectionoffset_;
    }
    virtual IRFormBlock * clone() const {
        return new IRFormBlock(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    std::vector<char> blockdata_;
    Dwarf_Small fromloclist_;
    Dwarf_Unsigned sectionoffset_;

    void insertBlock(Dwarf_Block *bl) {
        char *d = static_cast<char *>(bl->bl_data);
        Dwarf_Unsigned len = bl->bl_len;
        blockdata_.clear();
        blockdata_.insert(blockdata_.end(),d+0,d+len);
        fromloclist_ = bl->bl_from_loclist;
        sectionoffset_ = bl->bl_section_offset;
    };
};  
class IRFormConstant : public IRForm {
public: 
    IRFormConstant():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_CONSTANT),
        signedness_(SIGN_NOT_SET),
        uval_(0), sval_(0)
        {}
    IRFormConstant(IRFormInterface *);
    ~IRFormConstant() {};
    IRFormConstant & operator=(const IRFormConstant &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        signedness_ = r.signedness_;
        uval_ = r.uval_;
        sval_ = r.sval_;
    };
    IRFormConstant(const IRFormConstant &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        signedness_ = r.signedness_;
        uval_ = r.uval_;
        sval_ = r.sval_;
    }
    virtual IRFormConstant * clone() const {
        return new IRFormConstant(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    // Starts at SIGN_NOT_SET.
    // SIGN_UNKNOWN means it was a DW_FORM_data* of some
    // kind so we do not really know.
    enum Signedness {SIGN_NOT_SET,SIGN_UNKNOWN,UNSIGNED, SIGNED };
    enum Signedness signedness_;
    // Both uval_ and sval_ are always set to the same bits.
    Dwarf_Unsigned uval_;
    Dwarf_Signed sval_;

    void setValues(Dwarf_Signed sval, Dwarf_Unsigned uval,
        enum Signedness s) {
        signedness_ = s;
        uval_ = uval;
        sval_ = sval;
    }
};  

class IRFormExprloc : public IRForm {
public: 
    IRFormExprloc():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_EXPRLOC)
        {};
    IRFormExprloc(IRFormInterface *);
    ~IRFormExprloc() {};
    IRFormExprloc & operator=(const IRFormExprloc &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        exprlocdata_ = r.exprlocdata_;
    };
    IRFormExprloc(const IRFormExprloc &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        exprlocdata_ = r.exprlocdata_;
        
    }
    virtual IRFormExprloc * clone() const {
        return new IRFormExprloc(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    std::vector<char> exprlocdata_;
    void insertBlock(Dwarf_Unsigned len, Dwarf_Ptr data) {
        char *d = static_cast<char *>(data);
        exprlocdata_.clear();
        exprlocdata_.insert(exprlocdata_.end(),d+0,d+len);
    };
};  


class IRFormFlag : public IRForm {
public: 
    IRFormFlag():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_FLAG),
        flagval_(0)
        {};
    IRFormFlag(IRFormInterface*);
    ~IRFormFlag() {};
    IRFormFlag & operator=(const IRFormFlag &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        flagval_ = r.flagval_;
    };
    IRFormFlag(const IRFormFlag &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        flagval_ = r.flagval_;
    }
    virtual IRFormFlag * clone() const {
        return new IRFormFlag(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
    Dwarf_Bool getFlagVal() { return flagval_; }
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Bool flagval_; 
};  


class IRFormLinePtr : public IRForm {
public: 
    IRFormLinePtr():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_LINEPTR),
        debug_line_offset_(0)
        {};
    IRFormLinePtr(IRFormInterface *);
    ~IRFormLinePtr() {};
    IRFormLinePtr & operator=(const IRFormLinePtr &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        debug_line_offset_ = r.debug_line_offset_;
    };
    IRFormLinePtr(const IRFormLinePtr &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        debug_line_offset_ = r.debug_line_offset_;
    }
    virtual IRFormLinePtr * clone() const {
        return new IRFormLinePtr(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Off debug_line_offset_;

    void setOffset(Dwarf_Unsigned uval) {
        debug_line_offset_ = uval;
    };
};  


class IRFormLoclistPtr : public IRForm {
public: 
    IRFormLoclistPtr():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_LOCLISTPTR),
        loclist_offset_(0)
        {};
    IRFormLoclistPtr(IRFormInterface *);
    ~IRFormLoclistPtr() {};
    IRFormLoclistPtr & operator=(const IRFormLoclistPtr &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        loclist_offset_ = r.loclist_offset_;
    };
    IRFormLoclistPtr(const IRFormLoclistPtr &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        loclist_offset_ = r.loclist_offset_;
    }
    virtual IRFormLoclistPtr * clone() const {
        return new IRFormLoclistPtr(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Off loclist_offset_;

    void setOffset(Dwarf_Unsigned uval) {
        loclist_offset_ = uval;
    };
};  


class IRFormMacPtr : public IRForm {
public: 
    IRFormMacPtr():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_MACPTR),
        macro_offset_(0)
        {};
    IRFormMacPtr(IRFormInterface *);
    ~IRFormMacPtr() {};
    IRFormMacPtr & operator=(const IRFormMacPtr &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        macro_offset_ = r.macro_offset_;
    };
    IRFormMacPtr(const IRFormMacPtr &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        macro_offset_ = r.macro_offset_;
    }
    virtual IRFormMacPtr * clone() const {
        return new IRFormMacPtr(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Off macro_offset_;

    void setOffset(Dwarf_Unsigned uval) {
        macro_offset_ = uval;
    };
};  


class IRFormRangelistPtr : public IRForm {
public: 
    IRFormRangelistPtr():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_RANGELISTPTR),
        rangelist_offset_(0)
        {};
    IRFormRangelistPtr(IRFormInterface *);
    ~IRFormRangelistPtr() {};
    IRFormRangelistPtr & operator=(const IRFormRangelistPtr &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        rangelist_offset_ = r.rangelist_offset_;
    };
    IRFormRangelistPtr(const IRFormRangelistPtr &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        rangelist_offset_ = r.rangelist_offset_;
    }
    virtual IRFormRangelistPtr * clone() const {
        return new IRFormRangelistPtr(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Off rangelist_offset_;

    void setOffset(Dwarf_Unsigned uval) {
        rangelist_offset_ = uval;
    };
};  

class IRFormFramePtr : public IRForm {
public:
    IRFormFramePtr():
        directform_(0), indirectform_(0),
        formclass_(DW_FORM_CLASS_FRAMEPTR),
        frame_offset_(0)
        {};
    IRFormFramePtr(IRFormInterface *);
    ~IRFormFramePtr() {};
    IRFormFramePtr & operator=(const IRFormFramePtr &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        frame_offset_ = r.frame_offset_;
    };
    IRFormFramePtr(const IRFormFramePtr &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        frame_offset_ = r.frame_offset_;
    }
    virtual IRFormFramePtr * clone() const {
        return new IRFormFramePtr(*this);
    }
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    Dwarf_Off frame_offset_;

    void setOffset(Dwarf_Unsigned uval) {
        frame_offset_ = uval;
    };
}; 



class IRFormReference : public IRForm {
public: 
    IRFormReference():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_REFERENCE),
        reftype_(RT_NONE),
        globalOffset_(0),cuRelativeOffset_(0)
        {initSig8();};
    IRFormReference(IRFormInterface *);
    ~IRFormReference() {};
    IRFormReference & operator=(const IRFormReference &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        reftype_ = r.reftype_;
        globalOffset_ = r.globalOffset_;
        cuRelativeOffset_ = r.cuRelativeOffset_;
        typeSig8_ = r.typeSig8_;
    };
    IRFormReference(const IRFormReference &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        reftype_ = r.reftype_;
        globalOffset_ = r.globalOffset_;
        cuRelativeOffset_ = r.cuRelativeOffset_;
        typeSig8_ = r.typeSig8_;
    }
    virtual IRFormReference * clone() const {
        return new IRFormReference(*this);
    }
    void setOffset(Dwarf_Off off) { globalOffset_ = off; 
        reftype_ = RT_GLOBAL;};
    void setCUOffset(Dwarf_Off off) { cuRelativeOffset_= off; 
        reftype_ = RT_CUREL;};
    void setSignature(Dwarf_Sig8 * sig) { typeSig8_ = *sig;
        reftype_ = RT_SIG;};
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:        
    void initSig8();

    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    enum RefType { RT_NONE,RT_GLOBAL, RT_CUREL,RT_SIG }; 
    enum RefType reftype_;
    Dwarf_Off globalOffset_;
    Dwarf_Off cuRelativeOffset_;
    Dwarf_Sig8 typeSig8_;
};  


class IRFormString: public IRForm {
public:
    IRFormString():
        directform_(0), indirectform_(0), 
        formclass_(DW_FORM_CLASS_STRING),
        strpoffset_(0) {};
    ~IRFormString() {};
    IRFormString(IRFormInterface *);
    IRFormString(const IRFormString &r) {
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        formdata_= r.formdata_;
        strpoffset_= r.strpoffset_;
    }
    virtual IRFormString * clone() const {
        return new IRFormString(*this);
    }
    IRFormString & operator=(const IRFormString &r) {
        if(this == &r) return *this;
        directform_ = r.directform_;
        indirectform_ = r.indirectform_;
        formclass_ = r.formclass_;
        formdata_ = r.formdata_;
        strpoffset_ = r.strpoffset_;
    };
    void setString(const char *s) {formdata_ = s; };
    const std::string & getString() const {return formdata_; };
    enum Dwarf_Form_Class getFormClass() const { return formclass_; };
private:
    Dwarf_Half directform_;
    // In most cases directform == indirect form.
    // Otherwise, directform == DW_FORM_indirect.
    Dwarf_Half indirectform_;
    enum Dwarf_Form_Class formclass_;
    std::string formdata_;
    Dwarf_Unsigned strpoffset_;
};


// Factory Method.
IRForm *formFactory(Dwarf_Debug dbg, Dwarf_Attribute attr,
    IRCUdata &cudata,IRAttr & irattr);

