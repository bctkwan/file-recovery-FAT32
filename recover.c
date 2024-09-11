/*
 * CSCI3150
 * Programming Assignment 2
 * 
 * Kwan Chun Tat
 * 1155033423
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#pragma pack(push, 1)
struct BootEntry {
	uint8_t BS_jmpBoot[3]; /* Assembly instruction to jump to boot code */
	uint8_t BS_OEMName[8]; /* OEM Name in ASCII */
	uint16_t BPB_BytsPerSec; /* Bytes per sector. Allowed values include 512, 1024, 2048, and 4096 */
	uint8_t BPB_SecPerClus; /* Sectors per cluster (data unit). Allowed values are powers of 2, but the cluster size must be 32KB or smaller */
	uint16_t BPB_RsvdSecCnt; /* Size in sectors of the reserved area */
	uint8_t BPB_NumFATs; /* Number of FATs */
	uint16_t BPB_RootEntCnt; /* Maximum number of files in the root directory for FAT12 and FAT16. This is 0 for FAT32 */
	uint16_t BPB_TotSec16; /* 16-bit value of number of sectors in file system */
	uint8_t BPB_Media; /* Media type */
	uint16_t BPB_FATSz16; /* 16-bit size in sectors of each FAT for FAT12 and FAT16. For FAT32, this field is 0 */
	uint16_t BPB_SecPerTrk; /* Sectors per track of storage device */
	uint16_t BPB_NumHeads; /* Number of heads in storage device */
	uint32_t BPB_HiddSec; /* Number of sectors before the start of partition */
	uint32_t BPB_TotSec32; /* 32-bit value of number of sectors in file system. Either this value or the 16-bit value above must be 0 */
	uint32_t BPB_FATSz32; /* 32-bit size in sectors of one FAT */
	uint16_t BPB_ExtFlags; /* A flag for FAT */
	uint16_t BPB_FSVer; /* The major and minor version number */
	uint32_t BPB_RootClus; /* Cluster where the root directory can be found */
	uint16_t BPB_FSInfo; /* Sector where FSINFO structure can be found */
	uint16_t BPB_BkBootSec; /* Sector where backup copy of boot sector is located */
	uint8_t BPB_Reserved[12]; /* Reserved */
	uint8_t BS_DrvNum; /* BIOS INT13h drive number */
	uint8_t BS_Reserved1; /* Not used */
	uint8_t BS_BootSig; /* Extended boot signature to identify if the next three values are valid */
	uint32_t BS_VolID; /* Volume serial number */
	uint8_t BS_VolLab[11]; /* Volume label in ASCII. User defines when creating the file system */
	uint8_t BS_FilSysType[8]; /* File system type label in ASCII */
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DirEntry {
	uint8_t DIR_Name[11]; /* File name */
	uint8_t DIR_Attr; /* File attributes */
	uint8_t DIR_NTRes; /* Reserved */
	uint8_t DIR_CrtTimeTenth; /* Created time (tenths of second) */
	uint16_t DIR_CrtTime; /* Created time (hours, minutes, seconds) */
	uint16_t DIR_CrtDate; /* Created day */
	uint16_t DIR_LstAccDate; /* Accessed day */
	uint16_t DIR_FstClusHI; /* High 2 bytes of the first cluster address */
	uint16_t DIR_WrtTime; /* Written time (hours, minutes, seconds */
	uint16_t DIR_WrtDate; /* Written day */
	uint16_t DIR_FstClusLO; /* Low 2 bytes of the first cluster address */
	uint32_t DIR_FileSize; /* File size in bytes. (0 for directories) */
};
#pragma pack(pop)

