#ifndef ELF_H
#define ELF_H
#include "types.h"

//ELF special typedefs
typedef uint 		Elf32_Addr;
typedef ushort		Elf32_Half;
typedef uint		Elf32_Off;
typedef uint		Elf32_Sword;
typedef uint		Elf32_Word;

//File types
#define ET_NONE		0x00
#define ET_REL		0x01	//Relocateable file
#define ET_EXEC		0x02	//Executeable file
#define ET_DYN		0x03	//Shared object file
#define ET_CORE		0x04	//Core file
#define ET_LOPROC	0xff00
#define ET_HIPROC	0xffff

//Machine types
#define EM_NONE 	0x00
#define EM_M32		0x01	//AT&T
#define EM_SPARC	0x02	//SPARC
#define EM_386		0x03	//Intel 80386
#define EM_68K		0x04	//Motorola 68000
#define EM_88K		0x05	//Motorola 88000
#define EM_860		0x06	//Intel 80860
#define EM_MIPS		0x08	//MIPS RS3000

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define EI_NIDENT 16
typedef struct
{
	byte		e_ident[EI_NIDENT];		//identify the file
	Elf32_Half	e_type;					//elf file type
	Elf32_Half 	e_machine;				//built for?
	Elf32_Word	e_version;				//current version number
	Elf32_Addr	e_entry;				//Virtual address for the entry point
	Elf32_Off   e_phoff;				//File offset to the header table
	Elf32_Off	e_shoff;				//File offset to the section header table
	Elf32_Word	e_flags;				//Processor specific flags
	Elf32_Half	e_ehsize;				//ELF header size in bytes
	Elf32_Half	e_phentsize;			//Program header table size
	Elf32_Half	e_phnum;				//Number of entries int the program header table
	Elf32_Half	e_shentsize;			//Section header size in bytes
	Elf32_Half	e_shnum;				//Number of entries in the section header table
	Elf32_Half	e_shstrnxd;				//Section header table index to 
} Elf32_Ehdr;

//Sector types
#define SHT_NULL		0x00
#define SHT_PROGBITS	0x01
#define SHT_SYMTAB		0x02
#define SHT_STRTAB		0x03
#define SHT_RELA		0x04
#define SHT_HASH		0x05
#define SHT_DYNAMIC		0x06
#define SHT_NOTE		0x07
#define SHT_NOBITS		0x08
#define SHT_REL			0x09
#define SHT_SHLIB		0x0A
#define SHT_DYNSYM		0x0B
#define SHT_LOPROC		0x70000000
#define SHT_HIPROC		0x7fffffff
#define SHT_LOUSER		0x80000000
#define SHT_HIUSER		0xffffffff

typedef struct
{
	Elf32_Word	sh_name;		//Section name
	Elf32_Word	sh_type;		//Categorizes the section content and semantics
	Elf32_Word	sh_flags;		//1-bit flags to describe attributes
	Elf32_Addr	sh_addr;		//default 0 - initialize later
	Elf32_Off	sh_offset;		//Offset to the first byte of the section
	Elf32_Word	sh_size;		//Section size
	Elf32_Word	sh_link;		//Table index link
	Elf32_Word	sh_info;		//Extra information
	Elf32_Word	sh_addralign;	//Address alignement?
	Elf32_Word	sh_entsize;		//for specific tables it gives the fixed size of the sectors
} Elf32_Shdr;

#endif
