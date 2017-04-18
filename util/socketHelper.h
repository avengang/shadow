/*
 * socketHelper.h
 *
 *  Created on: Apr 17, 2017
 *      Author: aven
 */

#ifndef SOCKETHELPER_H_
#define SOCKETHELPER_H_
#include <sys/socket.h>
class SocketHelper {
public:
	static int getAndBindSockaddr_in(int &serv_sock, struct sockaddr_in &serv_addr, const char* ip, int port) {

		    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
		    serv_addr.sin_family = AF_INET;  //使用IPv4地址
		    serv_addr.sin_addr.s_addr = inet_addr(ip);  //具体的IP地址
		    serv_addr.sin_port = htons(port);  //端口
//		    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

		    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
				return -1;
			}
		    return 0;
	}
	static void read() {

	}
};


#endif /* SOCKETHELPER_H_ */
