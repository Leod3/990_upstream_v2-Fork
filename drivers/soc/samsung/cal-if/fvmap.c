#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <soc/samsung/cal-if.h>

#include "fvmap.h"
#include "cmucal.h"
#include "vclk.h"
#include "ra.h"

#define FVMAP_SIZE		(SZ_8K)
#define STEP_UV			(6250)

void __iomem *fvmap_base;
void __iomem *sram_fvmap_base;

static int init_margin_table[MAX_MARGIN_ID];
static int volt_offset_percent = 0;
static int percent_margin_table[MAX_MARGIN_ID];

static int __init get_mif_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MIF] = volt;

	return 0;
}
early_param("mif", get_mif_volt);

static int __init get_int_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_INT] = volt;

	return 0;
}
early_param("int", get_int_volt);

static int __init get_big_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_BIG] = volt;

	return 0;
}
early_param("big", get_big_volt);

static int __init get_mid_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MID] = volt;

	return 0;
}
early_param("mid", get_mid_volt);

static int __init get_lit_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_LIT] = volt;

	return 0;
}
early_param("lit", get_lit_volt);

static int __init get_g3d_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_G3D] = volt;

	return 0;
}
early_param("g3d", get_g3d_volt);

static int __init get_intcam_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_INTCAM] = volt;

	return 0;
}
early_param("intcam", get_intcam_volt);

static int __init get_cam_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_CAM] = volt;

	return 0;
}
early_param("cam", get_cam_volt);

static int __init get_disp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DISP] = volt;

	return 0;
}
early_param("disp", get_disp_volt);

static int __init get_g3dm_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_G3DM] = volt;

	return 0;
}
early_param("g3dm", get_g3dm_volt);

static int __init get_cp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_CP] = volt;

	return 0;
}
early_param("cp", get_cp_volt);

static int __init get_fsys0_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_FSYS0] = volt;

	return 0;
}
early_param("fsys0", get_fsys0_volt);

static int __init get_aud_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_AUD] = volt;

	return 0;
}
early_param("aud", get_aud_volt);

static int __init get_iva_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_IVA] = volt;

	return 0;
}
early_param("iva", get_iva_volt);

static int __init get_score_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_SCORE] = volt;

	return 0;
}
early_param("score", get_score_volt);

static int __init get_npu_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_NPU] = volt;

	return 0;
}
early_param("npu", get_npu_volt);

static int __init get_mfc_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_MFC] = volt;

	return 0;
}
early_param("mfc", get_mfc_volt);

static int __init get_dsp_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DSP] = volt;

	return 0;
}
early_param("dsp", get_dsp_volt);

static int __init get_dnc_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_DNC] = volt;

	return 0;
}
early_param("dnc", get_dnc_volt);

static int __init get_tnr_volt(char *str)
{
	int volt;

	get_option(&str, &volt);
	init_margin_table[MARGIN_TNR] = volt;

	return 0;
}
early_param("tnr", get_tnr_volt);

static int __init get_percent_margin_volt(char *str)
{
	int percent;

	get_option(&str, &percent);
	volt_offset_percent = percent;

	return 0;
}
early_param("volt_offset_percent", get_percent_margin_volt);

int fvmap_set_raw_voltage_table(unsigned int id, int uV)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int num_of_lv;
	int idx, i;

	idx = GET_IDX(id);

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		fv_table->table[i].volt += uV;

	return 0;
}

int fvmap_get_voltage_table(unsigned int id, unsigned int *table)
{
	struct fvmap_header *fvmap_header = fvmap_base;
	struct rate_volt_header *fv_table;
	int idx, i;
	int num_of_lv;

	if (!IS_ACPM_VCLK(id))
		return 0;

	idx = GET_IDX(id);

	fvmap_header = fvmap_base;
	fv_table = fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		table[i] = fv_table->table[i].volt;

	return num_of_lv;

}

int fvmap_get_raw_voltage_table(unsigned int id)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int idx, i;
	int num_of_lv;
	unsigned int table[20];

	idx = GET_IDX(id);

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[idx].o_ratevolt;
	num_of_lv = fvmap_header[idx].num_of_lv;

	for (i = 0; i < num_of_lv; i++)
		table[i] = fv_table->table[i].volt;

	for (i = 0; i < num_of_lv; i++)
		printk("dvfs id : %d  %d Khz : %d uv\n", ACPM_VCLK_TYPE | id, fv_table->table[i].rate, table[i]);

	return 0;
}

