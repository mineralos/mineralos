/******************************************************************************
 *
 * 文件名  ： mcompat_lib.h
 * 负责人  ： pengp
 * 创建日期： 20180129
 * 版本号  ： v1.0
 * 文件描述： 各种im库函数 接口
 * 版权说明： Copyright (c) 2000-2020 im
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _MCOMPAT_LIB_H_
#define _MCOMPAT_LIB_H_

/************************************ 头文件 ***********************************/
#include <stdbool.h>
#include <stdint.h>

/************************************ 宏定义 ***********************************/
/* 信息文件 */
#define MCOMPAT_LIB_MINER_TYPE_FILE             ("/tmp/type")
#define MCOMPAT_LIB_HARDWARE_VERSION_FILE       ("/tmp/hwver")

#define MCOMPAT_LIB_HARDWARE_VERSION_G9         (9)
#define MCOMPAT_LIB_HARDWARE_VERSION_G19        (19)
#define MCOMPAT_LIB_HARDWARE_VERSION_ERR        (-1)

#define MCOMPAT_LIB_MINER_TYPE_ERR              (-1)
#define MCOMPAT_LIB_MINER_TYPE_T1               (1)
#define MCOMPAT_LIB_MINER_TYPE_A5               (2)
#define MCOMPAT_LIB_MINER_TYPE_A6               (3)
#define MCOMPAT_LIB_MINER_TYPE_A8               (4)
#define MCOMPAT_LIB_MINER_TYPE_A9               (5)
#define MCOMPAT_LIB_MINER_TYPE_D9               (6)
#define MCOMPAT_LIB_MINER_TYPE_S11              (7)

#define MCOMPAT_LIB_VID_UNKNOWN                 (-1)
#define MCOMPAT_LIB_VID_I2C_TYPE                (0)
#define MCOMPAT_LIB_VID_UART_TYPE               (2)
#define MCOMPAT_LIB_VID_VID_TYPE                (3)

#define REG_LENGTH      12

#ifndef MIN
#define MIN(x, y)	((x) > (y) ? (y) : (x))
#endif
#ifndef MAX
#define MAX(x, y)	((x) > (y) ? (x) : (y))
#endif


/*********************************** 类型定义 **********************************/


/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
/*******************************************************************************
 *
 * 函数名  : mcompat_get_shell_cmd_rst
 * 负责人  : Duan Hao
 * 创建日期: 无
 * 函数功能: 运行shell命令并获取返回，仅支持管道命令
 * 输入参数: cmd     - shell命令
 *           result  - 执行命令的返回
 *           size    - result最大长度
 * 输出参数: 无
 * 返回值:   返回的result的实际长度
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int mcompat_get_shell_cmd_rst(char *cmd, char *result, int size);

/*******************************************************************************
 *
 * 函数名  : misc_call_api
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 通过socket向cgminer请求信息
 * 输入参数: command - cgminer需要处理的命令
 *           host    - 矿机主机地址(名)
 *           port    - 矿机监听端口
 * 输出参数: 无
 * 返回值:   0   : 正常退出
 *           其它: 异常退出
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int misc_call_api(char *command, char *host, short int port);

/*******************************************************************************
 *
 * 函数名  : misc_tcp_is_ok
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 通过socket向向公网host:port调用connect判断网络状态
 * 输入参数: host - 主机地址(名)
 *           port - 监听端口
 * 输出参数: 无
 * 返回值:   0   : 正常退出
 *           其它: 异常退出
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
bool misc_tcp_is_ok(char *host, short int port);

/*******************************************************************************
 *
 * 函数名  : misc_trim
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 返回值为一个指向字符串的指针,
 *           该字符串为str去掉两端空白后的字符串.
 * 输入参数: str - 待处理的字符串
 * 输出参数: 无
 * 返回值:   处理后的字符串
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
char *misc_trim(char *str);

/*******************************************************************************
 *
 * 函数名  : misc_get_board_version
 * 负责人  : 王鹏
 * 创建日期: 无
 * 函数功能: 返回值控制板版本
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   MCOMPAT_LIB_HARDWARE_VERSION_G9  - G9 控制板
 *           MCOMPAT_LIB_HARDWARE_VERSION_G19 - G19控制板
 *           MCOMPAT_LIB_HARDWARE_VERSION_ERR - 错误版本
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int misc_get_board_version(void);

/*******************************************************************************
 *
 * 函数名  : misc_get_miner_type
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 返回矿机芯片类型
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   MCOMPAT_LIB_MINER_TYPE_T1 - T1芯片
 *           MCOMPAT_LIB_MINER_TYPE_T2 - T2芯片
 *           MCOMPAT_LIB_MINER_TYPE_T3 - T3芯片
 *           MCOMPAT_LIB_MINER_TYPE_T4 - T4芯片
 *           MCOMPAT_LIB_MINER_TYPE_ERR- 错误芯片
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int misc_get_miner_type(void);

/*******************************************************************************
 *
 * 函数名  : misc_get_vid_type
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 返回算力板使用的vid类型
 * 输入参数: 无
 * 输出参数: 无
 * 返回值:   MCOMPAT_LIB_VID_VID_TYPE        vid脉冲(小黑牛)
 *           MCOMPAT_LIB_VID_GPIO_I2C_TYPE   gpio模拟的i2c方案
 *           MCOMPAT_LIB_VID_UART_TYPE       串口方案
 *           MCOMPAT_LIB_VID_I2C_TYPE        物理i2c方案
 *           MCOMPAT_LIB_VID_ERR_TYPE        错误类型
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int misc_get_vid_type(void);

/*******************************************************************************
 *
 * 函数名  : misc_system
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 执行shell命令并将返回值存入rst_buf
 * 输入参数: - cmd      shell 命令
 *           - buf_size 返回缓存长度
 * 输出参数: - rst_buf  返回缓存
 * 返回值:   无
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
void misc_system(const char *cmd, char *rst_buf, int buf_size);

/*******************************************************************************
 *
 * 函数名  : mcompat_find_chain_vid
 * 负责人  : sunjw
 * 创建日期: 20180329
 * 函数功能: 根据目标电压寻找对应的vid值
 * 输入参数: - chain_id     链编号
 *           - chip_num     芯片个数
 *           - vid_start    开始搜寻的vid值
 *           - volt_target  需要达到的目标电压值
 * 输出参数: 无
 * 返回值:   volt_target对应的目标vid值
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
int mcompat_find_chain_vid(int chain_id, int chip_num, int vid_start, double volt_target);


int mcompat_get_chip_tech();

#endif // #ifndef _MCOMPAT_LIB_H_
