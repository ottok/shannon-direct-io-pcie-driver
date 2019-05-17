#ifndef __SHANNON_SCSI_H
#define __SHANNON_SCSI_H

#include "shannon_workqueue.h"

#define SHANNON_SCSI_SENSE_LEN	32

/*
 * Host byte codes
 */

#define SHN_DID_OK          0x00	/* NO error                                */
#define SHN_DID_NO_CONNECT  0x01	/* Couldn't connect before timeout period  */
#define SHN_DID_BUS_BUSY    0x02	/* BUS stayed busy through time out period */
#define SHN_DID_TIME_OUT    0x03	/* TIMED OUT for other reason              */
#define SHN_DID_BAD_TARGET  0x04	/* BAD target.                             */
#define SHN_DID_ABORT       0x05	/* Told to abort for some other reason     */
#define SHN_DID_PARITY      0x06	/* Parity error                            */
#define SHN_DID_ERROR       0x07	/* Internal error                          */
#define SHN_DID_RESET       0x08	/* Reset by somebody.                      */
#define SHN_DID_BAD_INTR    0x09	/* Got an interrupt we weren't expecting.  */
#define SHN_DID_PASSTHROUGH 0x0a	/* Force command past mid-layer            */
#define SHN_DID_SOFT_ERROR  0x0b	/* The low level driver just wish a retry  */
#define SHN_DID_IMM_RETRY   0x0c	/* Retry without decrementing retry count  */
#define SHN_DID_REQUEUE	0x0d	/* Requeue command (no immediate retry) also
				 * without decrementing the retry count	   */
#define SHN_DID_TRANSPORT_DISRUPTED 0x0e /* Transport error disrupted execution
				      * and the driver blocked the port to
				      * recover the link. Transport class will
				      * retry or fail IO */
#define SHN_DID_TRANSPORT_FAILFAST	0x0f /* Transport class fastfailed the io */
#define SHN_DID_TARGET_FAILURE 0x10 /* Permanent target failure, do not retry on
				 * other paths */
#define SHN_DID_NEXUS_FAILURE 0x11  /* Permanent nexus failure, retry on other
				 * paths might yield different results */
#define SHN_DRIVER_OK       0x00	/* Driver status                           */

/*
 *  These indicate the error that occurred, and what is available.
 */

#define SHN_DRIVER_BUSY         0x01
#define SHN_DRIVER_SOFT         0x02
#define SHN_DRIVER_MEDIA        0x03
#define SHN_DRIVER_ERROR        0x04

#define SHN_DRIVER_INVALID      0x05
#define SHN_DRIVER_TIMEOUT      0x06
#define SHN_DRIVER_HARD         0x07
#define SHN_DRIVER_SENSE	    0x08

/*
 *  SENSE KEYS
 */

#define SENSE_KEY_NO_SENSE            0x00
#define SENSE_KEY_RECOVERED_ERROR     0x01
#define SENSE_KEY_NOT_READY           0x02
#define SENSE_KEY_MEDIUM_ERROR        0x03
#define SENSE_KEY_HARDWARE_ERROR      0x04
#define SENSE_KEY_ILLEGAL_REQUEST     0x05
#define SENSE_KEY_UNIT_ATTENTION      0x06
#define SENSE_KEY_DATA_PROTECT        0x07
#define SENSE_KEY_BLANK_CHECK         0x08
#define SENSE_KEY_COPY_ABORTED        0x0a
#define SENSE_KEY_ABORTED_COMMAND     0x0b
#define SENSE_KEY_VOLUME_OVERFLOW     0x0d
#define SENSE_KEY_MISCOMPARE          0x0e