static void check_percent_margin(struct rate_volt_header *head, unsigned int num_of_lv)
{
	int org_volt;
	int percent_volt;
	int i;

	if (!volt_offset_percent)
		return;

	for (i = 0; i < num_of_lv; i++) {
		org_volt = head->table[i].volt;
		percent_volt = org_volt * volt_offset_percent / 100;
		head->table[i].volt = org_volt + rounddown(percent_volt, STEP_UV);
	}
}

static int get_vclk_id_from_margin_id(int margin_id)
{
	int size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	int i;
	struct vclk *vclk;

	for (i = 0; i < size; i++) {
		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);

		if (vclk->margin_id == margin_id)
			return i;
	}

	return -EINVAL;
}

#define attr_percent(margin_id, type)								\
static ssize_t show_##type##_percent								\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return snprintf(buf, PAGE_SIZE, "%d\n", percent_margin_table[margin_id]);		\
}												\
												\
static ssize_t store_##type##_percent								\
(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)		\
{												\
	int input, vclk_id;									\
												\
	if (!sscanf(buf, "%d", &input))								\
		return -EINVAL;									\
												\
	if (input < -100 || input > 100)							\
		return -EINVAL;									\
												\
	vclk_id = get_vclk_id_from_margin_id(margin_id);					\
	if (vclk_id == -EINVAL)									\
		return vclk_id;									\
	percent_margin_table[margin_id] = input;						\
	cal_dfs_set_volt_margin(vclk_id | ACPM_VCLK_TYPE, input);				\
												\
	return count;										\
}												\
												\
