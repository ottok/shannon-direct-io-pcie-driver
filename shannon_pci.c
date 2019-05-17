#include "shannon_pci.h"
#include "shannon_time.h"
#include <linux/pci.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
#include <linux/pci_hotplug.h>
#endif

shannon_pci_dev_t *shannon_pci_get_slot(shannon_pci_dev_t *dev, unsigned int devfn)
{
	return (shannon_pci_dev_t *)pci_get_slot(((struct pci_dev *)dev)->bus, devfn);
}

void shannon_pci_disable_device(shannon_pci_dev_t *dev)
{
	pci_disable_device((struct pci_dev *)dev);
}

void shannon_pci_set_master(shannon_pci_dev_t *dev)
{
	pci_set_master((struct pci_dev *)dev);
}

void * shannon_pci_alloc_consistent(shannon_pci_dev_t *hwdev, shannon_size_t size, shannon_dma_addr_t *dma_handle)
{
	return pci_alloc_consistent((struct pci_dev *)hwdev, size, dma_handle);
}

void shannon_pci_free_consistent(shannon_pci_dev_t *hwdev, shannon_size_t size, void *vaddr, shannon_dma_addr_t dma_handle)
{
	pci_free_consistent((struct pci_dev *)hwdev, size, vaddr, dma_handle);
}

void shannon_pci_set_drvdata(shannon_pci_dev_t *pdev, void *data)
{
	pci_set_drvdata((struct pci_dev *)pdev, data);
}

int shannon_pci_enable_device(shannon_pci_dev_t *dev)
{
	return pci_enable_device((struct pci_dev *)dev);
}

int shannon_pci_request_region(shannon_pci_dev_t *dev, int i, const char *p)
{
	return pci_request_region((struct pci_dev *)dev, i, p);
}

void shannon_pci_release_regions(shannon_pci_dev_t *pdev)
{
	pci_release_regions((struct pci_dev *)pdev);
}

shannon_resource_size_t shannon_pci_resource_start(shannon_pci_dev_t *dev, int bar)
{
	return pci_resource_start((struct pci_dev *)dev, bar);
}

shannon_resource_size_t shannon_pci_resource_len(shannon_pci_dev_t *dev, int bar)
{
	return pci_resource_len((struct pci_dev *)dev, bar);
}

int shannon_pci_enable_msi(shannon_pci_dev_t *dev)
{
	return pci_enable_msi((struct pci_dev *)dev);
}

int shannon_pci_enable_msix(shannon_pci_dev_t *dev, void **msix_data, int nvec)
{
	int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
	int ret;
#endif

	*msix_data = shannon_kzalloc(nvec * sizeof(struct msix_entry), GFP_SHANNON);
	if (!(*msix_data)) {
		shannon_err("alloc msix data failed.\n");
		return -ENOMEM;
	}
	for (i = 0; i < nvec; i++)
		((struct msix_entry*)(*msix_data))[i].entry = i;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
	return pci_enable_msix((struct pci_dev *)dev, (struct msix_entry*)(*msix_data), nvec);
#else
	ret = pci_enable_msix_range((struct pci_dev *)dev, (struct msix_entry*)(*msix_data), nvec, nvec);
	if (ret < 0)
		return ret;
	return 0;
#endif
}

void shannon_pci_disable_msi(shannon_pci_dev_t *dev)
{
	pci_disable_msi((struct pci_dev *)dev);
}

void shannon_pci_disable_msix(shannon_pci_dev_t *dev, void **msix_data)
{
	shannon_kfree(*msix_data);
	*msix_data = NULL;
	pci_disable_msix((struct pci_dev *)dev);
}

int shannon_pci_get_msix_entry(shannon_msix_entry_t *msix_data, int irq, int entry_count)
{
	int entry;

	for (entry = 0; entry < entry_count; entry++)
		if (((struct msix_entry*)msix_data)[entry].vector == irq)
			break;

	return entry;
}

int shannon_pci_get_msix_vector(shannon_msix_entry_t *msix_data, int index, int entry_count)
{
	if (index < entry_count)
		return ((struct msix_entry*)msix_data)[index].vector;

	return -1;
}

int check_hot_pluggable(shannon_pci_dev_t *pdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
	struct pci_dev *pci_dev = ((struct pci_dev *)pdev);

	if ((pci_dev->slot == NULL) || (pci_dev->slot->hotplug == NULL))
		return 0;
	else
		return 1;
#else
	return 0;
#endif
}

