#define _GNU_SOURCE // pentru DT_REG, DT_LNK, DT_DIR
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <ctype.h>

typedef struct
{
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_pixels_per_m;
    uint32_t y_pixels_per_m;
    uint32_t colors_used;
    uint32_t colors_important;
} BMP_Header;

void close_file(int file_descriptor)
{
    int close_status;
    close_status = close(file_descriptor);
    if (close_status < 0)
    {
        perror("Eroare la inchidere fisier");
        exit(1);
    }
}


BMP_Header get_bmp_header(char *file_path)
{
    BMP_Header bmp_header;

    int file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor == -1)
    {
        perror("Eroare deschidere imagine bmp pentru a lua headerul");
        exit(1);
    }

    read(file_descriptor, &bmp_header.signature, 2);
    read(file_descriptor, &bmp_header.file_size, 4);
    read(file_descriptor, &bmp_header.reserved, 4);
    read(file_descriptor, &bmp_header.data_offset, 4);
    read(file_descriptor, &bmp_header.size, 4);
    read(file_descriptor, &bmp_header.width, 4);
    read(file_descriptor, &bmp_header.height, 4);
    read(file_descriptor, &bmp_header.planes, 2);
    read(file_descriptor, &bmp_header.bit_count, 2);
    read(file_descriptor, &bmp_header.compression, 4);
    read(file_descriptor, &bmp_header.image_size, 4);
    read(file_descriptor, &bmp_header.x_pixels_per_m, 4);
    read(file_descriptor, &bmp_header.y_pixels_per_m, 4);
    read(file_descriptor, &bmp_header.colors_used, 4);
    read(file_descriptor, &bmp_header.colors_important, 4);

    close_file(file_descriptor);

    return bmp_header;
}

int readIntFromHeader(char *header, int offset)
{
    unsigned char b1 = (unsigned char)header[offset];
    unsigned char b2 = (unsigned char)header[offset + 1];
    unsigned char b3 = (unsigned char)header[offset + 2];
    unsigned char b4 = (unsigned char)header[offset + 3];
    return (int)b1 | ((int)b2 << 8) | ((int)b3 << 16) | ((int)b4 << 24);
}

void writeToFile(int output_fd, const char *data)
{
    if (write(output_fd, data, strlen(data)) == -1)
    {
        perror("Eroare la scriere in fisier");
    }
}

