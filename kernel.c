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
extern Uchar keyboard_map[128];

/* external funtions */
extern void keyboard_handler();
extern char read_port(Ushort port);
extern void write_port(Ushort port, Uchar data);
extern void load_idt(Ulong *idt_ptr);

struct IDT_entry {
  Ushort offset_lowerbits;
  Ushort selector;
  Uchar zero;
  Uchar type_attr;
  Uint offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];
volatile char *vidptr = (char*) 0xb8000; /* video memory begins at 0xb8000 */


void idt_init()
{
  Ulong keyboard_address;
  Ulong idt_address;
  Ulong idt_ptr[2];

  /* hackety hack hack!! */
  keyboard_address = (Ulong) keyboard_handler;
  IDT[0x21].offset_lowerbits = keyboard_address & 0xFFFF;
  IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
  IDT[0x21].zero = 0;
  IDT[0x21].type_attr = INTERRUPT_GATE;
  IDT[0x21].offset_higherbits = (keyboard_address & 0xFFFF0000) >> 16;

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

void kb_init()
{
  /* 0xFD enables IRQ1 only (the keyboard) */
  write_port(0x21, 0xFD);
}

void kprint(const char str[], Uint *current_loc_ptr)
{
  Uint i;
  for (i = 0; str[i] != '\0'; i++) {
    vidptr[(*current_loc_ptr)++] = str[i]; /* set foreground to str[i] glyph*/
    vidptr[(*current_loc_ptr)++] = 0x07;   /* set background to black*/
  }
}

void kprint_newline(Uint *current_loc_ptr)
{
  Uint line_size = VIDEO_ELEMENT_SIZE_BYTE * VIDEO_COLUMNS_IN_LINE;
  *current_loc_ptr += line_size - *(current_loc_ptr) % line_size;
}

void clear_screen(void)
{
  Uint i = 0;
  while (i < VIDEO_SCREENSIZE_BYTE) {
    vidptr[i++] = ' ';
    vidptr[i++] = 0x07;
  }
}

void keyboard_handler_main(Uint *current_loc_ptr)
{
  Uchar status;
  char keycode;

  /* write EOI */
  write_port(0x20, 0x20);

  status = read_port(PORT_KEYBOARD_STATUS);
  /* LSB of status will be set if buffer isn't empty */
  if (status & 0x01) {
    keycode = read_port(PORT_KEYBOARD_DATA);

    if (keycode < 0) return ;

    if (keycode == ENTER_KEY_CODE) {
      kprint_newline(current_loc_ptr);
      return ;
    }

    vidptr[(*current_loc_ptr)++] = keyboard_map[(Uchar) keycode];
    vidptr[(*current_loc_ptr)++] = 0x07;

  }
}

void kmain()
{
  const char *str = "got gud";
  Uint current_loc = 0;

  clear_screen();
  kprint(str, &current_loc);
  kprint_newline(&current_loc);
  kprint_newline(&current_loc);

  idt_init();
  kb_init();

  return ;
}
