;=========================================================================
;
; Sarien - A Sierra AGI resource interpreter engine
; Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
;
; $Id$
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; see docs/COPYING for further details.
;
;=========================================================================

DGROUP	group	_DATA,_BSS
	assume	cs:PCCGA_TEXT,ds:DGROUP

_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends

_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends

;=========================================================================
; Data
;=========================================================================

_DATA	segment word public 'DATA'
gfx_pccga	label	word
	dd	pc_init_vidmode
	dd	pc_deinit_vidmode
	dd	pc_put_block
	dd	pc_put_pixels
	dd	pc_timer
	dd	pc_keypress
	dd	pc_get_key

cga_map	label	byte
	db	0, 1, 1, 1, 2, 2, 2, 3
	db	0, 1, 1, 1, 2, 2, 2, 3
_DATA	ends


;=========================================================================
; Code
;=========================================================================

PCCGA_TEXT	segment byte public 'CODE'


;-------------------------------------------------------------------------
; static void pc_timer ()
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

pc_timer	proc	far
	push	bp
	mov	bp,sp
@1@86:
   ;	
   ;	{
   ;		static UINT32 cticks = 0;
   ;	
   ;		while (cticks == clock_ticks);
   ;	
	mov	ax,seg _clock_ticks
	mov	es,ax
	mov	ax,word ptr es:_clock_ticks+2
	mov	dx,word ptr es:_clock_ticks
	cmp	ax,word ptr DGROUP:d@w+44+2
	jne	short @1@142
	cmp	dx,word ptr DGROUP:d@w+44
	je	short @1@86
@1@142:
   ;	
   ;		cticks = clock_ticks;
   ;	
	mov	ax,seg _clock_ticks
	mov	es,ax
	mov	ax,word ptr es:_clock_ticks+2
	mov	dx,word ptr es:_clock_ticks
	mov	word ptr DGROUP:d@w+44+2,ax
	mov	word ptr DGROUP:d@w+44,dx
   ;	
   ;	}
   ;	
	pop	bp
	ret	
pc_timer	endp


;-------------------------------------------------------------------------
; int init_machine (int argc, char **argv)
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT
_init_machine	proc	far
	push	bp
	mov	bp,sp
@2@86:
   ;	
   ;	{
   ;		gfx = &gfx_pccga;
   ;	
	mov	ax,seg _gfx
	mov	es,ax
	mov	word ptr es:_gfx+2,ds
	mov	word ptr es:_gfx,offset DGROUP:gfx_pccga
   ;	
   ;	
   ;		clock_count = 0;
   ;	
	mov	ax,seg _clock_count
	mov	es,ax
	mov	word ptr es:_clock_count+2,0
	mov	word ptr es:_clock_count,0
   ;	
   ;		clock_ticks = 0;
   ;	
	mov	ax,seg _clock_ticks
	mov	es,ax
	mov	word ptr es:_clock_ticks+2,0
	mov	word ptr es:_clock_ticks,0
   ;	
   ;	
   ;		prev_08 = _dos_getvect (0x08);
   ;	
	mov	ax,8
	push	ax
	call	far ptr __dos_getvect
	pop	cx
	mov	word ptr DGROUP:_prev_08+2,dx
	mov	word ptr DGROUP:_prev_08,ax
   ;	
   ;		_dos_setvect (0x08, tick_increment);
   ;	
	mov	ax,seg _tick_increment
	push	ax
	mov	ax,offset _tick_increment
	push	ax
	mov	ax,8
	push	ax
	call	far ptr __dos_setvect
	add	sp,6
   ;	
   ;		opt.cgaemu = TRUE;
   ;	
	mov	ax,seg _opt
	mov	es,ax
	mov	word ptr es:_opt+18,1
   ;	
   ;	
   ;		return err_OK;
   ;	
	xor	ax,ax
   ;	
   ;	}
   ;	
	pop	bp
	ret	
_init_machine	endp

