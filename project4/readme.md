The goal of this project is to design and implement a simple file system called cpmFS (i.e., CP/M file sytem). Through the late 1970s and into the mid-1980s, CP/M (Control Program for Microcomputers) – a disk-based operating system – had dominated its era as much as MS-DOS and later Windows dominated the IBM PC world [1]. CP/M is clearly not the last word in advanced file systems, but it is simple, fast, and can be implemented by a competent programmer in less than a week [2].

Your simple file system allows users to list directory entries, rename files, copy files, delete files, as well as code to read/write/open/close files. We will use a version of the CP/M file system used on 5.25” and 8” floppy disks in the 1970’s (support for CP/M file systems is still included in Linux to this day). You will develop your code in C. You may use any computer with an ANSI C compiler (e,g. gcc, clang, etc.). You will not be modifying the linux kernel but developing a stand-alone program (i.e., a simulated file-system).  