/*-
 * Copyright (c) 2008, 2009 Yahoo!, Inc.
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
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/usr.sbin/mfiutil/mfi_show.c,v 1.1.2.2.4.1 2010/06/14 02:09:06 kensmith Exp $
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <err.h>
#include <libutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mfiutil.h"

MFI_TABLE(top, show);

static void
format_stripe(char *buf, size_t buflen, uint8_t stripe)
{

	humanize_number(buf, buflen, (1 << stripe) * 512, "", HN_AUTOSCALE,  
	    HN_B | HN_NOSPACE);
}

static int
show_adapter(int ac, char **av)
{
	struct mfi_ctrl_info info;
	char stripe[5];
	int fd, comma;

	if (ac != 1) {
		warnx("show adapter: extra arguments");
		return (EINVAL);
	}

	fd = mfi_open(mfi_unit);
	if (fd < 0) {
		warn("mfi_open");
		return (errno);
	}

	if (mfi_ctrl_get_info(fd, &info, NULL) < 0) {
		warn("Failed to get controller info");
		return (errno);
	}
	printf("mfi%d Adapter:\n", mfi_unit);
	printf("    Product Name: %.80s\n", info.product_name);
	printf("   Serial Number: %.32s\n", info.serial_number);
	if (info.package_version[0] != '\0')
		printf("        Firmware: %s\n", info.package_version);
	printf("     RAID Levels:");
#ifdef DEBUG
	printf(" (%#x)", info.raid_levels);
#endif
	comma = 0;
	if (info.raid_levels & MFI_INFO_RAID_0) {
		printf(" JBOD, RAID0");
		comma = 1;
	}
	if (info.raid_levels & MFI_INFO_RAID_1) {
		printf("%s RAID1", comma ? "," : "");
		comma = 1;
	}
	if (info.raid_levels & MFI_INFO_RAID_5) {
		printf("%s RAID5", comma ? "," : "");
		comma = 1;
	}
	if (info.raid_levels & MFI_INFO_RAID_1E) {
		printf("%s RAID1E", comma ? "," : "");
		comma = 1;
	}
	if (info.raid_levels & MFI_INFO_RAID_6) {
		printf("%s RAID6", comma ? "," : "");
		comma = 1;
	}
	if ((info.raid_levels & (MFI_INFO_RAID_0 | MFI_INFO_RAID_1)) ==
	    (MFI_INFO_RAID_0 | MFI_INFO_RAID_1)) {
		printf("%s RAID10", comma ? "," : "");
		comma = 1;
	}
	if ((info.raid_levels & (MFI_INFO_RAID_0 | MFI_INFO_RAID_5)) ==
	    (MFI_INFO_RAID_0 | MFI_INFO_RAID_5)) {
		printf("%s RAID50", comma ? "," : "");
		comma = 1;
	}
	printf("\n");
	printf("  Battery Backup: ");
	if (info.hw_present & MFI_INFO_HW_BBU)
		printf("present\n");
	else
		printf("not present\n");
	if (info.hw_present & MFI_INFO_HW_NVRAM)
		printf("           NVRAM: %uK\n", info.nvram_size);
	printf("  Onboard Memory: %uM\n", info.memory_size);
	format_stripe(stripe, sizeof(stripe), info.stripe_sz_ops.min);
	printf("  Minimum Stripe: %s\n", stripe);
	format_stripe(stripe, sizeof(stripe), info.stripe_sz_ops.max);
	printf("  Maximum Stripe: %s\n", stripe);

	close(fd);

	return (0);
}
MFI_COMMAND(show, adapter, show_adapter);

static int
show_battery(int ac, char **av)
{
	struct mfi_bbu_capacity_info cap;
	struct mfi_bbu_design_info design;
	uint8_t status;
	int fd;

	if (ac != 1) {
		warnx("show battery: extra arguments");
		return (EINVAL);
	}

	fd = mfi_open(mfi_unit);
	if (fd < 0) {
		warn("mfi_open");
		return (errno);
	}

	if (mfi_dcmd_command(fd, MFI_DCMD_BBU_GET_CAPACITY_INFO, &cap,
	    sizeof(cap), NULL, 0, &status) < 0) {
		if (status == MFI_STAT_NO_HW_PRESENT) {
			printf("mfi%d: No battery present\n", mfi_unit);
			return (0);
		}
		warn("Failed to get capacity info");
		return (errno);
	}

	if (mfi_dcmd_command(fd, MFI_DCMD_BBU_GET_DESIGN_INFO, &design,
	    sizeof(design), NULL, 0, NULL) < 0) {
		warn("Failed to get design info");
		return (errno);
	}

	printf("mfi%d: Battery State:\n", mfi_unit);
	printf(" Manufacture Date: %d/%d/%d\n", design.mfg_date >> 5 & 0x0f,
	    design.mfg_date & 0x1f, design.mfg_date >> 9 & 0xffff);
	printf("    Serial Number: %d\n", design.serial_number);
	printf("     Manufacturer: %s\n", design.mfg_name);
	printf("            Model: %s\n", design.device_name);
	printf("        Chemistry: %s\n", design.device_chemistry);
	printf("  Design Capacity: %d mAh\n", design.design_capacity);
	printf("   Design Voltage: %d mV\n", design.design_voltage);
	printf("   Current Charge: %d%%\n", cap.relative_charge);

	close(fd);

	return (0);
}
MFI_COMMAND(show, battery, show_battery);

static void
print_ld(struct mfi_ld_info *info, int state_len)
{
	struct mfi_ld_params *params = &info->ld_config.params;
	const char *level;
	char size[6], stripe[5];

	humanize_number(size, sizeof(size), info->size * 512,
	    "", HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL);
	format_stripe(stripe, sizeof(stripe),
	    info->ld_config.params.stripe_size);
	level = mfi_raid_level(params->primary_raid_level,
	    params->secondary_raid_level);
	if (state_len > 0)
		printf("(%6s) %-8s %6s %-*s", size, level, stripe, state_len,
		    mfi_ldstate(params->state));
	else
		printf("(%s) %s %s %s", size, level, stripe,
		    mfi_ldstate(params->state));
}

static void
print_pd(struct mfi_pd_info *info, int state_len, int location)
{
	const char *s;
	char buf[6];

	humanize_number(buf, sizeof(buf), info->raw_size * 512, "",
	    HN_AUTOSCALE, HN_B | HN_NOSPACE |HN_DECIMAL);
	printf("(%6s) ", buf);
	if (state_len > 0)
		printf("%-*s", state_len, mfi_pdstate(info->fw_state));
	else
		printf("%s", mfi_pdstate(info->fw_state));
	s = mfi_pd_inq_string(info);
	if (s != NULL)
		printf(" %s", s);
	if (!location)
		return;
	if (info->encl_device_id == 0xffff)
		printf(" slot %d", info->slot_number);
	else if (info->encl_device_id == info->ref.v.device_id)
		printf(" enclosure %d", info->encl_index);
	else
		printf(" enclosure %d, slot %d", info->encl_index,
		    info->slot_number);
}

static int
show_config(int ac, char **av)
{
	struct mfi_config_data *config;
	struct mfi_array *ar;
	struct mfi_ld_config *ld;
	struct mfi_spare *sp;
	struct mfi_ld_info linfo;
	struct mfi_pd_info pinfo;
	uint16_t device_id;
	char *p;
	int fd, i, j;

	if (ac != 1) {
		warnx("show config: extra arguments");
		return (EINVAL);
	}

	fd = mfi_open(mfi_unit);
	if (fd < 0) {
		warn("mfi_open");
		return (errno);
	}

	/* Get the config from the controller. */
	if (mfi_config_read(fd, &config) < 0) {
		warn("Failed to get config");
		return (errno);
	}

	/* Dump out the configuration. */
	printf("mfi%d Configuration: %d arrays, %d volumes, %d spares\n",
	    mfi_unit, config->array_count, config->log_drv_count,
	    config->spares_count);
	p = (char *)config->array;

	for (i = 0; i < config->array_count; i++) {
		ar = (struct mfi_array *)p;
		printf("    array %u of %u drives:\n", ar->array_ref,
		    ar->num_drives);
		for (j = 0; j < ar->num_drives; j++) {
			device_id = ar->pd[j].ref.v.device_id;
			if (device_id == 0xffff)
				printf("        drive MISSING\n");
			else {
				printf("        drive %u ", device_id);
				if (mfi_pd_get_info(fd, device_id, &pinfo,
				    NULL) < 0)
					printf("%s",
					    mfi_pdstate(ar->pd[j].fw_state));
				else
					print_pd(&pinfo, -1, 1);
				printf("\n");
			}
		}
		p += config->array_size;
	}

	for (i = 0; i < config->log_drv_count; i++) {
		ld = (struct mfi_ld_config *)p;
		printf("    volume %s ",
		    mfi_volume_name(fd, ld->properties.ld.v.target_id));
		if (mfi_ld_get_info(fd, ld->properties.ld.v.target_id, &linfo,
		    NULL) < 0) {
			printf("%s %s",
			    mfi_raid_level(ld->params.primary_raid_level,
				ld->params.secondary_raid_level),
			    mfi_ldstate(ld->params.state));
		} else
			print_ld(&linfo, -1);
		if (ld->properties.name[0] != '\0')
			printf(" <%s>", ld->properties.name);
		printf(" spans:\n");
		for (j = 0; j < ld->params.span_depth; j++)
			printf("        array %u\n", ld->span[j].array_ref);
		p += config->log_drv_size;
	}

	for (i = 0; i < config->spares_count; i++) {
		sp = (struct mfi_spare *)p;
		printf("    %s spare %u ",
		    sp->spare_type & MFI_SPARE_DEDICATED ? "dedicated" :
		    "global", sp->ref.v.device_id);
		if (mfi_pd_get_info(fd, sp->ref.v.device_id, &pinfo, NULL) < 0)
			printf("%s", mfi_pdstate(MFI_PD_STATE_HOT_SPARE));
		else
			print_pd(&pinfo, -1, 1);
		if (sp->spare_type & MFI_SPARE_DEDICATED) {
			printf(" backs:\n");
			for (j = 0; j < sp->array_count; j++)
				printf("        array %u\n", sp->array_ref[j]);
		} else
			printf("\n");
		p += config->spares_size;
	}
	close(fd);

	return (0);
}
MFI_COMMAND(show, config, show_config);

