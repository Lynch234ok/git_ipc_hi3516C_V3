
#ifndef __HISILICON_LINUX_QUEUE_H
#define __HISILICON_LINUX_QUEUE_H

struct hil_vqueue {
	int back;	/* increase */
	int front;	/* decrease */
	int nr;
	int max_size;

	void *priv;
};

#define hil_vqueue_dump(level, vq) hil_vqueue_trace_printf(level, \
		#vq "<%p>={ .back=%d, .front=%d, .nr=%d, .max_size=%d, .priv=%p\n", \
		(vq), (vq)->back, (vq)->front, (vq)->nr, (vq)->max_size, (vq)->priv)

#define hil_vqueue_assert_struct(vq) \
		hil_vqueue_assert( (vq->back>=0) && (vq->front<vq->max_size) && \
				(vq->nr>=0) && (vq->max_size>0) && (vq->nr<=vq->max_size) )

#define HIL_INIT_VQUEUE(size)	{ .back=0, .front=0, .nr=0, .max_size=size }

#define hil_vqueue_max_size(vq) (vq)->max_size
#define hil_vqueue_count(vq) (vq)->nr

static inline int hil_vqueue_iscut(struct hil_vqueue *vq0, struct hil_vqueue *vq1)
{
	int vq0_end, vq1_end;

	if(hil_vqueue_count(vq0)==0 || hil_vqueue_count(vq1)==0)
		return 0;

	vq0_end = vq0->front + vq0->nr;
	vq1_end = vq1->front + vq1->nr;

	if(vq0->front >= vq1_end)
		return 0;
	if(vq0_end <= vq1->front)
		return 0;

	return 1;
}

static inline void hil_vqueue_init(struct hil_vqueue *vq, int max_size)
{
	vq->back = max_size -1;
	vq->front = 0;
	vq->nr = 0;
	vq->max_size = max_size;

	hil_vqueue_assert_struct(vq);
}

/**
  * return new write-point on the back.
  * return -1 if failed.
  */
static inline int hil_vqueue_push_back(struct hil_vqueue *vq)
{
	hil_vqueue_assert_struct(vq);

	if(vq->nr == vq->max_size)
		return -1;

	vq->back ++;
	if(vq->back == vq->max_size)
		vq->back = 0;
	vq->nr ++;

	return vq->back;
}

static inline int hil_vqueue_push_front(struct hil_vqueue *vq)
{
	hil_vqueue_assert_struct(vq);

	if(vq->nr == vq->max_size)
		return -1;

	if(vq->front ==0)
		vq->front = vq->max_size -1;
	else
		vq->front --;

	vq->nr ++;

	return vq->front;
}

static inline int hil_vqueue_pop_front(struct hil_vqueue *vq)
{
	int front;

	hil_vqueue_assert_struct(vq);

	if(vq->nr == 0)
		return -1;

	front = vq->front ++;
	if(vq->front == vq->max_size)
		vq->front = 0;

	vq->nr --;

	return front;
}

static inline int hil_vqueue_pop_back(struct hil_vqueue *vq)
{
	int back;

	hil_vqueue_assert_struct(vq);

	if(vq->nr == 0)
		return -1;

	back = vq->back;

	if(vq->back ==0)
		vq->back = vq->max_size - 1;
	else
		vq->back --;

	vq->nr --;

	return back;
}

static inline int hil_vqueue_front(struct hil_vqueue *vq)
{
	hil_vqueue_assert_struct(vq);

	if(vq->nr == 0)
		return -1;

	return vq->front;
}

static inline int hil_vqueue_back(struct hil_vqueue *vq)
{
	hil_vqueue_assert_struct(vq);

	if(vq->nr == 0)
		return -1;

	return vq->back;
}

#define HIL_DECLARE_VQUEUE_TYPE(name, datatype, size) typedef \
	struct _hil_vqueue_##name##_template { \
		struct hil_vqueue vq; \
		datatype c[size]; \
	} name;

#define hil_vqueue_init_T(vq_t, size) ({ \
		int error = -1; \
		int _size = size; \
		typeof(*(vq_t)) *p = vq_t; \
		if(sizeof(p->c)/sizeof(p->c[0]) >= (_size)) { \
			hil_vqueue_init(&p->vq, _size); \
			error = 0; \
		} error; })

#define hil_vqueue_push_back_T(vq_t, indata) ({ \
		typeof(*(vq_t)) *p = vq_t; \
		int _id = hil_vqueue_push_back(&p->vq); \
		if(_id>=0)  p->c[_id] = indata; \
		_id; })

#define hil_vqueue_push_front_T(vq_t, indata) ({ \
		typeof(*(vq_t)) *p = vq_t; \
		int _id = hil_vqueue_push_front(&p->vq); \
		if(_id>=0)  p->c[_id] = indata; \
		_id; })

#define hil_vqueue_pop_front_T(vq_t, outdata) ({ \
		typeof(*(vq_t)) *p = vq_t; \
		int _id = hil_vqueue_pop_front(&p->vq); \
		if(_id>=0)  outdata = p->c[_id]; \
		_id; })

#define hil_vqueue_pop_back_T(vq_t, outdata) ({ \
		typeof(*(vq_t)) *p = vq_t; \
		int _id = hil_vqueue_pop_back(&p->vq); \
		if(_id>=0)  outdata = p->c[_id]; \
		_id; })

#define hil_vqueue_front_T(vq_t, outdata) ({ \
		typeof(*(vq_t)) *p = vq_t; \
		int _id = hil_vqueue_front(&p->vq); \
		if(_id>=0)  outdata = p->c[_id]; \
		_id; })

#define hil_vqueue_back_T(vq_t, outdata) ({ \
		typeof(*(vq_t)) *p = vq_t; \
		int _id = hil_vqueue_back(&p->vq); \
		if(_id>=0)  outdata = p->c[_id]; \
		_id; })

#endif