static struct kobj_attribute type##_percent =							\
__ATTR(type##_percent, 0600,									\
	show_##type##_percent, store_##type##_percent)

attr_percent(MARGIN_MIF, mif_margin);
attr_percent(MARGIN_INT, int_margin);
attr_percent(MARGIN_BIG, big_margin);
attr_percent(MARGIN_MID, mid_margin);
attr_percent(MARGIN_LIT, lit_margin);
attr_percent(MARGIN_G3D, g3d_margin);
attr_percent(MARGIN_INTCAM, intcam_margin);
attr_percent(MARGIN_CAM, cam_margin);
attr_percent(MARGIN_DISP, disp_margin);
attr_percent(MARGIN_CP, cp_margin);
attr_percent(MARGIN_FSYS0, fsys0_margin);
attr_percent(MARGIN_AUD, aud_margin);
attr_percent(MARGIN_IVA, iva_margin);
attr_percent(MARGIN_SCORE, score_margin);
attr_percent(MARGIN_NPU, npu_margin);
attr_percent(MARGIN_MFC, mfc_margin);

static struct attribute *percent_margin_attrs[] = {
	&mif_margin_percent.attr,
	&int_margin_percent.attr,
	&big_margin_percent.attr,
	&mid_margin_percent.attr,
	&lit_margin_percent.attr,
	&g3d_margin_percent.attr,
	&intcam_margin_percent.attr,
	&cam_margin_percent.attr,
	&disp_margin_percent.attr,
	&cp_margin_percent.attr,
	&fsys0_margin_percent.attr,
	&aud_margin_percent.attr,
	&iva_margin_percent.attr,
	&score_margin_percent.attr,
	&npu_margin_percent.attr,
	&mfc_margin_percent.attr,
	NULL,
};

static const struct attribute_group percent_margin_group = {
	.attrs = percent_margin_attrs,
};

#ifdef CONFIG_SEC_FACTORY
#ifdef CONFIG_SOC_EXYNOS9830
#include <soc/samsung/ect_parser.h>
#include <soc/samsung/cal-if.h>
#include "asv.h"

#define VUP_0 12500
#define VUP_1 25000

enum spec_volt_type {
	ASV_G_FUSED = 0,
	ASV_G_GRP
};

static int asv_g_yield_volt[4];

static int __init get_mvup(char *str)
{
	asv_g_yield_volt[1] = VUP_0;

	return 0;
}
early_param("mvup", get_mvup);

static int __init get_bvup(char *str)
{
	asv_g_yield_volt[2] = VUP_1;

	return 0;
}
early_param("bvup", get_bvup);

static int __init get_gvup(char *str)
{
	asv_g_yield_volt[3] = VUP_1;

	return 0;
}
early_param("gvup", get_gvup);

static ssize_t show_asv_g_spec(int id, enum spec_volt_type type, char *buf)
{
	void *gen_block;
	struct ect_gen_param_table *spec;
	int asv_tbl_ver, asv_grp, tbl_size, j, vtyp_freq, num_lv;
	unsigned int fused_volt, grp_volt = 0, volt;
	struct dvfs_rate_volt rate_volt[48];
	unsigned int (*spec_table)[10];
	char *spec_table_name[4] = { "SPEC_CPUCL0", "SPEC_CPUCL1", "SPEC_CPUCL2", "SPEC_G3D" };
	int cal_id[4] = { ACPM_VCLK_TYPE | 2, ACPM_VCLK_TYPE | 3, ACPM_VCLK_TYPE | 4, ACPM_VCLK_TYPE | 10 };
	ssize_t size = 0;

	asv_tbl_ver = asv_get_table_ver();
	if (asv_tbl_ver < 3) {
		pr_err("%s: Do not support ASV-G under ASV table version 3\n", __func__);
		goto out;
	}

	gen_block = ect_get_block("GEN");
	if (gen_block == NULL) {
		pr_err("%s: Failed to get gen block from ECT\n", __func__);
		goto out;
	}

	asv_grp = asv_get_grp(cal_id[id]);
	if (!asv_grp) {
		pr_err("%s: There has no ASV-G information for %s group 0\n", __func__, spec_table_name[id]);
		goto out;
	}

	spec = ect_gen_param_get_table(gen_block, spec_table_name[id]);
	if (spec == NULL) {
		pr_err("%s: Failed to get spec table from ECT\n", __func__);
		goto out;
	}
	spec_table = (unsigned int (*)[10])spec->parameter;

	for (j = 0; j < spec->num_of_row; j++) {
		if (spec_table[j][0] == asv_tbl_ver) {
			grp_volt = spec_table[j][asv_grp + 1];
			vtyp_freq = spec_table[j][1];
			break;
		}
	}

	if (!grp_volt) {
		pr_err("%s: Failed to get grp volt\n", __func__);
		goto out;
	}

	num_lv = cal_dfs_get_lv_num(cal_id[id]);
	tbl_size = cal_dfs_get_rate_asv_table(cal_id[id], rate_volt);
	if (!tbl_size) {
		pr_err("%s: Failed to get asv table\n", __func__);
		goto out;
	}

	for (j = 0; j < num_lv; j++){
		if (rate_volt[j].rate == vtyp_freq) {
			fused_volt = rate_volt[j].volt;
			break;
		}
	}
	if (j == num_lv) {
		pr_err("%s: There has no frequency %d on %d domain\n", __func__, vtyp_freq, spec_table_name[id]);
		goto out;
	}

	fused_volt -= asv_g_yield_volt[id];

	volt = (type == ASV_G_FUSED) ? fused_volt : grp_volt;

	size += snprintf(buf + size, PAGE_SIZE, "%d\n", volt);
out:

	return size;
}

#define asv_g_spec(domain, id)									\
static ssize_t show_asv_g_spec_##domain##_fused_volt						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return show_asv_g_spec(id, ASV_G_FUSED, buf);						\
}												\
static struct kobj_attribute asv_g_spec_##domain##_fused_volt =					\
__ATTR(domain##_fused_volt, 0400, show_asv_g_spec_##domain##_fused_volt, NULL);	\
static ssize_t show_asv_g_spec_##domain##_grp_volt						\
(struct kobject *kobj, struct kobj_attribute *attr, char *buf)					\
{												\
	return show_asv_g_spec(id, ASV_G_GRP, buf);						\
}												\
static struct kobj_attribute asv_g_spec_##domain##_grp_volt =					\
__ATTR(domain##_grp_volt, 0400, show_asv_g_spec_##domain##_grp_volt, NULL)

#define asv_g_spec_attr(domain)									\
	&asv_g_spec_##domain##_fused_volt.attr,							\
	&asv_g_spec_##domain##_grp_volt.attr

asv_g_spec(cpucl0, 0);
asv_g_spec(cpucl1, 1);
asv_g_spec(cpucl2, 2);
asv_g_spec(g3d, 3);

static struct attribute *asv_g_spec_attrs[] = {
	asv_g_spec_attr(cpucl0),
	asv_g_spec_attr(cpucl1),
	asv_g_spec_attr(cpucl2),
	asv_g_spec_attr(g3d),
	NULL,
};

static const struct attribute_group asv_g_spec_grp = {
	.attrs = asv_g_spec_attrs,
};
#endif
#endif /* CONFIG_SEC_FACTORY */

/* SRAM sysfs interface */
static ssize_t sram_raw_show(struct kobject *kobj, struct kobj_attribute *attr,
			     char *buf)
{
	if (!sram_fvmap_base)
		return snprintf(buf, PAGE_SIZE, "SRAM not mapped\n");

	/* Copy SRAM content to buffer (limited to PAGE_SIZE) */
	memcpy_fromio(buf, sram_fvmap_base, min_t(size_t, FVMAP_SIZE, PAGE_SIZE));

	return min_t(size_t, FVMAP_SIZE, PAGE_SIZE);
}

static ssize_t sram_raw_store(struct kobject *kobj, struct kobj_attribute *attr,
			      const char *buf, size_t count)
{
	if (!sram_fvmap_base) {
		pr_err("SRAM not mapped\n");
		return -ENOMEM;
	}

	/* Limit write to SRAM size */
	if (count > FVMAP_SIZE)
		count = FVMAP_SIZE;

	memcpy_toio(sram_fvmap_base, buf, count);
	pr_info("fvmap: %zu bytes written to SRAM\n", count);

	return count;
}

static ssize_t sram_hex_show(struct kobject *kobj, struct kobj_attribute *attr,
			     char *buf)
{
	ssize_t count = 0;
	int i, num_words;

	if (!sram_fvmap_base)
		return snprintf(buf, PAGE_SIZE, "SRAM not mapped\n");

	/* Show first 64 words (256 bytes) in hex format */
	num_words = min_t(int, 64, FVMAP_SIZE / 4);

	count += snprintf(buf + count, PAGE_SIZE - count,
			  "SRAM Dump (first %d words):\n", num_words);
	count += snprintf(buf + count, PAGE_SIZE - count,
			  "Offset    | Hex Value\n");
	count += snprintf(buf + count, PAGE_SIZE - count,
			  "----------+-----------\n");

	for (i = 0; i < num_words && count < PAGE_SIZE - 50; i++) {
		u32 value = ioread32(sram_fvmap_base + (i * 4));
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "0x%04x    | 0x%08x\n", i * 4, value);
	}

	return count;
}

static ssize_t sram_size_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", FVMAP_SIZE);
}