static int
show_volumes(int ac, char **av)
{
	struct mfi_ld_list list;
	struct mfi_ld_info info;
	u_int i, len, state_len;
	int fd;

	if (ac != 1) {
		warnx("show volumes: extra arguments");
		return (EINVAL);
	}

	fd = mfi_open(mfi_unit);
	if (fd < 0) {
		warn("mfi_open");
		return (errno);
	}

	/* Get the logical drive list from the controller. */
	if (mfi_ld_get_list(fd, &list, NULL) < 0) {
		warn("Failed to get volume list");
		return (errno);
	}

	/* List the volumes. */
	printf("mfi%d Volumes:\n", mfi_unit);
	state_len = strlen("State");
	for (i = 0; i < list.ld_count; i++) {
		len = strlen(mfi_ldstate(list.ld_list[i].state));
		if (len > state_len)
			state_len = len;
	}
	printf("  Id     Size    Level   Stripe ");
	len = state_len - strlen("State");
	for (i = 0; i < (len + 1) / 2; i++)
		printf(" ");
	printf("State");
	for (i = 0; i < len / 2; i++)
		printf(" ");
	printf("  Cache   Name\n");
	for (i = 0; i < list.ld_count; i++) {
		if (mfi_ld_get_info(fd, list.ld_list[i].ld.v.target_id, &info,
		    NULL) < 0) {
			warn("Failed to get info for volume %d",
			    list.ld_list[i].ld.v.target_id);
			return (errno);
		}
		printf("%6s ",
		    mfi_volume_name(fd, list.ld_list[i].ld.v.target_id));
		print_ld(&info, state_len);
		switch (info.ld_config.properties.current_cache_policy &
		    (MR_LD_CACHE_ALLOW_WRITE_CACHE |
		    MR_LD_CACHE_ALLOW_READ_CACHE)) {
		case 0:
			printf(" Disabled");
			break;
		case MR_LD_CACHE_ALLOW_READ_CACHE:
			printf(" Reads   ");
			break;
		case MR_LD_CACHE_ALLOW_WRITE_CACHE:
			printf(" Writes  ");
			break;
		case MR_LD_CACHE_ALLOW_WRITE_CACHE |
		    MR_LD_CACHE_ALLOW_READ_CACHE:
			printf(" Enabled ");
			break;
		}
		if (info.ld_config.properties.name[0] != '\0')
			printf(" <%s>", info.ld_config.properties.name);
		printf("\n");
	}
	close(fd);

	return (0);
}
MFI_COMMAND(show, volumes, show_volumes);

