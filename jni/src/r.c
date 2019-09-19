/**
 *
gcc r.c -lpthread -o ~/r && ~/r # | grep '^\[.*\]$' 
gcc r.c -lpthread -o ~/r && ~/r > ~/out.txt
ndk_home=~/android-ndk-r10 ; GCC=$ndk_home/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc ; sysroot=$ndk_home/platforms/android-9/arch-arm/usr ; $GCC --sysroot=$sysroot -I"$sysroot/include" r.c && adb push a.out /storage/sdcard1/a.out && adb shell /storage/sdcard1/a.out | grep adb shell mount -o remount,rw -t yaffs2 /dev/block/mtdblock3 /system /
adb shell mount -o remount,rw,exec -t yaffs2 /storage/sdcard1
> /home/libiao/test.txt
gcc r.c -lwsock32 -lws2_32 -lpthread -o r.exe && rm -rf log.txt && r.exe 

adb push raw.c /sdcard/raw.c
adb shell /sdcard/a.out
adb shell
你有什么意见，欢迎进qq群讨论：26509312
 ls | grep '^\[.*\]$' | tee --


git checkout
git push origin master 
git add --all && git commit --amend -m "More changes - now correct" && git pull  https://github.com/db00/raw.git && git push -u origin master
git clean -f

 */

#include <stdio.h>   
#include <stdlib.h>
#include <string.h>   
#include <ctype.h>   
#include <time.h>   
#ifdef linux
#include <linux/types.h>   
#include <netinet/in.h>   
#include <netinet/ip.h>   
#include <netinet/udp.h>   
#include <sys/socket.h>   
#include <unistd.h>  
#include <net/if.h>  
#include <sys/ioctl.h>  
#include <netinet/if_ether.h>
#endif

#ifdef __WIN32
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include<pthread.h>
#include <stdio.h>
/**
  首先，这是IP数据包，第一个字节是4位版本+首部长度，如图前四位是4，后四位是5，则这个包裹是IPv4，头长5*32bit=20Byte。第二个字节是服务类型，其值是0x00，第三第四字节是包裹总长，值为大端模式0x0034=52，可以据此计算包裹为红色部分。第五第六字节为重组标识，值为大端模式0x4986=18822。第七第八字节为3位标识和13位段偏移量，这两个字节(0x4000)的二进制值为01000000，可以算出该数据报不允许分段，段偏移量为0。第9字节为生存时间TTL=0x40=64，第十字节为协议代码，值为0x06=6（TCP）第11，12字节为头部校验和，值为0x6F77。下面为双方IP地址，0xC0A80164是大端模式为192.168.1.100，0xC0A80002一样，为192.168.0.2。也就是从192.168.1.100发向192.168.0.2（我给路由分了几个网段，呵呵）。因为头部长度已经标志为5，即没有选项，那么IP包头部到此结束。
  ●
  标志字段：占16比特。用来唯一地标识主机发送的每一份数据报。通常每发一份报文，它的值会加1。
  然后，TCP头部部分，前面两个短整型是始末端口，为大端模式0x06F9=1785，请求连接的话，本地端口是随机的；目的端口0x07C3=1987，这个就是服务端的监听端口。下来是数据序号0x73F5BBBE=1945484222，因为是手提，一直不关机，合上待机就拿到办公室，所以已经累计发了很多数据了；确认序号0x00000000，说明两者还没有开始传输数据。然后看偏移，值为80，前四位是8=1000，也就是数据距离包头8*32=32Byte（32正好是这个字节到IP包头的偏移 ！），后四位保留为0。下一个字节为0x02=00000010，前两位保留为0，后六位分别对应：UAPRSF，得出SYN=1，这是一个请求或者接受请求报文！窗口字段0x4000=16384，这个不管它。包校验和0x6DF1，紧急指针0x0000，因为URG位已经为0，即使紧急指针不为0也无效。
  TCP建立连接的三次握手过程    
  TCP会话通过三次握手来初始化。三次握手的目标是使数据段的发送和接收同步。同时也向其他主机表明其一次可接收的数据量（窗口大小），并建立逻辑连接。这三次握手的过程可以简述如下：
  ●源主机发送一个同步标志位（SYN）置1的TCP数据段。此段中同时标明初始序号（Initial Sequence Number，ISN）。ISN是一个随时间变化的随机值。    
  ●目标主机发回确认数据段，此段中的同步标志位（SYN）同样被置1，且确认标志位（ACK）也置1，同时在确认序号字段表明目标主机期待收到源主机下一个数据段的序号（即表明前一个数据段已收到并且没有错误）。此外，此段中还包含目标主机的段初始序号。    
  ●源主机再回送一个数据段，同样带有递增的发送序号和确认序号。
  ，TCP会话的三次握手完成。接下来，源主机和目标主机可以互相收发数据。
 */
