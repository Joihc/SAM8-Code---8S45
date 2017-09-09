# MyCode MCU
# IAR设置:
    .)  内存选择small
    .)  size与speed选择high
# 代码介绍:
    .) 与位比较时 example & 0x80 == 0 用 (example & (1<<7))代替
    .) 为防止被优化掉 特殊变量 多线程中使用的变量需要加volatile修饰符
    .) do..while()作为延时系统  延时时间为 6*While+5 us  Cpu指令周期 1us    
    .) 某位单独设置0 p&=~(1<<7). 设置1 p|=(1<<7).取反p^=(1<<7)
    .) 208byte内存 ，测试阶段需要disable EEPROM  待确定程序大小后再设置
    .) flush内存需要在数组前加 code 或者 root.只加const 不能有效果
    .) 不能在中断中使用AD转换函数。会导致某一时刻读数不准确
#  74HC164 介绍:  
    0 on 1 off                                             A    
                         CP  C  E  D  G  F  A  B        F| _ | B
                  EX:0   1   0  0  0  1  0  0  0           G
                                                        E| _ |C
                                                           D    CP
# TM1629介绍:
    1 on 0 off
    未填 0
            位置             1      2      3      4      5      6      7      8
            表示 C0H         无锅图 超温字 超温图 传感字   传感图   分     。c    kw
                 C1H                     全国电话xxxxxxxxxxxxxxxxxxxxxxxxx  无锅字
                 02H        G(左)   A      B     C      D      E       F      G    （大右）
                 03H                      A      B     C      D      E       F      (大左)
                 04H        G(左)   A      B     C      D      E       F      G     （下左 右）
                 05H                       A     B      C      D       E      F     （下左 左）
                 06H        G(左)   A      B     C      D      E       F      G    （下右 右）
                 07H                       A     B      C      D       E      F     （下右 左）
                 08H                护     保    热     加   电源图标  冒号
                 09H                       喷嘴1 喷嘴1  喷嘴2  喷嘴3   喷嘴4   喷嘴5
                 0AH        1       1      2     2      3      3      3       4      光线
                 0BH        2       2      0     0      0      0      1       1     光线
                 0CH        7       8      1上   2上     5上    6上    7上    8上    （数字和光标）
                 0DH        3上     4上    1     2       3       4      5     6      (数字和光标)
                 0EH        8       7      7     6      5       5      4     4    光线
                 0FH        6       5     惠州   云      能     公司    有     8    光线
# 所需配置:
    config.c中 显示器旋转和功率选择
