# DIM 32 路 DI：1014 辅机监控 CAN 协议

本版已切换为 250 kbit/s、29 位扩展数据帧，仅实现 DI 板相关功能。
默认设备地址 17、通道 28、从站 0；DI 状态及心跳均为 2000 ms 周期，双路发送。
32 路输入仍为低电平有效。旧版 50 kbit/s 标准帧/NMT 报文已停用。

详细报文、参数规则、待确认项及联调步骤见 [DI 协议实现说明](DI协议实现说明.md)。

## 构建与验证

- Keil 工程：`MDK-ARM/GD32F450ZE_DI1014.uvprojx`。
- 执行 `python build_keil.py` 可批量编译；非本机环境设置 `KEIL_UV4`。
- 输出：`MDK-ARM/output/DIM-DI-1014-250k.hex`。目录里的 `DIM-50k.*` 是旧产物。
- 执行 `python tests/run_tests.py` 运行主机模拟测试；需要 GCC，可设置 `DI_HOST_CC`。
- 本版已通过 Keil ARMCC 5 完整编译，主机协议与 CAN 队列测试通过；未连接硬件或烧录。
- 新工程按原理图 GD32F450ZET6 配置为 GD32F450ZE、512 KB Flash，引用 GigaDevice.GD32F4xx_DFP 2.0.0；烧录前仍需核对芯片实物丝印。旧 GD32450Z_EVAL.uvproj 保留作参考，不再作为构建入口。
- `EWARM` 是原有示例，路径和业务文件没有同步，当前交付以 Keil 工程为准。

## 数字量输入 GPIO 映射

原理图中的 `S_INx` 具有外部 1 kΩ 上拉，未接入时为高电平，软件上报值为 0；输入动作后为低电平，软件上报值为 1。GPIO 输入上下拉配置与已交付 HEX 保持一致，使用内部下拉。

输入缓冲器使能信号 `SIN_OE` 从底板 J4-38 连接到核心板 CT2-38，对应 MCU 的 PC6。固件初始化时将 PC6 配置为推挽输出低电平，使能底板数字量输入通道。

| 输入 | GPIO | 输入 | GPIO | 输入 | GPIO | 输入 | GPIO |
|---|---|---|---|---|---|---|---|
| IN0 | PC9 | IN8 | PC12 | IN16 | PG11 | IN24 | PB3 |
| IN1 | PA8 | IN9 | PD2 | IN17 | PG12 | IN25 | PB4 |
| IN2 | PE2 | IN10 | PE6 | IN18 | PA6 | IN26 | PB0 |
| IN3 | PE3 | IN11 | PC13 | IN19 | PA7 | IN27 | PB1 |
| IN4 | PC10 | IN12 | PD3 | IN20 | PG13 | IN28 | PB5 |
| IN5 | PC11 | IN13 | PG9 | IN21 | PG14 | IN29 | PA15 |
| IN6 | PE4 | IN14 | PC14 | IN22 | PC4 | IN30 | PB15 |
| IN7 | PE5 | IN15 | PC15 | IN23 | PC5 | IN31 | PB14 |

上表映射根据已交付且可正常运行的 HEX 反汇编确认。IN4/IN5 实际使用 PC10/PC11，与 CAN0 的 PA11/PA12 不冲突，32 路输入均会参与采样和 CAN 上报。

## LED GPIO 映射

运行状态双色灯的绿色、红色均为高电平点亮；CAN1、CAN2 通道灯同样为高电平点亮。

| 功能 | 原理图信号 | GPIO |
|---|---|---|
| 运行绿灯 | RUN_LED_L | PC1 |
| 故障红灯 | SYS_LED_L | PA1 |
| CAN1 灯 | CAN1_LED_L | PA4 |
| CAN2 灯 | CAN2_LED_L | PA5 |

## CAN 报文输入数据

状态反馈使用 8 字节扩展数据帧。Byte0～Byte3 为 32 路输入状态的大端表示，Byte4～Byte7 填 0。

- Byte0：IN24～IN31，bit0 对应 IN24。
- Byte1：IN16～IN23。
- Byte2：IN8～IN15。
- Byte3：IN0～IN7，bit0 对应 IN0。

默认状态 ID 为 `0x02881C00`。仅 IN0 动作时数据为 `00 00 00 01 00 00 00 00`。
运行及 CAN 灯按心跳周期闪烁；心跳关闭时灯默认每秒闪烁。CAN 异常时通道灯灭、红灯亮。
