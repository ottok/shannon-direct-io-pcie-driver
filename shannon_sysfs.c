#include "shannon_pci.h"
#include "shannon_sysfs.h"
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/genhd.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include "shannon_scsi.h"

struct shannon_attr {
	struct attribute attr;
	shannon_ssize_t (*show)(struct shannon_dev *, char *);
	shannon_ssize_t (*store)(struct shannon_dev *, const char *, shannon_size_t count);
};

struct shannon_device_attr {
	struct device_attribute attr;
	shannon_ssize_t (*show)(struct shannon_dev *, char *);
	shannon_ssize_t (*store)(struct shannon_dev *, const char *, shannon_size_t count);
};

#define define_one_rw(_name) \
static struct shannon_attr _name = \
__ATTR(_name, 0644, _name##_show, _name##_store)

#define define_one_ro(_name) \
static struct shannon_attr _name = \
__ATTR_RO(_name)

#define define_one_device_rw(_name) \
static struct shannon_device_attr _name = {\
	.attr = __ATTR(_name, 0644, shannon_device_show, shannon_device_store), \
	.show = _name##_show,\
	.store = _name##_store,\
}

#define define_one_device_ro(_name) \
static struct shannon_device_attr _name = {\
	.attr = __ATTR(_name, 0644, shannon_device_show, shannon_device_store), \
	.show = _name##_show,\
}

#define to_shannon_attr(a) container_of(a, struct shannon_attr, attr)
#define to_shannon_device_attr(a) container_of(a, struct shannon_device_attr, attr)

extern int shannon_scsi_mode;
static shannon_ssize_t shannon_device_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct pci_dev *pdev = NULL;
	struct shannon_dev *sdev = NULL;
	struct shannon_device_attr *sattr = NULL;
	shannon_ssize_t ret = -EINVAL;
	struct shannon_scsi_private *hostdata;

	pdev = container_of(dev, struct pci_dev, dev);
	if (shannon_scsi_mode) {
		hostdata = shannon_pci_get_drvdata(pdev);
		sdev = hostdata->sdev;
	} else
		sdev = shannon_pci_get_drvdata(pdev);
	sattr = to_shannon_device_attr(attr);

	if (sattr->show)
		ret = sattr->show(sdev, buf);
	else
		ret = -EIO;

	return ret;
}

static ssize_t shannon_device_store(struct device *dev, struct device_attribute *attr, const char *buf, shannon_size_t count)
{
	struct pci_dev *pdev = NULL;
	struct shannon_dev *sdev = NULL;
	struct shannon_device_attr *sattr = NULL;
	shannon_ssize_t ret = -EINVAL;
	struct shannon_scsi_private *hostdata;

	pdev = container_of(dev, struct pci_dev, dev);
	if (shannon_scsi_mode) {
		hostdata = shannon_pci_get_drvdata(pdev);
		sdev = hostdata->sdev;
	} else
		sdev = shannon_pci_get_drvdata(pdev);
	sattr = to_shannon_device_attr(attr);

	if (sattr->store)
		ret = sattr->store(sdev, buf, count);
	else
		ret = -EIO;

	return ret;
}

static shannon_ssize_t shannon_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct shannon_dev *sdev = to_shannon_dev((shannon_kobject_t *)kobj);
	struct shannon_attr *sattr = to_shannon_attr(attr);
	shannon_ssize_t ret = -EINVAL;

	if(sattr->show)
		ret = sattr->show(sdev, buf);
	else
		ret = -EIO;

	return ret;
}

static shannon_ssize_t shannon_store(struct kobject *kobj, struct attribute *attr, const char *buf, shannon_size_t count)
{
	struct shannon_dev *sdev = to_shannon_dev((shannon_kobject_t *)kobj);
	struct shannon_attr *sattr = to_shannon_attr(attr);
	shannon_ssize_t ret = -EINVAL;

	if(sattr->store)
		ret = sattr->store(sdev, buf, count);
	else
		ret = -EIO;

	return ret;
}

static void shannon_sysfs_release(struct kobject *kobj)
{
}

static struct sysfs_ops shannon_sysfs_ops = {
	.show = shannon_show,
	.store = shannon_store,
};

