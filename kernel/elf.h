/*
 * elf.h
 *
 * ELF Filesystem functionality for FUSION loader
 *
 */


#ifndef ELF_H
#define ELF_H

/*
 * ELF types
 */

typedef unsigned int   Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int   Elf32_Off;
typedef int            Elf32_Sword;
typedef unsigned int   Elf32_Word;



/**
 ** ELF header
 **/

#define EI_NIDENT 16 

typedef struct 
{ 
  unsigned char   e_ident [EI_NIDENT]; 
  Elf32_Half      e_type; 
  Elf32_Half      e_machine; 
  Elf32_Word      e_version; 

  /* e_entry
   * This member gives the virtual address to which the system first transfers control,            
   * thus starting the process. If the file has no associated entry point, this member holds zero. 
   */
  Elf32_Addr      e_entry;

  /* e_phoff
   * This member holds the program header table's file offset in bytes. 
   * If the file has no program header table, this member holds zero.     
   */ 
  Elf32_Off       e_phoff;

  /* e_shoff
   * This member holds the section header table's file offset in bytes. 
   * If the file has no sec-tion header table, this member holds zero. 
   */
  Elf32_Off       e_shoff; 

  /* e_flags
   * This member holds processor-specific flags associated with the file. 
   * Flag names take the form EF_machine_flag. See ''Machine Information'' in ELF spec. for flag definitions. 
   */
  Elf32_Word      e_flags; 
 
  /* e_ehsize 
   * This member holds the ELF header's size in bytes. 
   */
  Elf32_Half      e_ehsize; 
 
  /* e_phentsize 
   * This member holds the size in bytes of one entry in the file's program header table; 
   * all entries are the same size.
   */
  Elf32_Half      e_phentsize; 
 
  /* e_phnum 
   * This member holds the number of entries in the program header table. 
   * Thus the pro-duct of e_phentsize and e_phnum gives the table's size in bytes. 
   * If a file has no pro-gram header table, e_phnum holds the value zero. 
   */
  Elf32_Half      e_phnum; 

  /* e_shentsize 
   * This member holds a section header's size in bytes. 
   * A section header is one entry in the section header table; 
   * all entries are the same size. 
   */
  Elf32_Half      e_shentsize; 

  /* e_shnum 
   * This member holds the number of entries in the section header table. 
   * Thus the product of e_shentsize and e_shnum gives the section header table's size in bytes. 
   * If a file has no section header table, e_shnum holds the value zero.
   */
  Elf32_Half      e_shnum; 

  /* e_shstrndx 
   * This member holds the section header table index of the entry associated with the 
   * sec-tion name string table. If the file has no section name string table, this member 
   * holds the value SHN_UNDEF. See ''Sections'' and ''String Table'' below for more informa-tion. 
   */
  Elf32_Half      e_shstrndx; 

} Elf32_Ehdr;


/* e_type values */
#define ET_NONE   0       /* No file type        */ 
#define ET_REL    1       /* Relocatable file    */
#define ET_EXEC   2       /* Executable file     */
#define ET_DYN    3       /* Shared object file  */
#define ET_CORE   4       /* Core file           */
#define ET_LOPROC 0xff00  /* Processor-specific  */
#define ET_HIPROC 0xffff  /* Processor-specific  */

/* Values from ET_LOPROC through ET_HIPROC (inclusive) 
 * are reserved for processor-specific semantics. 
 */

/* e_machine values */
#define EM_NONE    0 /* No machine      */ 
#define EM_M32     1 /* AT&T WE 32100   */ 
#define EM_SPARC   2 /* SPARC           */
#define EM_386     3 /* Intel 80386     */
#define EM_68K     4 /* Motorola 68000  */
#define EM_88K     5 /* Motorola 88000  */
#define EM_860     7 /* Intel 80860     */
#define EM_MIPS    8 /* MIPS RS3000     */


/* e_version, This member identifies the object file version. */
#define EV_NONE    0 /* Invalid version */ 
#define EV_CURRENT 1 /* Current version */




/* e_ident[ ] Identification Indexes */
#define EI_MAG0    0  /* File identification     */ 
#define EI_MAG1    1  /* File identification     */ 
#define EI_MAG2    2  /* File identification     */ 
#define EI_MAG3    3  /* File identification     */ 
#define EI_CLASS   4  /* File class              */
#define EI_DATA    5  /* Data encoding           */

#define EI_VERSION 6  /* File version. Currently, this value must be EV_CURRENT, 
		       * as explained above for e_version. 
		       */

#define EI_PAD     7  /* Start of padding bytes  */
#define EI_NIDENT  16 /* Size of e_ident[]       */