static int
show_drives(int ac, char **av)
{
	struct mfi_pd_list *list;
	struct mfi_pd_info info;
	u_int i, len, state_len;
	int fd;

	if (ac != 1) {
		warnx("show drives: extra arguments");
		return (EINVAL);
	}

	fd = mfi_open(mfi_unit);
	if (fd < 0) {
		warn("mfi_open");
		return (errno);
	}

	if (mfi_pd_get_list(fd, &list, NULL) < 0) {
		warn("Failed to get drive list");
		return (errno);
	}

	/* Walk the list of drives to determine width of state column. */
	state_len = 0;
	for (i = 0; i < list->count; i++) {
		if (list->addr[i].scsi_dev_type != 0)
			continue;

		if (mfi_pd_get_info(fd, list->addr[i].device_id, &info,
		    NULL) < 0) {
			warn("Failed to fetch info for drive %u",
			    list->addr[i].device_id);
			return (errno);
		}
		len = strlen(mfi_pdstate(info.fw_state));
		if (len > state_len)
			state_len = len;
	}

	/* List the drives. */
	printf("mfi%d Physical Drives:\n", mfi_unit);
	for (i = 0; i < list->count; i++) {

		/* Skip non-hard disks. */
		if (list->addr[i].scsi_dev_type != 0)
			continue;

		/* Fetch details for this drive. */
		if (mfi_pd_get_info(fd, list->addr[i].device_id, &info,
		    NULL) < 0) {
			warn("Failed to fetch info for drive %u",
			    list->addr[i].device_id);
			return (errno);
		}

		print_pd(&info, state_len, 1);
		printf("\n");
	}
	close(fd);

	return (0);
}
MFI_COMMAND(show, drives, show_drives);