define_one_ro(model);
define_one_ro(firmware_version);
define_one_ro(firmware_build);
define_one_ro(driver_version);
define_one_ro(serial_number);
define_one_ro(nand_manufacturer);
define_one_ro(nand_flash_id);
define_one_ro(channels);
define_one_ro(lunsets_in_channel);
define_one_ro(luns_in_lunset);
define_one_ro(available_luns);
define_one_ro(eblocks_in_lun);
define_one_ro(pages_in_eblock);
define_one_ro(nand_page_size);
define_one_ro(block_size);
define_one_ro(use_dual_head);
define_one_ro(power_on_seconds);
define_one_ro(power_cycle_count);
define_one_ro(user_capacity);
define_one_ro(physical_capacity);
define_one_ro(overprovision);
define_one_ro(free_blkcnt);
define_one_ro(static_bad_blkcnt);
define_one_ro(dynamic_bad_blkcnt);
define_one_ro(max_err_blocks);
define_one_ro(reconfig_support);
define_one_ro(seu_flag);
define_one_ro(seu_crc_error);
define_one_ro(seu_crc_error_history);
define_one_ro(seu_ecc_error);
define_one_ro(seu_ecc_error_history);
define_one_ro(estimated_life_left);
define_one_ro(host_write_sectors);
define_one_ro(host_write_bandwidth);
define_one_ro(host_write_iops);
define_one_ro(host_write_latency);
define_one_ro(total_write_sectors);
define_one_ro(total_write_bandwidth);
define_one_ro(write_amplifier);
define_one_ro(write_amplifier_lifetime);
define_one_ro(host_read_sectors);
define_one_ro(host_read_bandwidth);
define_one_ro(host_read_iops);
define_one_ro(host_read_latency);
define_one_ro(total_gc_logicbs);
define_one_ro(total_wl_logicbs);
define_one_ro(total_err_recover_logicbs);
define_one_ro(temperature_int);
define_one_ro(temperature_int_max);
define_one_ro(temperature_flash);
define_one_ro(temperature_flash_max);
define_one_ro(temperature_board);
define_one_ro(temperature_board_max);
define_one_rw(temperature_warning_threshold);
define_one_ro(voltage_int);
define_one_ro(voltage_int_max);
define_one_ro(voltage_aux);
define_one_ro(voltage_aux_max);
define_one_ro(ecc_failure_times);
define_one_ro(ecc_statistics);
define_one_ro(device_state);
define_one_ro(access_mode);
define_one_ro(readonly_reason);
define_one_ro(reduced_write_reason);
define_one_rw(pm_qos_value);
define_one_rw(recover_rate);
define_one_rw(hot_block_reclaim_priority);
define_one_rw(suspicious_bad_lun_indicator);
define_one_rw(debug_level);
define_one_rw(shannon_buffer_write);
define_one_rw(shannon_poll_times);
define_one_rw(read_err_msg_level);
define_one_rw(wl_debug);
define_one_rw(err_check_debug);
define_one_rw(read_disturb_threshold);
define_one_rw(open_block_read_disturb_threshold);
define_one_rw(wl_timer_interval);
define_one_rw(max_in_wl_logicbs);
define_one_rw(wl_max_erase_count);
define_one_rw(wl_erase_count_delta_0);
define_one_rw(wl_erase_count_delta_1);
define_one_rw(fill_chunk_timer_expire);
define_one_rw(first_read_period);
define_one_rw(first_read_ppa);
define_one_rw(switch_microcode);
define_one_ro(service_tag);
define_one_ro(fpga_dna);
define_one_ro(udid);
define_one_ro(refresh_mbr_count);
define_one_ro(atomic_write);
define_one_ro(prioritize_write);
define_one_ro(nonaligned_bios);
define_one_rw(latency_threshold);
define_one_rw(print_latency_interval);
define_one_rw(hard_queue_limit);
define_one_rw(cmd_queue_writes_limit);
define_one_ro(hardware_version);
define_one_rw(ecc_failure_rate_threshold);
define_one_ro(cps_crc);
define_one_rw(fast_read);
define_one_rw(drop_cache);
define_one_rw(prefetch_seqread_threshold);
define_one_rw(prefetch_poll_times_threshold);
define_one_rw(prefetch_soft_bio_size_threshold);
define_one_rw(prefetch_hard_bio_size_threshold);
define_one_rw(prefetch_large_block_io_threshold);
define_one_rw(prefetch_distance_factor);
define_one_rw(prefetch_enable);
define_one_rw(prefetch_traffic_factor);

