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
			       VIDEO_LINES)

#define PORT_KEYBOARD_DATA 0x60
#define PORT_KEYBOARD_STATUS 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8E
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

/* external data */
/* extern UCHAR keyboard_map[128]; */

/* external funtions */
extern void keyboard_handler(void);
extern char read_port(USHORT port);
extern void write_port(USHORT port, UCHAR data);
extern void load_idt(ULONG *idt_ptr);

struct IDT_entry {
  USHORT offset_lowerbits;
  USHORT selector;
  UCHAR zero;
  UCHAR type_attr;
  USHORT offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

volatile char *vidptr = (char*) 0xb8000; // video memory begins at 0xb8000 
UINT current_loc = 0; // cursor location 

void idt_init(void)
{
  ULONG keyboard_address;
  ULONG idt_address;
  ULONG idt_ptr[2];

  /* populate IDT's entry of keyboard's interrupt */
  keyboard_address = (ULONG) keyboard_handler;
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

  /******************************************************/
  /* In x86 protected mode, we have to remap the PICs 	*/
  /* beyond 0x20 because Intel decided to designate the	*/
  /* first 32 interrupts as reserved for cpu exceptions */
  /******************************************************/

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

  /* fill the IDT descriptor  */
  idt_address = (ULONG) IDT;

  /* pointer = size + offset */
  idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xFFFF) << 16);
  idt_ptr[1] = idt_address >> 16;

  load_idt(idt_ptr);
}

void kb_init(void)
{
  /* 0xFD enables IRQ1 only (the keyboard) */
  write_port(0x21, 0xFD);
}

void kprint(const char str[])
{
   for (UINT i = 0; str[i] != '\0'; i++) {
    vidptr[current_loc++] = str[i]; // set foreground to str[i] glyph
    vidptr[current_loc++] = 0x07;   // set background to black
  }
}

void kprint_newline(void)
{
  UINT line_size = VIDEO_ELEMENT_SIZE_BYTE * VIDEO_COLUMNS_IN_LINE;
  current_loc += line_size - current_loc % line_size;
}

void clear_screen(void)
{
  UINT i = 0;
  while (i < VIDEO_SCREENSIZE_BYTE) {
    vidptr[i++] = ' ';
    vidptr[i++] = 0x07;
  }
}

void keyboard_handler_main(void)
{
  UCHAR status;
  char keycode;

  /* write EOI */
  write_port(0x20, 0x20);

  status = read_port(PORT_KEYBOARD_STATUS);
  /* LSB of status will be set if buffer isn't empty */
  if (status & 0x01) {
    keycode = read_port(PORT_KEYBOARD_DATA);

    if (keycode < 0)
      return ;

    if (keycode == ENTER_KEY_CODE) {
      kprint_newline();
      return ;
    }

    vidptr[current_loc++] = keyboard_map[(UCHAR) keycode];
    /* vidptr[current_loc++] = 'a'; */
    vidptr[current_loc++] = 0x07;
  }
}

void kmain(void)
{
  const char *str = "got gud";

  clear_screen();
  kprint(str);
  kprint_newline();
  kprint_newline();

  idt_init();
  kb_init();

  for(;;);

  /* TODO: dump whole memory using gdb and compare */
  return ;
}
