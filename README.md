# STM32空中升级
Demo发射板使用用串口收固件，后通过NRF24L01对目标板发射固件。

串口指令：
A,发送0x00, 系统提示等待执行指令;
B,发送0xff，系统接收固件状态;
C,发送0xfe，系统进入接收目标ID个数和ID状态;	
D,发送0xfd，系统进入发射固件状态。

固件发射机制：
将接收的固件按5KB分批发送，NRF单次最多发送32byte，前4个byte为发送数据包第N个标记，实则单次发送固件28byte。
丢包处理：NRF自动应答无效 -> 重新发送N次 -> 超过N次后自取消发送，提示发射失败，进行下一个目标板发射。

// 首次添加代码
// 接收端为STM32F103C8T6
// 发射端为STM32F103C8T6
