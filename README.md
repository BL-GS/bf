File system based on Chapter IV in OS experiment of Harbin Institution of technology, Shenzhen (http://hitsz-lab.gitee.io/os-labs-2021), which need ddriver as the virtual driver to disk.
Its mainly construction is as follow:
/------------------
/
/---include(which declares a series of definition and functions)
/   |
/   |--- bf.h (Declares several functions)
/   |--- types.h (Declares the mainly construction of file system and macros)
/   |--- ddriver.h
/   \--- ddriver_ctl_user.h
/    
/
/---src(which stores the mainly implements of functions)
/   |
/   |--- bf.c
/   |--- bf_utils.c
/
/
/---tests(which stores the test program provided by OS-experiment)