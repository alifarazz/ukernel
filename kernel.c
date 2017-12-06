/************/
/* kernel.c */
/************/

#include "./utils.h"

#define VIDEO_LINES 25
#define VIDEO_COLUMNS_IN_LINE 80
#define VIDEO_ELEMENT_SIZE_BYTE 2
#define VIDEO_SCREENSIZE_BYTE {				\
    (VIDEO_ELEMENT_SIZE_BYTE *				\
     VIDEO_COLUMNS_IN_LINE *				\
     VIDEO_LINES)					\
  }

#define PORT_KEYBOARD_DATA 0x60
#define PORT_KEYBOARD_STATUS 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8E
#define KERNEL_CODE_SEGMENT 0x08
/* void clear_screen() */

/* external data */
extern Uchar keyboad_map[128];

/* external funtions */
/* extern void keyboard_handler(); */
/* extern char read_port(Ushort port); */
/* extern void write_port(Ushort port, Uchar data); */
/* extern void loat_idt(Ulong *idt_ptr); */

char *vidptr = (char*) 0xb8000;


void kmain()
{
  const char *str = "git gud";

  Uint j;

  /* this loop clears the screen */
  /* 25 lines, 80 columns */  /* each element is a word */
  for (j = 0; j < 80 * 25 * 2; j += 2){
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
