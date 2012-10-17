/*
 * Copyright (c) 2005 Topspin Communications.  All rights reserved.
 * Copyright (c) 2005 Mellanox Technologies Ltd.  All rights reserved.
 * Copyright (c) 2008 Intel Corporation.  All rights reserved.
 *
 * This software is available to you under the OpenIB.org BSD license
 * below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AWV
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <time.h>

#include "getopt.c"
#include "perftest.h"
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

#define PINGPONG_RDMA_WRID	3

struct pingpong_context {
	struct ibv_context *context;
	struct ibv_pd      *pd;
	struct ibv_mr      *mr;
	struct ibv_cq      *rcq;
	struct ibv_cq      *scq;
	struct ibv_qp      *qp;
	struct ibv_comp_channel *ch;
	void               *buf;
	unsigned            size;
	int                 tx_depth;
	struct ibv_sge      list;
	struct ibv_send_wr  wr;
};

struct pingpong_dest {
	int lid;
	int qpn;
	int psn;
	unsigned rkey;
	unsigned long long vaddr;
};

struct pp_data {
	int							port;
	int							ib_port;
	unsigned            		size;
	int                 		tx_depth;
	int							use_cma;
	SOCKET 		    			sockfd;
	char						*servername;
	struct pingpong_dest		my_dest;
	struct pingpong_dest 		*rem_dest;
	struct ibv_device			*ib_dev;
	struct rdma_event_channel 	*cm_channel;
	struct rdma_cm_id 			*cm_id;
};

static void pp_post_recv(struct pingpong_context *);
static void pp_wait_for_done(struct pingpong_context *);
static void pp_send_done(struct pingpong_context *);
static void pp_wait_for_start(struct pingpong_context *);
static void pp_send_start(struct pingpong_context *);
static void pp_close_cma(struct pp_data );
static struct pingpong_context *pp_init_ctx(void *, struct pp_data *);

static uint16_t pp_get_local_lid(struct pingpong_context *ctx, int port)
{
	struct ibv_port_attr attr;

	if (ibv_query_port(ctx->context, (uint8_t) port, &attr))
		return 0;

	return attr.lid;
}

static struct pingpong_context *pp_client_connect_cma(struct pp_data *data)
{
	struct addrinfo *res;
	struct addrinfo hints;
	char service[6];
	int n;
	SOCKET sockfd = INVALID_SOCKET;
	struct rdma_cm_event *event;
	struct sockaddr_in sin;
	struct pingpong_context *ctx = NULL;
	struct rdma_conn_param conn_param;

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(service, "%d", data->port);

	n = getaddrinfo(data->servername, service, &hints, &res);

	if (n != 0) {
		fprintf(stderr, "%s for %s:%d\n", gai_strerror(n), data->servername, data->port);
		goto err1;
	}

	sin.sin_addr.s_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
	sin.sin_family = AF_INET;
	sin.sin_port = htons((u_short) data->port);
	if (rdma_resolve_addr(data->cm_id, NULL,
				 (struct sockaddr *)&sin, 2000)) {
		fprintf(stderr, "rdma_resolve_addr failed\n");
		goto err2;
	}

	if (rdma_get_cm_event(data->cm_channel, &event)) 
		goto err2;

	if (event->event != RDMA_CM_EVENT_ADDR_RESOLVED) {
		fprintf(stderr, "unexpected CM event resolving addr %d\n", event->event);
		goto err3;
	}
	rdma_ack_cm_event(event);

	if (rdma_resolve_route(data->cm_id, 2000)) {
		fprintf(stderr, "rdma_resolve_route failed\n");
		goto err2;
	}

	if (rdma_get_cm_event(data->cm_channel, &event))
		goto err2;

	if (event->event != RDMA_CM_EVENT_ROUTE_RESOLVED) {
		fprintf(stderr, "unexpected CM event resolving route %d\n", event->event);
		goto err3;
	}
	rdma_ack_cm_event(event);
	ctx = pp_init_ctx(data->cm_id, data);
	if (!ctx) {
		fprintf(stderr, "pp_init_ctx failed\n");
		goto err2;
	}
	data->my_dest.psn = rand() & 0xffffff;
	data->my_dest.qpn = 0;
	data->my_dest.rkey = ctx->mr->rkey;
	data->my_dest.vaddr = (uintptr_t)ctx->buf + ctx->size;

	memset(&conn_param, 0, sizeof conn_param);
	conn_param.responder_resources = 1;
	conn_param.initiator_depth = 1;
	conn_param.retry_count = 5;
	conn_param.private_data = &data->my_dest;
	conn_param.private_data_len = sizeof(data->my_dest);

	if (rdma_connect(data->cm_id, &conn_param)) {
		fprintf(stderr, "rdma_connect failure\n");
		goto err2;
	}

	if (rdma_get_cm_event(data->cm_channel, &event))
		goto err2;

	if (event->event != RDMA_CM_EVENT_ESTABLISHED) {
		fprintf(stderr, "unexpected CM event connecting %d\n", event->event);
		goto err3;
	}
	if (!event->param.conn.private_data || 
	    (event->param.conn.private_data_len < sizeof(*data->rem_dest))) {
		fprintf(stderr, "bad private data ptr %p len %d\n",
			event->param.conn.private_data, 
			event->param.conn.private_data_len);
		goto err3;
	}
	data->rem_dest = malloc(sizeof *data->rem_dest);
	if (!data->rem_dest)
		goto err3;
	
	memcpy(data->rem_dest, event->param.conn.private_data, sizeof(*data->rem_dest));
	rdma_ack_cm_event(event);
	freeaddrinfo(res);
	return ctx;

err3:
	rdma_ack_cm_event(event);
err2:
	freeaddrinfo(res);
err1: 
	return NULL;
}

static struct pingpong_context *pp_client_connect_socket(struct pp_data *data)
{
	data->sockfd = pp_client_connect(data->servername, data->port);
	if (data->sockfd == INVALID_SOCKET) {
		return NULL;
	}

	return pp_init_ctx(data->ib_dev, data);
}

static struct pingpong_context *pp_rdma_client_connect(struct pp_data *data)
{
	return data->use_cma ? pp_client_connect_cma(data) : pp_client_connect_socket(data);
}

static int pp_client_exch_dest(struct pp_data *data)
{
	char msg[sizeof "0000:000000:000000:00000000:0000000000000000"];
	int parsed;
	
	if (!data->use_cma) {
		sprintf(msg, "%04x:%06x:%06x:%08x:%016Lx", data->my_dest.lid, 
				data->my_dest.qpn, data->my_dest.psn,
				data->my_dest.rkey, data->my_dest.vaddr);
		if (send(data->sockfd, msg, sizeof msg, 0) != sizeof msg) {
			perror("client write");
			fprintf(stderr, "Couldn't send local address\n");
			goto err;
		}
	
		if (recv(data->sockfd, msg, sizeof msg, 0) != sizeof msg) {
			perror("client read");
			fprintf(stderr, "Couldn't read remote address\n");
			goto err;
		}
	
 		if (data->rem_dest != NULL)
			free(data->rem_dest);
		data->rem_dest = malloc(sizeof *data->rem_dest);
		if (!data->rem_dest)
			goto err;
	
		memset(data->rem_dest, 0, sizeof *data->rem_dest);
		parsed = sscanf(msg, "%x:%x:%x:%x:%Lx", &data->rem_dest->lid,
				&data->rem_dest->qpn, &data->rem_dest->psn,
				&data->rem_dest->rkey, &data->rem_dest->vaddr);
	
		if (parsed != 5) {
			fprintf(stderr, "Couldn't parse line <%.*s>\n", (int)sizeof msg, msg);
			free(data->rem_dest);
			goto err;
		}
	}
	return 0;
err:
	return 1;
}

static struct pingpong_context *pp_server_connect_cma(struct pp_data *data)
{
	struct addrinfo *res;
	struct addrinfo hints;
	char service[6];
	SOCKET sockfd = INVALID_SOCKET;
	int n;
	struct rdma_cm_event *event, *accept_event;
	struct sockaddr_in sin;
	struct pingpong_context *ctx = NULL;
	struct rdma_cm_id *child_cm_id;
	struct rdma_conn_param conn_param;

	memset(&hints, 0, sizeof hints);
	hints.ai_flags    = AI_PASSIVE;
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(service, "%d", data->port);

	if ( (n = getaddrinfo(NULL, service, &hints, &res)) != 0 ) {
		fprintf(stderr, "%s for port %d\n", gai_strerror(n), data->port);
		goto err1;
	}

	sin.sin_addr.s_addr = 0;
	sin.sin_family = AF_INET;
	sin.sin_port = htons((u_short) data->port);
	if (rdma_bind_addr(data->cm_id, (struct sockaddr *)&sin)) {
		fprintf(stderr, "rdma_bind_addr failed\n");
		goto err2;
	}

	if (rdma_listen(data->cm_id, 0)) {
		fprintf(stderr, "rdma_listen failed\n");
		goto err2;
	}

	if (rdma_get_cm_event(data->cm_channel, &event)) 
		goto err2;

	if (event->event != RDMA_CM_EVENT_CONNECT_REQUEST) {
		fprintf(stderr, "bad event waiting for connect request %d\n", event->event);
		goto err3;
	}

	if (!event->param.conn.private_data ||
	    (event->param.conn.private_data_len < sizeof(*data->rem_dest))) {
		fprintf(stderr, "bad private data len %d\n", event->param.conn.private_data_len);
		goto err3;
	}
	
	child_cm_id = (struct rdma_cm_id *)event->id;
	data->rem_dest = malloc(sizeof *data->rem_dest);
	if (!data->rem_dest)
		goto err4;

	memcpy(data->rem_dest, event->param.conn.private_data, sizeof(*data->rem_dest));

	ctx = pp_init_ctx(child_cm_id, data);
	if (!ctx) {
		goto err5;
	}
	data->my_dest.psn = rand() & 0xffffff;
	data->my_dest.qpn = 0;
	data->my_dest.rkey = ctx->mr->rkey;
	data->my_dest.vaddr = (uintptr_t)ctx->buf + ctx->size;

	memset(&conn_param, 0, sizeof conn_param);
	conn_param.responder_resources = 1;
	conn_param.initiator_depth = 1;
	conn_param.private_data = &data->my_dest;
	conn_param.private_data_len = sizeof(data->my_dest);
	if (rdma_accept(child_cm_id, &conn_param)) {
		fprintf(stderr, "rdma_accept failed\n");
		goto err5;
	}	
	if (rdma_get_cm_event(data->cm_channel, &accept_event)) {
		fprintf(stderr, "rdma_get_cm_event error\n");
		goto err5;
	}
	if (accept_event->event != RDMA_CM_EVENT_ESTABLISHED) {
		fprintf(stderr, "bad event waiting for established %d\n", event->event);
		goto err6;
	}
	rdma_ack_cm_event(event);
	rdma_ack_cm_event(accept_event);	
	freeaddrinfo(res);
	return ctx;

err6:
	rdma_ack_cm_event(accept_event);
err5:
	free(data->rem_dest);
err4:
	rdma_destroy_id(child_cm_id);
err3:
	rdma_ack_cm_event(event);
err2:
	freeaddrinfo(res);
err1:
	return NULL;
}

static struct pingpong_context *pp_server_connect_socket(struct pp_data *data)
{
	data->sockfd = pp_server_connect(data->port);
	if (data->sockfd == INVALID_SOCKET) {
		return NULL;
	}

	return pp_init_ctx(data->ib_dev, data);
}

static struct pingpong_context *pp_rdma_server_connect(struct pp_data *data)
{
	return data->use_cma ? pp_server_connect_cma(data) : pp_server_connect_socket(data);
}

static int pp_server_exch_dest(struct pp_data *data)
{
	char msg[sizeof "0000:000000:000000:00000000:0000000000000000"];
	int parsed;
	int n;
	
	if (!data->use_cma) {
		n = recv(data->sockfd, msg, sizeof msg, 0);
		if (n != sizeof msg) {
			perror("server read");
			fprintf(stderr, "%d/%d Couldn't read remote address\n", 
						n, (int) sizeof msg);
			goto err;
		}
	
		if (data->rem_dest != NULL)
			free(data->rem_dest);
		data->rem_dest = malloc(sizeof *data->rem_dest);
		if (!data->rem_dest)
			goto err;
	
		memset(data->rem_dest, 0, sizeof *data->rem_dest);
		parsed = sscanf(msg, "%x:%x:%x:%x:%Lx", &data->rem_dest->lid,
			      &data->rem_dest->qpn, &data->rem_dest->psn,
			      &data->rem_dest->rkey, &data->rem_dest->vaddr);
		if (parsed != 5) {
			fprintf(stderr, "Couldn't parse line <%.*s>\n", (int)sizeof msg, msg);
			free(data->rem_dest);
			goto err;
		}
	
		sprintf(msg, "%04x:%06x:%06x:%08x:%016Lx", data->my_dest.lid,
					 data->my_dest.qpn, data->my_dest.psn,
					 data->my_dest.rkey, data->my_dest.vaddr);
		if (send(data->sockfd, msg, sizeof msg, 0) != sizeof msg) {
			perror("server write");
			fprintf(stderr, "%Couldn't send local address\n");
			free(data->rem_dest);
			goto err;
		}
	}

	return 0;
err:
	return 1;
}

static struct pingpong_context *pp_init_ctx(void *ptr, struct pp_data *data)
{
	struct pingpong_context *ctx;
	struct ibv_device *ib_dev;
	struct rdma_cm_id *cm_id;
	struct ibv_qp_init_attr attr;

	ctx = malloc(sizeof *ctx);
	if (!ctx)
		return NULL;

	ctx->size     = data->size;
	ctx->tx_depth = data->tx_depth;

	ctx->buf = malloc(ctx->size * 2);
	if (!ctx->buf) {
		fprintf(stderr, "Couldn't allocate work buf.\n");
		return NULL;
	}

	memset(ctx->buf, 0, ctx->size * 2);

	if (data->use_cma) {
		cm_id = (struct rdma_cm_id *)ptr;
		ctx->context = cm_id->verbs;
		if (!ctx->context) {
			fprintf(stderr, "Unbound cm_id!!\n");
			return NULL;
		}
	} else {
		ib_dev = (struct ibv_device *)ptr;
		ctx->context = ibv_open_device(ib_dev);
		if (!ctx->context) {
			fprintf(stderr, "Couldn't get context for %s\n", ibv_get_device_name(ib_dev));
			return NULL;
		}
	}

	ctx->pd = ibv_alloc_pd(ctx->context);
	if (!ctx->pd) {
		fprintf(stderr, "Couldn't allocate PD\n");
		return NULL;
	}

        /* We dont really want IBV_ACCESS_LOCAL_WRITE, but IB spec says:
         * The Consumer is not allowed to assign Remote Write or Remote Atomic to
         * a Memory Region that has not been assigned Local Write. */
	ctx->mr = ibv_reg_mr(ctx->pd, ctx->buf, ctx->size * 2,
			     IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE);
	if (!ctx->mr) {
		fprintf(stderr, "Couldn't allocate MR\n");
		return NULL;
	}

	ctx->ch = ibv_create_comp_channel(ctx->context);
	if (!ctx->ch) {
		fprintf(stderr, "Couldn't create comp channel\n");
		return NULL;
	}

	ctx->rcq = ibv_create_cq(ctx->context, 1, NULL, NULL, 0);
	if (!ctx->rcq) {
		fprintf(stderr, "Couldn't create recv CQ\n");
		return NULL;
	}

	ctx->scq = ibv_create_cq(ctx->context, ctx->tx_depth, ctx, ctx->ch, 0);
	if (!ctx->scq) {
		fprintf(stderr, "Couldn't create send CQ\n");
		return NULL;
	}

	memset(&attr, 0, sizeof attr);
	attr.send_cq = ctx->scq;
	attr.recv_cq = ctx->rcq;
	attr.cap.max_send_wr  = ctx->tx_depth;
	attr.cap.max_recv_wr  = 1;
	attr.cap.max_send_sge = 1;
	attr.cap.max_recv_sge = 1;
	attr.cap.max_inline_data = 0;
	attr.qp_type = IBV_QPT_RC;

	if (data->use_cma) {
		if (rdma_create_qp(cm_id, ctx->pd, &attr)) {
			fprintf(stderr, "Couldn't create QP\n");
			return NULL;
		}
		ctx->qp = cm_id->qp;
		pp_post_recv(ctx);
		return ctx;
	} else {
		ctx->qp = ibv_create_qp(ctx->pd, &attr);
		if (!ctx->qp)  {
			fprintf(stderr, "Couldn't create QP\n");
			return NULL;
		}
		{
			struct ibv_qp_attr attr;
	
			attr.qp_state        = IBV_QPS_INIT;
			attr.pkey_index      = 0;
			attr.port_num        = (uint8_t) data->ib_port;
			attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE;
	
			if (ibv_modify_qp(ctx->qp, &attr,
					IBV_QP_STATE              |
					IBV_QP_PKEY_INDEX         |
					IBV_QP_PORT               |
					IBV_QP_ACCESS_FLAGS)) {
				fprintf(stderr, "Failed to modify QP to INIT\n");
				return NULL;
			}
		}
	
		return ctx;
	}
	
}