static ssize_t sram_addr_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	return snprintf(buf, PAGE_SIZE, "virtual: %p\n", sram_fvmap_base);
}

static ssize_t sram_offset_show(struct kobject *kobj, struct kobj_attribute *attr,
				char *buf)
{
	return snprintf(buf, PAGE_SIZE,
			"Usage: echo 'offset value' > sram_offset\n"
			"Example: echo '0x100 0x12345678' > sram_offset\n");
}

static ssize_t sram_offset_store(struct kobject *kobj, struct kobj_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int offset, value;
	int ret;

	if (!sram_fvmap_base) {
		pr_err("SRAM not mapped\n");
		return -ENOMEM;
	}

	ret = sscanf(buf, "%x %x", &offset, &value);
	if (ret != 2) {
		pr_err("Invalid format. Use: offset(hex) value(hex)\n");
		return -EINVAL;
	}

	if (offset >= FVMAP_SIZE) {
		pr_err("Offset 0x%x out of range (max: 0x%x)\n", offset, FVMAP_SIZE);
		return -EINVAL;
	}

	iowrite32(value, sram_fvmap_base + offset);
	pr_info("fvmap: Written 0x%08x to offset 0x%04x\n", value, offset);

	return count;
}

static struct kobj_attribute sram_raw_attr =
	__ATTR(sram_raw, 0600, sram_raw_show, sram_raw_store);

static struct kobj_attribute sram_hex_attr =
	__ATTR(sram_hex, 0400, sram_hex_show, NULL);

static struct kobj_attribute sram_size_attr =
	__ATTR(sram_size, 0400, sram_size_show, NULL);

static struct kobj_attribute sram_addr_attr =
	__ATTR(sram_addr, 0400, sram_addr_show, NULL);

static struct kobj_attribute sram_offset_attr =
	__ATTR(sram_offset, 0600, sram_offset_show, sram_offset_store);