typedef struct _tcphdr //定义TCP首部
{
    unsigned short th_sport; //16位源端口
    unsigned short th_dport; //16位目的端口
    unsigned int th_seq; //32位序列号 比特。用来标识从TCP源端向TCP目标端发送的数据字节流，它表示在这个报文段中的第一个数据字节。
    unsigned int th_ack; //32位确认号 是期望收到对方下一个报文段的数据的第一个字节的序号。可选项字段：占32比特。用来定义一些任选项：如记录路径、时间戳等。这些选项很少被使用，同时并不是所有主机和路由器都支持这些选项。可选项字段的长度必须是32比特的整数倍，如果不足，必须填充0以达到此长度要求。
    unsigned char th_lenres;   //4位首部长度/6位保留字
    unsigned char th_flag; //6位标志位  ◆URG：紧急指针（urgent pointer）有效      ∮美幢ＶCP连接不被中断，并且督促中间层设备要尽快处理这些数据！鬉CK：确认序号有效。◆PSH：接收方应该尽快将这个报文段交给应用层。◆RST：重建连接　SYN：发起一个连接。◆FIN：释放一个连接
    unsigned short th_win; //16位窗口大小 此字段用来进行流量控制。单位为字节数，这个值是本机期望一次接收的字节数。
    unsigned short th_sum; //16位校验和 对整个TCP报文段，即TCP头部和TCP数据进行校验和计算，并由目标端进行验证。
    unsigned short th_urp; //16位紧急数据偏移量 它是一个偏移量，和序号字段中的值相加表示紧急数据最后一个字节的序号。

    //其他选项字段：占32比特。可能包括"窗口扩大因子"、"时间戳"等选项。
} TCPHEADER,*PTCPHEADER;

typedef struct _iphdr //定义IP首部
{
    unsigned char h_lenver; //4位首部长度+4位IP版本号
    unsigned char tos; //8位服务类型TOS 服务类型（Type of Service ，TOS）字段：占8比特。其中前3比特为优先权子字段（Precedence，现已被忽略）。第8比特保留未用。第4至第7比特分别代表延迟、吞吐量、可靠性和花费。当它们取值为1时分别代表要求最小时延、最大吞吐量、最高可靠性和最小费用。这4比特的服务类型中只能置其中1比特为1。可以全为0，若全为0则表示一般服务。服务类型字段声明了数据报被网络系统传输时可以被怎样处理。例如：TELNET协议可能要求有最小的延迟，FTP协议（数据）可能要求有最大吞吐量，SNMP协议可能要求有最高可靠性，NNTP（Network News Transfer Protocol，网络新闻传输协议）可能要求最小费用，而ICMP协议可能无特殊要求（4比特全为0）。实际上，大部分主机会忽略这个字段，但一些动态路由协议如OSPF（Open Shortest Path First Protocol）、IS-IS（Intermediate System to Intermediate System Protocol）可以根据这些字段的值进行路由决策
    unsigned short total_len; //16位总长度（字节）
    unsigned short ident; //16位标识 用来唯一地标识主机发送的每一份数据报。通常每发一份报文，它的值会加1
    unsigned short frag_and_flags; //标志位字段：占3比特。标志一份数据报是否要求分段。●段偏移字段：占13比特。如果一份数据报要求分段的话，此字段指明该段偏移距原始数据报开始的位置
    unsigned char ttl; //8位生存时间 TTL 用来设置数据报最多可以经过的路由器数
    unsigned char protocol; //8位协议 (TCP, UDP 或其他)
    unsigned short checksum; //16位IP首部校验和 对头部中每个16比特进行二进制反码求和
    unsigned int sourceip; //32位源IP地址
    unsigned int destip; //32位目的IP地址

    //其他***可选项字段：占32比特。用来定义一些任选项：如记录路径、时间戳等。这些选项很少被使用，同时并不是所有主机和路由器都支持这些选项。可选项字段的长度必须是32比特的整数倍，如果不足，必须填充0以达到此长度要求。
} IPHEADER,*PIPHEADER;