define_one_rw(update_irq_delay_interval);
define_one_rw(read_latency_divide);
define_one_rw(write_latency_divide);
define_one_rw(read_threshold_factor);
define_one_rw(write_threshold_factor);
define_one_rw(dynamic_irq_delay);
define_one_rw(irq_delay_factor);
define_one_rw(min_irq_delay);
define_one_rw(max_irq_delay);
define_one_rw(discard_large_unit_threshold);
define_one_rw(in_write_chunk_factor);
define_one_rw(buffer_write_policy);

#ifdef CONFIG_HWMON
define_one_device_ro(name);
define_one_device_ro(temp1_input);
define_one_device_ro(temp1_label);
define_one_device_ro(temp1_max);
define_one_device_ro(temp1_crit);
define_one_device_ro(temp2_input);
define_one_device_ro(temp2_label);
define_one_device_ro(temp2_max);
define_one_device_ro(temp2_crit);
define_one_device_ro(temp3_input);
define_one_device_ro(temp3_label);
define_one_device_ro(temp3_max);
define_one_device_ro(temp3_crit);

static struct device_attribute *shannon_hwmon_attrs[] = {
	&name.attr,
	&temp1_input.attr,
	&temp1_label.attr,
	&temp1_max.attr,
	&temp1_crit.attr,
	&temp2_input.attr,
	&temp2_label.attr,
	&temp2_max.attr,
	&temp2_crit.attr,
	&temp3_input.attr,
	&temp3_label.attr,
	&temp3_max.attr,
	&temp3_crit.attr,
	NULL,
};
#endif

static struct attribute *shannon_default_attrs[] = {
	&model.attr,
	&firmware_version.attr,
	&firmware_build.attr,
	&driver_version.attr,
	&serial_number.attr,
	&nand_manufacturer.attr,
	&nand_flash_id.attr,
	&channels.attr,
	&lunsets_in_channel.attr,
	&luns_in_lunset.attr,
	&available_luns.attr,
	&eblocks_in_lun.attr,
	&pages_in_eblock.attr,
	&nand_page_size.attr,
	&block_size.attr,
	&use_dual_head.attr,
	&power_on_seconds.attr,
	&power_cycle_count.attr,
	&user_capacity.attr,
	&physical_capacity.attr,
	&overprovision.attr,
	&free_blkcnt.attr,
	&static_bad_blkcnt.attr,
	&dynamic_bad_blkcnt.attr,
	&max_err_blocks.attr,
	&reconfig_support.attr,
	&seu_flag.attr,
	&seu_crc_error.attr,
	&seu_crc_error_history.attr,
	&seu_ecc_error.attr,
	&seu_ecc_error_history.attr,
	&estimated_life_left.attr,
	&host_write_sectors.attr,
	&host_write_bandwidth.attr,
	&host_write_iops.attr,
	&host_write_latency.attr,
	&total_write_sectors.attr,
	&total_write_bandwidth.attr,
	&write_amplifier.attr,
	&write_amplifier_lifetime.attr,
	&host_read_sectors.attr,
	&host_read_bandwidth.attr,
	&host_read_iops.attr,
	&host_read_latency.attr,
	&total_gc_logicbs.attr,
	&total_wl_logicbs.attr,
	&total_err_recover_logicbs.attr,
	&temperature_int.attr,
	&temperature_int_max.attr,
	&temperature_flash.attr,
	&temperature_flash_max.attr,
	&temperature_board.attr,
	&temperature_board_max.attr,
	&temperature_warning_threshold.attr,
	&voltage_int.attr,
	&voltage_int_max.attr,
	&voltage_aux.attr,
	&voltage_aux_max.attr,
	&ecc_failure_times.attr,
	&ecc_statistics.attr,
	&device_state.attr,
	&access_mode.attr,
	&readonly_reason.attr,
	&reduced_write_reason.attr,
	&pm_qos_value.attr,
	&recover_rate.attr,
	&hot_block_reclaim_priority.attr,
	&suspicious_bad_lun_indicator.attr,
	&debug_level.attr,
	&shannon_buffer_write.attr,
	&shannon_poll_times.attr,
	&read_err_msg_level.attr,
	&wl_debug.attr,
	&err_check_debug.attr,
	&read_disturb_threshold.attr,
	&open_block_read_disturb_threshold.attr,
	&wl_timer_interval.attr,
	&max_in_wl_logicbs.attr,
	&wl_max_erase_count.attr,
	&wl_erase_count_delta_0.attr,
	&wl_erase_count_delta_1.attr,
	&fill_chunk_timer_expire.attr,
	&first_read_period.attr,
	&first_read_ppa.attr,
	&switch_microcode.attr,
	&service_tag.attr,
	&fpga_dna.attr,
	&udid.attr,
	&refresh_mbr_count.attr,
	&atomic_write.attr,
	&prioritize_write.attr,
	&nonaligned_bios.attr,
	&latency_threshold.attr,
	&print_latency_interval.attr,
	&hard_queue_limit.attr,
	&cmd_queue_writes_limit.attr,
	&hardware_version.attr,
	&ecc_failure_rate_threshold.attr,
	&cps_crc.attr,
	&update_irq_delay_interval.attr,
	&read_latency_divide.attr,
	&write_latency_divide.attr,
	&write_threshold_factor.attr,
	&read_threshold_factor.attr,
	&dynamic_irq_delay.attr,
	&irq_delay_factor.attr,
	&min_irq_delay.attr,
	&max_irq_delay.attr,
	&fast_read.attr,
	&discard_large_unit_threshold.attr,
	&drop_cache.attr,
	&prefetch_seqread_threshold.attr,
	&prefetch_poll_times_threshold.attr,
	&prefetch_soft_bio_size_threshold.attr,
	&prefetch_hard_bio_size_threshold.attr,
	&prefetch_large_block_io_threshold.attr,
	&prefetch_distance_factor.attr,
	&prefetch_enable.attr,
	&prefetch_traffic_factor.attr,
	&in_write_chunk_factor.attr,
	&buffer_write_policy.attr,
	NULL,
};