/* Undervolt interface */
static ssize_t undervolt_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	ssize_t count = 0;
	int size, i, j;
	struct vclk *vclk;

	if (!sram_fvmap_base)
		return snprintf(buf, PAGE_SIZE, "SRAM not mapped\n");

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	fvmap_header = sram_fvmap_base;

	count += snprintf(buf + count, PAGE_SIZE - count,
			  "Domain ID | Domain Name     | Levels | Current Voltages (uV)\n");
	count += snprintf(buf + count, PAGE_SIZE - count,
			  "----------+-----------------+--------+----------------------\n");

	for (i = 0; i < size && count < PAGE_SIZE - 200; i++) {
		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
		if (!vclk)
			continue;

		fv_table = sram_fvmap_base + fvmap_header[i].o_ratevolt;
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "%-9d | %-15s | %-6d |",
				  i, vclk->name, fvmap_header[i].num_of_lv);

		/* Show first 3 voltage levels */
		for (j = 0; j < min_t(int, 3, fvmap_header[i].num_of_lv); j++) {
			count += snprintf(buf + count, PAGE_SIZE - count,
					  " %d", fv_table->table[j].volt);
		}
		if (fvmap_header[i].num_of_lv > 3)
			count += snprintf(buf + count, PAGE_SIZE - count, " ...");

		count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	}

	count += snprintf(buf + count, PAGE_SIZE - count,
			  "\nUsage: echo 'domain_id offset_uV' > undervolt\n");
	count += snprintf(buf + count, PAGE_SIZE - count,
			  "Example: echo '0 -50000' > undervolt (reduce 50mV on domain 0)\n");

	return count;
}

static ssize_t undervolt_store(struct kobject *kobj, struct kobj_attribute *attr,
			       const char *buf, size_t count)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int domain_id, offset_uv;
	int ret, i, size;

	if (!sram_fvmap_base) {
		pr_err("SRAM not mapped\n");
		return -ENOMEM;
	}

	ret = sscanf(buf, "%d %d", &domain_id, &offset_uv);
	if (ret != 2) {
		pr_err("Invalid format. Use: domain_id offset_uV\n");
		return -EINVAL;
	}

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	if (domain_id < 0 || domain_id >= size) {
		pr_err("Invalid domain_id %d (valid range: 0-%d)\n", domain_id, size - 1);
		return -EINVAL;
	}

	/* Safety check: limit undervolt to -200mV */
	if (offset_uv < -200000) {
		pr_err("Undervolt offset too aggressive: %d uV (min: -200000 uV)\n", offset_uv);
		return -EINVAL;
	}

	/* Safety check: limit overvolt to +100mV */
	if (offset_uv > 100000) {
		pr_err("Overvolt offset too high: %d uV (max: +100000 uV)\n", offset_uv);
		return -EINVAL;
	}

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[domain_id].o_ratevolt;

	/* Apply offset to all voltage levels in the domain */
	for (i = 0; i < fvmap_header[domain_id].num_of_lv; i++) {
		int old_volt = fv_table->table[i].volt;
		int new_volt = old_volt + offset_uv;

		/* Ensure voltage doesn't go below 400mV */
		if (new_volt < 400000) {
			pr_warn("Voltage would be too low (%d uV), clamping to 400000 uV\n", new_volt);
			new_volt = 400000;
		}

		fv_table->table[i].volt = new_volt;
		pr_info("Domain %d Level %d: %d uV -> %d uV (%+d uV)\n",
			domain_id, i, old_volt, new_volt, offset_uv);
	}

	pr_info("Undervolt applied: domain %d, offset %+d uV\n", domain_id, offset_uv);

	return count;
}

static struct kobj_attribute undervolt_attr =
	__ATTR(undervolt, 0600, undervolt_show, undervolt_store);

/* Frequency/Voltage tables display */
static ssize_t fv_tables_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	ssize_t count = 0;
	int size, i, j;
	struct vclk *vclk;

	if (!sram_fvmap_base)
		return snprintf(buf, PAGE_SIZE, "SRAM not mapped\n");

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	fvmap_header = sram_fvmap_base;

	count += snprintf(buf + count, PAGE_SIZE - count,
			  "=== DVFS Frequency/Voltage Tables ===\n\n");

	for (i = 0; i < size && count < PAGE_SIZE - 500; i++) {
		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
		if (!vclk || fvmap_header[i].num_of_lv == 0)
			continue;

		fv_table = sram_fvmap_base + fvmap_header[i].o_ratevolt;

		count += snprintf(buf + count, PAGE_SIZE - count,
				  "Domain %d: %s\n", i, vclk->name);
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "  DVFS Type: %d\n", fvmap_header[i].dvfs_type);
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "  Levels: %d\n", fvmap_header[i].num_of_lv);
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "  %-5s | %-12s | %-10s\n", "Level", "Freq (kHz)", "Volt (uV)");
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "  ------+--------------+-----------\n");

		for (j = 0; j < fvmap_header[i].num_of_lv && count < PAGE_SIZE - 200; j++) {
			count += snprintf(buf + count, PAGE_SIZE - count,
					  "  %-5d | %-12u | %-10u\n",
					  j,
					  fv_table->table[j].rate,
					  fv_table->table[j].volt);
		}

		count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	}

	return count;
}

