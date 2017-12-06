/************/
/* kernel.c */
/************/

#include "./keyboard_map.h"
#include "./utils.h"

#define VIDEO_LINES 25
#define VIDEO_COLUMNS_IN_LINE 80
#define VIDEO_ELEMENT_SIZE_BYTE 2
#define VIDEO_SCREENSIZE_BYTE (VIDEO_ELEMENT_SIZE_BYTE *	\
			      VIDEO_COLUMNS_IN_LINE *		\
			      VIDEO_LINES)			\


#define PORT_KEYBOARD_DATA 0x60
#define PORT_KEYBOARD_STATUS 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8E
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

/* external data */
extern Uchar keyboad_map[128];

/* external funtions */
extern void keyboard_handler();
extern char read_port(Ushort port);
extern void write_port(Ushort port, Uchar data);
extern void load_idt(Ulong *idt_ptr);

struct IDT_entry{
  Ushort offset_lowerbits;
  Ushort selector;
  Uchar zero;
  Uchar type_attr;
  Uint offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];
char *vidptr = (char*) 0xb8000;


void idt_init()
{
  Ulong keyboard_address;
  Ulong idt_address;
  Ulong idt_ptr[2];

  /* hackety hack hack!! */
  keyboard_address = (Ulong) keyboard_handler;
  IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
  IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
  IDT[0x21].zero = 0;
  IDT[0x21].type_attr = INTERRUPT_GATE;
  IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

  /*************************/
  /*    Ports		   */
  /*          PIC1    PIC2 */
  /* Command  0x20    0xA0 */
  /* Data     0x21    0xA1 */
  /*************************/

  /* ICW1 - begin initalization */
  write_port(0x20, 0x11);
  write_port(0xA0, 0x11);


  /***********************************************************************/
  /* In x86 protected mode, we have to remap the PICs beyod 0x20 because */
  /* Intel decided to reserve the first 32 interrupts for cpu exceptions */
  /***********************************************************************/
  /* ICW2 - remap offset address of IDT */
  write_port(0x21, 0x20);
  write_port(0xA1, 0x28);

  /* ICW3 - setup cascading */
  write_port(0x21, 0x00);
  write_port(0xA1, 0x00);

  /* ICW4 - environment info */
  write_port(0x21, 0x01);
  write_port(0xA1, 0x01);
  /* Done initializaing */

  /* mask interrupts */
  write_port(0x21, 0xFF);
  write_port(0xA1, 0xFF);

  /* fill the IDT  */
  idt_address = (Ulong) IDT;

  /* pointer = size + offset */
  idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xFFFF) << 16);
  idt_ptr[1] = idt_address >> 16;

  load_idt(idt_ptr);
}


void kmain()
{
  const char *str = "git gud";

  Uint j;

  /* this loop clears the screen */
  /* 25 lines, 80 columns */  /* each element is a word */
  for (j = 0; j < VIDEO_SCREENSIZE_BYTE; j += 2){
    /* character part */
    vidptr[j] = ' ';
    /* attribute-byte */
    vidptr[j + 1] = 0x07; /* light grey on black screen */
  }

  for (j = 0; str[j / 2] != '\0'; j += 2){
    /* set ascii code */
    vidptr[j] = str[j / 2];

    /* black char on grey fg */
    vidptr[j + 1] = 0x07;
  }

  return ;
}
