## 一、实验环境
* 操作系统`Ubuntu 18.04`
* 代码编辑器`Visual studio code`

## 二、实验测试
* 步骤一：将 server.c 代码和 client.c 代码分别放入 send 和 receive 文件夹，如下图所示
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/aa5c24d58b548917534ec574918efa83.png)

* 步骤二：分别对 server.c 和 client.c 代码进行编译运行，如下图所示（vscode演示）
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/f6204ab2003bcc22ec9794b804366f6e.png)
  * 注意：此时文件夹变化情况如下：
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/7f63d310f1b6ee290d88d3b1b3e86af6.png)

* 步骤三：在服务器端输入端口号
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/3f39bf8d2955a35e0fe3e0f158343436.png)

* 步骤四：根据服务器端的信息，在客户端输入服务器 IP 地址和端口号，完成操作后，效果如下图所示
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/ae5723fad238f75feefe49abd2b54cf3.png)

* 步骤五：在客户端输入文件信息，进行文件下载，这里所传输的是 server.c 文件
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/20646bc931d9c41482cbb0abf2853890.png)
  * 注意：传输完成后，文件夹变化情况如下，附上文件内容比对（使用 meld 指令）
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/d5e5ebfaac6fb9ba395ad79ba45a51e2.png)
  * ![image.png](https://pic.rmb.bdstatic.com/bjh/e1d2f13b429ee8bac9c3e44342bb3d98.png)
