

#ifndef __IFF_H
#define __IFF_H

#define IFF_NOBUFFER 0x0001

#define IFF_LITTLE_ENDIAN	0x01
#define IFF_FULL_CHUNK_SIZE	0x02

struct iff_header {
    UINT8 form[4];		/* FORM */
    UINT8 len[4];		/* File length */
    UINT8 id[4];		/* IFF type identifier */
};

struct iff_info {
    char id[5];
    void (*loader) ();
    struct iff_info *next;
    struct iff_info *prev;
};

void iff_chunk (FILE *);
void iff_register (char *, void ());
void iff_idsize (int);
void iff_setflag (int);
void iff_release (void);
int iff_process (char *, long, FILE *);

#endif /* __IFF_H */