static int pp_connect_ctx(struct pingpong_context *ctx, struct pp_data data)			  
{
	struct ibv_qp_attr attr;
	memset(&attr, 0, sizeof attr);

	attr.qp_state 		= IBV_QPS_RTR;
	attr.path_mtu 		= IBV_MTU_2048;
	attr.dest_qp_num 	= data.rem_dest->qpn;
	attr.rq_psn 		= data.rem_dest->psn;
	attr.max_dest_rd_atomic = 1;
	attr.min_rnr_timer 	= 12;
	attr.ah_attr.is_global  = 0;
	attr.ah_attr.dlid       = (uint16_t) data.rem_dest->lid;
	attr.ah_attr.sl         = 0;
	attr.ah_attr.src_path_bits = 0;
	attr.ah_attr.port_num   = (uint8_t) data.ib_port;
	if (ibv_modify_qp(ctx->qp, &attr,
			  IBV_QP_STATE              |
			  IBV_QP_AV                 |
			  IBV_QP_PATH_MTU           |
			  IBV_QP_DEST_QPN           |
			  IBV_QP_RQ_PSN             |
			  IBV_QP_MAX_DEST_RD_ATOMIC |
			  IBV_QP_MIN_RNR_TIMER)) {
		fprintf(stderr, "Failed to modify QP to RTR\n");
		return 1;
	}

	attr.qp_state 	    = IBV_QPS_RTS;
	attr.timeout 	    = 14;
	attr.retry_cnt 	    = 7;
	attr.rnr_retry 	    = 7;
	attr.sq_psn 	    =  data.my_dest.psn;
	attr.max_rd_atomic  = 1;
	if (ibv_modify_qp(ctx->qp, &attr,
			  IBV_QP_STATE              |
			  IBV_QP_TIMEOUT            |
			  IBV_QP_RETRY_CNT          |
			  IBV_QP_RNR_RETRY          |
			  IBV_QP_SQ_PSN             |
			  IBV_QP_MAX_QP_RD_ATOMIC)) {
		fprintf(stderr, "Failed to modify QP to RTS\n");
		return 1;
	}

	return 0;
}

