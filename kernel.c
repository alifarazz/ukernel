/************/
/* kernel.c */
/************/

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;

/* void clear_screen() */

char *vidptr = (char*) 0xb8000;

void kmain(void)
{
  const char *str = "git gud";

  uint j;

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
