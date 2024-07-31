# Qt  

**基于Qt5的小项目**  
前排提示：bug很多  


# 列表  

#  1. FileTransfer(FileClient/FileServer)  

| 版本 | 介绍                                                                                                                                                                                                                                                  |
| ---- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| v1   | 单线程实现，完成基本的传输功能                                                                                                                                                                                                                        |
| v2   | 多线程实现，主线程负责界面交互，子线程负责通信传输，客户端有传输大文件会内存溢出的bug                                                                                                                                                                 |
| v3   | 多线程实现。修复v2客户端传输大文件内存溢出的bug，新增进度条、传输速率显示                                                                                                                                                                             |
| v4   | 真·多线程实现，自定义线程类MThread继承QThread，自定义工作类MWork的槽函数依赖于MThread线程执行。实现多客户端多文件传输，新增样式表，新增列表窗口右键菜单操作列表项，新增浏览和修改保存路径                                                             |
| v4.3 | 新增局域网用户扫描，有客户端或服务端启动时自动加入客户端局域网列表，简化用户操作流程，双击列表中的服务端地址即可连接，客户端文件发送限制20个线程(5个文件)，且按文件大小从小到大依次发送。新增配置文件，程序启动时按配置文件初始化主题、端口、保存路径 |

 1. v1  
 ![preview/transfer_v1.png](preview/transfer_v1.png) 
 
 2. v2  
 ![preview/transfer_v2.png](preview/transfer_v2.png)
 
 3. v3  
 ![preview/transfer_v3.png](preview/transfer_v3.png)
 
 4. v4  
 ![preview/transfer_v4.png](preview/transfer_v4.png)  
 
 5. v4.3  
![preview/transfer_v4.3.png](preview/transfer_v4.3.png)

 

#  2. Game-2048  

 简单实现的2048  
 ![preview/2048.png](preview/2048.png)  
 

#  3. SerialAssistant  

  简单实现的串口助手  
![preview/serial_assisitant.png](preview/serial_assisitant.png)