static void pp_post_recv(struct pingpong_context *ctx)
{
        struct ibv_sge list;
        struct ibv_recv_wr wr, *bad_wr;
        int rc;

        list.addr = (uintptr_t) ctx->buf;
        list.length = 1;
        list.lkey = ctx->mr->lkey;
        wr.next = NULL;
        wr.wr_id = 0xdeadbeef;
        wr.sg_list = &list;
        wr.num_sge = 1;

        rc = ibv_post_recv(ctx->qp, &wr, &bad_wr);
        if (rc) {
                perror("ibv_post_recv");
                fprintf(stderr, "ibv_post_recv failed %d\n", rc);
        }
}

static void pp_wait_for_done(struct pingpong_context *ctx)
{
	struct ibv_wc wc;
	int ne;

	do {
		ne = ibv_poll_cq(ctx->rcq, 1, &wc);
	} while (ne == 0);

	if (wc.status) 
		fprintf(stderr, "bad wc status %d\n", wc.status);
	if (!(wc.opcode & IBV_WC_RECV))
		fprintf(stderr, "bad wc opcode %d\n", wc.opcode);
	if (wc.wr_id != 0xdeadbeef) 
		fprintf(stderr, "bad wc wr_id 0x%x\n", (int)wc.wr_id);
}

static void pp_send_done(struct pingpong_context *ctx)
{
	struct ibv_send_wr *bad_wr;
	struct ibv_wc wc;
	int ne;

	ctx->list.addr = (uintptr_t) ctx->buf;
	ctx->list.length = 1;
	ctx->list.lkey = ctx->mr->lkey;
	ctx->wr.wr_id      = 0xcafebabe;
	ctx->wr.sg_list    = &ctx->list;
	ctx->wr.num_sge    = 1;
	ctx->wr.opcode     = IBV_WR_SEND;
	ctx->wr.send_flags = IBV_SEND_SIGNALED;
	ctx->wr.next       = NULL;
	if (ibv_post_send(ctx->qp, &ctx->wr, &bad_wr)) {
		fprintf(stderr, "ibv_post_send failed\n");
		return;
	}
	do {
		ne = ibv_poll_cq(ctx->scq, 1, &wc);
	} while (ne == 0);

	if (wc.status) 
		fprintf(stderr, "bad wc status %d\n", wc.status);
	if (wc.opcode != IBV_WC_SEND)
		fprintf(stderr, "bad wc opcode %d\n", wc.opcode);
	if (wc.wr_id != 0xcafebabe) 
		fprintf(stderr, "bad wc wr_id 0x%x\n", (int)wc.wr_id);
}