;-------------------------------------------------------------------------
; int deinit_machine ()
;-------------------------------------------------------------------------

	assume	cs:PCCGA_TEXT
_deinit_machine	proc	far
	push	bp
	mov	bp,sp
@3@86:
   ;	
   ;	{
   ;		_dos_setvect (0x08, prev_08);
   ;	
	push	word ptr DGROUP:_prev_08+2
	push	word ptr DGROUP:_prev_08
	mov	ax,8
	push	ax
	call	far ptr __dos_setvect
	add	sp,6
   ;	
   ;	
   ;		return err_OK;
   ;	
	xor	ax,ax
   ;	
   ;	}
   ;	
	pop	bp
	ret	
_deinit_machine	endp
	
;-------------------------------------------------------------------------
; static int pc_init_vidmode ()
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

pc_init_vidmode	proc	far
	mov ax, 4
	int 10h
	xor ax, ax
	ret	
pc_init_vidmode	endp
	

;-------------------------------------------------------------------------
; static int pc_deinit_vidmode ()
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

pc_deinit_vidmode	proc	far
	mov ax, 3
	int 10h
	xor ax, ax
	ret	
pc_deinit_vidmode	endp
	

;-------------------------------------------------------------------------
;	static void pc_put_block (int x1, int y1, int x2, int y2)
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

screen_buffer	db	16000 dup (?)

pc_put_block	proc	far
	push	bp
	mov	bp,sp
	sub	sp,16
	push	si
	push	di
	mov	si,word ptr [bp+8]
@6@86:
   ;	
   ;	{
   ;		unsigned int i, h, w, p, p2;
   ;		UINT8 far *fbuffer;
   ;		UINT8 *sbuffer;
   ;	
   ;		if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
   ;	
	cmp	word ptr [bp+6],320
	jl	short @6@142
	mov	word ptr [bp+6],319
@6@142:
   ;	
   ;		if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
   ;	
	cmp	si,200
	jl	short @6@198
	mov	si,199
@6@198:
   ;	
   ;		if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
   ;	
	cmp	word ptr [bp+10],320
	jl	short @6@254
	mov	word ptr [bp+10],319
@6@254:
   ;	
   ;		if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;
   ;	
	cmp	word ptr [bp+12],200
	jl	short @6@310
	mov	word ptr [bp+12],199
@6@310:
   ;	
   ;	
   ;		y1 &= ~1;			/* Always start at an even line */
   ;	
	and	si,65534
   ;	
   ;	
   ;		h = y2 - y1 + 1;
   ;	
	mov	ax,word ptr [bp+12]
	sub	ax,si
	inc	ax
	mov	word ptr [bp-2],ax
   ;	
   ;		w = (x2 - x1 + 1) / 4 + 1;
   ;	
	mov	ax,word ptr [bp+10]
	sub	ax,word ptr [bp+6]
	inc	ax
	shr	ax, 1
	shr	ax, 1
	inc	ax
	mov	word ptr [bp-4],ax
   ;	
   ;		p = 40 * y1 + x1 / 4;		/* Note: (GFX_WIDTH / 4) * (y1 / 2) */
   ;	
	mov	ax,si
	mov	dx,40
	imul	dx
	push	ax
	mov	ax,word ptr [bp+6]
	shr	ax, 1
	shr	ax, 1
	pop	dx
	add	dx,ax
	mov	word ptr [bp-6],dx
   ;	
   ;		p2 = p + 40 * y1;
   ;	
	mov	ax,si
	mov	dx,40
	imul	dx
	mov	dx,word ptr [bp-6]
	add	dx,ax
	mov	word ptr [bp-8],dx
   ;	
   ;	
   ;		/* Write to the interlaced CGA framebuffer */
   ;	
   ;		fbuffer = (UINT8 far *)0xb8000000 + p;
   ;	
	mov	ax,word ptr [bp-6]
	mov	word ptr [bp-10],47104
	mov	word ptr [bp-12],ax
   ;	
   ;		sbuffer = screen_buffer + p2;
   ;	
	push	cs
	pop	ax
	mov	dx, offset screen_buffer

	add	dx,word ptr [bp-8]
	mov	word ptr [bp-14],ax
	mov	word ptr [bp-16],dx
   ;	
   ;		for (i = 0; i < h; i += 2) {
   ;	
	xor	di,di
	jmp	short @6@394
@6@338:
   ;	
   ;			_fmemcpy (fbuffer, sbuffer, w);
   ;	
	push	word ptr [bp-4]
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	call	far ptr __fmemcpy
	add	sp,10
   ;	
   ;			fbuffer += 80;
   ;	
	add	word ptr [bp-12],80
   ;	
   ;			sbuffer += 160;
   ;	
	add	word ptr [bp-16],160
	add	di,2
@6@394:
	cmp	di,word ptr [bp-2]
	jb	short @6@338
   ;	
   ;		}
   ;	
   ;		fbuffer = (UINT8 far *)0xb8002000 + p;
   ;	
	mov	ax,word ptr [bp-6]
	add	ax,8192
	mov	word ptr [bp-10],47104
	mov	word ptr [bp-12],ax
   ;	
   ;		sbuffer = screen_buffer + p2 + 80;
   ;	
	push	cs
	pop	ax
	mov	dx, offset screen_buffer

	add	dx,word ptr [bp-8]
	add	dx,80
	mov	word ptr [bp-14],ax
	mov	word ptr [bp-16],dx
   ;	
   ;		for (i = 1; i < h; i += 2) {
   ;	
	mov	di,1
	jmp	short @6@506
@6@450:
   ;	
   ;			_fmemcpy (fbuffer, sbuffer, w);
   ;	
	push	word ptr [bp-4]
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	call	far ptr __fmemcpy
	add	sp,10
   ;	
   ;			fbuffer += 80;
   ;	
	add	word ptr [bp-12],80
   ;	
   ;			sbuffer += 160;
   ;	
	add	word ptr [bp-16],160
	add	di,2
@6@506:
	cmp	di,word ptr [bp-2]
	jb	short @6@450
   ;	
   ;		}
   ;	}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
