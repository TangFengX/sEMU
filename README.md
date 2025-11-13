# sEMU for MINIRISV
## 功能
- 将 MINIRISV 指令翻译为 十六进制 ，支持写入 Logisim ROM
- 将 十六进制翻译为 MINIRISV 
- 搭建支持MINIRIXV指令集的sEMU，可以导出指定PC的内存，寄存器的hex镜像

## MINIRISV标准
<table style="border-collapse: collapse; width: 100%; border: 1px solid #ccc; font-family: sans-serif;">
  <thead>
    <tr style="text-align: center; background-color: #f2f2f2;">
      <th style="border: 1px solid #ccc; padding: 8px;">Command</th>
      <th style="border: 1px solid #ccc; padding: 8px;">31 - 25</th>
      <th style="border: 1px solid #ccc; padding: 8px;">24 - 20</th>
      <th style="border: 1px solid #ccc; padding: 8px;">19 - 15</th>
      <th style="border: 1px solid #ccc; padding: 8px;">14 - 12</th>
      <th style="border: 1px solid #ccc; padding: 8px;">11 - 7</th>
      <th style="border: 1px solid #ccc; padding: 8px;">6 - 0</th>
      <th style="border: 1px solid #ccc; padding: 8px;">Format</th>
      <th style="border: 1px solid #ccc; padding: 8px;">Function</th>
    </tr>
  </thead>
  <tbody>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>lui</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;" colspan="4">imm[19:0] (20)<br>(constant)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rd (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0110111</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>lui rd, imm</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Place the upper 20 bit of 32-bit value into <code>rd</code>, filling in the lowest 12 bits with zeros.</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>jalr</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;" colspan="2">imm[11:0] (12)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)<br>(base)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">000</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rd (5)<br>(return address)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">1100111</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>jalr rd, offset(rs1)</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Set <code>PC</code> to <code>bits_in_rs1 + offset</code>(target address) and write return address to <code>rd</code>.The LSB of target address will be set to <code>zero</code></td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>addi</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;" colspan="2">imm[11:0] (12)<br>(constant,signed)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">000</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rd (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0010011</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>addi rd, rs1, imm</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Sign expand to 32 bit.Add immediate to register <code>rs1</code> .Send result to <code>rd</code>.</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>lw</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;" colspan="2">imm[11:0] (12)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)<br>(base)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">010</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rd (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0000011</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>lw rd, offset(rs1)</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Load 32-bit word from memory <code>bits_in_rs1+offset</code> to register <code>rd</code> .</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>lbu</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;" colspan="2">imm[11:0] (12)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)<br>(base)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">100</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rd (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0000011</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>lbu rd, offset(rs1)</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Load 8-bit unsigned byte from memory <code>bits_in_rs1+offset</code> to register <code>rd</code>.Expand it to 32 bit by padding <code>zero</code> to MSB .</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>sw</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;">imm[11:5] (7)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs2 (5)<br>(src)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)<br>(base)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">010</td>
      <td style="border: 1px solid #ccc; padding: 8px;">imm[4:0] (5)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0100011</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>sw rs2, offset(rs1)</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Store 32-bit word from register <code>rs2</code> to memory <code>bits_in_rs1+offset</code>.</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>sb</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;">imm[11:5] (7)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs2 (5)<br>(src)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)<br>(base)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">000</td>
      <td style="border: 1px solid #ccc; padding: 8px;">imm[4:0] (5)<br>(offset)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0100011</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>sb rs2, offset(rs1)</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Store the lower 8-bit word from register <code>rs2</code> to memory <code>bits_in_rs1+offset</code>.</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #ccc; padding: 8px;"><code>add</code></td>
      <td style="border: 1px solid #ccc; padding: 8px;">0000000</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs2 (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rs1 (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">000</td>
      <td style="border: 1px solid #ccc; padding: 8px;">rd (5)</td>
      <td style="border: 1px solid #ccc; padding: 8px;">0110011</td>
      <td style="border: 1px solid #ccc; padding: 8px;"><strong>add rd, rs1, rs2</strong></td>
      <td style="border: 1px solid #ccc; padding: 8px; text-align: left;">Add register <code>rs1</code> and <code>rs2</code>.Send result to <code>rd</code></td>
    </tr>
  </tbody>
</table>

## sEMU标准
### 通用寄存器
- 32bit ，从x0到x32
- x0恒定为0
<table style="border-collapse: collapse; width: 100%; border: 1px solid #333; font-family: sans-serif;">
  <thead>
    <tr style="text-align: center; background-color: #e6e6fa;">
      <th style="border: 1px solid #333; padding: 10px; width: 10%;">寄存器</th>
      <th style="border: 1px solid #333; padding: 10px; width: 15%;">别名</th>
      <th style="border: 1px solid #333; padding: 10px; width: 40%;">描述 (特殊用途)</th>
      <th style="border: 1px solid #333; padding: 10px; width: 35%;">调用约定（保存责任）</th>
    </tr>
  </thead>
  <tbody>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x0</td>
      <td style="border: 1px solid #333; padding: 8px;">zero</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold; color: #cc0000;">硬连线零（Hard-wired zero），始终为 0。</td>
      <td style="border: 1px solid #333; padding: 8px;">不适用</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x1</td>
      <td style="border: 1px solid #333; padding: 8px;">ra</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold;">返回地址（Return Address）</td>
      <td style="border: 1px solid #333; padding: 8px;">调用者保存 (Caller-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x2</td>
      <td style="border: 1px solid #333; padding: 8px;">sp</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold;">栈指针（Stack Pointer）</td>
      <td style="border: 1px solid #333; padding: 8px;">被调用者保存 (Callee-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x3</td>
      <td style="border: 1px solid #333; padding: 8px;">gp</td>
      <td style="border: 1px solid #333; padding: 8px;">全局指针（Global Pointer）</td>
      <td style="border: 1px solid #333; padding: 8px;">被调用者保存 (Callee-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x4</td>
      <td style="border: 1px solid #333; padding: 8px;">tp</td>
      <td style="border: 1px solid #333; padding: 8px;">线程指针（Thread Pointer）</td>
      <td style="border: 1px solid #333; padding: 8px;">被调用者保存 (Callee-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x5 - x7</td>
      <td style="border: 1px solid #333; padding: 8px;">t0 - t2</td>
      <td style="border: 1px solid #333; padding: 8px;">临时寄存器（Temporary Registers）</td>
      <td style="border: 1px solid #333; padding: 8px;">调用者保存 (Caller-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x8</td>
      <td style="border: 1px solid #333; padding: 8px;">s0 / fp</td>
      <td style="border: 1px solid #333; padding: 8px;">保存的寄存器 / 帧指针（Frame Pointer）</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold;">被调用者保存 (Callee-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x9</td>
      <td style="border: 1px solid #333; padding: 8px;">s1</td>
      <td style="border: 1px solid #333; padding: 8px;">保存的寄存器 1</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold;">被调用者保存 (Callee-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x10 - x11</td>
      <td style="border: 1px solid #333; padding: 8px;">a0 - a1</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold; color: #008000;">函数参数 / 函数返回值</td>
      <td style="border: 1px solid #333; padding: 8px;">调用者保存 (Caller-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x12 - x17</td>
      <td style="border: 1px solid #333; padding: 8px;">a2 - a7</td>
      <td style="border: 1px solid #333; padding: 8px;">函数参数</td>
      <td style="border: 1px solid #333; padding: 8px;">调用者保存 (Caller-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x18 - x27</td>
      <td style="border: 1px solid #333; padding: 8px;">s2 - s11</td>
      <td style="border: 1px solid #333; padding: 8px;">保存的寄存器</td>
      <td style="border: 1px solid #333; padding: 8px; font-weight: bold;">被调用者保存 (Callee-Saved)</td>
    </tr>
    <tr style="text-align: center;">
      <td style="border: 1px solid #333; padding: 8px;">x28 - x31</td>
      <td style="border: 1px solid #333; padding: 8px;">t3 - t6</td>
      <td style="border: 1px solid #333; padding: 8px;">临时寄存器</td>
      <td style="border: 1px solid #333; padding: 8px;">调用者保存 (Caller-Saved)</td>
    </tr>
  </tbody>
</table>

### 内存

- 默认地址位宽：16
- 默认数据位宽：32
- 小端存储

## 输入输出格式

指令<->中间文件<->十六进制文件