static void pp_wait_for_start(struct pingpong_context *ctx)
{
	struct ibv_wc wc;
	int ne;

	do {
		ne = ibv_poll_cq(ctx->rcq, 1, &wc);
	} while (ne == 0);

	if (wc.status) 
		fprintf(stderr, "bad wc status %d\n", wc.status);
	if (!(wc.opcode & IBV_WC_RECV))
		fprintf(stderr, "bad wc opcode %d\n", wc.opcode);
	if (wc.wr_id != 0xdeadbeef) 
		fprintf(stderr, "bad wc wr_id 0x%x\n", (int)wc.wr_id);
	pp_post_recv(ctx);
}

static void pp_send_start(struct pingpong_context *ctx)
{
	struct ibv_send_wr *bad_wr;
	struct ibv_wc wc;
	int ne;

	ctx->list.addr = (uintptr_t) ctx->buf;
	ctx->list.length = 1;
	ctx->list.lkey = ctx->mr->lkey;
	ctx->wr.wr_id      = 0xabbaabba;
	ctx->wr.sg_list    = &ctx->list;
	ctx->wr.num_sge    = 1;
	ctx->wr.opcode     = IBV_WR_SEND;
	ctx->wr.send_flags = IBV_SEND_SIGNALED;
	ctx->wr.next       = NULL;
	if (ibv_post_send(ctx->qp, &ctx->wr, &bad_wr)) {
		fprintf(stderr, "ibv_post_send failed\n");
		return;
	}
	do {
		ne = ibv_poll_cq(ctx->scq, 1, &wc);
	} while (ne == 0);

	if (wc.status) 
		fprintf(stderr, "bad wc status %d\n", wc.status);
	if (wc.opcode != IBV_WC_SEND)
		fprintf(stderr, "bad wc opcode %d\n", wc.opcode);
	if (wc.wr_id != 0xabbaabba) 
		fprintf(stderr, "bad wc wr_id 0x%x\n", (int)wc.wr_id);
}

