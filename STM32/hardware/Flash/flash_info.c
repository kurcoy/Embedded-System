
#ifndef BOOTLOADER
__attribute__ ((section(".reserved")))

/* this array is place at the .reserved section at the 0x80E0000 to keep information of SW */
const char VersionInfor[] = "FreeRtos 2017-6-29\n";

#endif/*BOOTLOADER*/
