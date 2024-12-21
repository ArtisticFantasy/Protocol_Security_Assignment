# 论文复现大作业
本项目对发表在SIGCOMM'22上的论文"Predicting IPv4 Services Across All Ports"进行了复现，并且额外引入了两个创新点，分别是迭代扫描机制和跳出局部端口约束的随机扫描机制，对清华大学5个校园网段进行扫描验证，发现可以第二轮迭代相比第一轮多扫描出了约6000个服务，提升率达到了约27%

本项目将GPS完整地组成了一个pipeline，以下是对pipeline的具体介绍

## GPS Pipeline
pipeline分为两类，分别是first iteration和remain iterations，first iteration指初次对指定网段的扫描，而remain iterations需要指定last iteration的结果文件夹，基于上轮结果继续扫描（**创新点1**）

### Pre Scan
预先扫描阶段，仅在first iteration中执行，用于探测指定网段内的活跃IP

具体做法是，指定扫描比例，使用zmap扫描指定的network span下的22个常用端口，将有结果的IP记录下来得到active_ip_list.txt

### Seed Scan
种子扫描阶段，first iteration和remain iterations执行两种不同的逻辑

#### first iteration
根据指定的比例从active_ip_list.txt中抽取一部分，使用lzr进行全端口扫描，得到的结果存储进seed.json

#### remain iterations
基于上一轮结果中的final.json，使用lzr扫描其中出现的IP的44个保留端口和50个随机端口，得到extra_seed.json（**创新点2**），将final.json和extra_seed.json合并得到seed.json

### GPS Part One
基于种子集seed.json生成prediction pattern以及需要扫描的（IP网段，端口）的组合，即scanning_plan_one.txt

只考虑同一IP下开放端口数不超过10个的IP，超过10个的很可能是虚假服务

如果一个IP下只有一个端口开放，那么将其(ip, port, -1, service)加入prediction pattern中，否则根据hitrate判断

hitrate的计算方法是全局的，考虑一种(port, service)组合，假设存在另一个port2在同一IP下，那么计算存在(port, service)的情况下，同时存在port2的占存在(port, service)的概率，记为hitrate，即hitrate(port2, (port, service))=P((port, service, port2) | (port, service))

对于所有(ip, port2)组合，找到hitrate(port2, (port, service))最大的(port, service)，且(port, service)也应存在在同一ip下，如果这个hitrate大于阈值（0.00001），那么将(ip, port, port2, service)加入prediction pattern中

对于prediction pattern中的每一行，计算其中ip所属的网段net（可以任意指定前缀长度，比如16），如果说在net内port的出现频率大于阈值（0.00001），那么将(net, port)加入scanning_plan_one.txt

### Prior Scan
基于scanning_plan_one.txt优先扫描，即扫描较大概率推出同一主机上其他服务的所有服务

首先通过zmap扫描这些(net, port)，不进行L7握手，之后使用lzr扫描得到的活跃端口确定服务类型，结果存入prior.json

### GPS Part Two
基于扫描出的IP上的至少一个服务（prior.json），推断其余服务

读入GPS Part One得到的prediction pattern，考虑其中每行(ip, port, port2, service)，将其中的ip转换为其所属网段net（前缀长度16），对于net下的prior.json中的所有(ip2, port, service)的扫描结果，如果(ip2, port2)未被scanning_plan_one.txt覆盖，则将其加入scanning_plan_two.txt

### Post Scan
使用lzr扫描scanning_plan_two.txt中的所有(ip, port)对，将结果保存至post.json

### Combine Results
将seed.json, prior.json, post.json结果合并至final.json，即为本轮最终扫描结果，一轮迭代结束

## 参考文献
[1] Izhikevich L, Teixeira R, Durumeric Z. Predicting ipv4 services across all ports[C]//Proceedings of the ACM SIGCOMM 2022 Conference. 2022: 503-515.

[2] Izhikevich L, Teixeira R, Durumeric Z. {LZR}: Identifying unexpected internet services[C]//30th USENIX Security Symposium (USENIX Security 21). 2021: 3111-3128.