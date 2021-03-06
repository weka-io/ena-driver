/*-
 * BSD LICENSE
 *
 * Copyright (c) 2015-2017 Amazon.com, Inc. or its affiliates.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * * Neither the name of copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ENA_H
#define	ENA_H

#include <sys/types.h>

#include "ena_com/ena_com.h"
#include "ena_com/ena_eth_com.h"

/* 1 for AENQ + ADMIN */
#define	ENA_MAX_MSIX_VEC(io_queues)	(1 + (io_queues))

#define	ENA_REG_BAR			0
#define	ENA_MEM_BAR			2

#define	ENA_BUS_DMA_SEGS		32

#define	ENA_DEFAULT_RING_SIZE		1024
#define	ENA_DEFAULT_SMALL_PACKET_LEN	128
#define	ENA_DEFAULT_MAX_RX_BUFF_ALLOC_SIZE	1536
/*
 * minimum the buffer size to 600 to avoid situation the mtu will be changed
 * from too little buffer to very big one and then the number of buffer per
 * packet could reach the maximum ENA_PKT_MAX_BUFS
 */
#define	ENA_DEFAULT_MIN_RX_BUFF_ALLOC_SIZE 600

#define	ENA_RX_REFILL_THRESH_DEVIDER	8

#define	ENA_MAX_PUSH_PKT_SIZE		128

#define	ENA_NAME_MAX_LEN		20
#define	ENA_IRQNAME_SIZE		40

#define	ENA_PKT_MAX_BUFS 		19
#define	ENA_STALL_TIMEOUT		100

#define	ENA_RX_RSS_TABLE_LOG_SIZE	7
#define	ENA_RX_RSS_TABLE_SIZE		(1 << ENA_RX_RSS_TABLE_LOG_SIZE)

#define	ENA_HASH_KEY_SIZE		40

#define	ENA_DMA_BITS_MASK		40
#define	ENA_MAX_FRAME_LEN		10000
#define	ENA_MIN_FRAME_LEN 		60
#define	ENA_RX_HASH_KEY_NUM		10
#define	ENA_RX_THASH_TABLE_SIZE 	(1 << 8)

#define DB_TRESHOLD	16

/*
 * By default we continue to reclaim TX descriptors until none are left.
 * TX cleanup commit budget
 */
#define TX_BUDGET	32
/* RX cleanup budget. -1 stands for infinity. */
#define RX_BUDGET	256

#define RX_IRQ_INTERVAL 20
#define TX_IRQ_INTERVAL 50

#define	ENA_MAX_MTU		9216
#define	ENA_TSO_MAXSIZE		PAGE_SIZE
#define	ENA_TSO_NSEGS		ENA_PKT_MAX_BUFS
#define	ENA_RX_OFFSET		NET_SKB_PAD + NET_IP_ALIGN

#define	ENA_MMIO_DISABLE_REG_READ	BIT(0)

#define	ENA_TX_RING_IDX_NEXT(idx, ring_size) (((idx) + 1) & ((ring_size) - 1))

#define	ENA_RX_RING_IDX_NEXT(idx, ring_size) (((idx) + 1) & ((ring_size) - 1))
#define	ENA_RX_RING_IDX_ADD(idx, n, ring_size)	\
	(((idx) + (n)) & ((ring_size) - 1))

#define	ENA_IO_TXQ_IDX(q)		(2 * (q))
#define	ENA_IO_RXQ_IDX(q)		(2 * (q) + 1)

#define	ENA_MGMNT_IRQ_IDX		0
#define	ENA_IO_IRQ_FIRST_IDX		1
#define	ENA_IO_IRQ_IDX(q)		(ENA_IO_IRQ_FIRST_IDX + (q))

/*
 * Supported PCI vendor and devices IDs
 */
#define	PCI_VENDOR_ID_AMAZON	0x1d0f

#define	PCI_DEV_ID_ENA_PF	0x0ec2
#define	PCI_DEV_ID_ENA_LLQ_PF	0x1ec2
#define	PCI_DEV_ID_ENA_VF	0xec20
#define	PCI_DEV_ID_ENA_LLQ_VF	0xec21