static struct kobj_attribute fv_tables_attr =
	__ATTR(fv_tables, 0400, fv_tables_show, NULL);

/* Overclock interface */
static ssize_t overclock_show(struct kobject *kobj, struct kobj_attribute *attr,
			      char *buf)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	ssize_t count = 0;
	int size, i;
	struct vclk *vclk;
	unsigned int max_freq, max_volt;

	if (!sram_fvmap_base)
		return snprintf(buf, PAGE_SIZE, "SRAM not mapped\n");

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	fvmap_header = sram_fvmap_base;

	count += snprintf(buf + count, PAGE_SIZE - count,
			  "=== Overclock Configuration ===\n\n");

	for (i = 0; i < size && count < PAGE_SIZE - 300; i++) {
		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
		if (!vclk || fvmap_header[i].num_of_lv == 0)
			continue;

		fv_table = sram_fvmap_base + fvmap_header[i].o_ratevolt;
		max_freq = fv_table->table[0].rate;
		max_volt = fv_table->table[0].volt;

		count += snprintf(buf + count, PAGE_SIZE - count,
				  "Domain %d: %s\n", i, vclk->name);
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "  Current Max: %u kHz @ %u uV\n",
				  max_freq, max_volt);
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "  Levels: %d\n\n", fvmap_header[i].num_of_lv);
	}

	count += snprintf(buf + count, PAGE_SIZE - count,
			  "Usage: echo 'domain_id freq_khz volt_uv' > overclock\n");
	count += snprintf(buf + count, PAGE_SIZE - count,
			  "Example: echo '2 3500000 1000000' > overclock\n");
	count += snprintf(buf + count, PAGE_SIZE - count,
			  "  (Sets BIG cluster to 3.5 GHz @ 1.0V)\n");

	return count;
}

static ssize_t overclock_store(struct kobject *kobj, struct kobj_attribute *attr,
			       const char *buf, size_t count)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int domain_id, new_freq, new_volt;
	int ret, size;

	if (!sram_fvmap_base) {
		pr_err("SRAM not mapped\n");
		return -ENOMEM;
	}

	ret = sscanf(buf, "%d %d %d", &domain_id, &new_freq, &new_volt);
	if (ret != 3) {
		pr_err("Invalid format. Use: domain_id freq_khz volt_uv\n");
		return -EINVAL;
	}

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);
	if (domain_id < 0 || domain_id >= size) {
		pr_err("Invalid domain_id %d (valid range: 0-%d)\n", domain_id, size - 1);
		return -EINVAL;
	}

	/* Safety checks */
	if (new_freq < 100000 || new_freq > 5000000) {
		pr_err("Frequency %d kHz out of safe range (100-5000 MHz)\n", new_freq);
		return -EINVAL;
	}

	if (new_volt < 500000 || new_volt > 1300000) {
		pr_err("Voltage %d uV out of safe range (500-1300 mV)\n", new_volt);
		return -EINVAL;
	}

	fvmap_header = sram_fvmap_base;
	fv_table = sram_fvmap_base + fvmap_header[domain_id].o_ratevolt;

	/* Modify the first (highest) frequency level */
	int old_freq = fv_table->table[0].rate;
	int old_volt = fv_table->table[0].volt;

	fv_table->table[0].rate = new_freq;
	fv_table->table[0].volt = new_volt;

	pr_info("Overclock applied: Domain %d\n", domain_id);
	pr_info("  Frequency: %d kHz -> %d kHz (%+d kHz)\n",
		old_freq, new_freq, new_freq - old_freq);
	pr_info("  Voltage: %d uV -> %d uV (%+d uV)\n",
		old_volt, new_volt, new_volt - old_volt);

	pr_warn("WARNING: Overclocking can damage your hardware!\n");
	pr_warn("Monitor temperatures and stability carefully.\n");

	return count;
}

static struct kobj_attribute overclock_attr =
	__ATTR(overclock, 0600, overclock_show, overclock_store);

static struct attribute *sram_attrs[] = {
	&sram_raw_attr.attr,
	&sram_hex_attr.attr,
	&sram_size_attr.attr,
	&sram_addr_attr.attr,
	&sram_offset_attr.attr,
	&undervolt_attr.attr,
	&fv_tables_attr.attr,
	&overclock_attr.attr,
	NULL,
};