/* ELF Magic bytes */
#define ELFMAG0    0x7f  /* e_ident[EI_MAG0] */
#define ELFMAG1    'E'   /* e_ident[EI_MAG1] */ 
#define ELFMAG2    'L'   /* e_ident[EI_MAG2] */
#define ELFMAG3    'F'   /* e_ident[EI_MAG3] */


/* e_ident[EI_CLASS] identifies the file's class, or capacity. */
#define ELFCLASSNONE 0 /* Invalid class  */ 
#define ELFCLASS32   1 /* 32-bit objects */ 
#define ELFCLASS64   2 /* 64-bit objects */


/* EI_DATA Byte e_ident[EI_DATA] specifies the data encoding of the 
 * processor-specific data in the object file. The following encodings are currently defined.
 */

#define ELFDATANONE 0 /* Invalid data encoding          */ 
#define ELFDATA2LSB 1 /* Little endian (x86 byte order) */
#define ELFDATA2MSB 2 /* Big endian                     */ 



/**
 ** ELF Sections
 **/


/* Special Section Indexes 
 *
 * SHN_UNDEF 
 * This value marks an undefined, missing, irrelevant, or otherwise meaningless 
 * section reference. For example, a symbol ''defined'' relative to section number 
 * SHN_UNDEF is an undefined symbol. NOTE Although index 0 is reserved as the undefined value, 
 * the section header table contains an entry for index 0. 
 * That is, if the e_shnum member of the ELF header says a file has 6 entries in the 
 * section header table, they have the indexes 0 through 5. The contents of the initial 
 * entry are specified later in this section. 
 * 
 * SHN_LORESERVE 
 * This value specifies the lower bound of the range of reserved indexes. 
 *
 * SHN_LOPROC through SHN_HIPROC 
 * Values in this inclusive range are reserved for processor-specific semantics. 
 * 
 * SHN_ABS 
 * This value specifies absolute values for the corresponding reference. 
 * For example, symbols defined relative to section number SHN_ABS have 
 * absolute values and are not affected by relocation. 
 *
 * SHN_COMMON 
 * Symbols defined relative to this section are common symbols, 
 * such as FORTRAN COMMON or unallocated C external variables. 
 * 
 * SHN_HIRESERVE 
 * This value specifies the upper bound of the range of reserved indexes. 
 * The system reserves indexes between SHN_LORESERVE and SHN_HIRESERVE, inclusive; 
 * the values do not reference the section header table. That is, the section header 
 * table does not contain entries for the reserved indexes.
 *
 */

#define SHN_UNDEF     0 
#define SHN_LORESERVE 0xff00 
#define SHN_LOPROC    0xff00 
#define SHN_HIPROC    0xff1f 
#define SHN_ABS       0xfff1 
#define SHN_COMMON    0xfff2 
#define SHN_HIRESERVE 0xffff 


/*
 * ELF Section header
 */

typedef struct 
{ 
  /* This member specifies the name of the section. 
   * Its value is an index into the section header string table section, 
   * giving the location of a null-terminated string.
   */
  Elf32_Word   sh_name;
 
  /* This member categorizes the section's contents and semantics. 
   * Section types and their descriptions appear below.
   */
  Elf32_Word   sh_type; 

  /* Sections support 1-bit flags that describe miscellaneous attributes. 
   * Flag definitions appear below.
   */
  Elf32_Word   sh_flags; 

  /* If the section will appear in the memory image of a process, 
   * this member gives the address at which the section's first byte should reside. 
   * Otherwise, the member contains 0.
   */
  Elf32_Addr   sh_addr; 

  /* This member's value gives the byte offset from the beginning of the file to the 
   * first byte in the section. One section type, SHT_NOBITS described below, occupies 
   * no space in the file, and its sh_offset member locates the conceptual placement in the file.
   */
  Elf32_Off    sh_offset; 

  /* This member gives the section's size in bytes. Unless the section type is SHT_NOBITS, 
   * the section occupies sh_size bytes in the file. A section of type SHT_NOBITS may have a 
   * non-zero size, but it occupies no space in the file.
   */
  Elf32_Word   sh_size; 

  /* This member holds a section header table index link, whose interpretation depends 
   * on the section type. A table below describes the values.
   */
  Elf32_Word   sh_link; 

  /* This member holds extra information, whose interpretation depends on the section 
   * type. A table below describes the values.
   */
  Elf32_Word   sh_info; 

  /* Some sections have address alignment constraints. For example, if a section holds a 
   * doubleword, the system must ensure doubleword alignment for the entire section. 
   * That is, the value of sh_addr must be congruent to 0, modulo the value of sh_addralign. 
   * Currently, only 0 and positive integral powers of two are allowed. 
   * Values 0 and 1 mean the section has no alignment constraints.
   */
  Elf32_Word   sh_addralign; 

  /* Some sections hold a table of fixed-size entries, such as a symbol table. 
   * For such a section, this member gives the size in bytes of each entry. 
   * The member contains 0 if the section does not hold a table of fixed-size entries.
   */
  Elf32_Word   sh_entsize;
 
} Elf32_Shdr;