void printf_data(void *pdata, int len)
{
    while(len > 0){
        if(isascii((unsigned char)*(unsigned char *)pdata))
        {
            if(isprint((unsigned char)*(unsigned char *)pdata)){
                printf("%c",(unsigned char)*(unsigned char *)pdata);
            }else{
                printf("(0x%01x)",(unsigned char)(*(unsigned char*)pdata));
            }
        }else{
            printf("(0x%01x)",(unsigned char)(*(unsigned char*)pdata));
        }
        pdata+=1;
        --len;
    }
    printf("\n");
}

void show_buff(void *buf, int len)
{
    time_t ptime;
    ptime = time(NULL);
    struct tm * mytime=localtime(&ptime);
    printf("%d-%d-%d %d:%d:%d\n",mytime->tm_year+1900,mytime->tm_mon+1,mytime->tm_mday ,mytime->tm_hour,mytime->tm_min,mytime->tm_sec);
    static char out[65535];
    PIPHEADER pIpHeader;
    PTCPHEADER pTcpHeader;
    //bufwork = buf ;
    pIpHeader = (IPHEADER *)buf ;
    int ver = pIpHeader->h_lenver >> 4;//4: ipv4; 6:IPv6
    int ipHeadLen = ((pIpHeader->h_lenver) & 0x0f)*4;
    int totalLen = ntohs(pIpHeader->total_len);


    //标记（Flag）：长度3比特。该字段第一位不使用。第二位是DF位，DF位设为1时表明路由器不能对该上层数据包分段。如果一个上层数据包无法在不分段的情况下进行转发，则路由器会丢弃该上层数据包并返回一个错误信息。第三位是MF位，当路由器对一个上层数据包分段，则路由器会在最后一个分段的IP包的包头中将MF位设为0,其余IP包的包头中将MF设为1
    int cannotfrag=ntohs(pIpHeader->frag_and_flags)  & 0x4000;
    int morefrag=ntohs(pIpHeader->frag_and_flags) & 0x2000;

    //分段序号（Fragment Offset）：长度13比特。该字段对包含分段的上层数据包的IP包赋予序号。由于IP包在网络上传送的时候不一定能按顺序到达，这个字段保证了目标路由器在接受到IP包之后能够还原分段的上层数据包。到某个包含分段的上层数据包的IP包在传送是丢失，则整个一系列包含分段的上层数据包的IP包都会被要求重传。
    int fragdif=ntohs(pIpHeader->frag_and_flags) & 0x1fff;

    //if(pIpHeader->protocol != 6 )continue;
    // 处理数据
    sprintf(out,
            "ipv%d ipHeadLen:%d tos:0x%02x (len:%d,%d) id:0x%04x cannot_frag:(df:%d)  morefrag:(mf:%d)  fragdif:%d  ttl:0x%02x protocol:0x%02x checksum:%4x %d.%d.%d.%d => %d.%d.%d.%d",
            ver,
            ipHeadLen,
            pIpHeader->tos,
            ntohs(pIpHeader->total_len),
            len,
            ntohs(pIpHeader->ident),
            cannotfrag,
            morefrag,
            fragdif,
            pIpHeader->ttl,
            pIpHeader->protocol,
            ntohs(pIpHeader->checksum),
            *((unsigned char*)&pIpHeader->sourceip+0),
            *((unsigned char*)&pIpHeader->sourceip+1),
            *((unsigned char*)&pIpHeader->sourceip+2),
            *((unsigned char*)&pIpHeader->sourceip+3),

            *((unsigned char*)&pIpHeader->destip+0),
            *((unsigned char*)&pIpHeader->destip+1),
            *((unsigned char*)&pIpHeader->destip+2),
            *((unsigned char*)&pIpHeader->destip+3)
                );
    printf("%s\t",out);
    fflush(stdout);
    switch(pIpHeader->protocol){
        case 1:
            printf("----icmp----");
            unsigned char *icmp = (buf+ipHeadLen) ;
            switch(*icmp){
                case 0:
                case 8:
                    if(*icmp == 0){
                        printf("is request");
                    }else{
                        printf("is ack");
                    }
                    printf(":%d ",*icmp);
                    printf("checksum:%d",(unsigned short)*(icmp+2));
                    printf("flag:%d",(unsigned short)*(icmp+4));
                    printf("seq:%d",(unsigned short)*(icmp+6));
                    break;
                case 3:
                    printf("unreachable!");
                    char code = *(icmp+1);
                    if(code == 0){
                        printf("net unreachable!");
                    }else if(code == 1){
                        printf("pc unreachable!");
                    }else if(code == 2){
                        printf("protocol unreachable!");
                    }else if(code == 3){
                        printf("port unreachable!");
                    }else if(code == 4){
                        printf(" frag  forbiddon!");
                    }else if(code == 5){
                        printf("rount fail!");
                    }else if(code == 6){
                        printf("dest net unknown!");
                    }else if(code == 7){
                        printf("dest pc unknown!");
                    }
                    break;
                case 11:
                    printf("timeout!");
                    char code2 = *(icmp+1);
                    if(code2 == 0){
                        printf("ttl timeout!");
                    }else if(code2 == 1){
                        printf("frag timeout!");
                    }else{
                        printf("unknown reason!");
                    }
                    break;
            }

            fflush(stdout);
            break;
        case 17:
            printf(" udp----");
            //break;
            unsigned char *udp = (buf+ipHeadLen) ;
            printf("source port:%d ",(unsigned short)(*udp));
            printf("dest port:%d ",(unsigned short)(*(udp+2)));
            printf("len:%d ",(unsigned short)(*(udp+4)));
            printf("checksum:%d ",(unsigned short)(*(udp+6)));//占16比特。用来对UDP头部和UDP数据进行校验。和TCP不同的是，对UDP来说，此字段是可选项，而TCP数据段中的校验和字段是必须有的
            printf("%s",udp+8);

            printf("-------udpdata--------");
            printf_data(udp+8,len -8 - ipHeadLen);

            fflush(stdout);
            break;
        case 89:
            printf("----ospf----");

            fflush(stdout);
            break;
        case 6:
            printf(" tcp----");
            //break;

            pTcpHeader = (PTCPHEADER)(buf+ipHeadLen) ;
            int tcpHeadLen =( ((pTcpHeader->th_lenres)>>4)*4);
            unsigned char *data = (unsigned char*)pTcpHeader+tcpHeadLen;

            unsigned char *th_flag = (unsigned char*)(&pTcpHeader->th_flag);
            char urg = *th_flag & (0x20);
            if(urg)printf("(urg): ");
            char ack = *th_flag & (0x10);
            if(ack)printf("(ack): ");
            char psh = *th_flag & (0x08);
            if(psh)printf("(psh): ");
            char rst = *th_flag & (0x04);
            if(rst)printf("(rst): ");
            char syn = *th_flag & (0x02);
            if(syn)printf("(syn): ");
            char fin = *th_flag & (0x01);
            if(fin)printf("(fin): ");
            fflush(stdout);


            sprintf(out,
                    "%d => %d seq:0x%8x ack:0x%8x \t len_flag:0x%02x flag:0x%02x windowsize:%5d checksum:%04x ;urg:%04x",
                    ntohs(pTcpHeader->th_sport),
                    ntohs(pTcpHeader->th_dport),

                    (unsigned)pTcpHeader->th_seq,
                    (unsigned)pTcpHeader->th_ack,
                    pTcpHeader->th_lenres,
                    pTcpHeader->th_flag,
                    ntohs(pTcpHeader->th_win),
                    ntohs(pTcpHeader->th_sum),
                    ntohs(pTcpHeader->th_urp)
                   );

            printf("%s\t",out);
            printf("totalLen:%d; ipHeadLen:%d; TcpHeaderlen:%d;\n%s",totalLen,ipHeadLen,tcpHeadLen,data);


            printf("-------tcpdata--------");
            printf_data((char *)buf + (tcpHeadLen + ipHeadLen),len -tcpHeadLen - ipHeadLen);

            fflush(stdout);
            break;
        default:
            printf("-------unknown_protocol--------");
            printf_data((char *)buf + ( ipHeadLen),len - ipHeadLen);

    }
    fflush(stdout);
}