static void pp_close_cma(struct pp_data data)
{
        struct rdma_cm_event *event;
        int rc;

        if (data.servername) {
                rc = rdma_disconnect(data.cm_id);
                if (rc) {
					perror("rdma_disconnect");
					fprintf(stderr, "rdma disconnect error\n");
					return;
                }
        }

        rdma_get_cm_event(data.cm_channel, &event);
        if (event->event != RDMA_CM_EVENT_DISCONNECTED)
                fprintf(stderr, "unexpected event during disconnect %d\n", event->event);
        rdma_ack_cm_event(event);
        rdma_destroy_id(data.cm_id);
        rdma_destroy_event_channel(data.cm_channel);
}

static void usage(const char *argv0)
{
	printf("Usage:\n");
	printf("  %s            start a server and wait for connection\n", argv0);
	printf("  %s -h <host>  connect to server at <host>\n", argv0);
	printf("\n");
	printf("Options:\n");
	printf("  -p <port>     listen on/connect to port <port> (default 18515)\n");
	printf("  -d <dev>      use IB device <dev> (default first device found)\n");
	printf("  -i <port>     use port <port> of IB device (default 1)\n");
	printf("  -s <size>     size of message to exchange (default 65536)\n");
	printf("  -t <dep>      size of tx queue (default 100)\n");
	printf("  -n <iters>    number of exchanges (at least 2, default 1000)\n");
	printf("  -b            measure bidirectional bandwidth (default unidirectional)\n");
	printf("  -c 		    use RDMA CM\n");
}

