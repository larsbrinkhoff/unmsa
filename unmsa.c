#include <stdio.h>

unsigned char buf[512 * 11 * 90];
size_t size;
int verbose = 0;

static int word (unsigned char *p)
{
  return p[0] << 8 | p[1];
}

static unsigned char *uncompress (unsigned char *p, int size)
{
  unsigned char *end = p + size;
  unsigned char data;
  int i, length;

  while (p < end) {
    if (verbose) {
      fprintf (stderr, "Location: %p < %p\n", p, end);
      fprintf (stderr, "Buffer: %02x %02x %02x %02x %02x ...\n",
               p[0], p[1], p[2], p[3], p[4]);
    }
    if (*p == 0xE5) {
      data = *++p;
      length = word (p + 1);
      p += 3;
      if (verbose)
        fprintf (stderr, "Run length %d, data %02x\n", length, data);
      for (i = 0; i < length; i++)
        fputc (data, stdout);
    } else {
      if (verbose)
        fprintf (stderr, "Data octet %02x\n", *p);
      fputc (*p++, stdout);
    }
  }
}

static unsigned char *track (unsigned char *p, int sectors, int sides)
{
  int i, size;

  for (i = 0; i < sides; i++) {
    size = word (p);
    p = p + 2;
    if (size == 512 * sectors) {
      if (verbose)
        fprintf (stderr, "Uncompressed track\n");
      fwrite (p, 1, size, stdout);
      p = p + size;
    } else {
      if (verbose)
        fprintf (stderr, "Compressed track: %d\n", size);
      p = uncompress (p, size);
    }
  }

  return p;
}

int main (void)
{
  int i, sectors, sides, start, end;
  unsigned char *p;

  size = fread (buf, 1, sizeof buf, stdin);

  sectors = word (buf + 2);
  sides = word (buf + 4) + 1;
  start = word (buf + 6);
  end = word (buf + 8);
  fprintf (stderr, "ID: %04x\n", word (buf));
  fprintf (stderr, "Sectors: %d\n", sectors);
  fprintf (stderr, "Sides: %d\n", sides);
  fprintf (stderr, "Start track: %d\n", start);
  fprintf (stderr, "End track: %d\n", end);

  p = buf + 10;
  for (i = start; i <= end; i++)
    p = track (p, sectors, sides);

  if (p != buf + size)
    fprintf (stderr, "Complain!\n");

  return 0;
}