void *ThreadProc(void* lpParameter)   // thread data
{
    //新线程中读取socket的代码也非常简单，只需要反复调用recv( )即可
    int m_s=(int)*((int*)lpParameter);
    //    printf("%d,%d,%d",sizeof(int),sizeof(short),sizeof(char));
    //printf("%d\n",m_s);
    long dwErr;
    char szErr[128];
    static char buf[65535];
    int iRet;

    while(1)
    {
        //Sleep(1);
        memset( buf , 0 , sizeof(buf) ) ;
        iRet = recv( m_s , buf , sizeof(buf) , 0 ) ;
        if( iRet < 0)
        {
            sprintf( szErr , "Error recv()  " ) ;
            //SetConsoleTitle(szErr);
        }
        else
        {
            if( *buf )
            {
                //printf("%d,",iRet);
                show_buff(buf,iRet);
            }
        }
    }
    fflush(stdout);
    return NULL;
}

#ifdef linux

int set_netpromisc(char* interface)  
{  
    int fd, s;  
    struct ifreq ifr;  

    fd = socket(AF_INET, SOCK_PACKET, htons(0x800));  
    if (fd < 0)  
    {  
        perror("can not get SOCK_PACKET socket");  
        return 0;  
    }  

    printf("sniffer in interface=%s.....\n\n\n", interface);  
    strcpy(ifr.ifr_name, interface);  

    s = ioctl(fd, SIOCGIFFLAGS, &ifr);  

    if (s < 0)  
    {  
        close(fd);  
        perror("can not get flags");  
        return 0;  
    }  

    ifr.ifr_flags |= IFF_PROMISC;  

    s = ioctl(fd, SIOCSIFFLAGS, &ifr);  
    if (s < 0)  
    {  
        close(fd);  
        perror("can not set flags");  
        return 0;  
    }  
    return fd;  
}  
#endif