struct msix_entry {
	int entry;
	int vector;
};

typedef struct _ena_vendor_info_t {
	unsigned int vendor_id;
	unsigned int device_id;
	unsigned int index;
} ena_vendor_info_t;

struct ena_irq {
	/* Interrupt resources */
	struct resource *res;
	driver_filter_t *handler;
	void *data;
	void *cookie;
	unsigned int vector;
	bool requested;
	int cpu;
	char name[ENA_IRQNAME_SIZE];
};

struct ena_que {
	struct ena_adapter *adapter;
	struct ena_ring *tx_ring;
	struct ena_ring *rx_ring;
	uint32_t id;
	int cpu;
};

struct ena_tx_buffer {
	struct mbuf *mbuf;
	/* # of ena desc for this specific mbuf
	 * (includes data desc and metadata desc) */
	unsigned int tx_descs;
	/* # of buffers used by this mbuf */
	unsigned int num_of_bufs;
	bus_dmamap_t map;

	struct ena_com_buf bufs[ENA_PKT_MAX_BUFS];
} __aligned(CACHE_LINE_SIZE);

struct ena_rx_buffer {
	struct mbuf *mbuf;
	unsigned int data_size;
	bus_dmamap_t map;
	struct ena_com_buf ena_buf;
} __aligned(CACHE_LINE_SIZE);

#ifdef ENA_DEBUG
struct ena_current_state {
	uint64_t rx_sq_state;
	uint64_t tx_sq_state;
};
#endif

struct ena_stats_tx {
	counter_u64_t cnt;
	counter_u64_t bytes;
	counter_u64_t queue_stop;
	counter_u64_t prepare_ctx_err;
	counter_u64_t queue_wakeup;
	counter_u64_t dma_mapping_err;
	/* Not counted */
	counter_u64_t unsupported_desc_num;
	/* Not counted */
	counter_u64_t napi_comp;
	/* Not counted */
	counter_u64_t tx_poll;
	counter_u64_t doorbells;
	counter_u64_t missing_tx_comp;
	counter_u64_t bad_req_id;
};

struct ena_stats_rx {
	counter_u64_t cnt;
	counter_u64_t bytes;
	counter_u64_t refil_partial;
	counter_u64_t bad_csum;
	/* Not counted */
	counter_u64_t page_alloc_fail;
	counter_u64_t mbuf_alloc_fail;
	counter_u64_t dma_mapping_err;
	counter_u64_t bad_desc_num;
	/* Not counted */
	counter_u64_t small_copy_len_pkt;
};


struct ena_ring {
	/* Holds the empty requests for TX out of order completions */
	uint16_t *free_tx_ids;
	struct ena_com_dev *ena_dev;
	struct ena_adapter *adapter;
	struct ena_com_io_cq *ena_com_io_cq;
	struct ena_com_io_sq *ena_com_io_sq;

	/* The maximum length the driver can push to the device (For LLQ) */
	enum ena_admin_placement_policy_type tx_mem_queue_type;
	uint16_t rx_small_copy_len;
	uint16_t qid;
	uint16_t mtu;
	uint8_t tx_max_header_size;

	struct ena_com_rx_buf_info ena_bufs[ENA_PKT_MAX_BUFS];
	uint32_t  smoothed_interval;
	enum ena_intr_moder_level moder_tbl_idx;

	struct ena_que *que;
	struct lro_ctrl lro;

	uint16_t next_to_use;
	uint16_t next_to_clean;

	union {
		struct ena_tx_buffer *tx_buffer_info; /* contex of tx packet */
		struct ena_rx_buffer *rx_buffer_info; /* contex of rx packet */
	};
	int ring_size; /* number of tx/rx_buffer_info's entries */

	struct buf_ring *br; /* only for TX */
	struct mtx ring_mtx;
	char mtx_name[16];
	struct task enqueue_task;
	struct taskqueue *enqueue_tq;
	struct task cmpl_task;
	struct taskqueue *cmpl_tq;