int fw_name_width, fw_version_width, fw_date_width, fw_time_width;

static void
scan_firmware(struct mfi_info_component *comp)
{
	int len;

	len = strlen(comp->name);
	if (fw_name_width < len)
		fw_name_width = len;
	len = strlen(comp->version);
	if (fw_version_width < len)
		fw_version_width = len;
	len = strlen(comp->build_date);
	if (fw_date_width < len)
		fw_date_width = len;
	len = strlen(comp->build_time);
	if (fw_time_width < len)
		fw_time_width = len;
}

static void
display_firmware(struct mfi_info_component *comp, const char *tag)
{

	printf("%-*s  %-*s  %-*s  %-*s  %s\n", fw_name_width, comp->name,
	    fw_version_width, comp->version, fw_date_width, comp->build_date,
	    fw_time_width, comp->build_time, tag);
}

static int
show_firmware(int ac, char **av)
{
	struct mfi_ctrl_info info;
	struct mfi_info_component header;
	int fd;
	u_int i;

	if (ac != 1) {
		warnx("show drives: extra arguments");
		return (EINVAL);
	}

	fd = mfi_open(mfi_unit);
	if (fd < 0) {
		warn("mfi_open");
		return (errno);
	}

	if (mfi_ctrl_get_info(fd, &info, NULL) < 0) {
		warn("Failed to get controller info");
		return (errno);
	}

	if (info.package_version[0] != '\0')
		printf("mfi%d Firmware Package Version: %s\n", mfi_unit,
		    info.package_version);
	printf("mfi%d Firmware Images:\n", mfi_unit);
	strcpy(header.name, "Name");
	strcpy(header.version, "Version");
	strcpy(header.build_date, "Date");
	strcpy(header.build_time, "Time");
	scan_firmware(&header);
	if (info.image_component_count > 8)
		info.image_component_count = 8;
	for (i = 0; i < info.image_component_count; i++)
		scan_firmware(&info.image_component[i]);
	if (info.pending_image_component_count > 8)
		info.pending_image_component_count = 8;
	for (i = 0; i < info.pending_image_component_count; i++)
		scan_firmware(&info.pending_image_component[i]);
	display_firmware(&header, "Status");
	for (i = 0; i < info.image_component_count; i++)
		display_firmware(&info.image_component[i], "active");
	for (i = 0; i < info.pending_image_component_count; i++)
		display_firmware(&info.pending_image_component[i], "pending");

	close(fd);

	return (0);
}
MFI_COMMAND(show, firmware, show_firmware);