static void print_report(unsigned int iters, unsigned size, int duplex,
			 cycles_t *tposted, cycles_t *tcompleted)
{
	cycles_t cycles_to_units;
	unsigned long tsize;	/* Transferred size, in megabytes */
	int i, j;
	int opt_posted = 0, opt_completed = 0;
	cycles_t opt_delta;
	cycles_t t;

	opt_delta = tcompleted[opt_posted] - tposted[opt_completed];

	/* Find the peak bandwidth */
	for (i = 0; i < (int) iters; ++i)
		for (j = i; j < (int) iters; ++j) {
			t = (tcompleted[j] - tposted[i]) / (j - i + 1);
			if (t < opt_delta) {
				opt_delta  = t;
				opt_posted = i;
				opt_completed = j;
			}
		}

	cycles_to_units = get_freq();

	tsize = duplex ? 2 : 1;
	tsize = tsize * size;

	{
		double sec = (double) opt_delta / (double) cycles_to_units;
		double mbytes = (double) tsize / (double) 0x100000;
		printf("\nBandwidth peak (#%d to #%d): %7.2f MB/sec\n",
			   opt_posted, opt_completed, mbytes / sec);

		sec = (double) (tcompleted[iters - 1] - tposted[0]) / (double) cycles_to_units;
		mbytes = (double) tsize * (double) iters / (double) 0x100000;
		printf("Bandwidth average: %7.2f MB/sec\n", mbytes / sec);

		printf("Service Demand peak (#%d to #%d): %7.2f cycles/KB\n",
				 opt_posted, opt_completed,
				 (double) opt_delta * 1024. / (double) tsize);
		printf("Service Demand Avg  : %7.2f cycles/KB\n",
				 (double) (tcompleted[iters - 1] - tposted[0]) * 1024. / (double) (tsize * iters));	
	}
}


