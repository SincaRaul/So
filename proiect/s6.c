
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

void parsePermissions(mode_t mode, char *permissions) {
    permissions[0] = (mode & S_IRUSR) ? 'R' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'W' : '-';
    permissions[2] = (mode & S_IXUSR) ? 'X' : '-';
    permissions[3] = (mode & S_IRGRP) ? 'R' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'W' : '-';
    permissions[5] = (mode & S_IXGRP) ? 'X' : '-';
    permissions[6] = (mode & S_IROTH) ? 'R' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'W' : '-';
    permissions[8] = (mode & S_IXOTH) ? 'X' : '-';
    permissions[9] = '\0';
}

int readIntFromHeader(char *header, int offset) {
    unsigned char b1 = (unsigned char)header[offset];
    unsigned char b2 = (unsigned char)header[offset + 1];
    unsigned char b3 = (unsigned char)header[offset + 2];
    unsigned char b4 = (unsigned char)header[offset + 3];

    return (int)b1 | ((int)b2 << 8) | ((int)b3 << 16) | ((int)b4 << 24);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <fisier_intrare>\n", argv[0]);
        return 1;
    }

    char *nume_fisier_intrare = argv[1];
    char nume_fisier_statistica[] = "statistica.txt";

    int f_intrare = open(nume_fisier_intrare, O_RDONLY);
    if (f_intrare == -1) {
        perror("Nu am putut deschide fisierul de intrare");
        return 1;
    }

    int f_statistica = open(nume_fisier_statistica, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (f_statistica == -1) {
        perror("Nu am putut deschide fisierul de statistica");
        close(f_intrare);
        return 1;
    }

    // Citeste header-ul BMP pentru a obtine dimensiunile imaginii
    char bmp_header[54];
    if (read(f_intrare, bmp_header, sizeof(bmp_header)) == -1) {
        perror("Eroare la citirea header-ului BMP");
        close(f_intrare);
        close(f_statistica);
        return 1;
    }

    int height = abs(readIntFromHeader(bmp_header, 22));  // BMP height offset
    int width = abs(readIntFromHeader(bmp_header, 18));   // BMP width offset

    struct stat st;
    if (fstat(f_intrare, &st) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        close(f_intrare);
        close(f_statistica);
        return 1;
    }

    char permissions[10];
    parsePermissions(st.st_mode, permissions);

    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", localtime(&st.st_mtime));

    char statistica[300];
    sprintf(statistica, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",
            nume_fisier_intrare, height, width, st.st_size, st.st_uid, timeStr, st.st_nlink, permissions, permissions + 3, permissions + 6);

    if (write(f_statistica, statistica, strlen(statistica)) == -1) {
        perror("Eroare la scrierea in fisierul de statistica");
    }

    close(f_intrare);
    close(f_statistica);

    return 0;
}