static const struct attribute_group sram_group = {
	.attrs = sram_attrs,
};

/* Permanent overclock configuration - Edit these values */
struct overclock_config {
	int domain_id;
	const char *name;
	unsigned int new_freq_khz;
	unsigned int new_volt_uv;
	int enabled;
};

/* 
 * Configure your permanent overclocks here.
 * Set enabled = 1 to apply, 0 to disable.
 * domain_id: Check with fv_tables to find the correct ID for your device
 */
static struct overclock_config permanent_oc[] = {
	/* Example: GPU from 897 MHz to 933 MHz */
	{ .domain_id = 10, .name = "GPU", .new_freq_khz = 933000, .new_volt_uv = 768000, .enabled = 1 },
	
	/* Example: CPU MID from 2600 MHz to 2803MHz */
	{ .domain_id = 3, .name = "CPUCL1", .new_freq_khz = 2803000, .new_volt_uv = 1075000, .enabled = 1 },
	
	/* Example: CPU LIT from 2106 MHz to 2300 MHz */
	{ .domain_id = 2, .name = "CPUCL0", .new_freq_khz = 2304000, .new_volt_uv = 1087500, .enabled = 1 },
	
	/* Add more entries as needed */
};

static void apply_permanent_overclock(void __iomem *sram_base)
{
	struct fvmap_header *fvmap_header;
	struct rate_volt_header *fv_table;
	int i, num_configs;

	if (!sram_base) {
		pr_err("fvmap: Cannot apply permanent OC - SRAM not mapped\n");
		return;
	}

	fvmap_header = sram_base;
	num_configs = ARRAY_SIZE(permanent_oc);

	pr_info("fvmap: Applying permanent overclock configuration...\n");

	for (i = 0; i < num_configs; i++) {
		if (!permanent_oc[i].enabled)
			continue;

		if (permanent_oc[i].domain_id < 0 || 
		    permanent_oc[i].domain_id >= cmucal_get_list_size(ACPM_VCLK_TYPE)) {
			pr_err("fvmap: Invalid domain_id %d for %s\n",
			       permanent_oc[i].domain_id, permanent_oc[i].name);
			continue;
		}

		fv_table = sram_base + fvmap_header[permanent_oc[i].domain_id].o_ratevolt;

		/* Modify the first (highest) frequency level */
		unsigned int old_freq = fv_table->table[0].rate;
		unsigned int old_volt = fv_table->table[0].volt;

		fv_table->table[0].rate = permanent_oc[i].new_freq_khz;
		fv_table->table[0].volt = permanent_oc[i].new_volt_uv;

		pr_info("fvmap: [%s] Permanent OC applied:\n", permanent_oc[i].name);
		pr_info("  Frequency: %u kHz -> %u kHz (%+d kHz)\n",
			old_freq, permanent_oc[i].new_freq_khz,
			permanent_oc[i].new_freq_khz - old_freq);
		pr_info("  Voltage: %u uV -> %u uV (%+d uV)\n",
			old_volt, permanent_oc[i].new_volt_uv,
			permanent_oc[i].new_volt_uv - old_volt);
	}

	pr_info("fvmap: Permanent overclock configuration complete.\n");
}

