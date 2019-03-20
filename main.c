#include "main.h"

#define SRC_IP      "172.16.8.100"  // IP источника
#define DST_IP      "172.16.8.10"  // IP назначения
#define SRC_PORT    35120            // Порт источника
#define DST_PORT    62000            // Порт назначения
uint8_t smac[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};   // MAC источника
uint8_t dmac[] = {0x76, 0x27, 0xa0, 0xa0, 0x4a, 0x78};   // MAC назначения
const uint8_t MSG[4] = {0xAA,0xAA,0,0};

int main(int argc, char * argv[]) {
	// Тут создается очень случайное зерно
	//srand(time(0) + getpid());
	int mode = 0;
	unsigned int port = DEFAULT_PORT;
	struct in_addr out_ip = {0};
	char* hostname = DEFAULT_IP;
	if (2 > argc || argc > 4)
		argerror:
		DieWithError("args error. example:\n ./sock ct <ip> <port>\n ./sock st <ip> <port>\n ./sock su <ip> <port>\n ./sock cu <ip> <port>\n");
	// Просто парсинг параметров
	if (strlen(argv[1]) != 1 || !(mode = (!strchr(server_key, argv[1][0])?0:SERVER_MODE) | (!strchr(client_key, argv[1][0])?0:CLIET_MODE)))
		goto argerror;
	if (argc >= 3)
		hostname = argv[2];
	if (argc == 4 && ((port = atoi(argv[3])) > 65535))
		goto argerror;
	fprintf(stdout, "Режим %s с %s протоколом\n", mode==SERVER_MODE?"сервера":"клиента", "UDP");
	// Резолв хоста через DNS
	if (1) {
		struct hostent* he;
		if ((he = gethostbyname(hostname)) == NULL)
			DieWithError("DNS сломан\n");
		struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;
		for(int i = 0; addr_list[i] != NULL; ++i) {
			out_ip = *addr_list[i];
			break;
		}
	}
	fprintf(stdout, "Используется хост '%s' -> %s:%d\n", hostname, inet_ntoa(out_ip), port);
	//signal(SIGPIPE, SIG_IGN);	// На случай неудачной передачи
	// Запуск отсоединенных потоков
	pthread_t thread_handler;
	struct main_handler_args_s srv_args = {out_ip.s_addr, port};
	if (mode == SERVER_MODE) {
		if (pthread_create(&thread_handler, NULL, server_handler_udp, &srv_args) < 0)
			DieWithError("thread_create\n");
	}
	else {
		//if (pthread_create(&thread_handler, NULL, client_handler_udp, &srv_args) < 0)
		//	DieWithError("thread_create\n");
		int sock;
		struct sockaddr_ll addr;
		uint8_t* data;
		int psize = 0;  // Размер данных
		
		// Открытие RAW сокета	//AF_INET	//AF_PACKET
		sock = socket(AF_PACKET, SOCK_RAW, IPPROTO_UDP);
		if (sock == -1)
			DieWithError("create RAW socket\n");
			
/*			
int one = 1;
const int *val = &one;
if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
	perror("setsockopt() error");
	//exit(-1);
}
else
	printf("setsockopt() is OK.\n");


int msglen;
struct sockaddr_in sockstr;
		sockstr.sin_family = AF_INET;
		sockstr.sin_port = htons(SRC_PORT);
		sockstr.sin_addr.s_addr = inet_addr(SRC_IP);
		socklen_t socklen = (socklen_t) sizeof(sockstr);


		if (bind(sock, (struct sockaddr*) &sockstr, socklen) == -1) {
			perror("bind");
		}

		//memset(msg, 0, MSG_SIZE);
*/




		// Транспортный уровень
		data = make_udp(inet_addr(SRC_IP), inet_addr(DST_IP), SRC_PORT, DST_PORT, MSG, 4);
		psize = 4 + sizeof(struct udp_header);

		// Сетевой уровень
		data = make_ipv4(inet_addr(SRC_IP), inet_addr(DST_IP), data, psize);
		psize += sizeof(struct ipv4_header);

		// Канальный уровень
		data = make_fr(dmac, smac, data, psize);
		psize += sizeof(struct eth_header);

		// Отправка
		memset(&addr, 0, sizeof(addr));
		addr.sll_family = AF_PACKET;
		addr.sll_ifindex = 2;
		addr.sll_halen = 6;
		memcpy(addr.sll_addr, dmac, 6);

//struct sockaddr_in sin;
//sin.sin_family=AF_INET;
//sin.sin_port = htons(SRC_PORT);
//sin.sin_addr.s_addr = inet_addr(SRC_IP);



		if (sendto(sock, data, psize, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		//sendto(sock,data,psize,0,(struct sockaddr *)&sin, sizeof(sin));
			DieWithError("sendto\n");
			close(sock);
		}
		free(data);
		
		
//				char buf[260];
//		if ((msglen = recv(sock, buf, 260, 0)) == -1) {
//			perror("recv");
//		}
//		if (msglen == 256) printf("OK\n"); else printf("neOK\n");
		sleep(1);
		
		return EXIT_SUCCESS;
	}
	// Ну основной поток ничего делать не будет
	while (1) {
		sleep(1);
	}
	
	return EXIT_SUCCESS;
}
