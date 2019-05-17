#ifndef __SHANNON_SYSFS_H
#define __SHANNON_SYSFS_H

#include "shannon_kcore.h"

struct __shannon_kobject {
	RESERVE_MEM(152);
};

typedef struct __shannon_kobject shannon_kobject_t;

extern int shannon_sysfs_init(shannon_kobject_t *skobj);
extern void shannon_sysfs_exit(shannon_kobject_t *skobj);
extern int shannon_sysfs_link(shannon_kobject_t *skobj);
extern void shannon_sysfs_unlink(shannon_kobject_t *skobj);
extern shannon_device_t *shannon_hwmon_init(shannon_pci_dev_t *pdev);
extern void shannon_hwmon_exit(shannon_pci_dev_t *pdev, shannon_device_t *hwmon_dev);

struct shannon_dev;
//  functions below are implemented in shannon_sysfs_core.c
extern struct shannon_dev * to_shannon_dev(shannon_kobject_t *skobj);
extern void *to_sdev_misc(shannon_kobject_t *skobj);
extern void *to_shannon_disk(shannon_kobject_t *skobj);
extern shannon_kobject_t *to_shannon_pci_info_kobj(shannon_kobject_t *skobj);
extern char * shannon_disk_name(struct shannon_dev *sdev);
extern shannon_ssize_t model_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t firmware_version_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t firmware_build_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t driver_version_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t serial_number_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t nand_manufacturer_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t nand_flash_id_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t channels_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t lunsets_in_channel_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t luns_in_lunset_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t available_luns_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t eblocks_in_lun_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t pages_in_eblock_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t nand_page_size_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t block_size_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t use_dual_head_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t power_on_seconds_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t power_cycle_count_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t user_capacity_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t physical_capacity_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t overprovision_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t free_blkcnt_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t static_bad_blkcnt_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t dynamic_bad_blkcnt_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t max_err_blocks_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t reconfig_support_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t seu_flag_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t seu_crc_error_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t seu_crc_error_history_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t seu_ecc_error_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t seu_ecc_error_history_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t estimated_life_left_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_write_sectors_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_write_bandwidth_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_write_iops_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_write_latency_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t total_write_sectors_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t total_write_bandwidth_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t write_amplifier_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t write_amplifier_lifetime_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_read_sectors_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_read_bandwidth_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_read_iops_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t host_read_latency_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t total_gc_logicbs_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t total_wl_logicbs_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t total_err_recover_logicbs_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_int_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_int_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_flash_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_flash_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_board_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_board_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_warning_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temperature_warning_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t voltage_int_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t voltage_int_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t voltage_aux_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t voltage_aux_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t ecc_failure_times_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t ecc_statistics_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t device_state_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t access_mode_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t reduced_write_reason_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t readonly_reason_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t pm_qos_value_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t pm_qos_value_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t recover_rate_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t recover_rate_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t hot_block_reclaim_priority_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t hot_block_reclaim_priority_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t suspicious_bad_lun_indicator_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t suspicious_bad_lun_indicator_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t debug_level_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t debug_level_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t shannon_buffer_write_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t shannon_buffer_write_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t shannon_poll_times_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t shannon_poll_times_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t read_err_msg_level_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t read_err_msg_level_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t wl_debug_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t wl_debug_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t err_check_debug_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t err_check_debug_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t read_disturb_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t read_disturb_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t open_block_read_disturb_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t open_block_read_disturb_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t wl_timer_interval_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t wl_timer_interval_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t max_in_wl_logicbs_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t max_in_wl_logicbs_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t wl_max_erase_count_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t wl_max_erase_count_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t wl_erase_count_delta_0_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t wl_erase_count_delta_0_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t wl_erase_count_delta_1_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t wl_erase_count_delta_1_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t fill_chunk_timer_expire_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t fill_chunk_timer_expire_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t service_tag_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t fpga_dna_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t udid_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t refresh_mbr_count_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t atomic_write_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prioritize_write_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t nonaligned_bios_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t name_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp1_input_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp1_label_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp1_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp1_crit_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp2_input_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp2_label_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp2_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp2_crit_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp3_input_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp3_label_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp3_max_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t temp3_crit_show(struct shannon_dev *sdev, char *buf);

