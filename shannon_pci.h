#ifndef __SHANNON_PCI_H
#define __SHANNON_PCI_H

#include "shannon_kcore.h"
#include "shannon_dma.h"

struct shannon_pci_info {
	u32	devfn;
	u16	vendor_id;
	u16	device_id;
	u16	subsystem_vendor_id;
	u16	subsystem_device_id;
	u32	class;
	u8	pci_bus_number;
	u8	pci_slot_number;
	u8	pci_func_number;
	u32	lnkcap;
	u16	lnksta;
};

extern shannon_pci_dev_t *shannon_pci_get_slot(shannon_pci_dev_t *dev, unsigned int devfn);
extern int shannon_pci_enable_device(shannon_pci_dev_t *dev);
extern void shannon_pci_disable_device(shannon_pci_dev_t *dev);

extern void *shannon_pci_alloc_consistent(shannon_pci_dev_t *hwdev, shannon_size_t size, shannon_dma_addr_t *dma_handle);
extern void shannon_pci_free_consistent(shannon_pci_dev_t *hwdev, shannon_size_t size, void *vaddr, shannon_dma_addr_t dma_handle);

extern void shannon_pci_set_master(shannon_pci_dev_t *dev);
extern void shannon_pci_set_drvdata(shannon_pci_dev_t *pdev, void *data);
extern void *shannon_pci_get_drvdata(shannon_pci_dev_t *pdev);
extern int shannon_pci_request_region(shannon_pci_dev_t *dev, int i, const char *p);
extern void shannon_pci_release_regions(shannon_pci_dev_t *pdev);
extern shannon_resource_size_t shannon_pci_resource_start(shannon_pci_dev_t *dev, int bar);
extern shannon_resource_size_t shannon_pci_resource_len(shannon_pci_dev_t *dev, int bar);
extern int shannon_pci_enable_msi(shannon_pci_dev_t *dev);
extern int shannon_pci_enable_msix(shannon_pci_dev_t *dev, void **msix_data, int nvec);
extern void shannon_pci_disable_msi(shannon_pci_dev_t *dev);
extern void shannon_pci_disable_msix(shannon_pci_dev_t *dev, void **msix_data);
extern int shannon_pci_get_msix_entry(shannon_msix_entry_t *msix_data, int irq, int entry_count);
extern int shannon_pci_get_msix_vector(shannon_msix_entry_t *msix_data, int index, int entry_count);
extern int check_hot_pluggable(shannon_pci_dev_t *pdev);
extern int shannon_get_adapter_status(shannon_pci_dev_t *dev);
extern int shannon_disable_slot(shannon_pci_dev_t *dev);
extern void shannon_disable_correctable_aer(shannon_pci_dev_t *dev);
extern void shannon_set_max_payload_size(shannon_pci_dev_t *dev);
extern void shannon_set_bridge_timeout(shannon_pci_dev_t *dev);
extern int shannon_pcie_set_readrq(shannon_pci_dev_t *pdev, int rq);

extern void get_pci_info(shannon_pci_dev_t *pdev, struct shannon_pci_info *info);
extern unsigned int get_pci_irq_num(shannon_pci_dev_t *pdev);
extern shannon_device_t * get_device_from_pci_dev(shannon_pci_dev_t *pdev);
extern int dev_is_8639(shannon_pci_dev_t *pdev);
extern int dev_is_g5_ffsa(shannon_pci_dev_t *pdev);
extern int dev_is_g5_fpga(shannon_pci_dev_t *pdev);
extern int shannon_pci_get_max_linkwidth(struct shannon_pci_info *info);
extern int shannon_pci_get_cur_linkwidth(struct shannon_pci_info *info);
extern int shannon_pci_get_max_linkspeed(struct shannon_pci_info *info);
extern int shannon_pci_get_cur_linkspeed(struct shannon_pci_info *info);
extern int shannon_pci_reset_function(shannon_pci_dev_t *pdev);
extern int shannon_pci_get_node(shannon_pci_dev_t *pdev);
extern void get_pci_bus_info(shannon_pci_dev_t *pdev, struct shannon_pci_info *info);
extern int shannon_pci_bus_retrain(shannon_pci_dev_t *pdev);

#endif /* __SHANNON_PCI_H */