static shannon_ssize_t pci_info_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
        struct shannon_dev *sdev = to_shannon_dev((shannon_kobject_t *)kobj->parent);
        struct shannon_attr *sattr = to_shannon_attr(attr);
        shannon_ssize_t ret = -EINVAL;

        if(sattr->show)
                ret = sattr->show(sdev, buf);
        else
                ret = -EIO;

        return ret;
}

static shannon_ssize_t pci_info_store(struct kobject *kobj, struct attribute *attr, const char *buf, shannon_size_t count)
{
        struct shannon_dev *sdev = to_shannon_dev((shannon_kobject_t *)kobj->parent);
        struct shannon_attr *sattr = to_shannon_attr(attr);
        shannon_ssize_t ret = -EINVAL;

        if(sattr->store)
                ret = sattr->store(sdev, buf, count);
        else
                ret = -EIO;

        return ret;
}

static struct sysfs_ops pci_info_sysfs_ops = {
        .show = pci_info_show,
        .store = pci_info_store,
};

define_one_ro(vendor_id);
define_one_ro(device_id);
define_one_ro(subsystem_vendor_id);
define_one_ro(subsystem_device_id);
define_one_ro(pci_address);
define_one_ro(pci_class);
define_one_ro(linkcap);
define_one_ro(linksta);

static struct attribute *pci_info_default_attrs[] = {
	&vendor_id.attr,
	&device_id.attr,
	&subsystem_vendor_id.attr,
	&subsystem_device_id.attr,
	&pci_address.attr,
	&pci_class.attr,
	&linkcap.attr,
	&linksta.attr,
	NULL,
};

static struct kobj_type shannon_ktype = {
	.sysfs_ops = &shannon_sysfs_ops,
	.default_attrs = shannon_default_attrs,
	.release = shannon_sysfs_release,
};

static struct kobj_type pci_info_ktype = {
	.sysfs_ops = &pci_info_sysfs_ops,
	.default_attrs = pci_info_default_attrs,
	.release = shannon_sysfs_release,
};

