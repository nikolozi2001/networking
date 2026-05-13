SECTIONS {
    . = 0x1000;
    .text   : { *(.text) }   :text
    .rodata : { *(.rodata) } :text
    .data   : { *(.data) }   :text
    .bss    : { *(.bss) }    :text
}
PHDRS {
    text  PT_LOAD       FLAGS(5);
    stack PT_GNU_STACK  FLAGS(6);
}
