#ifndef __HI_MOTOR_USER_H_H__
#define __HI_MOTOR_USER_H_H__

typedef enum hi35xx_mt_direction {
	MT_DIR_UP,
	MT_DIR_DOWN,
	MT_DIR_LEFT,
	MT_DIR_RIGHT,
	MT_DIR_INVALID,
} hi35xx_mt_dir;

typedef struct _hi35xx_mt_point {
	int x;
	int y;
} hi35xx_mt_point;

struct hi35xx_mt_attr {
	unsigned int speed;
	unsigned int step;
	unsigned int max_step;
	hi35xx_mt_point origin_pt;
	hi35xx_mt_point current_pt;
	hi35xx_mt_point next_pt;
	hi35xx_mt_dir direct;
};

#define HIMT_IOC_TYPE_S		's'
#define HIMT_IOC_TYPE_G		'g'
#define HIMT_IOC_TYPE_D		'd'

#define IOC_HIMT_GET_ATTR	_IOR(HIMT_IOC_TYPE_G, 1, struct hi35xx_mt_attr)
#define IOC_HIMT_SET_ATTR	_IOWR(HIMT_IOC_TYPE_S, 1, struct hi35xx_mt_attr)

#define IOC_HIMT_SET_ST		_IOWR(HIMT_IOC_TYPE_S, 2, struct hi35xx_mt_attr)

#define IOC_HIMT_DIR_UP		_IOWR(HIMT_IOC_TYPE_D, 3, struct hi35xx_mt_attr)
#define IOC_HIMT_DIR_DOWN	_IOWR(HIMT_IOC_TYPE_D, 4, struct hi35xx_mt_attr)
#define IOC_HIMT_DIR_LEFT	_IOWR(HIMT_IOC_TYPE_D, 5, struct hi35xx_mt_attr)
#define IOC_HIMT_DIR_RIGHT	_IOWR(HIMT_IOC_TYPE_D, 6, struct hi35xx_mt_attr)

#define IOC_HIMT_DIR_P2P	_IOWR(HIMT_IOC_TYPE_D, 7, struct hi35xx_mt_attr)

#endif
