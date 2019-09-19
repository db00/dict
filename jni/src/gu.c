/**
 * @file gu.c
 gcc gu.c -o ~/gu && ~/gu
 gcc gu.c -o ~/gu && ~/gu
 > ~/gu.txt
 curl "http://hyjf.vriworks.cn:8081/JF/my/bill?openid=PC_PAY_USER&userAccount=5809216"  | grep '<td>.*</td>'
http://j3.dfcfw.com/images/JJJZ1/005918.png
http://j3.dfcfw.com/images/JJJZ1/000085.png

curl http://www.dyhjw.com/guzhi.html | grep 'swingRange\|<td class="left"><a  href="http://www.dyhjw.com/[^/\.]\+/' | sed -e 's/<[^>]*>//g'
curl http://www.dyhjw.com/zhipan.html | grep 'swingRange\|<td class="left"><a  href="http://www.dyhjw.com/[^/\.]\+/' | sed -e 's/<[^>]*>//g'
curl http://www.dyhjw.com/shanghaihuangjin/ | grep 'swingRange\|<td class="left"><a  href="http://www.dyhjw.com/[^/\.]\+/' | sed -e 's/<[^>]*>//g'
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2018-08-09
 */

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

char * arr[] = {\
	"天弘沪深300指数C", "005918",\
		"博时沪深300指数C", "002385",\
		"华夏沪深300ETF联接C", "005658",\

		"天弘中证500指数C ", "005919",\
		"天弘中证800指数C", "001589",\

		"天弘上证50指数C", "001549",\
		"博时上证50ETF联接C", "005737",\
		"华夏上证50ETF联接C", "005733",\
		"易方达上证50指数C", "004746",\

		"天弘创业板指数C", "001593",\
		"易方达创业板ETF联接C", "004744",\

		"易方达深证100ETF联接C", "004742",\
		"易方达上证中盘ETF联接C", "004743",\

		"天弘中证电子指数C", "001618",\
		"天弘中证计算机指数C", "001630",\
		"天弘中证食品饮料指数C", "001632",\
		"天弘中证医药100指数C", "001551",\
		"天弘中证银行指数C", "001595",\
		"天弘中证证券保险指数C", "001553",\
		"南方全指证券联接C", "004070",\

		"南方H股联接C", "005555",\
		"博时安盈债券C", "000085",\
		"博时黄金ETF联接C", "002611",\
		//"博时合惠货币A", "004841",\
		"天弘弘运宝货币B", "001391",\
		"华安日日鑫货币A", "040038",
	 NULL
};

int main()
{
	//system("curl -s http://www.dyhjw.com/guzhi.html | grep 'swingRange\\|<td class=\"left\"><a  href=\"http://www.dyhjw.com/[^/\\.]\\+/' | sed -e 's/<[^>]*>//g'");
	//system("curl -s http://www.dyhjw.com/zhipan.html | grep 'swingRange\\|<td class=\"left\"><a  href=\"http://www.dyhjw.com/[^/\\.]\\+/' | sed -e 's/<[^>]*>//g'");
	//return 0;
	int i = 0;
	while(arr[i]!=NULL)
	{
		//printf("%s\n",arr[i]);
		++i;
		char * id = arr[i];
		//printf("%s\n",id);
		char cmd[512];
		printf("---------------");
		fflush(stdout);
		//memset(cmd,0,sizeof(cmd));\
		sprintf(cmd,"curl -s http://fundf10.eastmoney.com/jjjz_%s.html | "\
		"grep 'fund_gsz\\|fund_gszf\\|h4 class=\"title\"\\|( .*% )' | sed -e 's/<[^>]*>//g' -e 's/^[ ]*//g'",\
			id);\
		system(cmd);
		memset(cmd,0,sizeof(cmd));\
		sprintf(cmd,"curl -s http://fundf10.eastmoney.com/jjjz_%s.html | "\
				"grep 'fund_gszf\\|h4 class=\"title\"\\|( .*% )' | sed -e 's/<[^>]*>//g' -e 's/^[ ]*//g'",\
				id);\
		system(cmd);
		//printf("%s\n",cmd);
		fflush(stdout);
		//memset(cmd,0,sizeof(cmd));\
		sprintf(cmd,"xdg-open http://j3.dfcfw.com/images/JJJZ1/%s.png",id); system(cmd);
		//memset(cmd,0,sizeof(cmd));\
		sprintf(cmd,"curl http://quotes.money.163.com/fund/%s.html |"\
				" grep '\\(    		<span>.*</span>"\
				"\\|            <span class=\"\\(cGreen\\|cRed\\)\">.*</span>"\
				"\\|                           <td><span class=\"\\(cGreen\\|cRed\\)\">.*</span></td>\\)'",id);\
			system(cmd);
		//memset(cmd,0,sizeof(cmd));\
		sprintf(cmd,"curl http://fund.eastmoney.com/%s.html |"\
				" sed"\
				" -e 's/\"gz_gsz\">\\([^<]*\\)</\\r\\n gz_gsz: \\1\\r\\n/g' "\
				" -e 's/\"gz_gszze\">\\([^<]*\\)</\\r\\n gz_gszze: \\1\\r\\n/g' "\
				" -e 's/\"gz_gszzl\">\\([^<]*\\)</\\r\\n gz_gszzl: \\1\\r\\n/g' "\
				" -e 's/\"fix_fname\">\\([^<]*\\)</\\r\\n fix_fname:\\1\\r\\n/g' "\
				" | grep \"\\(gz_gszzl\\|gz_gsz\\|fix_fname\\)\"",\
				id);\
			system(cmd);
		++i;
	}

	return 0;
}