int shannon_sysfs_init(shannon_kobject_t *skobj)
{
	int ret;
	struct kobject *pci_info = (struct kobject *)to_shannon_pci_info_kobj(skobj);
	struct miscdevice *misc = (struct miscdevice *)to_sdev_misc(skobj);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	struct kobject *dev_kobj = &misc->this_device->kobj;
#else
	struct kobject *dev_kobj = &misc->class->kobj;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	memset(skobj, 0x00, sizeof(shannon_kobject_t));
	ret = kobject_init_and_add((struct kobject *)skobj, &shannon_ktype, dev_kobj, "shannon");
	if (ret)
		return ret;

	memset(pci_info, 0x00, sizeof(shannon_kobject_t));
	ret = kobject_init_and_add(pci_info, &pci_info_ktype, (struct kobject *)skobj, "pci_info");
	if (ret)
		goto del_skobj;
#else
	struct kobject *kobj = (struct kobject*)skobj;

	memset(skobj, 0x00, sizeof(shannon_kobject_t));
	kobject_init(kobj);
	kobj->parent = dev_kobj;
	kobject_set_name(kobj, "shannon");
	kobj->ktype = &shannon_ktype;
	ret = kobject_add(kobj);
	if (ret)
		return ret;

	memset(pci_info, 0x00, sizeof(shannon_kobject_t));
	kobject_init(pci_info);
	pci_info->parent = kobj;
	kobject_set_name(pci_info, "pci_info");
	pci_info->ktype = &pci_info_ktype;
	ret = kobject_add(pci_info);
	if (ret)
		goto del_skobj;
#endif

	return ret;
del_skobj:
	kobject_del((struct kobject *)skobj);

	return ret;
}

void shannon_sysfs_exit(shannon_kobject_t *skobj)
{
	struct kobject *pci_info = (struct kobject *)to_shannon_pci_info_kobj(skobj);

	kobject_del(pci_info);
	kobject_del((struct kobject *)skobj);
}

struct kobject *to_sdev_kobj(shannon_kobject_t *skobj)
{
	struct gendisk *disk = (struct gendisk *)to_shannon_disk(skobj);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	return &disk_to_dev(disk)->kobj;
#else
	return &disk->dev.kobj;
#endif

#else
	return &disk->kobj;
#endif
}

int shannon_sysfs_link(shannon_kobject_t *skobj)
{
	struct kobject *sdev_kobj = to_sdev_kobj(skobj);
	return sysfs_create_link(sdev_kobj, (struct kobject *)skobj, "shannon");
}

void shannon_sysfs_unlink(shannon_kobject_t *skobj)
{
	struct kobject *sdev_kobj = to_sdev_kobj(skobj);
	sysfs_remove_link(sdev_kobj, "shannon");
}