int main(int argc, char **argv)
{
	if (argc == 4 && strcmp(argv[1], "-d") == 0 && strcmp(argv[3], "-i") == 0) {
		FILE *fp;
		struct BootEntry boot_entry;
		fp = fopen(argv[2], "r");
		fread(&boot_entry, 1, sizeof(struct BootEntry), fp);
		printf("Number of FATs = %d\n", boot_entry.BPB_NumFATs);
		printf("Number of bytes per sector = %d\n", boot_entry.BPB_BytsPerSec);
		printf("Number of sectors per cluster = %d\n", boot_entry.BPB_SecPerClus);
		printf("Number of reserved sectors = %d\n", boot_entry.BPB_RsvdSecCnt);
		printf("First FAT starts at byte = %d\n", boot_entry.BPB_BytsPerSec * boot_entry.BPB_RsvdSecCnt);
		printf("Data area starts at byte = %d\n", boot_entry.BPB_BytsPerSec * (boot_entry.BPB_RsvdSecCnt + boot_entry.BPB_NumFATs * boot_entry.BPB_FATSz32));
		fclose(fp);
	}
	else if (argc == 4 && strcmp(argv[1], "-d") == 0 && strcmp(argv[3], "-l") == 0) {
		uint32_t i, j, k, currentCluster, nextCluster, orderNum;
		char dirName[14];
		FILE *fp;
		struct BootEntry boot_entry;
		struct DirEntry dir_entry;
		fp = fopen(argv[2], "r");
		fread(&boot_entry, 1, sizeof(struct BootEntry), fp);
		nextCluster = 2;
		orderNum = 1;
		do {
			currentCluster = nextCluster;
			fseek(fp, boot_entry.BPB_BytsPerSec * (boot_entry.BPB_RsvdSecCnt + boot_entry.BPB_NumFATs * boot_entry.BPB_FATSz32 + (currentCluster - 2) * boot_entry.BPB_SecPerClus), SEEK_SET);
			for (k = 0; k < boot_entry.BPB_BytsPerSec * boot_entry.BPB_SecPerClus / sizeof(struct DirEntry); k++) {
				fread(&dir_entry, 1, sizeof(struct DirEntry), fp);
				if (dir_entry.DIR_Name[0] == 0x00) break;
				if (dir_entry.DIR_Attr == 0x0F) printf("%d, LFN entry\n", orderNum);
				else {
					if (dir_entry.DIR_Name[0] == 0xE5) dirName[0] = '?';
					else dirName[0] = dir_entry.DIR_Name[0];
					j = 1;
					for (i = 1; i < 8 && dir_entry.DIR_Name[i] != 0x20; i++) {
						dirName[j] = dir_entry.DIR_Name[i];
						j++;
					}
					if (dir_entry.DIR_Name[8] != 0x20) {
						dirName[j] = '.';
						j++;
						for (i = 8; i < 11 && dir_entry.DIR_Name[i] != 0x20; i++) {
							dirName[j] = dir_entry.DIR_Name[i];
							j++;
						}
					}
					if ((dir_entry.DIR_Attr & 0x10) != 0) {
						dirName[j] = '/';
						j++;
					}
					dirName[j] = '\0';
					printf("%d, %s, %d, %d\n", orderNum, dirName, dir_entry.DIR_FileSize, dir_entry.DIR_FstClusHI << 16 | dir_entry.DIR_FstClusLO);
				}
				orderNum++;
			}
			fseek(fp, boot_entry.BPB_BytsPerSec * boot_entry.BPB_RsvdSecCnt + 4 * currentCluster, SEEK_SET);
			fread(&nextCluster, 4, 1, fp);
		} while (nextCluster < 0x0FFFFFF8);
		fclose(fp);
	}
	else if (argc == 7 && strcmp(argv[1], "-d") == 0 && strcmp(argv[3], "-r") == 0 && strcmp(argv[5], "-o") == 0) {
		uint32_t i, j, k, currentCluster, nextCluster;
		int searchStatus;
		char dirName[13];
		FILE *fp, *dest;
		void *data;
		struct BootEntry boot_entry;
		struct DirEntry dir_entry;
		fp = fopen(argv[2], "r");
		fread(&boot_entry, 1, sizeof(struct BootEntry), fp);
		nextCluster = 2;
		searchStatus = 0;
		do {
			currentCluster = nextCluster;
			fseek(fp, boot_entry.BPB_BytsPerSec * (boot_entry.BPB_RsvdSecCnt + boot_entry.BPB_NumFATs * boot_entry.BPB_FATSz32 + (currentCluster - 2) * boot_entry.BPB_SecPerClus), SEEK_SET);
			for (k = 0; k < boot_entry.BPB_BytsPerSec * boot_entry.BPB_SecPerClus / sizeof(struct DirEntry); k++) {
				fread(&dir_entry, 1, sizeof(struct DirEntry), fp);
				if (dir_entry.DIR_Name[0] == 0x00) {
					searchStatus = -1;
					break;
				}
				else if (dir_entry.DIR_Name[0] == 0xE5) {
					j = 0;
					for (i = 0; i < 8 && dir_entry.DIR_Name[i] != 0x20; i++) {
						dirName[j] = dir_entry.DIR_Name[i];
						j++;
					}
					if (dir_entry.DIR_Name[8] != 0x20) {
						dirName[j] = '.';
						j++;
						for (i = 8; i < 11 && dir_entry.DIR_Name[i] != 0x20; i++) {
							dirName[j] = dir_entry.DIR_Name[i];
							j++;
						}
					}
					dirName[j] = '\0';
					if (strcmp(&argv[4][1], &dirName[1]) == 0) {
						searchStatus = 1;
						break;
					}
				}
			}
			if (abs(searchStatus) == 1) break;
			fseek(fp, boot_entry.BPB_BytsPerSec * boot_entry.BPB_RsvdSecCnt + 4 * currentCluster, SEEK_SET);
			fread(&nextCluster, 4, 1, fp);
		} while (nextCluster < 0x0FFFFFF8);
		if (searchStatus == 1 ) {
			fseek(fp, boot_entry.BPB_BytsPerSec * boot_entry.BPB_RsvdSecCnt + 4 * (dir_entry.DIR_FstClusHI << 16 | dir_entry.DIR_FstClusLO), SEEK_SET);
			fread(&nextCluster, 4, 1, fp);
			if (dir_entry.DIR_FileSize == 0) {
				dest = fopen(argv[6], "w");
				if (dest != NULL) {
					printf("[%s]: recovered\n", argv[4]);
					fclose(dest);
				}
				else printf("[%s]: failed to open\n", argv[6]);
			}
			else if (nextCluster == 0) {
				dest = fopen(argv[6], "w");
				if (dest != NULL) {
					currentCluster = dir_entry.DIR_FstClusHI << 16 | dir_entry.DIR_FstClusLO;
					fseek(fp, boot_entry.BPB_BytsPerSec * (boot_entry.BPB_RsvdSecCnt + boot_entry.BPB_NumFATs * boot_entry.BPB_FATSz32 + (currentCluster - 2) * boot_entry.BPB_SecPerClus), SEEK_SET);
					data = malloc(dir_entry.DIR_FileSize);
					fread(data, dir_entry.DIR_FileSize, 1, fp);
					fwrite(data, dir_entry.DIR_FileSize, 1, dest);
					printf("[%s]: recovered\n", argv[4]);
					free(data);
					fclose(dest);
				}
				else printf("[%s]: failed to open\n", argv[6]);
			}
			else printf("[%s]: error - fail to recover\n", argv[4]);
		}
		else printf("[%s]: error - file not found\n", argv[4]);
		fclose(fp);
	}
	else if (argc == 5 && strcmp(argv[1], "-d") == 0 && strcmp(argv[3], "-x") == 0) {
		uint32_t i, j, k, currentCluster, nextCluster;
		int searchStatus;
		char dirName[13];
		FILE *fp;
		uint8_t zero = 0x00;
		struct BootEntry boot_entry;
		struct DirEntry dir_entry;
		fp = fopen(argv[2], "r+");
		fread(&boot_entry, 1, sizeof(struct BootEntry), fp);
		nextCluster = 2;
		searchStatus = 0;
		do {
			currentCluster = nextCluster;
			fseek(fp, boot_entry.BPB_BytsPerSec * (boot_entry.BPB_RsvdSecCnt + boot_entry.BPB_NumFATs * boot_entry.BPB_FATSz32 + (currentCluster - 2) * boot_entry.BPB_SecPerClus), SEEK_SET);
			for (k = 0; k < boot_entry.BPB_BytsPerSec * boot_entry.BPB_SecPerClus / sizeof(struct DirEntry); k++) {
				fread(&dir_entry, 1, sizeof(struct DirEntry), fp);
				if (dir_entry.DIR_Name[0] == 0x00) {
					searchStatus = -1;
					break;
				}
				else if (dir_entry.DIR_Name[0] == 0xE5) {
					j = 0;
					for (i = 0; i < 8 && dir_entry.DIR_Name[i] != 0x20; i++) {
						dirName[j] = dir_entry.DIR_Name[i];
						j++;
					}
					if (dir_entry.DIR_Name[8] != 0x20) {
						dirName[j] = '.';
						j++;
						for (i = 8; i < 11 && dir_entry.DIR_Name[i] != 0x20; i++) {
							dirName[j] = dir_entry.DIR_Name[i];
							j++;
						}
					}
					dirName[j] = '\0';
					if (strcmp(&argv[4][1], &dirName[1]) == 0) {
						searchStatus = 1;
						break;
					}
				}
			}
			if (abs(searchStatus) == 1) break;
			fseek(fp, boot_entry.BPB_BytsPerSec * boot_entry.BPB_RsvdSecCnt + 4 * currentCluster, SEEK_SET);
			fread(&nextCluster, 4, 1, fp);
		} while (nextCluster < 0x0FFFFFF8);
		if (searchStatus == 1 ) {
			fseek(fp, boot_entry.BPB_BytsPerSec * boot_entry.BPB_RsvdSecCnt + 4 * (dir_entry.DIR_FstClusHI << 16 | dir_entry.DIR_FstClusLO), SEEK_SET);
			fread(&nextCluster, 4, 1, fp);
			if (nextCluster == 0 && dir_entry.DIR_FileSize != 0) {
				currentCluster = dir_entry.DIR_FstClusHI << 16 | dir_entry.DIR_FstClusLO;
				fseek(fp, boot_entry.BPB_BytsPerSec * (boot_entry.BPB_RsvdSecCnt + boot_entry.BPB_NumFATs * boot_entry.BPB_FATSz32 + (currentCluster - 2) * boot_entry.BPB_SecPerClus), SEEK_SET);
				for (i = 0; i < dir_entry.DIR_FileSize; i++) fwrite(&zero, 1, 1, fp);
				printf("[%s]: cleansed\n", argv[4]);
			}
			else printf("[%s]: error - fail to cleanse\n", argv[4]);
		}
		else printf("[%s]: error - file not found\n", argv[4]);
		fclose(fp);
	}
	else {
		printf("Usage: %s -d [device filename] [other arguments]\n", argv[0]);
		printf("-i                   Print file system information\n");
		printf("-l                   List the root directory\n");
		printf("-r target -o dest    Recover the target deleted file\n");
		printf("-x target            Cleanse the target deleted file\n");
	}
	return 0;
}

