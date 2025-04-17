ENTRY(start)

SECTIONS {
    . = 1M; /* this is empty mem */

    .boot :
    {
        /* ensure that the multiboot header is at the beginning */
        *(.multiboot_header)
    }

    .text :
    {
        *(.text)
    }
}