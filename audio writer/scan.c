#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Note {
  double freq;
  double start;
  double end;
  char wave[10];
};

#define BUF_SIZE 65536

int count_lines(FILE* file) // stolen from https://stackoverflow.com/a/70708991
{
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
}

int main() {
  FILE *fp;
  double freq;
  double start;
  double end;
  char wave[10];
  int num_lines;
  int max_end = 0;
  int i = 0;
 
  fp = fopen("notes.csv", "r");
  num_lines = count_lines(fp);
  printf("Number of lines is: %d\n", num_lines);
  rewind(fp);
  struct Note * notes = (struct Note *) malloc(sizeof(struct Note) * (num_lines+ 9)); // add a bit of space

  while (fscanf(fp, "%lf,%lf,%lf,%s", &freq, &start, &end, wave) !=EOF) {
    printf("wave type is %s\n", wave);
    printf("freq: %lf\n", freq);
    //struct Note current_note = {freq, start, end, *wave};
    notes[i].freq = freq;
    notes[i].start = start;
    notes[i].end = end;
    strcpy(notes[i].wave, wave);
    i++;
    max_end = end > max_end ? end : max_end; // figure out when the last note ends to determine buffer size
  }
  for (int j=0; j < 5; ++j){
    printf("type of wave is: %s\n", notes[j].wave);
  }

  printf("ends at: %d\n", max_end);

}
