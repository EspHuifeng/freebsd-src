/*-
 * Copyright (c) 2007, Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/dev/cfi/cfi_var.h,v 1.3.2.1.4.1 2010/06/14 02:09:06 kensmith Exp $
 */

#ifndef _DEV_CFI_VAR_H_
#define	_DEV_CFI_VAR_H_

struct cfi_region {
	u_int		r_blocks;
	u_int		r_blksz;
};

struct cfi_softc {
	device_t	sc_dev;

	struct resource	*sc_res;
	bus_space_handle_t sc_handle;
	bus_space_tag_t	sc_tag;
	int		sc_rid;

	u_int		sc_size;	/* Flash size. */
	u_int		sc_width;	/* Interface width. */
	u_int		sc_regions;	/* Erase regions. */
	struct cfi_region *sc_region;	/* Array of region info. */

	u_int		sc_cmdset;
	u_int		sc_erase_timeout;
	u_int		sc_write_timeout;

	struct cdev	*sc_nod;
	struct proc	*sc_opened;	/* Process that has us opened. */

	u_char		*sc_wrbuf;
	u_int		sc_wrbufsz;
	u_int		sc_wrofs;
	u_int		sc_writing;
};

extern char cfi_driver_name[];
extern devclass_t cfi_devclass;
extern devclass_t cfi_diskclass;

int cfi_probe(device_t);
int cfi_attach(device_t);
int cfi_detach(device_t);

uint32_t cfi_read(struct cfi_softc *, u_int);
uint8_t cfi_read_qry(struct cfi_softc *, u_int);
int cfi_write_block(struct cfi_softc *);
int cfi_block_start(struct cfi_softc *, u_int);
int cfi_block_finish(struct cfi_softc *);

#ifdef CFI_SUPPORT_STRATAFLASH
int	cfi_intel_get_factory_pr(struct cfi_softc *sc, uint64_t *);
int	cfi_intel_get_oem_pr(struct cfi_softc *sc, uint64_t *);
int	cfi_intel_set_oem_pr(struct cfi_softc *sc, uint64_t);
int	cfi_intel_get_plr(struct cfi_softc *sc, uint32_t *);
int	cfi_intel_set_plr(struct cfi_softc *sc);
#endif /* CFI_SUPPORT_STRATAFLASH */
#endif /* _DEV_CFI_VAR_H_ */