int create_statistics_file(char *cale_fisier)
{
    int statistics_file;
    statistics_file = open(cale_fisier, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (statistics_file == -1)
    {
        perror("Eroare la creare fisier statistica.");
        exit(1);
    }
    return statistics_file;
}


void parsePermissions(mode_t mode, char *permissions)
{
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

struct stat get_file_stats(char *file_path)
{
    int status;
    struct stat file_status;

    status = stat(file_path, &file_status);
    if (status < 0)
    {
        perror("Eroare la obtinerea informatiilor despre fisier");
        exit(1);
    }
    return file_status;
}

void distribuie_permisii(char *permissions, char *permisii_user, char *permisii_grup, char *permisii_other)
{
    permisii_user[0] = permissions[0];
    permisii_user[1] = permissions[1];
    permisii_user[2] = permissions[2];
    permisii_user[3] = 0;

    permisii_grup[0] = permissions[3];
    permisii_grup[1] = permissions[4];
    permisii_grup[2] = permissions[5];
    permisii_grup[3] = 0;

    permisii_other[0] = permissions[6];
    permisii_other[1] = permissions[7];
    permisii_other[2] = permissions[8];
    permisii_other[3] = 0;
}

void _8_biti_per_pixel(int image_file_descriptor)
{
    // culorile utilizate cand avem 8 biti per pixel se afla intr-o paleta de culori care incepe de la adresa 54 (ColorTable)

    if (lseek(image_file_descriptor, 54, SEEK_SET) == -1)
    {
        perror("Eroare la cautarea in fisier");
        exit(1);
    }

    for (int i = 0; i < 256; ++i)
    {
        unsigned char color[4];
        if (read(image_file_descriptor, color, 4) == -1)
        {
            perror("Eroare la citirea datelor pixelilor");
            break;
        }
        unsigned char gray = color[0] * 0.299 + color[1] * 0.587 + color[2] * 0.114;
        color[0] = color[1] = color[2] = gray;

        lseek(image_file_descriptor, -4, SEEK_CUR);
        if (write(image_file_descriptor, color, 4) == -1)
        {
            perror("Eroare la scrierea datelor pixelilor");
            break;
        }
    }
}

void _24_biti_per_pixel(int image_file_descriptor, BMP_Header bmp_header)
{
    // cand avem 24 de biti per pixel, vom merge la campul RasterData cu adresa definita de DataOffset, loc in care gasim pixelii pozei. Pentru un pixel citim cate 3 octeti, pentru culorile rosu, verde si albastru
    // din motive de performanta, se va citi cate o linie din poza

    size_t width_chunk_size = 3 * bmp_header.width;
    unsigned char width_chunk[width_chunk_size];

    // mergem la DataOffset pentru a obtine adresa de inceput a RasterData
    if (lseek(image_file_descriptor, bmp_header.data_offset, SEEK_SET) == -1)
    {
        perror("Error jumping to DataOffset");
        exit(1);
    }

    for (int i = 0; i < bmp_header.height; i++)
    {
        // citim o linie
        read(image_file_descriptor, width_chunk, width_chunk_size);

        // punem cursorul inapoi la inceputul liniei pentru ca urmeaza sa modificam pixelii
        lseek(image_file_descriptor, -width_chunk_size, SEEK_CUR);

        // calculam nuanta de gri pe care o vom aplica celor 3 culori per pixel
        for (int j = 0; j < bmp_header.width; j++)
        {
            unsigned char aux = width_chunk[j * 3] * 0.299 + width_chunk[j * 3 + 1] * 0.587 + width_chunk[j * 3 + 2] * 0.114;
            width_chunk[j * 3] = aux;
            width_chunk[j * 3 + 1] = aux;
            width_chunk[j * 3 + 2] = aux;
        }

        // scriu linia convertita peste cea curenta
        write(image_file_descriptor, width_chunk, width_chunk_size);
    }
}

void grey_scale_image(char *image_path)
{
    BMP_Header bmp_header;

    bmp_header = get_bmp_header(image_path);

    int image_file_descriptor = open(image_path, O_RDWR);
    if (image_file_descriptor == -1)
    {
        perror("Eroare la deschidere imagine");
        exit(1);
    }

    // alegem care tip de procesare de imagine va fi folosit
    switch (bmp_header.bit_count)
    {
    case 24:
        _24_biti_per_pixel(image_file_descriptor, bmp_header);
        break;
    case 8:
        _8_biti_per_pixel(image_file_descriptor);
        break;
    default:
        break;
    }


    close_file(image_file_descriptor);
}

void process_directory(const char *cale_intrare_director, const char *cale_iesire_director, char *caracter, int *total_propozitii_corecte)
{
    DIR *dir = opendir(cale_intrare_director);
    if (dir == NULL)
    {
        perror("Eroare la deschidere folder");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        int pid;

        int lungime_cale_intrare = strlen(entry->d_name) + strlen(cale_intrare_director) + 2;
        char cale_intrare[lungime_cale_intrare];
        snprintf(cale_intrare, lungime_cale_intrare, "%s/%s", cale_intrare_director, entry->d_name);

        // creez fisierul de statistica pentru entry-ul pe care ma aflu acum
        int lungime_cale_statistica = strlen(cale_iesire_director) + strlen(entry->d_name) + strlen("_statistica.txt") + 2;
        char cale_iesire_statistica[lungime_cale_statistica];
        snprintf(cale_iesire_statistica, lungime_cale_statistica, "%s/%s_statistica.txt", cale_iesire_director, entry->d_name);

        int fisier_statistica;
        fisier_statistica = create_statistics_file(cale_iesire_statistica);

        // luam statusul intrarii actuale din directorul de intrare
        struct stat file_stat;
        file_stat = get_file_stats(cale_intrare);

        // iau permisiunile intrarii actuale
        char permissions[10];
        parsePermissions(file_stat.st_mode, permissions);
        char permisii_user[4], permisii_grup[4], permisii_other[4];
        distribuie_permisii(permissions, permisii_user, permisii_grup, permisii_other);

        if (entry->d_type == DT_REG) // in cazul in care entry-ul actual este fisier regular
        {
            char *file_extension = strrchr(entry->d_name, '.'); // luam extensia fisierului la care ne aflam

            if (file_extension != NULL && strcmp(file_extension, ".bmp") == 0) /* daca este true, atunci sunt pe o imagine bmp */
            {

                if ((pid = fork()) < 0)
                {
                    perror("eroare la crearea procesului copil care se ocupa cu statistica");
                    exit(1);
                }
                if (pid == 0)
                {
                    char bmp_header[54];
                    int f_intrare = open(cale_intrare, O_RDONLY);
                    if (f_intrare == -1)
                    {
                        perror("Eroare deschidere imagine bmp");
                        exit(1);
                    }

                    struct stat st;
                    if (fstat(f_intrare, &st) == -1)
                    {
                        perror("Eroare la obtinere status imagine bmp");
                        close(f_intrare);
                        exit(1);
                    }

                    if (read(f_intrare, bmp_header, sizeof(bmp_header)) == -1)
                    {
                        perror("Eroare la citire header bmp");
                        close(f_intrare);
                        exit(1);
                    }

                    int height = abs(readIntFromHeader(bmp_header, 22)); // BMP height offset
                    int width = abs(readIntFromHeader(bmp_header, 18));  // BMP width offset

                    char time_string[15];
                    strftime(time_string, sizeof(time_string), "%d.%m.%Y", localtime(&st.st_mtime));

                    char statistica[500];
                    sprintf(statistica, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                            entry->d_name, height, width, st.st_size, st.st_uid, time_string, st.st_nlink, permisii_user, permisii_grup, permisii_other);

                    writeToFile(fisier_statistica, statistica);

                    exit(10);
                }

                if ((pid = fork()) < 0)
                {
                    perror("creare proces care face imaginea alb-negru");
                    exit(1);
                }
                if (pid == 0)
                {
                    grey_scale_image(cale_intrare);
                    exit(0);
                }
            }
            else
            {
                /* in caz contrar, sunt pe un fisier care NU este bmp */

                int pid2, frate_frate[2], tata_fiu[2];
                pipe(frate_frate);
                pipe(tata_fiu);

                if ((pid = fork()) < 0)
                {
                    perror("creare proces statistica fisier regular");
                    exit(1);
                }

                if (pid == 0)
                {
                    close(frate_frate[0]); // inchid capatul de citire de la frate deoarece eu scriu catre frate
                    close(tata_fiu[0]);    // nu am nevoie de pipe-ul tata-fiu
                    close(tata_fiu[1]);

                    char permissions[10];
                    struct stat st;
                    if (stat(cale_intrare, &st) == -1)
                    {
                        perror("Eroare la obtinere status fisier regular");
                        exit(1);
                    }
                    parsePermissions(st.st_mode, permissions);
                    char timeStr[20];
                    strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", localtime(&st.st_mtime));
                    char statistica[500];

                    sprintf(statistica, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                            entry->d_name, st.st_size, st.st_uid, timeStr, st.st_nlink, permisii_user, permisii_grup, permisii_other);

                    writeToFile(fisier_statistica, statistica);

                    dup2(frate_frate[1], 1);
                    close(frate_frate[1]);

                    execlp("cat", "cat", cale_intrare, (char *)NULL);

                    perror("execlp"); // in cazul in care am avut eroare la comanda executata, afisam eroarea
                }

                pid2 = fork();
                if (pid2 < 0)
                {
                    perror("creare proces apelare script");
                    exit(1);
                }
                if (pid2 == 0)
                {
                    close(frate_frate[1]); // inchid scrierea pentru ca primesc ceva de la frate, deci fac read
                    close(tata_fiu[0]);    // inchid citirea de la tata pentru ca eu (fiul) trimit catre tata (parinte)

                    dup2(frate_frate[0], 0); // redirectarea lui stdin
                    close(frate_frate[0]);   // acum pot inchide complet pipe-ul dintre frati

                    dup2(tata_fiu[1], 1); // redirectarea lui stdout
                    close(tata_fiu[1]);   // acum pot inchide comlpet pipe-ul dintre tata si fiu

                    execlp("bash", "bash", "script.sh", caracter, (char *)NULL); // acum pot apela scriptul pentru verificarea propozitiilor
                }

                close(frate_frate[0]); // procesul parinte nu are legatura cu pipe-ul dintre frati, deci il voi inchide complet
                close(frate_frate[1]);
                close(tata_fiu[1]); // parintele citeste de la fiu, deci inchid capatul de scriere

                waitpid(pid2, NULL, 0); // astept ca procesul al doilea, cel cu scriptul, sa se fi terminat pentru a putea trece mai departe cu codul si a citii numarul de propozitii corecte de la fiul care s-a ocupat de acest lucru

                char buffer[512];
                strcpy(buffer, "");

                read(tata_fiu[0], buffer, 512);              // citim din pipe numarul de propoozitii
                close(tata_fiu[0]);                          // acum pot inchid complet acest pipe
                (*total_propozitii_corecte) += atoi(buffer); // incrementam numarul de propozitii corecte
            }
        }
        else if (entry->d_type == DT_LNK) // verific daca intrarea este o legatura simbolica
        {
            if ((pid = fork()) < 0)
            {
                perror("eroare proces fisier legatura simbolica");
                exit(1);
            }

            if (pid == 0)
            {

                struct stat link_stat;
                if (lstat(cale_intrare, &link_stat) < 0)
                {
                    perror("Lstat error");
                    exit(1);
                }
                struct stat st;
                if (stat(cale_intrare, &st) == -1)
                {
                    perror("Eroare la obtinere status fisier regular");
                    exit(1);
                }

                char statistica[500];
                sprintf(statistica, "nume legatura: %s\nlink size: %ld\nlinked file size: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                        entry->d_name, link_stat.st_size, st.st_size, permisii_user, permisii_grup, permisii_other);

                writeToFile(fisier_statistica, statistica);

                exit(6);
            }
        }
        else if (entry->d_type == DT_DIR) // verific daca intrarea este un director
        {
            if ((pid = fork()) < 0)
            {
                perror("creare proces director");
                exit(1);
            }

            if (pid == 0)
            {
                char permissions[10];
                struct stat st;
                if (stat(cale_intrare, &st) == -1)
                {
                    perror("Eroare la obtinere status fisier regular");
                    exit(1);
                }
                parsePermissions(st.st_mode, permissions);
                char statistica[500];
                sprintf(statistica, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n",

                        entry->d_name, st.st_uid, permisii_user, permisii_grup, permisii_other);

                writeToFile(fisier_statistica, statistica);

                exit(5);
            }
            process_directory(cale_intrare, cale_iesire_director, caracter, total_propozitii_corecte);
        }



        close_file(fisier_statistica); // nu mai scriu nimic in fisierul de statistica, deci il pot inchide
    }

    if (closedir(dir) < 0) // am terminat de parcurs directorul, deci il pot inchide
    {
        perror("Eroare la inchidere director");
        exit(1);
    }
}

void close_processess()
{
    int pid, wait_status;

    while ((pid = wait(&wait_status)) > 0)
    {
        if (WIFEXITED(wait_status))
        {
            printf("Procesul fiu cu PID-ul: %d s-a terminat cu statusul: %d\n", pid, WEXITSTATUS(wait_status));
        }
    }
}

int main(int argc, char *argv[])
{
    // verificam argumentele din linia de comanda date programului nostru
    if (argc != 4)
    {
        printf("Utilizare: %s <director_intrare> <director_iesire> <character>\n", argv[0]);
        exit(1);
    }
    if (strlen(argv[3]) != 1)
    {
        printf("Al treilea argument nu este un caracter.\n");
        exit(1);
    }

    // verifica daca caracterul dat este alfanumeric
    if (isalnum(argv[3][0]) == 0)
    {
        printf("Caracterul dat nu este alfanumeric.\n");
        exit(1);
    }

    int status;
    char permissions[10];
    struct stat file_stat;

    // luam statusul de la primul argument
    status = stat(argv[1], &file_stat);
    if (status < 0)
    {
        perror("Eroare cand iau status de la primul argument");
        exit(1);
    }

    // verifica daca primul argument este director
    if (!S_ISDIR(file_stat.st_mode))
    {
        printf("Primul argument nu este un director.\n");
        exit(1);
    }

    // verific drepturile directorului
    parsePermissions(file_stat.st_mode, permissions);
    printf("%s\n", permissions);

    // luam statusul pentru al doilea argument dat
    status = stat(argv[2], &file_stat);
    if (status < 0)
    {
        perror("Eroare cand iau status de la al doilea argument");
        exit(1);
    }

    // verific daca al doilea argument este un director
    if (!S_ISDIR(file_stat.st_mode))
    {
        printf("Al doilea argument dat nu este un director.\n");
        exit(1);
    }

    // verific drepturile directorului
    parsePermissions(file_stat.st_mode, permissions);
    printf("%s\n", permissions);

    int total_propozitii_corecte = 0;

    process_directory(argv[1], argv[2], argv[3], &total_propozitii_corecte);

    close_processess();

    printf("Au fost identificate in total %d propozitii corecte care contin caracterul %s.\n", total_propozitii_corecte, argv[3]);

    return 0;
}
