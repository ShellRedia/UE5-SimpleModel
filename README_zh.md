# UE5-SimpleModel

这是一个关于使用5.3版本的虚幻引擎及其内置的OpenCV插件来实现人像“简单模型”内容的项目。

## 1. 快速上手

首先您需要下载该项目，然后经过以下步骤进行配置：

### 1.1 项目生成

找到 SimpleModel.uproject，右键，选择 Generate Visual Studio project files。

然后修改文件 [YOUR_ENGINE_DIR]\Engine\Plugins\Runtime\OpenCV\Source\ThirdParty\OpenCV\include\opencv2\core\utility.hpp 以下内容：

* 注释掉 **53行左右** 的以下代码：

```
    #if defined(check)
    #  warning Detected Apple 'check' macro definition, it can cause build conflicts. Please, include this header before any Apple headers.
    #endif
```

* 将 **936行左右** 的 **bool check() const;** 方法改为 **bool cv_check() const;**。

否则项目编译会报错。

通过 SimpleModel.uproject 打开项目，通过如图方式打开 **Quixel Bridge** 来下载 **MetaHuman** 模型。

![Open Quixel Bridge](Figures\OpenQuixelBridge.png)
![Download MetaHuman](Figures\DownloadMetaHuman.png)