int shannon_get_adapter_status(shannon_pci_dev_t *pdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
	struct pci_dev *dev = ((struct pci_dev *)pdev);
	struct hotplug_slot *hotplug = dev->slot->hotplug;
	int ret;
	u8 adapter_status;

	ret = hotplug->ops->get_adapter_status(hotplug, &adapter_status);
	if (ret) {
		shannon_err("get adapter status failed!\n");
		return ret;
	}

	return adapter_status;
#else
	return 1;
#endif
}

int shannon_disable_slot(shannon_pci_dev_t *pdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
	struct pci_dev *dev = ((struct pci_dev *)pdev);
	struct hotplug_slot *hotplug = dev->slot->hotplug;

	hotplug->ops->disable_slot(hotplug);
#endif
	return 0;
}

static inline int shannon_pci_pcie_cap(struct pci_dev *dev)
{
	return pci_find_capability(dev, PCI_CAP_ID_EXP);
}

void shannon_disable_correctable_aer(shannon_pci_dev_t *pdev)
{
	u16 bus_val;
	struct pci_dev *dev = (struct pci_dev *)pdev;
	int bus_cap;

	if (dev->bus->self == NULL)
		return;

	bus_cap = shannon_pci_pcie_cap(dev->bus->self);
	if (bus_cap == 0)
		return;
	pci_read_config_word(dev->bus->self, bus_cap + PCI_EXP_DEVCTL, &bus_val);
	pci_write_config_word(dev->bus->self, bus_cap + PCI_EXP_DEVCTL, (bus_val & ~PCI_EXP_DEVCTL_CERE));
}

void shannon_set_max_payload_size(shannon_pci_dev_t *pdev)
{
	u16 val, bus_val, mps, bus_mps;
	struct pci_dev *dev = (struct pci_dev *)pdev;
	int dev_cap, bus_cap;

	if (dev->bus->self == NULL)
		return;

	dev_cap = shannon_pci_pcie_cap(dev);
	if (dev_cap == 0)
		return;
	pci_read_config_word(dev, dev_cap + PCI_EXP_DEVCTL, &val);
	bus_cap = shannon_pci_pcie_cap(dev->bus->self);
	if (bus_cap == 0)
		return;
	pci_read_config_word(dev->bus->self, bus_cap + PCI_EXP_DEVCTL, &bus_val);
	mps = val & PCI_EXP_DEVCTL_PAYLOAD;
	bus_mps = bus_val & PCI_EXP_DEVCTL_PAYLOAD;
	if (mps != bus_mps) {
		val &= ~PCI_EXP_DEVCTL_PAYLOAD;
		val |= bus_mps;
		pci_write_config_word(dev, dev_cap + PCI_EXP_DEVCTL, val);
	}
}

static inline bool shannon_pci_is_pcie(struct pci_dev *dev)
{
	return !!shannon_pci_pcie_cap(dev);
}

#define  PCI_EXP_DEVCAP2_TIMEOUT	0xf
#define  PCI_EXP_TIMEOUT_RANGE_A	0x1
#define  PCI_EXP_TIMEOUT_RANGE_B	0x2
#define  PCI_EXP_TIMEOUT_RANGE_C	0x4
void shannon_set_bridge_timeout(shannon_pci_dev_t *pdev)
{
	struct pci_dev *bridge, *dev = (struct pci_dev *)pdev;
	u32 cap;
	u16 flags, ctrl;
	int pos;

	bridge = dev->bus->self;
	if (!bridge || !shannon_pci_is_pcie(bridge))
		return;

	pos = shannon_pci_pcie_cap(bridge);
	if (!pos)
		return;

	/* ARI is a PCIe v2 feature */
	pci_read_config_word(bridge, pos + PCI_EXP_FLAGS, &flags);
	if ((flags & PCI_EXP_FLAGS_VERS) < 2)
		return;

	pci_read_config_dword(bridge, pos + PCI_EXP_DEVCAP2, &cap);
	shannon_info("cap=0x%x.\n", cap);
	if (!(cap & PCI_EXP_DEVCAP2_TIMEOUT))
		return;

	if (cap & PCI_EXP_TIMEOUT_RANGE_B) {
		pci_read_config_word(bridge, pos + PCI_EXP_DEVCTL2, &ctrl);
		shannon_info("ctrl=0x%lx.\n", ctrl);
		ctrl &= 0xf;
		ctrl |= 0x6; /* 65ms to 210ms */
		pci_write_config_word(bridge, pos + PCI_EXP_DEVCTL2, ctrl);
	}
}

void *shannon_pci_get_drvdata(shannon_pci_dev_t *pdev)
{
	return pci_get_drvdata((struct pci_dev *)pdev);
}