extern shannon_ssize_t vendor_id_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t device_id_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t subsystem_vendor_id_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t subsystem_device_id_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t pci_address_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t pci_class_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t linkcap_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t linksta_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t period_read_period_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t period_read_period_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t period_read_ppa_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t period_read_ppa_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t switch_microcode_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t switch_microcode_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t print_latency_interval_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t print_latency_interval_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t hard_queue_limit_show(struct shannon_dev *, char *buf);
extern shannon_ssize_t hard_queue_limit_store(struct shannon_dev *, const char *buf, shannon_size_t count);
extern shannon_ssize_t cmd_queue_writes_limit_show(struct shannon_dev *, char *buf);
extern shannon_ssize_t cmd_queue_writes_limit_store(struct shannon_dev *, const char *buf, shannon_size_t count);
extern shannon_ssize_t hardware_version_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t ecc_failure_rate_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t ecc_failure_rate_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t cps_crc_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t fast_read_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t fast_read_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);

extern shannon_ssize_t update_irq_delay_interval_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t update_irq_delay_interval_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t dynamic_irq_delay_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t dynamic_irq_delay_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t irq_delay_factor_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t irq_delay_factor_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t max_irq_delay_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t max_irq_delay_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t min_irq_delay_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t min_irq_delay_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t read_latency_divide_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t read_latency_divide_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t write_latency_divide_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t write_latency_divide_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t write_threshold_factor_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t write_threshold_factor_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t read_threshold_factor_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t read_threshold_factor_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t drop_cache_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t drop_cache_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_seqread_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_seqread_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_poll_times_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_poll_times_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_soft_bio_size_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_soft_bio_size_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_hard_bio_size_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_hard_bio_size_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_large_block_io_threshold_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_large_block_io_threshold_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_distance_factor_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_distance_factor_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_enable_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_enable_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);
extern shannon_ssize_t prefetch_traffic_factor_show(struct shannon_dev *sdev, char *buf);
extern shannon_ssize_t prefetch_traffic_factor_store(struct shannon_dev *sdev, const char *buf, shannon_size_t count);


/* For namespace. */
extern char *to_ns_name(shannon_kobject_t *skobj);
extern void *ns_to_pool_misc(shannon_kobject_t *skobj);
extern struct shannon_namespace *to_shannon_namespace(shannon_kobject_t *skobj);
extern void *to_shannon_disk_ns(shannon_kobject_t *skobj);
extern shannon_ssize_t host_write_sectors_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t host_read_sectors_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t valid_sectors_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t pending_bios_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t nonaligned_bios_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t seq_num_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t rmw_list_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t print_latency_interval_ns_show(struct shannon_namespace *ns, char *buf);
extern shannon_ssize_t print_latency_interval_ns_store(struct shannon_namespace *ns, const char *buf, shannon_size_t count);
extern shannon_ssize_t user_defined_name_ns_show(struct shannon_namespace *ns, char *buf);
extern int shannon_sysfs_init_ns(shannon_kobject_t *skobj);
extern void shannon_sysfs_exit_ns(shannon_kobject_t *skobj);

/* For pool. */
extern struct shannon_pool *to_shannon_pool(shannon_kobject_t *skobj);
extern void *to_pool_misc(shannon_kobject_t *skobj);
extern shannon_ssize_t used_space_percentage_pool_show(struct shannon_pool *spool, char *buf);
extern shannon_ssize_t physical_capacity_pool_show(struct shannon_pool *spool, char *buf);
extern int shannon_sysfs_init_pool(shannon_kobject_t *skobj);
extern void shannon_sysfs_exit_pool(shannon_kobject_t *skobj);
extern shannon_ssize_t overprovision_pool_show(struct shannon_pool *spool, char *buf);
extern shannon_ssize_t overprovision_pool_store(struct shannon_pool *spool, const char *buf, shannon_size_t count);
extern shannon_ssize_t hard_queue_limit_pool_show(struct shannon_pool *spool, char *buf);
extern shannon_ssize_t hard_queue_limit_pool_store(struct shannon_pool *spool, const char *buf, shannon_size_t count);

#endif /* __SHANNON_SYSFS_H */