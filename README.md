银行系统API服务器 - 项目总结
项目概述
这是一个基于C++开发的银行系统API服务器，提供完整的银行业务功能，包括账户管理、交易处理和存款管理，并使用Redis作为数据存储后端以确保数据持久性。
主要功能

用户账户管理：注册、登录和认证
交易处理：存款、取款、账户间转账
存款产品：

活期存款（0.03%/秒利率）
定期存款（期限为2/3/5分钟，不同利率）


交易记录：完整的交易历史跟踪和查询

技术架构

后端：C++11编写，支持多线程并发处理
存储：Redis键值数据库，提供数据持久化
通信：基于HTTP的RESTful API接口
安全：支持密码认证和资源访问控制
客户端支持：提供API接口支持多种客户端接入

核心组件

AccountManager：用户账户管理
TransactionManager：交易处理与记录
DepositManager：存款产品管理与利息计算
HttpServer：RESTful API服务提供
RedisClient：Redis数据库交互封装
Serializer：数据对象序列化与反序列化

数据模型

用户信息（账户类型、密码、余额）
交易记录（类型、金额、时间戳）
存款信息（类型、期限、利率、到期状态）

构建与部署
依赖项

C++11兼容编译器
Redis服务器
hiredis库（Redis客户端）
POSIX线程库

CMake版本
项目使用CMake 2.8.12.2构建系统，支持较旧的Linux发行版。

# 基本启动（使用默认参数）
./banking_server

# 自定义配置启动
./banking_server --port 8080 --redis-host 127.0.0.1 --redis-port 6379

# 查看帮助信息
./banking_server --help