static void fvmap_copy_from_sram(void __iomem *map_base, void __iomem *sram_base)
{
	volatile struct fvmap_header *fvmap_header, *header;
	struct rate_volt_header *old, *new;
	struct clocks *clks;
	struct pll_header *plls;
	struct vclk *vclk;
	unsigned int member_addr;
	unsigned int blk_idx;
	int size, margin;
	int i, j;

	fvmap_header = map_base;
	header = sram_base;

	size = cmucal_get_list_size(ACPM_VCLK_TYPE);

	for (i = 0; i < size; i++) {
		/* load fvmap info */
		fvmap_header[i].dvfs_type = header[i].dvfs_type;
		fvmap_header[i].num_of_lv = header[i].num_of_lv;
		fvmap_header[i].num_of_members = header[i].num_of_members;
		fvmap_header[i].num_of_pll = header[i].num_of_pll;
		fvmap_header[i].num_of_mux = header[i].num_of_mux;
		fvmap_header[i].num_of_div = header[i].num_of_div;
		fvmap_header[i].gearratio = header[i].gearratio;
		fvmap_header[i].init_lv = header[i].init_lv;
		fvmap_header[i].num_of_gate = header[i].num_of_gate;
		fvmap_header[i].reserved[0] = header[i].reserved[0];
		fvmap_header[i].reserved[1] = header[i].reserved[1];
		fvmap_header[i].block_addr[0] = header[i].block_addr[0];
		fvmap_header[i].block_addr[1] = header[i].block_addr[1];
		fvmap_header[i].block_addr[2] = header[i].block_addr[2];
		fvmap_header[i].o_members = header[i].o_members;
		fvmap_header[i].o_ratevolt = header[i].o_ratevolt;
		fvmap_header[i].o_tables = header[i].o_tables;

		vclk = cmucal_get_node(ACPM_VCLK_TYPE | i);
		if (vclk == NULL)
			continue;
		pr_info("dvfs_type : %s - id : %x\n",
				vclk->name, fvmap_header[i].dvfs_type);
		pr_info("  num_of_lv      : %d\n", fvmap_header[i].num_of_lv);
		pr_info("  num_of_members : %d\n", fvmap_header[i].num_of_members);

		old = sram_base + fvmap_header[i].o_ratevolt;
		new = map_base + fvmap_header[i].o_ratevolt;

		check_percent_margin(old, fvmap_header[i].num_of_lv);

		margin = init_margin_table[vclk->margin_id];
		if (margin)
			cal_dfs_set_volt_margin(i | ACPM_VCLK_TYPE, margin);

		for (j = 0; j < fvmap_header[i].num_of_members; j++) {
			clks = sram_base + fvmap_header[i].o_members;

			if (j < fvmap_header[i].num_of_pll) {
				plls = sram_base + clks->addr[j];
				member_addr = plls->addr - 0x90000000;
			} else {

				member_addr = (clks->addr[j] & ~0x3) & 0xffff;
				blk_idx = clks->addr[j] & 0x3;

				member_addr |= ((fvmap_header[i].block_addr[blk_idx]) << 16) - 0x90000000;
			}


			vclk->list[j] = cmucal_get_id_by_addr(member_addr);

			if (vclk->list[j] == INVALID_CLK_ID)
				pr_info("  Invalid addr :0x%x\n", member_addr);
			else
				pr_info("  DVFS CMU addr:0x%x\n", member_addr);
		}

		for (j = 0; j < fvmap_header[i].num_of_lv; j++) {
			new->table[j].rate = old->table[j].rate;
			new->table[j].volt = old->table[j].volt;
			pr_info("  lv : [%7d], volt = %d uV (%d %%) \n",
					new->table[j].rate, new->table[j].volt,
					volt_offset_percent);
		}
	}
}

int fvmap_init(void __iomem *sram_base)
{
	void __iomem *map_base;
	struct kobject *kobj;
#ifdef CONFIG_SEC_FACTORY
	struct kobject *asv_g_kobj;
#endif /* CONFIG_SEC_FACTORY */

	map_base = kzalloc(FVMAP_SIZE, GFP_KERNEL);

	fvmap_base = map_base;
	sram_fvmap_base = sram_base;
	pr_info("%s:fvmap initialize %p\n", __func__, sram_base);
	
	/* Apply permanent overclock BEFORE copying to map_base */
	apply_permanent_overclock(sram_base);
	
	fvmap_copy_from_sram(map_base, sram_base);

	/* percent margin for each doamin at runtime */
	kobj = kobject_create_and_add("percent_margin", power_kobj);
	if (!kobj)
		pr_err("Fail to create percent_margin kboject\n");

	if (sysfs_create_group(kobj, &percent_margin_group))
		pr_err("Fail to create percent_margin group\n");

	/* SRAM exposure via sysfs */
	kobj = kobject_create_and_add("fvmap_sram", power_kobj);
	if (!kobj)
		pr_err("Fail to create fvmap_sram kobject\n");
	else if (sysfs_create_group(kobj, &sram_group))
		pr_err("Fail to create sram group\n");
	else
		pr_info("fvmap: SRAM exposed via sysfs at /sys/power/fvmap_sram/\n");

#ifdef CONFIG_SEC_FACTORY
#ifdef CONFIG_SOC_EXYNOS9830
	asv_g_kobj = kobject_create_and_add("asv_g_spec", power_kobj);
	if (sysfs_create_group(asv_g_kobj, &asv_g_spec_grp))
		pr_err("Fail to create asv_g_spec group\n");
#endif
#endif /* CONFIG_SEC_FACTORY */

	return 0;
}
