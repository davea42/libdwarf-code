
struct lnsix_s {
    char        *ln_informal;
    char        *ln_name;
    unsigned int ln_value;
    char         ln_low_bound;
    char        *ln_vscheme;
};

struct lnsix_s lnsix[] = 
{
{0,0,0,0,0}, /* [0] entry is not real */
{"ISO Ada",   "DW_LNAME_Ada",0x0001,1,"YYYY"},
{"BLISS",     "DW_LNAME_BLISS",0x0002,0,0},	
{"C (K&R and ISO)","DW_LNAME_C",0x0003,0,"YYYYMM"},
{"ISO C++",   "DW_LNAME_C_plus_plus",0x0004,0,"YYYYMM"},
{"ISO Cobol", "DW_LNAME_Cobol",0x0005,1,"YYYY"},
{"Crystal",   "DW_LNAME_Crystal",0x0006,0,0},	
{"D",         "DW_LNAME_D",0x0007,0,0},	
{"Dylan",     "DW_LNAME_Dylan",0x0008,0,0},	
{"ISO Fortran","DW_LNAME_Fortran",0x0009,1,"YYYY"},
{"Go",        "DW_LNAME_Go",0x000a,0,0},	
{"Haskell",   "DW_LNAME_Haskell",0x000b,0,0},	
{"Java",      "DW_LNAME_Java",0x000c,0,0},
{"Julia",     "DW_LNAME_Julia",0x000d,1,0},
{"Kotlin",    "DW_LNAME_Kotlin",0x000e,0,0},	
{"Modula 2",  "DW_LNAME_Modula2",0x000f,1,0},	
{"Modula 3",  "DW_LNAME_Modula3",0x0010,1,0},	
{"Objective C","DW_LNAME_ObjC",0x0011,0,"YYYYMM"},
{"Objective C++","DW_LNAME_ObjC_plus_plus",0x0012,0,"YYYYMM"},
{"OCaml",     "DW_LNAME_OCaml",0x0013,0,0},	
{"OpenCL C",  "DW_LNAME_OpenCL_C",0x0014,0,0},	
{"ISO Pascal","DW_LNAME_Pascal",0x0015,1,"YYYY"},
{"ANSI PL/I", "DW_LNAME_PLI",0x0016,1,0},	
{"Python",    "DW_LNAME_Python",0x0017,0,0},	
{"RenderScript Kernel Language","DW_LNAME_RenderScript",0x0018,0,0},
{"Rust",      "DW_LNAME_Rust",0x0019,0,0},	
{"Swift",     "DW_LNAME_Swift",0x001a,0,"VVMM"},
{"Unified Parallel C (UPC)","DW_LNAME_UPC",0x001b,0,0},	
{"Zig",       "DW_LNAME_Zig",0x001c,0,0},	
{"Assembly",  "DW_LNAME_Assembly",0x001d,0,0},	
{"C#",        "DW_LNAME_C_sharp",0x001e,0,0},	
{"Mojo",      "DW_LNAME_Mojo",0x001f,0,0},	
{"OpenGL Shading Language","DW_LNAME_GLSL",0x0020,0,"VVMMPP"},
{"OpenGL ES Shading Language","DW_LNAME_GLSL_ES",0x0021,0,"VVMMPP"},
{"High Level Shading Language","DW_LNAME_HLSL",0x0022,0,"YYYY"},
{"OpenCL C++","DW_LNAME_OpenCL_CPP",0x0023,0,"VVMM"},
{"C++ for OpenCL","DW_LNAME_CPP_for_OpenCL",0x0024,0,"VVMM"},
{"SYCL",      "DW_LNAME_SYCL",0x0025,0,"YYYYRR"},
{"Ruby",      "DW_LNAME_Ruby",0x0026,0,"VVMMPP"},
{"Move",      "DW_LNAME_Move",0x0027,0,"YYYYMM"},
{"Hylo",      "DW_LNAME_Hylo",0x0028,0,0},	
{"HIP",       "DW_LNAME_HIP",0x0029,0,0},	
{"Odin",      "DW_LNAME_Odin",0x002a,0,"YYYYMM"},
{"P4",        "DW_LNAME_P4",0x002b,0,"VVMMPP"},
{"Metal",     "DW_LNAME_Metal",0x002c,0,"VVMMPP"},
{"V",         "DW_LNAME_V",0x002d,0,"VVMMPP"},
{"Algol 68",  "DW_LNAME_Algol68",0x002e,1,"YYYY"},
{"Nim",      "DW_LNAME_Nim",0x002f,0,"VVMMPP"},
{"Erlang",   "DW_LNAME_Erlang",0x0030,1,"VVMMPP"},
{"Elixir",   "DW_LNAME_Elixir",0x0031,1,"VVMMPP"},
{"Gleam",    "DW_LNAME_Gleam",0x0032,0,"VVMMPP"},
};
