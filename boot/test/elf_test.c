/*
 * ELF testing
 *
 * (c) 1999 Jimmy Larsson
 *
 */

#include <stdio.h>
#include "../../kernel/elf.h"

void die (char *mess)
{
  printf (mess);
  exit(1);
}


int main (int     argc, 
	  char   *argv[]) 
{
  Elf32_Ehdr    elf_header;
  char         *filename;
  FILE         *elf_file;
  Elf32_Shdr   *section_header_table;
  Elf32_Phdr   *program_header_table;
  int           pht_entries;
  int           sht_entries;
  int           pht_size;
  int           sht_size;
  int           i;

  if (argc < 2)
    die ("Usage: elf_test <elf file>\n");

  filename = argv[1];

  elf_file = fopen (filename, "r");
  if (elf_file == NULL)
    die ("Cant open file!");

  if (fread ((void *) &elf_header, 1, sizeof(elf_header), elf_file) != sizeof(elf_header))
    die ("Read error!");

  printf ("\n\nELF Header:\n");

  printf ("  Magic bytes: ");

  if (elf_header.e_ident[EI_MAG0] == ELFMAG0 &&  
      elf_header.e_ident[EI_MAG1] == ELFMAG1 &&  
      elf_header.e_ident[EI_MAG2] == ELFMAG2 &&  
      elf_header.e_ident[EI_MAG3] == ELFMAG3)
  {
    printf ("OK\n");
  } else
  {
    printf ("BAD!\n");
  }

  printf ("  Object file type: ");
  if (elf_header.e_type == ET_EXEC)
    printf ("executable\n");
  else
    printf ("BAD! (non executable)\n");

  printf ("  Machine type: ");
  if (elf_header.e_machine == EM_386)
    printf ("i386\n");
  else
    printf ("BAD! (non i386)\n");

  printf ("  ELF version: ");
  if (elf_header.e_version == EV_CURRENT)
    printf ("OK\n");
  else
    printf ("BAD!\n");


  printf ("  Entry: 0x%x\n", elf_header.e_entry);
  printf ("  Program header offset: %u\n", elf_header.e_phoff);
  printf ("  Section header offset: %u\n", elf_header.e_shoff);
  printf ("  Flags: %u\n", elf_header.e_flags);
  printf ("  ELF header size: %u\n", elf_header.e_ehsize);
 
  printf ("  PHT entry size: %u\n", elf_header.e_phentsize);
  printf ("  PHT entries: %u\n", elf_header.e_phnum);
  printf ("  SHT entry size: %u\n", elf_header.e_shentsize);
  printf ("  SHT entries: %u\n", elf_header.e_shnum);
  
  printf ("  String table index: %u\n", elf_header.e_shstrndx);
 
  pht_entries = elf_header.e_phnum;
  sht_entries = elf_header.e_shnum;
  pht_size = pht_entries * elf_header.e_phentsize; 
  sht_size = sht_entries * elf_header.e_shentsize; 


  program_header_table = (Elf32_Phdr *) malloc (pht_size);
  if (program_header_table == NULL)
    die ("Can't alloc mem");

  section_header_table = (Elf32_Shdr *) malloc (sht_size);
  if (section_header_table == NULL)
    die ("Can't alloc mem");

  /* Read program header table */
  fseek (elf_file, elf_header.e_phoff, SEEK_SET);

  if (fread ((void *) program_header_table, 1, pht_size, elf_file) != pht_size)
    die ("Read error!");

  /* Read section header table */
  fseek (elf_file, elf_header.e_shoff, SEEK_SET);

  if (fread ((void *) section_header_table, 1, sht_size, elf_file) != sht_size)
    die ("Read error!");

  printf ("  \n  Program Header Table:\n");
  for (i = 0; i < pht_entries; i++)
  {
    printf ("    Type %u\n    file offset 0x%08x,    vaddr 0x%08x, paddr 0x%08x\n",
	    program_header_table[i].p_type,
	    program_header_table[i].p_offset,
	    program_header_table[i].p_vaddr,
	    program_header_table[i].p_paddr);

    printf ("      file size 0x%08x, mem size 0x%08x, flags %u, align %u\n\n",
	    program_header_table[i].p_filesz,
	    program_header_table[i].p_memsz,
	    program_header_table[i].p_flags,
	    program_header_table[i].p_align);
  }
  

  printf ("\n");
  return 0;
}