/* 
 * Values for sh_type
 *
 * SHT_NULL 
 * This value marks the section header as inactive; it does not have an associated 
 * section. Other members of the section header have undefined values. 
 * 
 * SHT_PROGBITS 
 * The section holds information defined by the program, whose format and meaning 
 * are determined solely by the program. 
 *
 * SHT_SYMTAB and SHT_DYNSYM 
 * These sections hold a symbol table. Currently, an object file may have only one section 
 * of each type, but this restriction may be relaxed in the future. Typically, SHT_SYMTAB 
 * provides symbols for link editing, though it may also be used for dynamic linking. 
 * As a complete symbol table, it may contain many symbols unneces-sary for dynamic linking. 
 * Consequently, an object file may also contain a SHT_DYNSYM section, which holds a minimal 
 * set of dynamic linking symbols, to save space. See ''Symbol Table'' below for details.
 *
 * SHT_STRTAB 
 * The section holds a string table. An object file may have multiple string table sections. 
 * See ''String Table'' below for details. 
 *
 * SHT_RELA 
 * The section holds relocation entries with explicit addends, such as type Elf32_Rela for 
 * the 32-bit class of object files. An object file may have multiple relocation sections. 
 * See ''Relocation'' below for details. 
 *
 * SHT_HASH 
 * The section holds a symbol hash table. All objects participating in dynamic linking must 
 * contain a symbol hash table. Currently, an object file may have only one hash table, 
 * but this restriction may be relaxed in the future. See ''Hash Table'' in Part 2 for details. 
 *
 * SHT_DYNAMIC 
 * The section holds information for dynamic linking. Currently, an object file may have only 
 * one dynamic section, but this restriction may be relaxed in the future. See ''Dynamic Section'' 
 * in Part 2 for details. 
 *
 * SHT_NOTE 
 * The section holds information that marks the file in some way. See ''Note Section'' in Part 2 
 * for details. 
 *
 * SHT_NOBITS 
 * A section of this type occupies no space in the file but otherwise resembles SHT_PROGBITS. 
 * Although this section contains no bytes, the sh_offset member contains the conceptual file offset. 
 *
 * SHT_REL 
 * The section holds relocation entries without explicit addends, such as type Elf32_Rel 
 * for the 32-bit class of object files. An object file may have multiple relocation sections. 
 * See ''Relocation'' below for details. 
 *
 * SHT_SHLIB 
 * This section type is reserved but has unspecified semantics. Programs that contain a 
 * section of this type do not conform to the ABI. 
 *
 * SHT_LOPROC through SHT_HIPROC 
 * Values in this inclusive range are reserved for processor-specific semantics. 
 *
 * SHT_LOUSER 
 * This value specifies the lower bound of the range of indexes reserved for application 
 * programs. 
 *
 * SHT_HIUSER 
 * This value specifies the upper bound of the range of indexes reserved for application 
 * programs. Section types between SHT_LOUSER and SHT_HIUSER may be used by the application, 
 * without conflicting with current or future system-defined section types.
 *
 */

#define SHT_NULL     0 
#define SHT_PROGBITS 1 
#define SHT_SYMTAB   2 
#define SHT_STRTAB   3 
#define SHT_RELA     4 
#define SHT_HASH     5 
#define SHT_DYNAMIC  6 
#define SHT_NOTE     7 
#define SHT_NOBITS   8 
#define SHT_REL      9 
#define SHT_SHLIB    10 
#define SHT_DYNSYM   11 
#define SHT_LOPROC   0x70000000 
#define SHT_HIPROC   0x7fffffff 
#define SHT_LOUSER   0x80000000 
#define SHT_HIUSER   0xffffffff

/*
 * sh_flags Values
 *
 * SHF_WRITE 
 * The section contains data that should be writable during process execution. 
 * 
 * SHF_ALLOC 
 * The section occupies memory during process execution. 
 * Some control sections do not reside in the memory image of an object file; 
 * this attribute is off for those sections. 
 *
 * SHF_EXECINSTR 
 * The section contains executable machine instructions. 
 *
 * SHF_MASKPROC 
 * All bits included in this mask are reserved for processor-specific semantics.
 *
 */