pc_put_block	endp


;-------------------------------------------------------------------------
; static void pc_put_pixels(int x, int y, int w, UINT8 *p)
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

pc_put_pixels	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8

	push	si
	push	di

	mov di, offset screen_buffer
@7@86:
   ;	
   ;	{
   ;		UINT8 *s, mask, val, shift, c;
   ;	
   ;	 	for (s = &screen_buffer[80 * y + x / 4]; w; w--, x++, p++) {
   ;	
	mov	ax, word ptr [bp+8]
	mov	dx, 80
	imul	dx				; y * 80

	mov	dx, word ptr [bp+6]
	shr	dx, 1
	shr	dx, 1				; x / 4

	add	dx, ax				; 80 * y + x / 4
	add	di, dx

	jmp	short condition
@7@114:
   ;	
   ;			shift = (x & 3) * 2;
   ;	
	mov	cl,byte ptr [bp+6]
	and	cl,3
	shl	cl,1				; cl = shift
   ;	
   ;	
   ;			/* Sorry, no transparent colors */
   ;			c = *p > 15 ? 0 : cga_map[*p];
   ;	
	les	bx,dword ptr [bp+12]
	mov	al,byte ptr es:[bx]

	cmp	al,15
	jbe	short remap
	xor	al, al				; al = color
	jmp	short color_ok
remap:
	mov	bx, offset cga_map
	xlat					; al = color

color_ok:
   ;	
   ;	
   ;			mask = 0xc0 >> shift;
   ;	
	mov	dx,192
	sar	dx,cl				; dx = mask
   ;	
   ;			val = (c & 0x03) << (6 - shift);
   ;	
	and	al, 3
	mov	bl, cl
	mov	cl, 6
	sub	cl, bl				;
	shl	al, cl				; al = val
   ;	
   ;			*s = (*s & ~mask) | val;
   ;	
	mov	ch, byte ptr cs:[di]
	not	dl
	and	ch, dl				; dl = !mask
	or	al, ch
	mov	byte ptr cs:[di], al
   ;	
   ;			
   ;			if ((x % 4) == 3)
   ;	
	mov	ax,word ptr [bp+6]
	and	al, 3
	cmp	al,3
	jne	short @7@254
   ;	
   ;				s++;
   ;	
	;inc	word ptr [bp-4]
	inc	di
@7@254:
	dec	word ptr [bp+10]		; w--
	inc	word ptr [bp+6]			; x++
	inc	word ptr [bp+12]		; p++

condition:
	mov	ax, word ptr[bp+10]
	or	ax,ax
	jne	short @7@114
   ;	
   ;		}
   ;	}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
