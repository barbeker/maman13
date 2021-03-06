/* Maman_13
 * Student : Bar Beker
 * ID      : 301518874
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fat12.h"

int fid; /* global variable set by the open() function */

int fd_read(int sector_number, char *buffer){
    int dest, len;
    int bps=DEFAULT_SECTOR_SIZE; //bytes per sector
    dest = lseek(fid, sector_number * DEFAULT_SECTOR_SIZE, SEEK_SET);
    if (dest != sector_number * bps){
        /* Error handling */
        printf("\nERROR: in fd_read\n");
        exit(1);
    }
    len = read(fid, buffer, bps);
    if (len != bps){
        /* error handling here */
        printf("\nERROR:  in fd_read\n");
        exit(1);
    }
    return len;
}

int fd_write(int sector_number, char *buffer){
    int dest, len;
    int bps=DEFAULT_SECTOR_SIZE; //bytes per sector
    dest = lseek(fid, sector_number * DEFAULT_SECTOR_SIZE, SEEK_SET);
    if (dest != sector_number * bps){
        /* Error handling */
        printf("\nERROR: in fd_writen");
        exit(1);
    }
    len = write(fid, buffer, bps);
    if (len != bps){
        /* error handling here */
        printf("\nERROR in fd_writen");
        exit(1);
    }
    return len;
}

#define SECTORS_PER_FAT 9
#define SECTORS_COUNT   2880
#define NUMBER_OF_FATS  2

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Usage: %s <floppy_image>\n", argv[0]);
        exit(1);
    }

    if ( (fid = open (argv[1], O_RDWR|O_CREAT, 0644))  < 0 )
    {
        perror("Error: ");
        exit(1);
    }


    /* See fat12.pdf for layout details */

    // Step 1. Create floppy.img with the school solution. Read the values
    // from the boot sector of the floppy.img and initialize boot sector
    // with those va#define DEFAULT_SECTOR_SIZE 512lues. If you read bootsector of the floppy.img correctly,
    // the values will be:

    //	sector_size: 512 bytes
    //	sectors_per_cluster: 1 sectors
    //	reserved_sector_count: 1
    //	number_of_fats: 2
    //	number_of_dirents: 224
    //	sector_count: 2880 sectors
    //	media_type: 0xf0
    //	fat_size_sectors: 9
    //	sectors_per_track: 18
    //	nheads: 2
    //	sectors_hidden: 0
    //	sector_count_large: 0 sectors

    boot_record_t boot;
    /*
    if (read(fid, &boot, sizeof (boot)) > 0){
        printf("sector_size: %d\n", boot.sector_size);
        printf("sectors_per_cluster: %d\n", boot.sectors_per_cluster);
        printf("reserved_sector_count: %d\n", boot.reserved_sector_count);
        printf("number_of_fats: %d\n", boot.number_of_fats);
        printf("number_of_dirents: %d\n", boot.number_of_dirents);
        printf("sector_count: %d\n", boot.sector_count);
        printf("media_type: %d\n", boot.media_type);
        printf("fat_size_sectors: %d\n", boot.fat_size_sectors);
        printf("sectors_per_track: %d\n", boot.sectors_per_track);
        printf("nheads: %d\n", boot.nheads);
        printf("sectors_hidden: %d\n", boot.sectors_hidden);
        printf("sector_count_large: %d\n", boot.sector_count_large);

        printf("oem_id: ");
        for(int i = 0; i < 8; i++) {
           printf("%d", boot.oem_id[i]);
        }
        printf("\n");
    }
    */

    // Step 1. initialize boot sector
    for(int i=0;i < 8; i++) {
        boot.oem_id[i] = 0;
    }

    boot.sector_size=DEFAULT_SECTOR_SIZE;
    boot.sectors_per_cluster=1;
    boot.reserved_sector_count=1;
    boot.number_of_fats=NUMBER_OF_FATS;
    boot.number_of_dirents=224;
    boot.sector_count=SECTORS_COUNT;
    boot.media_type=0xf0;
    boot.fat_size_sectors=SECTORS_PER_FAT;
    boot.sectors_per_track=18;
    boot.nheads=2;
    boot.sectors_hidden=0;
    boot.sector_count_large=0;

    // Set boot sector
    char buf[DEFAULT_SECTOR_SIZE]={0};
    memcpy(buf, &boot, sizeof(boot));
    fd_write(0,buf);

    // Step 2. Set FAT1/FAT2 table entires to 0x0000 (unused cluster)
    // according to the fat12.pdf.

    char bufReserved[DEFAULT_SECTOR_SIZE]={0};
    bufReserved[0]=0xF0; //first entry is reserved (0xFF0)
    bufReserved[1]=0xFF;
    bufReserved[2]=0xFF; //second and last entry is EOC (0xFFF)
    char bufZero[DEFAULT_SECTOR_SIZE]={0};

    int firstSectorOfFat1 = 1;
    int lastSectorOfFat2  = NUMBER_OF_FATS * SECTORS_PER_FAT;

    //printf("firstSectorOfFat1: %d\n", firstSectorOfFat1);
    //printf("lastSectorOfFat2: %d\n", lastSectorOfFat2);

    for (int sectorIdx = firstSectorOfFat1;
             sectorIdx <= lastSectorOfFat2; sectorIdx++) {
        if ((sectorIdx % SECTORS_PER_FAT) == 1) {
            //printf("sectorIdx: %d\n", sectorIdx);
            fd_write(sectorIdx, bufReserved);  // fat 1/2 first entry in first sector - reserved
        } else {
            fd_write(sectorIdx, bufZero); // zeroing rest of fat 1/2
        }
    }

     // Step 3. Set direntries as free (0xe5) according to the fat12.pdf.
  // While zeroing dentries will also work, we prefer a solution
  // that mark them free. In that case it will be possible to perform
  // unformat operation. However, school solution simply zeros dentries.

    int firstSectorOfDirEntry1   = lastSectorOfFat2 + 1;
    int lastSectorOfLastDirEntry = firstSectorOfDirEntry1 + 13;

    //printf("firstSectorOfDirEntry1: %d\n", firstSectorOfDirEntry1);
    //printf("lastSectorOfLastDirEntry: %d\n", lastSectorOfLastDirEntry);

    for (int sectorIdx = firstSectorOfDirEntry1;
             sectorIdx <= lastSectorOfLastDirEntry; sectorIdx++) {
        fd_write(sectorIdx, bufZero);
    }
  // Step 4. Handle data block (e.g you can zero them or just leave
  // untouched. What are the pros/cons?)

    int firstSectorOfData = lastSectorOfLastDirEntry + 1;

    //printf("firstSectorOfData: %d\n", firstSectorOfData);
    //printf("SECTORS_COUNT: %d\n", SECTORS_COUNT);

    for (int sectorIdx = firstSectorOfData;
             sectorIdx < SECTORS_COUNT; sectorIdx++) {
        fd_write(sectorIdx, bufZero);
    }

// For steps 2-3, you can also read the sectors from the image that was
// generated by the school solution if not sure what should be the values.


    close(fid);
    return 0;
}