shannon_device_t *shannon_hwmon_init(shannon_pci_dev_t *pdev, const char *hwmon_name)
{
#ifdef CONFIG_HWMON
	struct device *dev = &((struct pci_dev *)pdev)->dev;
	struct device *hwmon_dev = NULL;

	int err = 0;
	int i;


	for (i = 0; shannon_hwmon_attrs[i] && !err; i++)
		err = device_create_file(dev, shannon_hwmon_attrs[i]);

	if (err) {
		while (--i >= 0)
			device_remove_file(dev, shannon_hwmon_attrs[i]);
		shannon_warn("create hwmon sysfs files failed!");
		return NULL;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
	hwmon_dev = hwmon_device_register(dev);
#else
	hwmon_dev = hwmon_device_register_with_info(dev, hwmon_name, NULL, NULL, NULL);
#endif
	if (IS_ERR(hwmon_dev)) {
		shannon_warn("hwmon_device_register failed!");
		for (i = 0; shannon_hwmon_attrs[i]; i++)
			device_remove_file(dev, shannon_hwmon_attrs[i]);
		return NULL;
	}

	return hwmon_dev;
#else
	return NULL;
#endif
}

void shannon_hwmon_exit(shannon_pci_dev_t *pdev, shannon_device_t *hwmon_dev)
{
#ifdef CONFIG_HWMON
	struct device *dev = &((struct pci_dev *)pdev)->dev;
	int i;

	if (SHANNON_IS_ERR_OR_NULL(hwmon_dev))
		return;

	hwmon_device_unregister((struct device *)hwmon_dev);
	for (i = 0; shannon_hwmon_attrs[i]; i++)
		device_remove_file(dev, shannon_hwmon_attrs[i]);
#endif
}


/* For namespace. */
struct shannon_attr_ns {
	struct attribute attr;
	shannon_ssize_t (*show)(struct shannon_namespace *, char *);
	shannon_ssize_t (*store)(struct shannon_namespace *, const char *, shannon_size_t count);
};

#define define_one_rw_ns(_name) \
static struct shannon_attr_ns _name##_ns = \
__ATTR(_name, 0644, _name##_ns_show, _name##_ns_store)

/*
 * We can't use __ATTR_RO here. It's ugly or causes conflicts with
 * non-namespace version.
 */
#define define_one_ro_ns(_name) \
static struct shannon_attr_ns _name##_ns = \
__ATTR(_name, 0444, _name##_ns_show, NULL)


#define to_shannon_attr_ns(a) container_of(a, struct shannon_attr_ns, attr)

static shannon_ssize_t shannon_show_ns(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct shannon_namespace *ns = to_shannon_namespace((shannon_kobject_t *)kobj);
	struct shannon_attr_ns *sattr = to_shannon_attr_ns(attr);
	shannon_ssize_t ret = -EINVAL;

	if (sattr->show)
		ret = sattr->show(ns, buf);
	else
		ret = -EIO;

	return ret;
}

static shannon_ssize_t shannon_store_ns(struct kobject *kobj, struct attribute *attr, const char *buf, shannon_size_t count)
{
	struct shannon_namespace *ns = to_shannon_namespace((shannon_kobject_t *)kobj);
	struct shannon_attr_ns *sattr = to_shannon_attr_ns(attr);
	shannon_ssize_t ret = -EINVAL;

	if (sattr->store)
		ret = sattr->store(ns, buf, count);
	else
		ret = -EIO;

	return ret;
}

static void shannon_sysfs_release_ns(struct kobject *kobj)
{
}

static struct sysfs_ops shannon_sysfs_ops_ns = {
	.show = shannon_show_ns,
	.store = shannon_store_ns,
};

define_one_ro_ns(host_write_sectors);
define_one_ro_ns(host_read_sectors);
define_one_ro_ns(valid_sectors);
define_one_ro_ns(pending_bios);
define_one_ro_ns(nonaligned_bios);
define_one_ro_ns(seq_num);
define_one_ro_ns(rmw_list);
define_one_rw_ns(latency_threshold);
define_one_rw_ns(print_latency_interval);
define_one_ro_ns(user_defined_name);
define_one_rw_ns(discard_large_unit_threshold);

static struct attribute *shannon_default_attrs_ns[] = {
	&host_write_sectors_ns.attr,
	&host_read_sectors_ns.attr,
	&valid_sectors_ns.attr,
	&pending_bios_ns.attr,
	&nonaligned_bios_ns.attr,
	&seq_num_ns.attr,
	&rmw_list_ns.attr,
	&latency_threshold_ns.attr,
	&print_latency_interval_ns.attr,
	&user_defined_name_ns.attr,
	&discard_large_unit_threshold_ns.attr,
	NULL,
};

static struct kobj_type shannon_ktype_ns = {
	.sysfs_ops = &shannon_sysfs_ops_ns,
	.default_attrs = shannon_default_attrs_ns,
	.release = shannon_sysfs_release_ns,
};

int shannon_sysfs_init_ns(shannon_kobject_t *skobj)
{
	int ret;
	struct gendisk *disk = (struct gendisk *)to_shannon_disk_ns(skobj);
	char *ns_name = to_ns_name(skobj);
	struct kobject *ns_kobj;
	struct miscdevice *misc = (struct miscdevice *)ns_to_pool_misc(skobj);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	struct kobject *pool_kobj = &misc->this_device->kobj;
#else
	struct kobject *pool_kobj = &misc->class->kobj;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	struct device *ddev = disk_to_dev(disk);
#else
	struct device *ddev = &disk->dev;
#endif
	memset(skobj, 0x00, sizeof(shannon_kobject_t));
	ret = kobject_init_and_add((struct kobject *)skobj, &shannon_ktype_ns, &ddev->kobj, "shannon");
	ns_kobj = &ddev->kobj;
#else
	struct kobject *kobj = (struct kobject *)skobj;

	memset(skobj, 0x00, sizeof(shannon_kobject_t));
	kobject_init(kobj);
	kobj->parent = &disk->kobj;
	kobject_set_name(kobj, "shannon");
	kobj->ktype = &shannon_ktype_ns;
	ret = kobject_add(kobj);
	ns_kobj = &disk->kobj;
#endif

	if (ret)
		return ret;
	ret = sysfs_create_link(pool_kobj, ns_kobj, ns_name);
	if (ret)
		kobject_del(ns_kobj);
	return ret;
}

void shannon_sysfs_exit_ns(shannon_kobject_t *skobj)
{
	char *ns_name = to_ns_name(skobj);
	struct miscdevice *misc = (struct miscdevice *)ns_to_pool_misc(skobj);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	struct kobject *pool_kobj = &misc->this_device->kobj;
#else
	struct kobject *pool_kobj = &misc->class->kobj;
#endif
	kobject_del((struct kobject *)skobj);
	sysfs_remove_link(pool_kobj, ns_name);
}

/* For pool. */
struct shannon_attr_pool {
	struct attribute attr;
	shannon_ssize_t (*show)(struct shannon_pool *, char *);
	shannon_ssize_t (*store)(struct shannon_pool *, const char *, shannon_size_t count);
};

#define define_one_rw_pool(_name) \
static struct shannon_attr_pool _name##_pool = \
__ATTR(_name, 0644, _name##_pool_show, _name##_pool_store)

/*
 * We can't use __ATTR_RO here. It's ugly or causes conflicts with
 * non-pool version.
 */
#define define_one_ro_pool(_name) \
static struct shannon_attr_pool _name##_pool = \
__ATTR(_name, 0444, _name##_pool_show, NULL)

#define to_shannon_attr_pool(a) container_of(a, struct shannon_attr_pool, attr)

static shannon_ssize_t shannon_show_pool(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct shannon_pool *pool = to_shannon_pool((shannon_kobject_t *)kobj);
	struct shannon_attr_pool *sattr = to_shannon_attr_pool(attr);
	shannon_ssize_t ret = -EINVAL;

	if (sattr->show)
		ret = sattr->show(pool, buf);
	else
		ret = -EIO;

	return ret;
}

static shannon_ssize_t shannon_store_pool(struct kobject *kobj, struct attribute *attr, const char *buf, shannon_size_t count)
{
	struct shannon_pool *pool = to_shannon_pool((shannon_kobject_t *)kobj);
	struct shannon_attr_pool *sattr = to_shannon_attr_pool(attr);
	shannon_ssize_t ret = -EINVAL;

	if (sattr->store)
		ret = sattr->store(pool, buf, count);
	else
		ret = -EIO;

	return ret;
}

static void shannon_sysfs_release_pool(struct kobject *kobj)
{
}

static struct sysfs_ops shannon_sysfs_ops_pool = {
	.show = shannon_show_pool,
	.store = shannon_store_pool,
};

define_one_ro_pool(used_space_percentage);
define_one_ro_pool(physical_capacity);
define_one_rw_pool(overprovision);
define_one_rw_pool(hard_queue_limit);
define_one_rw_pool(read_cmd_limit);

static struct attribute *shannon_default_attrs_pool[] = {
	&used_space_percentage_pool.attr,
	&physical_capacity_pool.attr,
	&overprovision_pool.attr,
	&hard_queue_limit_pool.attr,
	&read_cmd_limit_pool.attr,
	NULL,
};

static struct kobj_type shannon_ktype_pool = {
	.sysfs_ops = &shannon_sysfs_ops_pool,
	.default_attrs = shannon_default_attrs_pool,
	.release = shannon_sysfs_release_pool,
};

int shannon_sysfs_init_pool(shannon_kobject_t *skobj)
{
	int ret;
	struct miscdevice *misc = (struct miscdevice *)to_pool_misc(skobj);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	struct device *pool_dev = misc->this_device;
#else
	struct class_device *pool_dev = misc->class;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	memset(skobj, 0x00, sizeof(shannon_kobject_t));
	ret = kobject_init_and_add((struct kobject *)skobj, &shannon_ktype_pool, &pool_dev->kobj, "shannon");
#else
	struct kobject *kobj = (struct kobject *)skobj;

	memset(skobj, 0x00, sizeof(shannon_kobject_t));
	kobject_init(kobj);
	kobj->parent = &pool_dev->kobj;
	kobject_set_name(kobj, "shannon");
	kobj->ktype = &shannon_ktype_pool;
	ret = kobject_add(kobj);
#endif

	return ret;
}

void shannon_sysfs_exit_pool(shannon_kobject_t *skobj)
{
	kobject_del((struct kobject *)skobj);
}