pc_put_pixels	endp


;-------------------------------------------------------------------------
; static int pc_keypress ()
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

pc_keypress	proc	far
   ;	{
   ;		return !!kbhit();
   ;	
	call	far ptr _kbhit
   ;	
   ;	}
   ;	
	ret	
pc_keypress	endp
	

;-------------------------------------------------------------------------
; static int pc_get_key ()
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

pc_get_key	proc	far
@9@86:
   ;	{
   ;		union REGS r;
   ;		UINT16 key;
   ;	
   ;		memset (&r, 0, sizeof(union REGS));
   ;		int86 (0x16, &r, &r);
   ;		key = r.h.al ? r.h.al : r.h.ah << 8;
   ;	
   ;		return key;
   ;	}

	xor	ax, ax
	int	16h
	or	al, al
	jz	high
	xor	ah, ah
high:
	ret	
pc_get_key	endp

	
;-------------------------------------------------------------------------
; void __interrupt __far tick_increment (void)
;-------------------------------------------------------------------------
	assume	cs:PCCGA_TEXT

_tick_increment	proc	far
	push	ax
	push	bx
	push	cx
	push	dx
	push	es
	push	ds
	push	si
	push	di
	push	bp

	mov	bp,DGROUP
	mov	ds,bp
	mov	bp,sp
   ;	
   ;	{
   ;		clock_ticks++;
   ;	
	mov	ax,seg _clock_ticks
	mov	es,ax
	add	word ptr es:_clock_ticks,1
	adc	word ptr es:_clock_ticks+2,0
   ;	
   ;		_chain_intr(prev_08);
   ;	
	push	word ptr DGROUP:_prev_08+2
	push	word ptr DGROUP:_prev_08
	call	far ptr __chain_intr
	pop	cx
	pop	cx
   ;	
   ;	}
   ;	
	pop	bp
	pop	di
	pop	si
	pop	ds
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret	
_tick_increment	endp

PCCGA_TEXT	ends


;=========================================================================
; BSS
;=========================================================================

_BSS	segment word public 'BSS'
_prev_08	label	dword
	db	4 dup (?)
_exec_name	label	dword
	db	4 dup (?)
	?debug	C E9
_BSS	ends



	extrn	_kbhit:far

_cga_map	equ	cga_map
_gfx_pccga	equ	gfx_pccga
_pc_keypress	equ	pc_keypress
_pc_get_key	equ	pc_get_key
_pc_timer	equ	pc_timer
_pc_put_pixels	equ	pc_put_pixels
_pc_put_block	equ	pc_put_block
_pc_deinit_vidmode	equ	pc_deinit_vidmode
_pc_init_vidmode	equ	pc_init_vidmode

	public	_tick_increment
	public	_prev_08
	extrn	_gfx:dword
	extrn	_clock_count:word
	extrn	_clock_ticks:word
	public	_exec_name
	extrn	_opt:word
	public	_deinit_machine
	public	_init_machine
	extrn	__dos_setvect:far
	extrn	__dos_getvect:far
	extrn	__chain_intr:far
	extrn	_int86:far
	extrn	__stklen:word
	extrn	__fmemcpy:far
	extrn	_memset:far
	extrn	_free:far
	extrn	_calloc:far

	end