#define SHF_WRITE     0x1 
#define SHF_ALLOC     0x2 
#define SHF_EXECINSTR 0x4 
#define SHF_MASKPROC  0xf0000000


/**
 ** ELF program header
 **/

typedef struct 
{
  /* p_type 
   * This member tells what kind of segment this array element describes 
   * or how to interpret the array element's information. 
   * Type values and their meanings appear below.
   */
  Elf32_Word   p_type; 

  /* p_offset
   * This member gives the offset from the beginning of the file at which 
   * the first byte of the segment resides.
   */
  Elf32_Off    p_offset; 
  
  /* p_vaddr 
   * This member gives the virtual address at which the first byte of the 
   * segment resides in memory.
   */
  Elf32_Addr   p_vaddr; 

  /* p_paddr 
   * On systems for which physical addressing is relevant, this member is reserved 
   * for the segment's physical address. Because System V ignores physical addressing 
   * for application programs, this member has unspecified contents for executable files 
   * and shared objects.
   */
  Elf32_Addr   p_paddr; 

  /* p_filesz 
   * This member gives the number of bytes in the file image of the segment; it may be zero.
   */
  Elf32_Word   p_filesz; 
 
  /* p_memsz 
   * This member gives the number of bytes in the memory image of the segment; it may be zero.
   */
  Elf32_Word   p_memsz; 

  /* p_flags 
   * This member gives flags relevant to the segment. Defined flag values appear below.
   */
  Elf32_Word   p_flags; 

  /* p_align 
   * As ''Program Loading'' later in this part describes, loadable process segments must 
   * have congruent values for p_vaddr and p_offset, modulo the page size. 
   * This member gives the value to which the segments are aligned in memory and in the 
   * file. Values 0 and 1 mean no alignment is required. Otherwise, p_align should be a 
   * positive, integral power of 2, and p_vaddr should equal p_offset, modulo p_align.
   */
  Elf32_Word   p_align;
 
} Elf32_Phdr;


/* p_type values
 *
 *
 * PT_NULL 
 * The array element is unused; other members' values are undefined. 
 * This type lets the program header table have ignored entries. 
 *
 * PT_LOAD 
 * The array element specifies a loadable segment, described by p_filesz and p_memsz. 
 * The bytes from the file are mapped to the beginning of the memory segment. 
 * If the segment's memory size (p_memsz) is larger than the file size (p_filesz), 
 * the ''extra'' bytes are defined to hold the value 0 and to follow the segment's 
 * initialized area. The file size may not be larger than the memory size. 
 * Loadable segment entries in the program header table appear in ascending order, 
 * sorted on the p_vaddr member. 
 *
 * PT_DYNAMIC 
 * The array element specifies dynamic linking information. See ''Dynamic Section'' 
 * below for more information. 
 *
 * PT_INTERP 
 * The array element specifies the location and size of a null-terminated path name to 
 * invoke as an interpreter. This segment type is meaningful only for executable files 
 * (though it may occur for shared objects); it may not occur more than once in a file. 
 * If it is present, it must precede any loadable segment entry. See ''Program Interpreter'' 
 * below for further information. 
 *
 * PT_NOTE 
 * The array element specifies the location and size of auxiliary information. 
 * See ''Note Sec-tion'' below for details. 
 *
 * PT_SHLIB 
 * This segment type is reserved but has unspecified semantics. Programs that contain 
 * an array element of this type do not conform to the ABI. 
 *
 * PT_PHDR 
 * The array element, if present, specifies the location and size of the program header 
 * table itself, both in the file and in the memory image of the program. This segment 
 * type may not occur more than once in a file. Moreover, it may occur only if the program 
 * header table is part of the memory image of the program. If it is present, it must 
 * precede any loadable segment entry. See ''Program Interpreter'' below for further 
 * information. 
 *
 * PT_LOPROC through PT_HIPROC 
 * Values in this inclusive range are reserved for processor-specific semantics.
 *
 */

#define PT_NULL    0 
#define PT_LOAD    1 
#define PT_DYNAMIC 2 
#define PT_INTERP  3 
#define PT_NOTE    4 
#define PT_SHLIB   5 
#define PT_PHDR    6 
#define PT_LOPROC  0x70000000 
#define PT_HIPROC  0x7fffffff

/* 
 * p_flags
 * 
 * PF_X 
 * executable
 *
 * PF_W
 * writeable
 *
 * PF_R
 * readable
 *
 * PF_MASKPROC
 * mask for processor specific flags
 *
 */

#define PF_X            (1 << 0)        /* executable */
#define PF_W            (1 << 1)        /* writable */
#define PF_R            (1 << 2)        /* readable */
#define PF_MASKPROC     0xf0000000
       

#endif
