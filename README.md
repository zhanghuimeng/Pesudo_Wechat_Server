# Pesudo_Wechat_Server

## checklist
* 用户登录（√）
* 用户账户、联系人信息同步（√）
* 查找并添加好友
* 好友间实时聊天
	- 发送消息（√）
	- 接收消息（√）
* 好友间非实时聊天
* 好友间文件传输
	- 发送文件（√）
	- 接收文件（√）
* 好友间非实时文件传输	

错误

发送文件的方法打算参照这篇文章，直接用json发送：[http://blog.csdn.net/xingty/article/details/44201379](http://blog.csdn.net/xingty/article/details/44201379) ，文件大小限制为1MB

## server和client传送消息的格式
### client登录
#### client发送
	action: "client_login"
	username: "zhm_x"
	password: "123456"
#### server回复
	action: "server_login_response"
	correct: "true/false"
	
### 好友列表同步
#### client发送
	action: "get_friends_list"
	username: "zhm_1"
#### server回复/发送
	action: "send_friends_list_to_client"
	friends: [{username: "zhm_2"}, {username: "zhm_3"}]
	// send_for_add: true/false
	// add_success: true/false

### 添加好友
#### client发送
	action: "new_friend"
	username: "zhm_x"
	
#### server回复
同上。

### client发送文本消息
#### client发送
	action: "send_text_to_server"
	text: {text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}

### client发送二进制文件
#### client发送
	action: "send_file_to_server"
	file: {filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}

### client添加好友
#### client发送

### server发送文本消息
#### server发送
	action: "send_text_to_client"
	text: {text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}

### server发送二进制文件
#### server发送
	action: "send_file_to_client"
	file: {filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}

### client接收（最近没读的）文本消息
#### client发送
#### server回复
	action: "send_recent_text_to_client"
	text: [{text: "send some text blabla...", time: "2017/1/1:00:00:00", snedby: "zhm_1", sendto: "zhm_2"}, {text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}]

### client接收（最近没读的）文件
#### client发送
#### server回复
	action: "send_recent_file_to_client"
	file: [{filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}, {filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}]

## server本地存储的格式
### 用户及最近没有接收的消息
	username: "zhm_1"
	text: [{text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}, {text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}]
	file: [{filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}, {filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}]