int main()
{
    unsigned int s = 0x01020304;
    unsigned char *c= (unsigned char*)&s;
    printf("0x01020304=[%d,%d,%d,%d]\n",c[0],c[1],c[2],c[3]);
    printf("[(unsigned char)%x,(unsigned short)%x]",(unsigned char)0x01020304,(unsigned short)0x01020304);
    
    int m_s;
    long dwErr;
    long dwBufferLen;
    long dwBufferInLen;
    long dwBytesReturned;
    long dwThreadId;
    char szErr[128];
    int rcvtimeo;
    //启动Socket
#ifdef __WIN32
    SOCKADDR_IN sa;
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(1,1), &wsd) != 0)
    {
        dwErr = WSAGetLastError() ;
        sprintf( szErr , "Error WSAStartup() = %ld " , dwErr ) ;
        MessageBox( NULL, szErr, "Error", MB_OK ) ;
        //return ;
    }
    //首先打开一个socket，参数必须是 AF_INET、SOCK_RAW和IPPROTO_IP，否则不能设置SIO_RCVALL属性
    m_s= socket( AF_INET , SOCK_RAW , IPPROTO_IP ) ;
    if( INVALID_SOCKET == m_s )
    {
        dwErr = WSAGetLastError() ;
        sprintf( szErr , "Error socket() = %ld " , dwErr ) ;
        MessageBox( NULL, szErr, "Error", MB_OK ) ;
        return 1;
    }