/* Additional Sense Code (ASC) */
#define ASC_NO_ADDITIONAL_SENSE				0x00
#define ASC_PERIPHERAL_DEV_WRITE_FAULT			0x03
#define ASC_LOGICAL_UNIT_NOT_READY			0x04
#define ASC_WARNING					0x0B
#define ASC_LOG_BLOCK_GUARD_CHECK_FAILED		0x10
#define ASC_LOG_BLOCK_APPTAG_CHECK_FAILED		0x10
#define ASC_LOG_BLOCK_REFTAG_CHECK_FAILED		0x10
#define ASC_UNRECOVERED_READ_ERR			0x11
#define ASC_PARAMETER_LIST_LENGTH_ERR			0x1a
#define ASC_MISCOMPARE_DURING_VERIFY			0x1D
#define ASC_INVALID_OPCODE				0x20
#define ASC_ADDR_OUT_OF_RANGE				0x21
#define ASC_INVALID_COMMAND_OPCODE			0x20
#define ASC_INVALID_FIELD_IN_CDB			0x24
#define ASC_INVALID_LUN					0x25
#define ASC_INVALID_FIELD_IN_PARAM_LIST			0x26
#define ASC_POWERON_RESET				0x29
#define ASC_FORMAT_COMMAND_FAILED			0x31
#define ASC_SAVING_PARAMS_UNSUP				0x39
#define ASC_INTERNAL_TARGET_FAILURE			0x44
#define ASC_TRANSPORT_PROBLEM				0x4b
#define ASC_THRESHOLD_EXCEEDED				0x5d
#define ASC_LOW_POWER_COND_ON				0x5e

/* Additional Sense Code Qualifier (ASCQ) */
#define ASCQ_CAUSE_NOT_REPORTABLE                  0x00
#define ASCQ_FORMAT_COMMAND_FAILED                 0x01
#define ASCQ_LOG_BLOCK_GUARD_CHECK_FAILED          0x01
#define ASCQ_LOG_BLOCK_APPTAG_CHECK_FAILED         0x02
#define ASCQ_LOG_BLOCK_REFTAG_CHECK_FAILED         0x03
#define ASCQ_ACK_NAK_TO                            0x03
#define ASCQ_FORMAT_IN_PROGRESS                    0x04
#define ASCQ_POWER_LOSS_EXPECTED                   0x08
#define ASCQ_INVALID_LUN_ID                        0x09

/* SCSI status values. SAM3r14 section 5.3.1 table 22. */
enum shannon_scsi_cmd_status
{
	STATUS_CODE_GOOD				= 0x00,
	STATUS_CODE_CHECK_CONDITION			= 0x02,
	STATUS_CODE_CONDITION_MET			= 0x04,
	STATUS_CODE_BUSY				= 0x08,
	STATUS_CODE_RESERVATION_CONFLICT		= 0x18,
	STATUS_CODE_TASK_SET_FULL			= 0x28,
	STATUS_CODE_ACA_ACTIVE				= 0x30,
	STATUS_CODE_TASK_ABORTED			= 0x40,
};

#define CHECK_CONDITION_STATUS	((SHN_DRIVER_SENSE << 24) | STATUS_CODE_CHECK_CONDITION)

struct shannon_scsi_private {
	void *scsi_host;
	void *sdev;

	shannon_spinlock_t stats_lock;
	unsigned long sectors[2];       /* READs and WRITEs */
	unsigned long ios[2];
	unsigned long ticks[2];
	unsigned long in_flight[2];

	int scsi_host_no;
};

extern int shannon_fill_from_dev_buffer(void *scsi_cmnd, unsigned char *arr, int arr_len);
extern void shannon_build_sense_buffer(char *sbuff, int key, int asc, int asq);
extern int shannon_attach_scsi(struct shannon_scsi_private *hostdata, void *data);
extern void shannon_detach_scsi(struct shannon_scsi_private *hostdata);
extern int shannon_scsi_disk_in_flight(struct shannon_scsi_private *hostdata);
extern unsigned long shannon_scsi_sectors(struct shannon_scsi_private *hostdata, int write);
extern unsigned long shannon_scsi_ios(struct shannon_scsi_private *hostdata, int write);
extern unsigned long shannon_scsi_msecs(struct shannon_scsi_private *hostdata, int write);

#endif /* __SHANNON_SCSI_H */