int shannon_pcie_set_readrq(shannon_pci_dev_t *pdev, int rq)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 23)
	return pcie_set_readrq((struct pci_dev *)pdev, rq);
#else
	struct pci_dev * dev = (struct pci_dev *)pdev;
	int cap, err = -EINVAL;
	u16 ctl, v;

	if (rq < 128 || rq > 4096 || (rq & (rq-1)))
		goto out;

	v = (ffs(rq) - 8) << 12;

	cap = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!cap)
		goto out;

	err = pci_read_config_word(dev, cap + PCI_EXP_DEVCTL, &ctl);
	if (err)
		goto out;

	if ((ctl & PCI_EXP_DEVCTL_READRQ) != v) {
		ctl &= ~PCI_EXP_DEVCTL_READRQ;
		ctl |= v;
		err = pci_write_config_word(dev, cap + PCI_EXP_DEVCTL, ctl);
	}

out:
	return err;
#endif
}

void get_pci_info(shannon_pci_dev_t *pdev, struct shannon_pci_info *info)
{
	struct pci_dev *pci_dev = (struct pci_dev *)pdev;

	int cap = -EINVAL;

	info->devfn = pci_dev->devfn;
	info->vendor_id = pci_dev->vendor;
	info->device_id = pci_dev->device;
	info->subsystem_vendor_id = pci_dev->subsystem_vendor;
	info->subsystem_device_id = pci_dev->subsystem_device;
	info->class = pci_dev->class;
	if (pci_dev->bus)
		info->pci_bus_number = pci_dev->bus->number;
	else
		info->pci_bus_number = 255;
	info->pci_slot_number = PCI_SLOT(pci_dev->devfn);
	info->pci_func_number = PCI_FUNC(pci_dev->devfn);

	cap = pci_find_capability(pci_dev, PCI_CAP_ID_EXP);

	if (cap) {
		if (pci_read_config_word(pci_dev, cap + PCI_EXP_LNKSTA, &info->lnksta))
			info->lnksta = 0;
		if (pci_read_config_dword(pci_dev, cap + PCI_EXP_LNKCAP, &info->lnkcap))
			info->lnkcap = 0;
	} else {
		info->lnksta = 0;
		info->lnkcap = 0;
	}
}

unsigned int get_pci_irq_num(shannon_pci_dev_t *pdev)
{
	return ((struct pci_dev *)pdev)->irq;
}

shannon_device_t *get_device_from_pci_dev(shannon_pci_dev_t *pdev)
{
	return (shannon_device_t *)(&(((struct pci_dev *)pdev)->dev));
}

int dev_is_8639(shannon_pci_dev_t *pdev)
{
	struct pci_dev *pci_dev = (struct pci_dev *)pdev;
	return (pci_dev->device == 0x1275)?1:0;
}

int dev_is_g5_ffsa(shannon_pci_dev_t *pdev)
{
	struct pci_dev *pci_dev = (struct pci_dev *)pdev;
	return (pci_dev->device == 0x05a5)?1:0;
}

int dev_is_g5_fpga(shannon_pci_dev_t *pdev)
{
	struct pci_dev *pci_dev = (struct pci_dev *)pdev;
	return ((pci_dev->device == 0x25a5) || (pci_dev->device == 0x35a5))?1:0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static int shannon_pcie_flr(struct pci_dev *dev)
{
	int i;
	int pos;
	u32 cap;
	u16 status, control;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pos)
		return -ENOTTY;

	pci_read_config_dword(dev, pos + PCI_EXP_DEVCAP, &cap);
	if (!(cap & PCI_EXP_DEVCAP_FLR))
		return -ENOTTY;

	/* Wait for Transaction Pending bit clean */
	for (i = 0; i < 4; i++) {
		if (i)
			shannon_msleep((1 << (i - 1)) * 100);

		pci_read_config_word(dev, pos + PCI_EXP_DEVSTA, &status);
		if (!(status & PCI_EXP_DEVSTA_TRPND))
			goto clear;
	}

	dev_err(&dev->dev, "transaction is not cleared; "
			"proceeding with reset anyway\n");

clear:
	pci_read_config_word(dev, pos + PCI_EXP_DEVCTL, &control);
	control |= PCI_EXP_DEVCTL_BCR_FLR;
	pci_write_config_word(dev, pos + PCI_EXP_DEVCTL, control);

	shannon_msleep(100);

	return 0;
}