#endif
#ifdef linux
/**
  也可先在终端上运行:
  ifconfig eth0 -promisc
  ifconfig ppp0 -promisc
  ifconfig lo -promisc
  将网卡设置成混杂模式.这样程序里就不必调用set_netpromisc这个函数了，
 */
    set_netpromisc("lo");
    //set_netpromisc("wlan0");
    int type = ETH_P_IP;  
    int ret;  
    type = ETH_P_ALL;  
    m_s= socket(PF_INET, SOCK_PACKET, htons(type));
    unsigned char buff[4096];
    memset(buff, 0x00, sizeof(buff));  

    /*
     * recvfrom all the packages.
     */  
    while ((ret = recvfrom(m_s, buff, sizeof(buff), 0, NULL, NULL)) > 0)  
    {
        printf("\n%1x:%1x:%1x:%1x:%1x:%1x=>%1x:%1x:%1x:%1x:%1x:%1x type:%1x:%1x\n",
                (unsigned char)*buff,
                (unsigned char)*(buff+1),
                (unsigned char)*(buff+2),
                (unsigned char)*(buff+3),
                (unsigned char)*(buff+4),
                (unsigned char)*(buff+5),
                (unsigned char)*(buff+6),
                (unsigned char)*(buff+7),
                (unsigned char)*(buff+8),
                (unsigned char)*(buff+9),
                (unsigned char)*(buff+10),
                (unsigned char)*(buff+11),
                (unsigned char)*(buff+12),
                (unsigned char)*(buff+13));
        if(*(buff+12)==8 && *(buff+13)==0)show_buff(buff+14, ret-14); // ip
        
        if(*(buff+12)==8 && *(buff+13)==6){//arp
            printf("hardware addr format:%x.%x",*(buff+14),*(buff+15));
            printf("protocol addr format:%x.%x",*(buff+16),*(buff+17));
            unsigned char hard_addr_len = *(buff+18);
            unsigned char protocol_addr_len = *(buff+19);
            printf("hardware addr len:%x",hard_addr_len);
            printf("protocol addr len:%x",protocol_addr_len);
            printf("other:%x.%x",*(buff+20),*(buff+21));
            
            printf("\nfrom addr :");
            printf("mac");
            int i = 0;
            while(i < hard_addr_len){
                printf(":%x",*(buff+22+i));
                ++i;
            }
            printf("\nip:");
            
            i = 0;
            while(i < protocol_addr_len){
                printf(".%d",*(buff+22+i+hard_addr_len));
                ++i;
            }
            
            printf("\nto addr :");
            printf("mac");
            i = 0;
            while(i < hard_addr_len){
                printf(":%x",*(buff+22+i+hard_addr_len + protocol_addr_len));
                ++i;
            }
            printf("\nip:");
            
            i = 0;
            while(i < protocol_addr_len){
                printf(".%d",*(buff+22+i+hard_addr_len*2+ protocol_addr_len));
                ++i;
            }
            
            int aprhead_len = hard_addr_len*2 + protocol_addr_len*2 + 22;
            printf_data(buff+aprhead_len, ret-aprhead_len); // display it now..  
        }
        memset(buff, 0x00, ret);  
    }  
    close(m_s);  
    return 0;  
#endif
#ifdef __WIN32
    //然后可以设置一下该socket的超时参数等选项
    rcvtimeo = 5000 ;
    if( setsockopt( m_s , SOL_SOCKET , SO_RCVTIMEO ,
                (const char *)&rcvtimeo , sizeof(rcvtimeo) ) == SOCKET_ERROR)
    {
        dwErr = WSAGetLastError() ;
        sprintf( szErr , "Error WSAIoctl = %ld " , dwErr ) ;
        MessageBox( NULL, szErr, "Error", MB_OK ) ;
        closesocket( m_s ) ;
        return 2;
    }
    //再将该socket与本机的某个网络接口绑定(注意绑定的IP地址不能是INADDR_ANY)
    sa.sin_family = AF_INET;
    sa.sin_port = htons(80);
    sa.sin_addr.s_addr = inet_addr("192.168.1.118");//iphostsource
    if (bind(m_s,(PSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        dwErr = WSAGetLastError() ;
        sprintf( szErr , "Error bind() = %ld " , dwErr ) ;
        MessageBox( NULL, szErr, "Error", MB_OK ) ;
        closesocket( m_s ) ;
        return 3;
    }
    //接下来就可以设置SIO_RCVALL属性。此后就可以利用这个socket来读取经过本机的数据包了。IPMon创建了一个新线程专门来读取该socket，以防止处理用户输入的主线程被阻塞。
    dwThreadId=GetCurrentThreadId();
    //    WSAIoctl(m_s, SIO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen), NULL, 0, &dwBytesReturned, NULL, NULL);
    //if( SOCKET_ERROR != WSAIoctl( m_s, SIO_RCVALL , &dwBufferInLen, sizeof(DWORD),&dwBufferLen, sizeof(DWORD), &dwBytesReturned , NULL , NULL ) )
    if( SOCKET_ERROR != WSAIoctl( m_s, SIO_RCVALL , &dwBufferInLen, sizeof(DWORD),&dwBufferLen, sizeof(DWORD), &dwBytesReturned , NULL , NULL ) )
    {
        pthread_t pThreadId;
        //printf("%d\n",m_s);
        pthread_create(&pThreadId, NULL, ThreadProc, &m_s);
        //CreateThread( NULL,0,ThreadProc,(LPVOID)m_s,0,&dwThreadId );
        for(;;)
        {
            Sleep(1000);
        }
    }
    else
    {
        dwErr = WSAGetLastError() ;
        sprintf( szErr , "Error WSAIoctl = %ld " , dwErr ) ;
        MessageBox( NULL, szErr, "Error", MB_OK ) ;
        closesocket( m_s ) ;
        return 0;
    }
#endif
}