	bus_dma_tag_t buf_tag;

	union {
		struct ena_stats_tx tx_stats;
		struct ena_stats_rx rx_stats;
	};

#ifdef ENA_DEBUG
	struct ena_current_state state;
#endif
} __aligned(CACHE_LINE_SIZE);

struct ena_stats_dev {
	/* Not counted */
	counter_u64_t tx_timeout;
	/* Not counted */
	counter_u64_t io_suspend;
	/* Not counted */
	counter_u64_t io_resume;
	/* Not counted */
	counter_u64_t wd_expired;
	counter_u64_t interface_up;
	counter_u64_t interface_down;
	/* Not counted */
	counter_u64_t admin_q_pause;
};

struct ena_hw_stats {
	uint64_t rx_packets;
	uint64_t tx_packets;

	uint64_t rx_bytes;
	uint64_t tx_bytes;

	uint64_t rx_drops;
};

/* Board specific private data structure */
struct ena_adapter {
	struct ena_com_dev *ena_dev;

	/* OS defined structs */
	if_t ifp;
	device_t pdev;
	struct ifmedia	media;

	/* OS resources */
	struct resource * memory;
	struct resource * registers;

	struct mtx global_mtx;

	/* MSI-X */
	uint32_t msix_enabled;
	struct msix_entry *msix_entries;
	int msix_vecs;

	/*
	 * RX packets that shorter that this len will be copied to the skb
	 * header
	 */
	unsigned int small_copy_len;

	/* Size of rx mbuf */
	uint32_t rx_mbuf_sz;

	uint16_t max_tx_sgl_size;
	uint16_t max_rx_sgl_size;

	uint32_t tx_offload_cap;

	/* Tx fast path data */
	int num_queues;

	unsigned int tx_usecs, rx_usecs; /* Interrupt coalescing */

	unsigned int tx_ring_size;
	unsigned int rx_ring_size;

	/* RSS*/
	uint8_t	 rss_ind_tbl[ENA_RX_RSS_TABLE_SIZE];
	bool rss_support;

	uint32_t msg_enable;

	uint8_t mac_addr[ETHER_ADDR_LEN];
	/* mdio and phy*/

	char name[ENA_NAME_MAX_LEN];
	bool link_status;

	bool up;

	uint32_t wol;

	/* Queue will represent one TX and one RX ring */
	struct ena_que que[ENA_MAX_NUM_IO_QUEUES]
	    __aligned(CACHE_LINE_SIZE);

	/* TX */
	struct ena_ring tx_ring[ENA_MAX_NUM_IO_QUEUES]
	    __aligned(CACHE_LINE_SIZE);

	/* RX */
	struct ena_ring rx_ring[ENA_MAX_NUM_IO_QUEUES]
	    __aligned(CACHE_LINE_SIZE);

	struct ena_irq irq_tbl[ENA_MAX_MSIX_VEC(ENA_MAX_NUM_IO_QUEUES)];

	/* Timer service (not implemented yet) */
	//struct callout timer_service;

	/* Statistics */
	struct ena_stats_dev dev_stats;
	struct ena_hw_stats hw_stats;
};


#define	ENA_DEV_LOCK			mtx_lock(&adapter->global_mtx)
#define	ENA_DEV_UNLOCK			mtx_unlock(&adapter->global_mtx)

#define	ENA_RING_MTX_LOCK(_ring)		mtx_lock(&(_ring)->ring_mtx)
#define	ENA_RING_MTX_TRYLOCK(_ring)		mtx_trylock(&(_ring)->ring_mtx)
#define	ENA_RING_MTX_UNLOCK(_ring)		mtx_unlock(&(_ring)->ring_mtx)

struct ena_dev *ena_efa_enadev_get(device_t pdev);

int ena_register_adapter(struct ena_adapter *adapter);
void ena_unregister_adapter(struct ena_adapter *adapter);

int ena_update_stats_counters(struct ena_adapter *adapter);

#endif /* !(ENA_H) */
