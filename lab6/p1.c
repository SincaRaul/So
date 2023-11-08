#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
  if (argc != 4)
    {
      printf("Mod de utilizare: %s <fisier_intrare> <fisier_statistica> <caracter>\n", argv[0]);
      return 1;
    }

  char *nume_fisier_intrare = argv[1];
  char *nume_fisier_statistica = argv[2];
  char caracter = argv[3][0];

  int fd_intrare = open(nume_fisier_intrare, O_RDONLY);
  if (fd_intrare == -1)
    {
      printf("Nu am putut deschide fisierul de intrare: %s\n", nume_fisier_intrare);
      return 1;
    }

  int fd_statistica = open(nume_fisier_statistica, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd_statistica == -1)
    {
      printf("Nu am putut deschide fisierul de statistica: %s\n", nume_fisier_statistica);
      close(fd_intrare);
      return 1;
    }

  int a = 0, A = 0, d = 0, numar_aparitii = 0;
  char caracter_citit;

  struct stat st;
  if (fstat(fd_intrare, &st) == -1)
    {
      perror("Eroare la obținerea dimensiunii fișierului");
      close(fd_intrare);
      close(fd_statistica);
      return 1;
    }

  while (read(fd_intrare, &caracter_citit, 1) > 0)
    {
      if (caracter_citit == caracter)
	{
	  numar_aparitii++;
	}
      if (islower(caracter_citit))
	{
	  a++;
	} else if (isupper(caracter_citit))
	{
	  A++;
	} else if (isdigit(caracter_citit))
	{
	  d++;
	}
    }

  char statistica_text[100];
  sprintf(statistica_text, "numar litere mici: %d\nnumar litere mari: %d\nnumar cifre: %d\nnumar aparitii caracter: %d\ndimensiune fisier: %ld\n", a, A, d, numar_aparitii, st.st_size);

  write(fd_statistica, statistica_text, strlen(statistica_text));

  close(fd_intrare);
  close(fd_statistica);

  return 0;
}
