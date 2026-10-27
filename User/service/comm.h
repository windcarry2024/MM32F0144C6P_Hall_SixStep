#ifndef COMM_H
#define COMM_H

#include "mm32_device.h"

typedef struct
{
    uint8_t exec : 1;  // 换相执行事件
    uint8_t cplt : 1;  // 换相完毕事件
} COMMFlag;

typedef struct
{
    uint8_t high : 2;  // 上桥导通相
    uint8_t low : 2;  // 下桥导通相
    uint8_t idx : 3;  // 霍尔索引值
} COMMStep;

/* 六步换相 */
enum
{
    COMM_UH,
    COMM_VH,
    COMM_WH
};
enum
{
    COMM_UL,
    COMM_VL,
    COMM_WL
};

void comm_process(void);

extern COMMFlag commFlag;
extern uint8_t comm_dir_tab_idx;

#endif