static int shannon_pci_af_flr(struct pci_dev *dev)
{
	int i;
	int pos;
	u8 cap;
	u8 status;

	pos = pci_find_capability(dev, PCI_CAP_ID_AF);
	if (!pos)
		return -ENOTTY;

	pci_read_config_byte(dev, pos + PCI_AF_CAP, &cap);
	if (!(cap & PCI_AF_CAP_TP) || !(cap & PCI_AF_CAP_FLR))
		return -ENOTTY;

	/* Wait for Transaction Pending bit clean */
	for (i = 0; i < 4; i++) {
		if (i)
			shannon_msleep((1 << (i - 1)) * 100);

		pci_read_config_byte(dev, pos + PCI_AF_STATUS, &status);
		if (!(status & PCI_AF_STATUS_TP))
			goto clear;
	}

	dev_err(&dev->dev, "transaction is not cleared; "
			"proceeding with reset anyway\n");

clear:
	pci_write_config_byte(dev, pos + PCI_AF_CTRL, PCI_AF_CTRL_FLR);
	shannon_msleep(100);

	return 0;
}

static void shannon_pci_dev_d3_sleep(struct pci_dev *dev)
{
	unsigned int delay = 100;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	delay = dev->d3_delay;
#endif
	shannon_msleep(delay);
}

static int shannon_pci_pm_reset(struct pci_dev *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	u16 csr;

	if (!dev->pm_cap)
		return -ENOTTY;

	pci_read_config_word(dev, dev->pm_cap + PCI_PM_CTRL, &csr);
	if (csr & PCI_PM_CTRL_NO_SOFT_RESET)
		return -ENOTTY;

	if (dev->current_state != PCI_D0)
		return -EINVAL;

	csr &= ~PCI_PM_CTRL_STATE_MASK;
	csr |= PCI_D3hot;
	pci_write_config_word(dev, dev->pm_cap + PCI_PM_CTRL, csr);
	shannon_pci_dev_d3_sleep(dev);

	csr &= ~PCI_PM_CTRL_STATE_MASK;
	csr |= PCI_D0;
	pci_write_config_word(dev, dev->pm_cap + PCI_PM_CTRL, csr);
	shannon_pci_dev_d3_sleep(dev);
#endif
	return -ENOTTY;
}

static int shannon_pci_parent_bus_reset(struct pci_dev *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	u16 ctrl;
	struct pci_dev *pdev;

	if (pci_is_root_bus(dev->bus) || dev->subordinate || !dev->bus->self)
		return -ENOTTY;

	list_for_each_entry(pdev, &dev->bus->devices, bus_list)
		if (pdev != dev)
			return -ENOTTY;

	pci_read_config_word(dev->bus->self, PCI_BRIDGE_CONTROL, &ctrl);
	ctrl |= PCI_BRIDGE_CTL_BUS_RESET;
	pci_write_config_word(dev->bus->self, PCI_BRIDGE_CONTROL, ctrl);
	shannon_msleep(100);

	ctrl &= ~PCI_BRIDGE_CTL_BUS_RESET;
	pci_write_config_word(dev->bus->self, PCI_BRIDGE_CONTROL, ctrl);
	shannon_msleep(100);
#endif
	return -ENOTTY;
}
#endif

int shannon_pci_get_max_linkwidth(struct shannon_pci_info *info)
{
	return info ? ((info->lnkcap >> 4) & 0x3f) : 0;
}

int shannon_pci_get_cur_linkwidth(struct shannon_pci_info *info)
{
	return info ? ((info->lnksta >> 4) & 0x3f) : 0;
}

int shannon_pci_get_max_linkspeed(struct shannon_pci_info *info)
{
	return info ? (info->lnkcap & 0xf) : 0;
}

int shannon_pci_get_cur_linkspeed(struct shannon_pci_info *info)
{
	return info ? (info->lnksta & 0xf) : 0;
}

int shannon_pci_reset_function(shannon_pci_dev_t *pdev)
{
	int rc = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)

	if (!pdev)
		return -1;

	pci_save_state(pdev);
	pci_write_config_word(pdev, PCI_COMMAND, PCI_COMMAND_INTX_DISABLE);

	rc = shannon_pcie_flr(pdev);
	if (rc != -ENOTTY)
		goto done;

	rc = shannon_pci_af_flr(pdev);
	if (rc != -ENOTTY)
		goto done;

	rc = shannon_pci_pm_reset(pdev);
	if (rc != -ENOTTY)
		goto done;

	// if device`s pci bus only have this device, reset it!
	rc = shannon_pci_parent_bus_reset(pdev);
done:
	pci_restore_state(pdev);
#endif
	return rc;
}

int shannon_pci_get_node(shannon_pci_dev_t *pdev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	return dev_to_node(&(((struct pci_dev *)pdev)->dev));
#else
	return -1;
#endif
}
