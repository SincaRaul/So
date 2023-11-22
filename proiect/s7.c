#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
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



void process_file(const char *filename, int output_fd) {
    int f_intrare = open(filename, O_RDONLY);
    if (f_intrare == -1) {
        perror("Nu am putut deschide fisierul de intrare");
        return;
    }



    struct stat st;

    if (fstat(f_intrare, &st) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        close(f_intrare);
        return;
    }



    char *extension = strrchr(filename, '.');

    if (extension != NULL && strcmp(extension, ".bmp") == 0) {
        // Este un fișier BMP
        char bmp_header[54];
        if (read(f_intrare, bmp_header, sizeof(bmp_header)) == -1) {
            perror("Eroare la citirea header-ului BMP");
            close(f_intrare);
            return;
        }

        int height = abs(readIntFromHeader(bmp_header, 22));  // BMP height offset
        int width = abs(readIntFromHeader(bmp_header, 18));   // BMP width offset
        char permissions[10];
        parsePermissions(st.st_mode, permissions);
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", localtime(&st.st_mtime));

        char statistica[300];

        sprintf(statistica, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                filename, height, width, st.st_size, st.st_uid, timeStr, st.st_nlink, permissions, permissions + 3, permissions + 6);

        if (write(output_fd, statistica, strlen(statistica)) == -1) {
            perror("Eroare la scrierea in fisierul de statistica");
        }

    } else if (S_ISREG(st.st_mode)) {
        // Este un fișier obișnuit, dar fără extensia .bmp
        char permissions[10];
        parsePermissions(st.st_mode, permissions);

        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", localtime(&st.st_mtime));

        char statistica[500];

        sprintf(statistica, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                filename, st.st_size, st.st_uid, timeStr, st.st_nlink, permissions, permissions + 3, permissions + 6);

        if (write(output_fd, statistica, strlen(statistica)) == -1) {
            perror("Eroare la scrierea in fisierul de statistica");
        }

    } else if (S_ISDIR(st.st_mode)) {
        // Este un director
        char permissions[10];
        parsePermissions(st.st_mode, permissions);

        char statistica[300];

        sprintf(statistica, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                filename, st.st_uid, permissions, permissions + 3, permissions + 6);

        if (write(output_fd, statistica, strlen(statistica)) == -1) {
            perror("Eroare la scrierea in fisierul de statistica");
        }

    } else if (S_ISLNK(st.st_mode)) {
        // Este o legătură simbolică
        char link_target[PATH_MAX];
        ssize_t target_len = readlink(filename, link_target, sizeof(link_target) - 1);
        link_target[target_len] = '\0';

        char permissions[10];
        parsePermissions(st.st_mode, permissions);

        char statistica[500];

        sprintf(statistica, "nume legatura: %s\ndimensiune: %ld\ndimensiune fisier: %ld\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                filename, st.st_size, st.st_size, st.st_uid, permissions, permissions + 3, permissions + 6);

        if (write(output_fd, statistica, strlen(statistica)) == -1) {
            perror("Eroare la scrierea in fisierul de statistica");
        }
    }
    close(f_intrare);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <director_intrare>\n", argv[0]);
        return 1;
    }



    char *nume_director_intrare = argv[1];
    char nume_fisier_statistica[] = "statistica.txt";

    int f_statistica = open(nume_fisier_statistica, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (f_statistica == -1) {
        perror("Nu am putut deschide fisierul de statistica");
        return 1;
    }



    DIR *dir = opendir(nume_director_intrare);
    if (dir == NULL) {
        perror("Nu am putut deschide directorul");
        close(f_statistica);
        return 1;
    }



    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            char entry_path[PATH_MAX];
            sprintf(entry_path, "%s/%s", nume_director_intrare, entry->d_name);
            process_file(entry_path, f_statistica);
        }

    }

    closedir(dir);
    close(f_statistica);
    return 0;
}