int __cdecl main(int argc, char *argv[])
{
	struct ibv_device		**dev_list;
	struct pingpong_context *ctx = NULL;
	char                    *ib_devname = NULL;
	int                      iters = 1000;
	int                      scnt, ccnt;
	int                      duplex = 0;
	struct ibv_qp			*qp;
	cycles_t				*tposted;
	cycles_t				*tcompleted;
	struct pp_data	 		 data;
	WORD					 version;
	WSADATA					 wsdata;
	int						 err;

	srand((unsigned int) time(NULL));
	version = MAKEWORD(2, 2);
	err = WSAStartup(version, &wsdata);
	if (err)
		return -1;

	memset(&data, 0, sizeof data);
	data.port	    = 18515;
	data.ib_port    = 1;
	data.size       = 65536;
	data.tx_depth   = 100;
	data.use_cma    = 0;
	data.servername = NULL;
	data.rem_dest   = NULL;
	data.ib_dev     = NULL;
	data.cm_channel = NULL;
	data.cm_id      = NULL;

	/* Parameter parsing. */
	while (1) {
		int c;

		c = getopt(argc, argv, "h:p:d:i:s:n:t:bc");
		if (c == -1)
			break;

		switch (c) {
		case 'p':
			data.port = strtol(optarg, NULL, 0);
			if (data.port < 0 || data.port > 65535) {
				usage(argv[0]);
				return 1;
			}
			break;

		case 'd':
			ib_devname = _strdup(optarg);
			break;

		case 'i':
			data.ib_port = strtol(optarg, NULL, 0);
			if (data.ib_port < 0) {
				usage(argv[0]);
				return 1;
			}
			break;

		case 's':
			data.size = strtol(optarg, NULL, 0);
			break;

		case 't':
			data.tx_depth = strtol(optarg, NULL, 0);
			if (data.tx_depth < 1) {
				usage(argv[0]);
				return 1;
			}
			break;

		case 'n':
			iters = strtol(optarg, NULL, 0);
			if (iters < 2) {
				usage(argv[0]);
				return 1;
			}

			break;

		case 'b':
			duplex = 1;
			break;

		case 'c':
			data.use_cma = 1;
			break;

		case 'h':
			if (optarg) {
				data.servername = _strdup(optarg);
				break;
			}

		default:
			usage(argv[0]);
			return 1;
		}
	}

	printf("port=%d | ib_port=%d | size=%d | tx_depth=%d | iters=%d | duplex=%d | cma=%d |\n",
		 data.port, data.ib_port, data.size, data.tx_depth,
		 iters, duplex, data.use_cma);
		
	/* Done with parameter parsing. Perform setup. */

	if (data.use_cma) {
		data.cm_channel = rdma_create_event_channel();
		if (!data.cm_channel) {
			fprintf(stderr, "rdma_create_event_channel failed\n");
			return 1;
		}
		if (rdma_create_id(data.cm_channel, &data.cm_id, NULL, RDMA_PS_TCP)) {
			fprintf(stderr, "rdma_create_id failed\n");
			return 1;
		}
	
		if (data.servername) {
			ctx = pp_rdma_client_connect(&data);
			if (!ctx) 
				return 1;			
		} else {
			ctx = pp_rdma_server_connect(&data);
			if (!ctx) 
				return 1;			
		}
	} else {
		dev_list = ibv_get_device_list(NULL);
	
		if (!ib_devname) {
			data.ib_dev = dev_list[0];
			if (!data.ib_dev) {
				fprintf(stderr, "No IB devices found\n");
				return 1;
			}
		} else {
			for (; (data.ib_dev = *dev_list); ++dev_list)
				if (!strcmp(ibv_get_device_name(data.ib_dev), ib_devname))
					break;
			if (!data.ib_dev) {
				fprintf(stderr, "IB device %s not found\n", ib_devname);
				return 1;
			}
		}
		if (data.servername) {
			ctx = pp_rdma_client_connect(&data);
			if (!ctx) 
				return 1;
		} else {
			ctx = pp_rdma_server_connect(&data);
			if (!ctx) 
				return 1;
		}
		data.my_dest.lid = pp_get_local_lid(ctx, data.ib_port);
		if (!data.my_dest.lid) {
			fprintf(stderr, "Local lid 0x0 detected. Is an SM running?\n");
			return 1;
		}
		data.my_dest.qpn = ctx->qp->qp_num;
		data.my_dest.psn = rand() & 0xffffff;
		data.my_dest.rkey = ctx->mr->rkey;
		data.my_dest.vaddr = (uintptr_t) ctx->buf + ctx->size;
	
		/* Create connection between client and server.
		* We do it by exchanging data over a TCP socket connection. */
		if (data.servername) {
			if (pp_client_exch_dest(&data))
				return 1;
		} else {
			if (pp_server_exch_dest(&data)) 
				return 1;
		}
	}

	printf("%d: Local address:  LID %#04x, QPN %#06x, PSN %#06x "
			"RKey %#08x VAddr %p\n",
			data.my_dest.lid, data.my_dest.qpn, data.my_dest.psn,
			data.my_dest.rkey, data.my_dest.vaddr);	

	printf("%d: Remote address: LID %#04x, QPN %#06x, PSN %#06x, "
			"RKey %#08x VAddr %p\n\n",
			data.rem_dest->lid, data.rem_dest->qpn, data.rem_dest->psn,
			data.rem_dest->rkey, data.rem_dest->vaddr);

	if (data.use_cma) {

		/*
	 	 * Synch up and force the server to wait for the client to send
		 * the first message (MPA requirement).
		 */
		if (data.servername) {			
			pp_send_start(ctx);
		} else {
			pp_wait_for_start(ctx);
		}

	} else {
		if (pp_connect_ctx(ctx, data))
			return 1;
	
		/* An additional handshake is required *after* moving qp to RTR.
		Arbitrarily reuse exch_dest for this purpose. */
		if (data.servername) {
			if (pp_client_exch_dest(&data))
				return 1;
		} else {
			if (pp_server_exch_dest(&data))
				return 1;
		}
	}

	/* For half duplex tests, server just waits for client to exit */
	if (!data.servername && !duplex) {
		if (data.use_cma) {
			pp_wait_for_done(ctx);
			pp_send_done(ctx);
			//pp_close_cma(data);
		} else {
			pp_server_exch_dest(&data);
			send(data.sockfd, "done", sizeof "done", 0);
			closesocket(data.sockfd);
		}
		return 0;
	}

	ctx->list.addr = (uintptr_t) ctx->buf;
	ctx->list.length = ctx->size;
	ctx->list.lkey = ctx->mr->lkey;
	ctx->wr.wr.rdma.remote_addr = data.rem_dest->vaddr;
	ctx->wr.wr.rdma.rkey = data.rem_dest->rkey;
	ctx->wr.wr_id      = PINGPONG_RDMA_WRID;
	ctx->wr.sg_list    = &ctx->list;
	ctx->wr.num_sge    = 1;
	ctx->wr.opcode     = IBV_WR_RDMA_WRITE;
	ctx->wr.send_flags = IBV_SEND_SIGNALED;
	ctx->wr.next       = NULL;

	scnt = 0;
	ccnt = 0;

	qp = ctx->qp;

	tposted = malloc(iters * sizeof *tposted);

	if (!tposted) {
		perror("malloc");
		return 1;
	}

	tcompleted = malloc(iters * sizeof *tcompleted);

	if (!tcompleted) {
		perror("malloc");
		return 1;
	}

	/* Done with setup. Start the test. */

	while (scnt < iters || ccnt < iters) {

		while (scnt < iters && scnt - ccnt < data.tx_depth) {
			struct ibv_send_wr *bad_wr;
			if (data.servername)
				tposted[scnt] = get_cycles();

			if (ibv_post_send(qp, &ctx->wr, &bad_wr)) {
				fprintf(stderr, "Couldn't post send: scnt=%d\n", scnt);
				return 1;
			}
			++scnt;
		}

		if (ccnt < iters) {
			struct ibv_wc wc;
			int ne;
			do {
				ne = ibv_poll_cq(ctx->scq, 1, &wc);
			} while (ne == 0);

			if (data.servername)
				tcompleted[ccnt] = get_cycles();

			if (ne < 0) {
				fprintf(stderr, "poll CQ failed %d\n", ne);
				return 1;
			}
			if (wc.status != IBV_WC_SUCCESS) {
				fprintf(stderr, "Completion with error at %s:\n",
					data.servername ? "client" : "server");
				fprintf(stderr, "Failed status %d: wr_id %d\n", wc.status, (int) wc.wr_id);
				fprintf(stderr, "scnt=%d, ccnt=%d\n", scnt, ccnt);
				return 1;
			}
			ccnt += 1;
		}
	}

	if (data.use_cma) {
		/* This is racy when duplex mode is used*/
		pp_send_done(ctx);
               	pp_wait_for_done(ctx);
		pp_close_cma(data);
	} else {
		if (data.servername) 
			pp_client_exch_dest(&data);
		else
			pp_server_exch_dest(&data);
		
		send(data.sockfd, "done", sizeof "done", 0);
		closesocket(data.sockfd);
		
	}
	
	print_report(iters, data.size, duplex, tposted, tcompleted);

	free(tposted);
	free(tcompleted);
	return 0;
}
