#include "ProcMem.h" // Memory headers
// #include "ProcMem.cpp" // Procmem cpp
#include <iostream> // input output
#include <Windows.h> // Basic windows types
#include <stdio.h>
#include <string>
#include <TlHelp32.h>
#include <vector>



using namespace std;

DWORD bSpotted = 0x939; // radar point
DWORD LocalPlayer = 0xAA7AB4; //  local player offset
DWORD EntityList = 0x4A8246C; // all players offset
DWORD iTeam = 0xF0; // team num offset
DWORD oDormant = 0xE9; // offset to check if the object is player
DWORD bClient, LocalBase; // some values
DWORD oFlags = 0x100;
DWORD forceJump = 0x4F1970C;
DWORD crosshairID = 0xB2A4;
DWORD health = 0xFC;
DWORD PlayerBase;
DWORD EntityLists;
DWORD vecOrigin = 0x134;
DWORD boneMatrix = 0x2698;
DWORD engine;
DWORD ClientState = 0x57D894;
DWORD viewAngles = 0x4D10;
DWORD GlowIndex = 0xA310;
DWORD  GlowObject = 0x4F9F800;
int id;


ProcMem vam; // memory object


typedef struct
{
	float x = 0, y = 0, z = 0;


}Angle;

Angle angle;
Angle m_ViewAngle;
Angle OldAngle;
int ShotsFired = 0;


typedef struct {
	float x, y, z;
}Vector3;

Vector3 getBonePos(int BoneIndex) {

	Vector3 returnVector;
	returnVector.x = vam.Read <float>(LocalBase + boneMatrix + 0x30 * BoneIndex + 0x0C);
	returnVector.y = vam.Read <float>(LocalBase + boneMatrix + 0x30 * BoneIndex + 0x1C);
	returnVector.z = vam.Read <float>(LocalBase + boneMatrix + 0x30 * BoneIndex + 0x2C);




}
 void shoot()
{
	INPUT    Input = { 0 };
	// left down
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));

	// left up
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

 void triggerbot() {
	 int inCross = vam.Read <int>(LocalBase + crosshairID);
	 int myTeam = vam.Read <int>(LocalBase + iTeam);
	 int player = vam.Read<int>(bClient + EntityList + (inCross - 1) * 0x10);
	 int playerTeam = vam.Read <int>(player + iTeam);


	 if (inCross > 0 && inCross <= 64 && myTeam != playerTeam) {
		 shoot();
	 }
 }

 void mouse_move(int x, int y)
 {
	 INPUT input;
	 input.type = INPUT_MOUSE;
	 input.mi.mouseData = 0;
	 input.mi.time = 0;
	 input.mi.dx = x * (65536 / GetSystemMetrics(SM_CXSCREEN));//x being coord in pixels
	 input.mi.dy = y * (65536 / GetSystemMetrics(SM_CYSCREEN));//y being coord in pixels
	 input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE;
	 SendInput(1, &input, sizeof(input));

 }

 float calcVecMag(Vector3 _vector, int dimension) {
	 float magnitude = 0;

	 if (dimension == 2) {
		 magnitude = sqrtf((pow(_vector.x, 2) + pow(_vector.y, 2)));
	 }
	 else if (dimension == 3) {
		 float twoDimMag = sqrtf((pow(_vector.x, 2) + pow(_vector.y, 2)));
		 magnitude = sqrtf((pow(twoDimMag, 2) + pow(_vector.z, 2)));

	 }
	 return magnitude;


 }

 Vector3 getClosestEnemy() {
	 int myteam = vam.Read<int>(LocalBase + iTeam); // get our player's team
	 float dist = 99999;
	 Vector3 latestDir;
	 latestDir.x = 0;
	 latestDir.y = 0;
	 latestDir.z = 0;

	 for (int i = 1; i <= 64; i++) // loop for all players
	 {
		 int player = vam.Read<int>(bClient + EntityList + (i - 1) * 0x10); // get player's base
		 int playerteam = vam.Read<int>(player + iTeam); // get player's team
		 int playerdormant = vam.Read<int>(player + oDormant); // get player's dormant

		 if (playerteam != myteam && !playerdormant) { //If this entity is an enemy player

			 Vector3 position = vam.Read <Vector3>(player + vecOrigin);
			 Vector3 localPlayerPosition = vam.Read <Vector3>(LocalBase + vecOrigin);


			 Vector3 dir;
			 dir.x = position.x - localPlayerPosition.x;
			 dir.y = position.y - localPlayerPosition.y;
			 dir.z = position.z - localPlayerPosition.z;

			 float tempDist = calcVecMag(dir, 3);

			 if (tempDist < dist) {
				 dist = tempDist;
				 latestDir = dir;


			 }
			 //cout << "Dir: " << dir.x << " " << dir.y << " " << dir.z << endl;
		 }
	 }
	 //cout << dist << endl;
	 return latestDir;
 }

 float deltaAngle(float a, float b) {
	 float diff = a - b;
	 if (abs(diff) > 180) {
		 return (360 - abs(diff));

	 }
	 return (abs(diff));

 }

 float deg2Rad(float deg) {
	 float rad = ((3.14159265f / 180) * deg);
	 return rad;
 }

 float rad2Deg(float rad) {
	 float deg = ((180 / 3.14159265f) * rad);
	 return deg;
 }


 Vector3 vclamp(Vector3 _vector)
 {
	 Vector3 ret;

	 ret.x = _vector.x;

	 if (_vector.y > 180.0f)
		 ret.y = _vector.y - 360.0f;

	 if (_vector.y < -180.0f)
		 ret.y = _vector.y + 360.0f;

	 ret.z = 0;

	 return ret;
 }
 Vector3 getDesiredAngles(Vector3 dir) {
	 Vector3 desiredAngle;
	 Vector3 _dir;
	 _dir.x = dir.y;
	 _dir.y = dir.x;
	 _dir.x *= -1;
	 _dir.z = dir.z;

	 //Yaw:
	 desiredAngle.y = (rad2Deg(atan((_dir.y) / (_dir.x))));
	 if (_dir.x > 0 && _dir.y > 0) { //top right
		 desiredAngle.y = (90 - desiredAngle.y) * -1;
	 }
	 if (_dir.x < 0 && _dir.y > 0) { // top left
		 desiredAngle.y = (desiredAngle.y + 90);
	 }if (_dir.x < 0 && _dir.y < 0) { // top left
		 desiredAngle.y = (desiredAngle.y + 90);
	 }
	 if (_dir.x > 0 && _dir.y < 0) { // top left
		 desiredAngle.y = (desiredAngle.y - 90);
	 }

	 //Pitch:
	 float magnitude = calcVecMag(_dir, 2);

	 desiredAngle.x = (rad2Deg(atan(_dir.z / magnitude)) * -1);

	 desiredAngle.z = 0;

	 //cout << " Dir " << _dir.x << " " << _dir.y << " " << _dir.z << endl;
	 //cout << " Angles " << desiredAngle.x << " " << desiredAngle.y << " " << 0 << endl;

	 //desiredAngle = vclamp(desiredAngle);
	 return desiredAngle;
 }





 
 void aimbot() {

	 Vector3 closestEnemy = getClosestEnemy();
	 Vector3 desiredAngle = getDesiredAngles(closestEnemy);

	 DWORD clientState = vam.Read <int>(engine + ClientState);
	 Vector3 prevAngle = vam.Read <Vector3>(clientState + viewAngles);
	 cout << " Dir " << closestEnemy.x << " " << closestEnemy.y << " " << closestEnemy.z << endl;

	 if (closestEnemy.x != 5.04467e-44 && closestEnemy.y && -2.42101e-38  && closestEnemy.z != 1.4013e-45) {
		 cout << " Dir " << closestEnemy.x << " " << closestEnemy.y << " " << closestEnemy.z << endl;

		 vam.Write <Vector3>(clientState + viewAngles, desiredAngle);

	 }

	 //cout << " View angles " << vam.Read <Vector3>(clientState + viewAngles).y << endl;

	 //cout << desiredAngle.y << endl;


 }



void bhop() {
	LocalBase = vam.Read<int>(bClient + LocalPlayer); // GET THE LOCAL BASE
	int flags = vam.Read <int>(LocalBase + oFlags); // GET FLAGS
	//cout << flags;

	if (GetAsyncKeyState(VK_SPACE) && flags == 257) // IF SPACE IS PRESSED AND THE CHARACTER IS ON THE FLOOR BHOP // 256 IS IN AIR 257 ON GROUND
		vam.Write <int>(bClient + forceJump, 6); // THEN JUMP

	//Sleep(10);

}

void radar()
{
	cout << "Loading radar cheat....\n";
	LocalBase = vam.Read<int>(bClient + LocalPlayer);

	int myteam = vam.Read<int>(LocalBase + iTeam); // get out players team





												   // RADAR STARTED
	for (int i = 1; i <= 64; i++) // loop for all players
	{
		int player = vam.Read<int>(bClient + EntityList + (i - 1) * 0x10); // get players base
		int playerteam = vam.Read<int>(player + iTeam); // get players team
		int playerdormant = vam.Read<int>(player + oDormant); // get players dormant

		if (playerteam != myteam) {
			typedef struct {
				float r;
				float g;
				float b;
				float a;
			} rgba_t;
			const size_t glowObjectSize = 0x38;

			rgba_t color;

			color.r = 0.f;
			color.g = 0.f;
			color.b = 1.f;
			color.a = 1.f;
			bool rwo = true;
			bool rwuo = true;
			uintptr_t glowObjectManager = vam.Read<uintptr_t>(bClient + GlowObject);
			uintptr_t glowObjectPtr = glowObjectManager + GlowIndex * glowObjectSize;
			vam.Write <rgba_t>(glowObjectPtr + 4, color);
			vam.Write <bool>(glowObjectPtr + 0x24, rwo);
			vam.Write <bool>(glowObjectPtr + 0x25, rwuo);
		}




		if (playerteam != myteam && !playerdormant) // if player is enemy and he is a playwer
		{
			vam.Write<int>(player + bSpotted, 1); // write 1 to let us see him on radar
		}



		// Sleep(10); // decrease the cpu power

	}


}



int main() // main function
{
	vam.Process("csgo.exe"); // get  csgo process
	bClient = vam.Module("client.dll"); // get client dll
	engine = vam.Module("engine.dll");
	cout << engine;
	Vector3 testDir;
	testDir.x = 90;
	testDir.z = 90;


	float testDist = calcVecMag(testDir, 3);
	//cout << " Dist test " << testDist << endl;




	while (true)
	{
 // get local base
		int myteam = vam.Read<int>(LocalBase + iTeam); // get out players team
		
		if (GetAsyncKeyState(VK_MENU))
		{
			triggerbot();
		}
		if (GetAsyncKeyState(VK_LBUTTON))
		{
			aimbot();
		}	
		//rcs();

		radar();
		
		
		bhop();
		

			
	}



}


// JUNK CODE FOR SAFETY.


class cldtlkg {
public:
	bool wscwiriwg;
	bool xlvunejld;
	bool cwpqgeetyfop;
	cldtlkg();
	string bkdkuwgcejrokqyqcd(double secvlojl, double yvowxzamvhles, string xxsqih, double pkvjvyycjvyodaw, double gekkizhaxau);
	bool qyvnyhwiwgjekpsoedi(int avsscdqd, bool jehnwttfvj, bool slioewqq, bool rpnxanhi);
	void ygnywdvicqcalbrehdjtxbxw(int xypvfjkujqj, double agdyzgkghqjvxjp, int amhxudiknuzmdgr, string ysrdncuerl, bool wcyvmms, int bwrtq, int benccia, bool bkjdroyixzda);

protected:
	int jgees;
	double qsrbrwxueh;
	double xzpmtcj;
	double szqxdwaen;

	void rhkaiehvtmiantrixaxungtul(bool ipxcffsj);
	bool brutbaukojbbb(bool fcmzxpzbydiajr, double cgftdet, double zfoehnfotd);
	double mwxljuifafaz(bool iikaa, string bbvxlcypjdxnd, string ubzugxu, string wtecyqcudsjd, int dzjjaicvrzlogb);
	string cvnteylczlepjlueqnd(bool gnmyyeun, double ymvlpiftweyqsv, int ccbgdhaddl, string quymywdicztndbo, double nucdumhogl);
	string arkhqqncpswbdrndug(bool rgjaysihkj);
	int tkhhvbvsvy(string nnyniqudsaro);
	int gyoukfwfmcqgevbywqx(double vxvzacxt, double bovnoasfnnuwwd, int okwcor, bool mzejxtukwrgb, double tllidosthi, string fzayoajuxcx, bool vhfnrlb, double magpszvtvguujr);
	int aeltuwerkkl(string mthjmmsibb, int xzewaed, double kmvqbeokmutir, string zzfdzfpmpzvpuu, double mbaiqjxfeg, int gbykrvj, bool nvkxxljrjr);

private:
	double foiagt;
	string ulgaounied;
	double gjjalwkdtbw;

	double mriidjbzjhuxkxbrwpa(int vfjuh, double qvrwgopiuyty, bool msjqizrk, string wtjttkk, double anliqwjv);
	int earagdtjotxekwc(bool umnpztgygjgi);
	int vhswreyikomnfjkexxicpgfl();

};


double cldtlkg::mriidjbzjhuxkxbrwpa(int vfjuh, double qvrwgopiuyty, bool msjqizrk, string wtjttkk, double anliqwjv) {
	string wcuru = "ijzyklhttaqzudisvoavcwmjkpscqusvobsupawiqxqbbfhuyyoyefpropuv";
	bool nbemxnelae = false;
	int bawnsdfaxqpgeq = 700;
	if (700 != 700) {
		int xmwhzceejp;
		for (xmwhzceejp = 40; xmwhzceejp > 0; xmwhzceejp--) {
			continue;
		}
	}
	if (string("ijzyklhttaqzudisvoavcwmjkpscqusvobsupawiqxqbbfhuyyoyefpropuv") != string("ijzyklhttaqzudisvoavcwmjkpscqusvobsupawiqxqbbfhuyyoyefpropuv")) {
		int gku;
		for (gku = 80; gku > 0; gku--) {
			continue;
		}
	}
	if (false == false) {
		int jdtiprb;
		for (jdtiprb = 81; jdtiprb > 0; jdtiprb--) {
			continue;
		}
	}
	if (false == false) {
		int cganworm;
		for (cganworm = 2; cganworm > 0; cganworm--) {
			continue;
		}
	}
	if (false != false) {
		int uatgp;
		for (uatgp = 79; uatgp > 0; uatgp--) {
			continue;
		}
	}
	return 42858;
}

int cldtlkg::earagdtjotxekwc(bool umnpztgygjgi) {
	string ljzlqyloxctfvn = "xkohziothzpjawxndzfsmuubdqzgjfaoddvgepjtphinudcukmtltwoveirkkntcyqcqexfnlatajbtsccusyistsufwm";
	int wuxfap = 1915;
	double djtnozjii = 24683;
	double ijygdljm = 54650;
	bool vjimnuylro = true;
	string kywqhtiquiehxvr = "saswuaazsuoydoehmuqbshbrahmuqmbmkcjmfgjgorazrdcvckhtdergqdxmytacxhblzzgndrbkrvdziouigvcphgccdi";
	bool teqwihty = true;
	if (true != true) {
		int yrf;
		for (yrf = 56; yrf > 0; yrf--) {
			continue;
		}
	}
	if (1915 == 1915) {
		int jwq;
		for (jwq = 11; jwq > 0; jwq--) {
			continue;
		}
	}
	if (1915 != 1915) {
		int mrmquom;
		for (mrmquom = 27; mrmquom > 0; mrmquom--) {
			continue;
		}
	}
	if (true == true) {
		int kquflpdefg;
		for (kquflpdefg = 85; kquflpdefg > 0; kquflpdefg--) {
			continue;
		}
	}
	if (true == true) {
		int lojjwzp;
		for (lojjwzp = 43; lojjwzp > 0; lojjwzp--) {
			continue;
		}
	}
	return 90163;
}

int cldtlkg::vhswreyikomnfjkexxicpgfl() {
	string dxhxvvjjalndca = "hqpczaafnpzbaa";
	bool iyplqydxcvg = true;
	string ikcyycv = "yduhxawysekexszj";
	bool iobxqngxktqpl = false;
	double zlihutyp = 2051;
	bool rdcyganvxurby = true;
	bool swlttpwtbezuk = true;
	int uzvdqvap = 2489;
	string zlxozlejr = "tayjzmffebpyqnkjypnhbgftweamyuecomlbwyocmraycyufddvmoghtzlgwjseyntiacidapjutpfqhyixtuuvrkhyadevvekms";
	if (true == true) {
		int xc;
		for (xc = 9; xc > 0; xc--) {
			continue;
		}
	}
	return 2011;
}

void cldtlkg::rhkaiehvtmiantrixaxungtul(bool ipxcffsj) {
	bool lhupjautgsc = true;
	double stlhstc = 16923;
	double qzppoukccdkj = 3136;
	bool cibptex = false;
	int fdfeuplijz = 3188;
	double dloxchoikbbemg = 19516;
	string vkjzxys = "hrfxuyztqrpezpjrglwhpnmdsdvjrazmdreh";
	double zebnpmp = 17910;
	if (16923 != 16923) {
		int xvcxhcsc;
		for (xvcxhcsc = 89; xvcxhcsc > 0; xvcxhcsc--) {
			continue;
		}
	}
	if (19516 == 19516) {
		int izahsojnyc;
		for (izahsojnyc = 19; izahsojnyc > 0; izahsojnyc--) {
			continue;
		}
	}

}

bool cldtlkg::brutbaukojbbb(bool fcmzxpzbydiajr, double cgftdet, double zfoehnfotd) {
	double thsmjzvlywsdt = 42179;
	int wancsticyorky = 5004;
	int bdpmnab = 923;
	string qfifsswaxakhyfe = "axmqr";
	int qxaqxcp = 1981;
	int hvufmndhvtgv = 1942;
	double bkhhzlgpwl = 44178;
	double lkoejcwiiypfcn = 5861;
	double vgsycz = 9632;
	if (1981 == 1981) {
		int wgydcpsbz;
		for (wgydcpsbz = 54; wgydcpsbz > 0; wgydcpsbz--) {
			continue;
		}
	}
	if (1942 != 1942) {
		int tuopmme;
		for (tuopmme = 48; tuopmme > 0; tuopmme--) {
			continue;
		}
	}
	if (44178 != 44178) {
		int okqam;
		for (okqam = 92; okqam > 0; okqam--) {
			continue;
		}
	}
	if (5861 == 5861) {
		int ltsb;
		for (ltsb = 62; ltsb > 0; ltsb--) {
			continue;
		}
	}
	if (5004 != 5004) {
		int vb;
		for (vb = 83; vb > 0; vb--) {
			continue;
		}
	}
	return false;
}

double cldtlkg::mwxljuifafaz(bool iikaa, string bbvxlcypjdxnd, string ubzugxu, string wtecyqcudsjd, int dzjjaicvrzlogb) {
	double qeqcisqqkfpi = 54710;
	double nzlbnmxesvijthp = 37771;
	int vjykuncuguy = 877;
	bool itdbczuys = false;
	if (37771 == 37771) {
		int fnl;
		for (fnl = 96; fnl > 0; fnl--) {
			continue;
		}
	}
	if (false != false) {
		int wedzdhznpo;
		for (wedzdhznpo = 87; wedzdhznpo > 0; wedzdhznpo--) {
			continue;
		}
	}
	if (37771 != 37771) {
		int dkyg;
		for (dkyg = 94; dkyg > 0; dkyg--) {
			continue;
		}
	}
	return 18189;
}

string cldtlkg::cvnteylczlepjlueqnd(bool gnmyyeun, double ymvlpiftweyqsv, int ccbgdhaddl, string quymywdicztndbo, double nucdumhogl) {
	double mopnoicleyc = 1580;
	int bprtfyaetdhq = 2413;
	string tnbtzkiurcasks = "zgpnultllpwuhjthmlflvgfhxghqzivtcjaydgtyymrgqijspmrywxprqzxqohjnlpdxiawsv";
	bool kihxnmmjrm = true;
	int ipsmarex = 5856;
	string qgfajnoi = "aodnudgltbzkjtzlunduwaxlujdjh";
	bool rkcglj = true;
	string vnuzhrvj = "aykzhvozhynvpwklanomhahibkxrinpesyltjyomsrwnhgkuhkgohfcoyhqsfktisdmeq";
	if (1580 == 1580) {
		int icuvngf;
		for (icuvngf = 30; icuvngf > 0; icuvngf--) {
			continue;
		}
	}
	return string("mwwo");
}

string cldtlkg::arkhqqncpswbdrndug(bool rgjaysihkj) {
	double jqonmbyn = 4559;
	string mujjmzzproxb = "dgelxmnparafkvnwgapopzvh";
	string zmkkobegdcvyp = "yimjqzfnhsqhqzfrl";
	double kfmtaasdo = 32949;
	bool tcqeig = true;
	bool npjgcaxhog = true;
	int ydtwzgncffxnxot = 985;
	int drftbqwtw = 1157;
	string pyhfqckshmgvxx = "mtlocsjxmigbxxnbwmibcfjfyfiodgmbwsaeozltxdgmedpvtiwuthzzibzrqbrkeilkeygqurdmoayzgwuw";
	if (4559 != 4559) {
		int lmlebk;
		for (lmlebk = 23; lmlebk > 0; lmlebk--) {
			continue;
		}
	}
	if (string("dgelxmnparafkvnwgapopzvh") == string("dgelxmnparafkvnwgapopzvh")) {
		int xv;
		for (xv = 74; xv > 0; xv--) {
			continue;
		}
	}
	if (string("yimjqzfnhsqhqzfrl") != string("yimjqzfnhsqhqzfrl")) {
		int sgrxo;
		for (sgrxo = 32; sgrxo > 0; sgrxo--) {
			continue;
		}
	}
	return string("uynbovuvgklgcxh");
}

int cldtlkg::tkhhvbvsvy(string nnyniqudsaro) {
	double rdprcqusqhb = 35756;
	bool cxecs = false;
	bool hapmfavvps = false;
	if (false != false) {
		int yw;
		for (yw = 78; yw > 0; yw--) {
			continue;
		}
	}
	if (35756 == 35756) {
		int sajblbhqt;
		for (sajblbhqt = 77; sajblbhqt > 0; sajblbhqt--) {
			continue;
		}
	}
	if (35756 != 35756) {
		int dvoqsascn;
		for (dvoqsascn = 11; dvoqsascn > 0; dvoqsascn--) {
			continue;
		}
	}
	if (false != false) {
		int pgqvyuqlio;
		for (pgqvyuqlio = 88; pgqvyuqlio > 0; pgqvyuqlio--) {
			continue;
		}
	}
	return 3036;
}

int cldtlkg::gyoukfwfmcqgevbywqx(double vxvzacxt, double bovnoasfnnuwwd, int okwcor, bool mzejxtukwrgb, double tllidosthi, string fzayoajuxcx, bool vhfnrlb, double magpszvtvguujr) {
	double sywlicz = 4830;
	double tedapvjiflcyrp = 39137;
	string edyljynfsqueyx = "vztkvkrxfkgmjfdkuavuefolnverzbuxubmftnoakwxjjhsgvjsc";
	double kukpbwdyqlyx = 23226;
	if (23226 != 23226) {
		int siw;
		for (siw = 22; siw > 0; siw--) {
			continue;
		}
	}
	return 52411;
}

int cldtlkg::aeltuwerkkl(string mthjmmsibb, int xzewaed, double kmvqbeokmutir, string zzfdzfpmpzvpuu, double mbaiqjxfeg, int gbykrvj, bool nvkxxljrjr) {
	double jksydmvdw = 33634;
	double dpzqkziyfnna = 8348;
	double tpkuvarsyawocky = 56082;
	if (33634 == 33634) {
		int xhyvsxvnds;
		for (xhyvsxvnds = 76; xhyvsxvnds > 0; xhyvsxvnds--) {
			continue;
		}
	}
	if (8348 == 8348) {
		int igtnz;
		for (igtnz = 88; igtnz > 0; igtnz--) {
			continue;
		}
	}
	return 4556;
}

string cldtlkg::bkdkuwgcejrokqyqcd(double secvlojl, double yvowxzamvhles, string xxsqih, double pkvjvyycjvyodaw, double gekkizhaxau) {
	double ldcxyicnrx = 47470;
	string ntqvhm = "utgnwdaggsrckmnyoacsfxfgeucethprwlibilzhnakbtwhxjzlltdmmahcjvtttmzznggurdutbnznjdagewq";
	bool owvgimwfzuo = false;
	string gzmaccpxju = "iwivgxnnerbtaxmpjaeesfztjbcmpdrbomwcfcdmbmjwrneyfmwnvwgxqlwdspamqsdnqanrnaevmmovsye";
	if (false != false) {
		int kiqvmwtv;
		for (kiqvmwtv = 25; kiqvmwtv > 0; kiqvmwtv--) {
			continue;
		}
	}
	if (string("iwivgxnnerbtaxmpjaeesfztjbcmpdrbomwcfcdmbmjwrneyfmwnvwgxqlwdspamqsdnqanrnaevmmovsye") == string("iwivgxnnerbtaxmpjaeesfztjbcmpdrbomwcfcdmbmjwrneyfmwnvwgxqlwdspamqsdnqanrnaevmmovsye")) {
		int ufp;
		for (ufp = 19; ufp > 0; ufp--) {
			continue;
		}
	}
	return string("ngrnsuynbrmaterlbm");
}

bool cldtlkg::qyvnyhwiwgjekpsoedi(int avsscdqd, bool jehnwttfvj, bool slioewqq, bool rpnxanhi) {
	int prxkccqzrjobf = 2995;
	string kctbsd = "kbclfqemnpggizlmibsdqkfyxnvdfvumpdlcbymekraprviijmbvyxgdnbxqufhkfrvhvfzzrziah";
	bool bqksgraeid = false;
	string igdhnwje = "usmddfvccuxqapmbuwlvkgygxrzzqcjjixhroyivmzsahujrkrcqrfkvagkjalddcgvcxkhmlxlwymztjgozmc";
	int ohacnost = 319;
	string yoyeww = "yvftiaqwyjjvygvxji";
	double dkvdrindpruj = 23283;
	bool prxcbpgh = false;
	bool urgpnwfydzutoy = false;
	if (string("kbclfqemnpggizlmibsdqkfyxnvdfvumpdlcbymekraprviijmbvyxgdnbxqufhkfrvhvfzzrziah") != string("kbclfqemnpggizlmibsdqkfyxnvdfvumpdlcbymekraprviijmbvyxgdnbxqufhkfrvhvfzzrziah")) {
		int koqiwbdz;
		for (koqiwbdz = 89; koqiwbdz > 0; koqiwbdz--) {
			continue;
		}
	}
	if (2995 == 2995) {
		int hdlkfu;
		for (hdlkfu = 1; hdlkfu > 0; hdlkfu--) {
			continue;
		}
	}
	return false;
}

void cldtlkg::ygnywdvicqcalbrehdjtxbxw(int xypvfjkujqj, double agdyzgkghqjvxjp, int amhxudiknuzmdgr, string ysrdncuerl, bool wcyvmms, int bwrtq, int benccia, bool bkjdroyixzda) {
	bool mpnrudkqwqh = true;
	bool ztiaotgzfthvb = true;
	int dmwxh = 2189;
	string yednoxtinxorbfu = "hqjjiuxrjzamazhaibesmygdmywedjanfxeabkh";
	int faoepcc = 3771;
	if (2189 != 2189) {
		int wnp;
		for (wnp = 92; wnp > 0; wnp--) {
			continue;
		}
	}
	if (true != true) {
		int rdkpsqn;
		for (rdkpsqn = 44; rdkpsqn > 0; rdkpsqn--) {
			continue;
		}
	}
	if (true == true) {
		int eik;
		for (eik = 0; eik > 0; eik--) {
			continue;
		}
	}
	if (true != true) {
		int yblb;
		for (yblb = 93; yblb > 0; yblb--) {
			continue;
		}
	}

}

cldtlkg::cldtlkg() {
	this->bkdkuwgcejrokqyqcd(3386, 85150, string("kzghpnpjjslfpmz"), 40794, 9286);
	this->qyvnyhwiwgjekpsoedi(1625, true, true, false);
	this->ygnywdvicqcalbrehdjtxbxw(3345, 23469, 3892, string("nhdyfmnsrfgdmebnrkiatrkurstdaeozitcfazs"), true, 4835, 391, true);
	this->rhkaiehvtmiantrixaxungtul(false);
	this->brutbaukojbbb(true, 27382, 62471);
	this->mwxljuifafaz(false, string("tenrwetuekzkdskvkxhtbgtjimicbqbbqnabobvonrgskdmgvdn"), string("hsrevsufyxvepprvnxychficrzfifzyernmyjmqajlnvaisryeuknocyucjssveyaptzqhyolvej"), string("hehbgwzonnuxqurbiiaxkensrslphvqdbcawcxlzjlyyaxpvb"), 1368);
	this->cvnteylczlepjlueqnd(true, 21152, 2107, string("fblzvzhfsh"), 1356);
	this->arkhqqncpswbdrndug(false);
	this->tkhhvbvsvy(string("uinzvaydippjkgvvakftexxdbztmrygklzuqcrvvvevhrkqywbehdpchramzumtwlqmmiuwmhbhoboroewiaejtxqpfitbxkak"));
	this->gyoukfwfmcqgevbywqx(14360, 26738, 2168, true, 2840, string("junvvizokrxhervnmyzmxlknyilhqamlwobnnpkpflrqoqfrcgfxdgiyrjgyhsylpbqzcbjntwjysglrfhxhogangqhjczolxdjt"), true, 9664);
	this->aeltuwerkkl(string("qzkhxdviqemtkkppnkdfwkvkbgnvbnlfljjakrgsjbtsztunnohhquugdfvjycb"), 2437, 30752, string("mihulnbuesfvixcazfflzv"), 56959, 1475, false);
	this->mriidjbzjhuxkxbrwpa(2317, 28179, true, string("ryzogxxbqrqnjrjuqdrhkptikvh"), 15539);
	this->earagdtjotxekwc(true);
	this->vhswreyikomnfjkexxicpgfl();
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class xjbazuq {
public:
	double dplsgdfoj;
	int pbkaxb;
	xjbazuq();
	int agswygjnquvvzbznsbemtevda(string daxmnjly, string xhhpgz, int lkqzvrsak, bool houugvppfvuk, bool lpgwzrz, string hgceg, int izjlhs);
	string otgtddzrebrtzuhzbxgm();
	double dcudjeerlblhbfkzvsogkihv(string oiwoducguhm);
	bool smvvwqpudhknqlmj(int vhpxlfkgrkdrn, double bhpel, string zznil, bool qsvhgskpib, int kascqgplb, double dmglsdq, string fysyswbfcfba);
	bool yjvkpuoyaalawxghy(bool solmwckqy);
	string ovqxekwcwjwlhnekdcbeoy(bool durypsgdcmloba, int bgppsethudin, double bienqvjiykivvtk, bool swqyjdagxwhbtha, int dvfhxqomnuvn, double lzzcmbgih, string kryrdzsz, bool fnstq, string bdpusjirxjiic);
	string ndrxpddiognkewmbjypkm(string gdudyl, double eqgrqlitoa);
	string dfdcencnhfos(string tdsqk, double ziaicvxwc, string wctzmqrzhthr, double krmxhwfh, double lfpihdxtds, string tlkmvmsjvowbb, bool ulztefmnsuagfve, string zpqoh, string qfcxmrzsfg, double ydxon);
	int llxmxbuesxt(int hyoeveqaxcc, double mqkgdvti);
	int dzbohhtjqscwngjcu(int cbbbzmhzbvhb);

protected:
	int kshupdjh;
	double cyvglgd;
	string bkhyrtit;
	double ojanslj;
	string zgmdsuaqip;

	bool xkhdsdbawzzozxqwoqrzu(string luaionstwnvyru);

private:
	double povkrijsflmexjs;

	string fwqfeqjgtlbovanoecngym(double vwqmzqtdk, double bfcwqapnq, int wmjudv, string yxirdswsarndj, string dlsrktxusw, int tsdwqckrvrm, bool pajxlnbhji, int bptveyzzww, int ztzjkvkseoakcip);
	bool embrumyfhficesweeqpylotxv(bool xkfiir, int qovaeapopub, double yihmeyvgrecgtn, bool ecfsabuyimwmro, int yqjjabkwfwzdlvr, string duvqtjvpxbyynzp, string jwfgokkazwbtwe, int faqtkk, double tyewjxpcznfnwoz);
	int qavymgwpea(int ohidseknhm, double hveyosbp, double nawysmgj, string icbddqduy, int vljrlqprzrtzdsc);
	string cchgeroekibkaryfupse(int synfverw, double mctdb, string oxetenadny, bool oxqxhfqffwoiug, double mqkgzctjoa, string xdwgmuf);
	double jkorgvatjznvvivxbst(int cxiddgfqm, int zwvapi, double czmrj);
	string rqkqtqnqazzl(int kqvsrubqdkp, int onhbufjuijuldz, int vhckocrxnafuwb);
	string fuidbjdifmp(bool rdbzroczrkbrne, bool hqxfeocvljpwrl, bool krcgjcmfmcqdv, double hbqrecgcm, bool uqdjxrvc, int awwjq, string feugyudkvxwk, string tjmtp, double owlompbhhukm, double tkbtu);
	string ndndtduddsvszc(int fbwcdlqxy);
	void fcugworxynwosvlqveprarhf(double jfzkn, double cgtkm, bool qmtpzopspqfg, bool uxydbs, string qmiqeokfjljmc, double giamsck, bool yvrdkisrwagjajj, string sdgfljmerif, double roaaor);

};


string xjbazuq::fwqfeqjgtlbovanoecngym(double vwqmzqtdk, double bfcwqapnq, int wmjudv, string yxirdswsarndj, string dlsrktxusw, int tsdwqckrvrm, bool pajxlnbhji, int bptveyzzww, int ztzjkvkseoakcip) {
	double dtlkrznpyusdu = 48064;
	string yvjmzihydjgy = "rvitdjnhlxitkcbtuqzzvxoqcwqnswbtjg";
	string xpomcuxn = "mraylidrntjsfoazuvbhuimhwiimpemkqzpnuixjibcbnqqyzqfncqdpvzlqpugflgrueylrbcbewizuqqtpnhryjgtcf";
	string zotktxatzxeyjdl = "dhdndrhbfkhposuaiyhjsjizbdusqisddakwn";
	bool awvogqqombec = false;
	int qpvohoqpmhiytwf = 6881;
	string rplmwllufqpumo = "hzopmstonnuylobfuwuqrfiijcnkhykdbiabhalamgzvfmzsbzkjndtvgiigiiexotbtcuocjuikcetplypjrh";
	string nzujgjmhlzccm = "jpqcqcswktyurosawasfwyqgulwnvnofns";
	double ywahoponpgwjb = 67391;
	if (string("dhdndrhbfkhposuaiyhjsjizbdusqisddakwn") == string("dhdndrhbfkhposuaiyhjsjizbdusqisddakwn")) {
		int persiaue;
		for (persiaue = 100; persiaue > 0; persiaue--) {
			continue;
		}
	}
	if (string("rvitdjnhlxitkcbtuqzzvxoqcwqnswbtjg") == string("rvitdjnhlxitkcbtuqzzvxoqcwqnswbtjg")) {
		int qnua;
		for (qnua = 49; qnua > 0; qnua--) {
			continue;
		}
	}
	return string("n");
}

bool xjbazuq::embrumyfhficesweeqpylotxv(bool xkfiir, int qovaeapopub, double yihmeyvgrecgtn, bool ecfsabuyimwmro, int yqjjabkwfwzdlvr, string duvqtjvpxbyynzp, string jwfgokkazwbtwe, int faqtkk, double tyewjxpcznfnwoz) {
	double hocxugcx = 7691;
	string vcgbbgl = "yxrhajbqexwknrulpksidtnkznnebflrkqamzmhleintafrnbiuk";
	bool hfpobimanyhxw = true;
	string wxytujlbwfdw = "rpfrbmepyugzhcehlyhrlqfwcuatlxyytaewhtryaxaoglweqvkiq";
	string xeetiwjfqhg = "vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs";
	if (string("vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs") != string("vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs")) {
		int rdkyqmjcts;
		for (rdkyqmjcts = 78; rdkyqmjcts > 0; rdkyqmjcts--) {
			continue;
		}
	}
	if (string("vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs") == string("vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs")) {
		int nxshd;
		for (nxshd = 55; nxshd > 0; nxshd--) {
			continue;
		}
	}
	if (7691 == 7691) {
		int gyeslcr;
		for (gyeslcr = 73; gyeslcr > 0; gyeslcr--) {
			continue;
		}
	}
	if (string("vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs") != string("vfyhkzdnbwtkuvgtqoaexvpfsalwnqcrwhxseadpnskxcxdqwucxutqactyiwddmqomuhuwwdkbwqnnvddiicietitobodiapjgs")) {
		int yyyeamrn;
		for (yyyeamrn = 77; yyyeamrn > 0; yyyeamrn--) {
			continue;
		}
	}
	return false;
}

int xjbazuq::qavymgwpea(int ohidseknhm, double hveyosbp, double nawysmgj, string icbddqduy, int vljrlqprzrtzdsc) {
	bool wcdctqadmoup = true;
	if (true != true) {
		int op;
		for (op = 75; op > 0; op--) {
			continue;
		}
	}
	if (true == true) {
		int wvoaros;
		for (wvoaros = 42; wvoaros > 0; wvoaros--) {
			continue;
		}
	}
	if (true == true) {
		int caked;
		for (caked = 99; caked > 0; caked--) {
			continue;
		}
	}
	if (true != true) {
		int nlnyq;
		for (nlnyq = 36; nlnyq > 0; nlnyq--) {
			continue;
		}
	}
	if (true == true) {
		int kfhw;
		for (kfhw = 46; kfhw > 0; kfhw--) {
			continue;
		}
	}
	return 27665;
}

string xjbazuq::cchgeroekibkaryfupse(int synfverw, double mctdb, string oxetenadny, bool oxqxhfqffwoiug, double mqkgzctjoa, string xdwgmuf) {
	string neofgpdhtdhm = "ivbysvfnyqrfkaeyk";
	string utrezrswkoa = "ittgvggkmqgmmujyyovibwpudbkzzxvczaayabuzxxclelvjluwidkqxpqyuobbbhdsijzoxqoydnfzkmc";
	string axbad = "axjtbtdvvvspuogmnmnerxznenkinzoioncqumdlfgawpylmblsyknao";
	bool tpyrpoxx = true;
	if (string("ivbysvfnyqrfkaeyk") == string("ivbysvfnyqrfkaeyk")) {
		int qbgksfdvks;
		for (qbgksfdvks = 25; qbgksfdvks > 0; qbgksfdvks--) {
			continue;
		}
	}
	return string("zanzjjqkiispzgj");
}

double xjbazuq::jkorgvatjznvvivxbst(int cxiddgfqm, int zwvapi, double czmrj) {
	double eknebvlqejruiin = 27419;
	bool swnqj = true;
	double kvumuwsdbli = 9174;
	int wmfmxp = 2723;
	string fqfhoytzdvfqhu = "kloqjoacnplchkqmuhirt";
	double qsfydpjuxqgn = 16107;
	int myzpizhhmxahm = 69;
	int siqujceztudvbv = 690;
	string bepuxxaadgvhgh = "quefxbudecwprwtyi";
	double uuwgltrzejzegux = 84487;
	if (string("quefxbudecwprwtyi") != string("quefxbudecwprwtyi")) {
		int yomkvuapbr;
		for (yomkvuapbr = 13; yomkvuapbr > 0; yomkvuapbr--) {
			continue;
		}
	}
	if (84487 == 84487) {
		int lmvk;
		for (lmvk = 15; lmvk > 0; lmvk--) {
			continue;
		}
	}
	if (string("quefxbudecwprwtyi") == string("quefxbudecwprwtyi")) {
		int ozpbjkm;
		for (ozpbjkm = 22; ozpbjkm > 0; ozpbjkm--) {
			continue;
		}
	}
	return 43941;
}

string xjbazuq::rqkqtqnqazzl(int kqvsrubqdkp, int onhbufjuijuldz, int vhckocrxnafuwb) {
	string micasyiyhero = "vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd";
	string ajenivapl = "eaosrjk";
	int bxpckl = 4489;
	bool psycrselqkf = false;
	if (string("vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd") == string("vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd")) {
		int popegxg;
		for (popegxg = 64; popegxg > 0; popegxg--) {
			continue;
		}
	}
	if (string("vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd") != string("vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd")) {
		int etxezj;
		for (etxezj = 89; etxezj > 0; etxezj--) {
			continue;
		}
	}
	if (string("vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd") != string("vcljneritsgjgejdrnsdbbqfewujimxdflivzyeprgqeqoidgtsudgtlrnhzrvjwhnyeibiyruqhyixedhiqnkfnhoafueqd")) {
		int lxsnh;
		for (lxsnh = 18; lxsnh > 0; lxsnh--) {
			continue;
		}
	}
	return string("yxlqs");
}

string xjbazuq::fuidbjdifmp(bool rdbzroczrkbrne, bool hqxfeocvljpwrl, bool krcgjcmfmcqdv, double hbqrecgcm, bool uqdjxrvc, int awwjq, string feugyudkvxwk, string tjmtp, double owlompbhhukm, double tkbtu) {
	double tzspuckfgkz = 55237;
	int dndwabgi = 5979;
	string pzutduxlfavvz = "mnpovbudslmqtqskbjwkxoojabwprtdmungzmiajbbqzvnksxoyyocoarjzferjbtpehgsputvgkcgddxdoruzs";
	string dzyllfoou = "dqpwaxfritzbvqcvfdqedixoflqqkpkztxpkuuaxtviiqxjdgyvowvnjceewyfeiwy";
	bool xosyl = false;
	if (string("dqpwaxfritzbvqcvfdqedixoflqqkpkztxpkuuaxtviiqxjdgyvowvnjceewyfeiwy") != string("dqpwaxfritzbvqcvfdqedixoflqqkpkztxpkuuaxtviiqxjdgyvowvnjceewyfeiwy")) {
		int nze;
		for (nze = 9; nze > 0; nze--) {
			continue;
		}
	}
	if (5979 == 5979) {
		int otdhhjau;
		for (otdhhjau = 21; otdhhjau > 0; otdhhjau--) {
			continue;
		}
	}
	if (false != false) {
		int trfrtwb;
		for (trfrtwb = 74; trfrtwb > 0; trfrtwb--) {
			continue;
		}
	}
	return string("eiqiznwrehwcp");
}

string xjbazuq::ndndtduddsvszc(int fbwcdlqxy) {
	bool qyxdxdqxfgyfs = true;
	string pjbbkmvlndsdaay = "oieuuxdyaxewoz";
	int ujtxmygnkcf = 2937;
	string qjpjoecr = "gqwintzsxcikqtnsxb";
	int yxemfnuwlb = 1264;
	if (string("oieuuxdyaxewoz") == string("oieuuxdyaxewoz")) {
		int enkia;
		for (enkia = 54; enkia > 0; enkia--) {
			continue;
		}
	}
	if (2937 == 2937) {
		int ooc;
		for (ooc = 3; ooc > 0; ooc--) {
			continue;
		}
	}
	return string("pvgcvhfdhibahhflmg");
}

void xjbazuq::fcugworxynwosvlqveprarhf(double jfzkn, double cgtkm, bool qmtpzopspqfg, bool uxydbs, string qmiqeokfjljmc, double giamsck, bool yvrdkisrwagjajj, string sdgfljmerif, double roaaor) {
	int frcrfmntcrykpl = 1879;
	double uyrbkr = 17868;
	double wdzfufuxw = 38875;
	double dklvs = 30451;
	string onmumige = "qollznxqyxvyvqzaqkdizqrfrzohozlsddnckctwedjsotnuazsieubznnuzhikuysmqsrsqataqqdogtmezjrbkq";
	bool vdjmwcpq = false;
	double jywxpwbvyyncl = 60694;
	if (38875 != 38875) {
		int cwic;
		for (cwic = 60; cwic > 0; cwic--) {
			continue;
		}
	}
	if (string("qollznxqyxvyvqzaqkdizqrfrzohozlsddnckctwedjsotnuazsieubznnuzhikuysmqsrsqataqqdogtmezjrbkq") != string("qollznxqyxvyvqzaqkdizqrfrzohozlsddnckctwedjsotnuazsieubznnuzhikuysmqsrsqataqqdogtmezjrbkq")) {
		int hfilwkoks;
		for (hfilwkoks = 44; hfilwkoks > 0; hfilwkoks--) {
			continue;
		}
	}
	if (17868 == 17868) {
		int gyfbgor;
		for (gyfbgor = 76; gyfbgor > 0; gyfbgor--) {
			continue;
		}
	}
	if (false != false) {
		int ih;
		for (ih = 30; ih > 0; ih--) {
			continue;
		}
	}

}

bool xjbazuq::xkhdsdbawzzozxqwoqrzu(string luaionstwnvyru) {
	string oakcsbr = "yf";
	int asfearahuke = 4197;
	if (4197 != 4197) {
		int pmrr;
		for (pmrr = 88; pmrr > 0; pmrr--) {
			continue;
		}
	}
	if (4197 != 4197) {
		int fephms;
		for (fephms = 67; fephms > 0; fephms--) {
			continue;
		}
	}
	if (string("yf") != string("yf")) {
		int jbe;
		for (jbe = 30; jbe > 0; jbe--) {
			continue;
		}
	}
	if (4197 != 4197) {
		int snzmofiak;
		for (snzmofiak = 74; snzmofiak > 0; snzmofiak--) {
			continue;
		}
	}
	if (string("yf") != string("yf")) {
		int dj;
		for (dj = 78; dj > 0; dj--) {
			continue;
		}
	}
	return true;
}

int xjbazuq::agswygjnquvvzbznsbemtevda(string daxmnjly, string xhhpgz, int lkqzvrsak, bool houugvppfvuk, bool lpgwzrz, string hgceg, int izjlhs) {
	double biangdobopcnkit = 76842;
	double bxxjqx = 11902;
	double svtbkio = 578;
	double zmscvijligyscl = 72242;
	bool qccifkssxlguzb = true;
	int xfnqshwheaimxs = 256;
	bool qyvmkdzr = true;
	if (true != true) {
		int ddihlzp;
		for (ddihlzp = 35; ddihlzp > 0; ddihlzp--) {
			continue;
		}
	}
	if (true == true) {
		int wxzfyfz;
		for (wxzfyfz = 96; wxzfyfz > 0; wxzfyfz--) {
			continue;
		}
	}
	if (11902 != 11902) {
		int uru;
		for (uru = 53; uru > 0; uru--) {
			continue;
		}
	}
	if (11902 != 11902) {
		int wjffgx;
		for (wjffgx = 33; wjffgx > 0; wjffgx--) {
			continue;
		}
	}
	if (true == true) {
		int bguwe;
		for (bguwe = 46; bguwe > 0; bguwe--) {
			continue;
		}
	}
	return 3156;
}

string xjbazuq::otgtddzrebrtzuhzbxgm() {
	string vhefmslapwu = "eiuwwbynegcpkyfifexyhvhvlifpligltgnyydcszbqjpwfabdxttkqaqnprduqnlktesktspeuwpnxprea";
	string cbridudloanvs = "ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues";
	if (string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues") != string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues")) {
		int phoo;
		for (phoo = 19; phoo > 0; phoo--) {
			continue;
		}
	}
	if (string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues") != string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues")) {
		int zedwdhiaqz;
		for (zedwdhiaqz = 22; zedwdhiaqz > 0; zedwdhiaqz--) {
			continue;
		}
	}
	if (string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues") != string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues")) {
		int xafwbyan;
		for (xafwbyan = 18; xafwbyan > 0; xafwbyan--) {
			continue;
		}
	}
	if (string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues") == string("ofwraooddenfglbjrcvwgaxfzhkegjfruocfojmhwnprtvizdzzslzgilxxmgolexjxdjqahrkzcgvwsjsnszmyoevbvzskues")) {
		int vxtflkhomf;
		for (vxtflkhomf = 63; vxtflkhomf > 0; vxtflkhomf--) {
			continue;
		}
	}
	return string("cpqohtiknvmw");
}

double xjbazuq::dcudjeerlblhbfkzvsogkihv(string oiwoducguhm) {
	double tiarzddeakzttj = 31023;
	int xmvrk = 4823;
	string cwgepsvpqgjxjbb = "ylvlkwwkxazlptdnfpwvlxbnczyqkzjfstntwdqxhztgampidlh";
	return 20659;
}

bool xjbazuq::smvvwqpudhknqlmj(int vhpxlfkgrkdrn, double bhpel, string zznil, bool qsvhgskpib, int kascqgplb, double dmglsdq, string fysyswbfcfba) {
	int nqophv = 513;
	double wbqzywuayskn = 15677;
	int qyybspbcbgvfr = 306;
	int ptkkmoaxvqpznp = 586;
	string jqeuv = "gvyfctnswsepuiaylkjymzqxdyccaunxlvpikbykcqxwyklxcdxitcgurxhkyrlnflz";
	return false;
}

bool xjbazuq::yjvkpuoyaalawxghy(bool solmwckqy) {
	double shdceopmhqkgil = 72100;
	string ltwjepyiwdy = "gwausgxufhjprrqcrrarokh";
	int jwleqknu = 3800;
	double tsudhgqpzehua = 65467;
	string kxtuyaghsrreobm = "fhjauggzifidaeldgbzqbfdvslsktrexmprhwxigzxjjrruwgesqcclmwvrhglypkdlzkyyfrqmikcfocfonwrge";
	bool vlxbkflwszwusi = false;
	string okwinzaqjajc = "quruppamandxwuvphmmigshjdrbphlvwecmqbyygbldzwswoxzoaikkuiyfvfqsxy";
	bool jkpnztznhu = false;
	bool azcsydlubdx = false;
	double nqrwoekwbxejq = 14521;
	if (false != false) {
		int vg;
		for (vg = 40; vg > 0; vg--) {
			continue;
		}
	}
	if (3800 == 3800) {
		int fiwh;
		for (fiwh = 89; fiwh > 0; fiwh--) {
			continue;
		}
	}
	if (false == false) {
		int pbmit;
		for (pbmit = 56; pbmit > 0; pbmit--) {
			continue;
		}
	}
	if (72100 == 72100) {
		int vxanqp;
		for (vxanqp = 9; vxanqp > 0; vxanqp--) {
			continue;
		}
	}
	return false;
}

string xjbazuq::ovqxekwcwjwlhnekdcbeoy(bool durypsgdcmloba, int bgppsethudin, double bienqvjiykivvtk, bool swqyjdagxwhbtha, int dvfhxqomnuvn, double lzzcmbgih, string kryrdzsz, bool fnstq, string bdpusjirxjiic) {
	string qxfnet = "vdfldzgkalskowixx";
	bool smfdcvhk = false;
	bool fpfjzu = true;
	bool nwrtuzbnrvnv = false;
	double ajdal = 3865;
	int esjvgk = 5934;
	bool sauvwvcd = false;
	double ngnmyui = 25301;
	int cnjzeq = 306;
	if (3865 != 3865) {
		int rqnog;
		for (rqnog = 95; rqnog > 0; rqnog--) {
			continue;
		}
	}
	if (true != true) {
		int lwqmj;
		for (lwqmj = 4; lwqmj > 0; lwqmj--) {
			continue;
		}
	}
	return string("mcmkepblgow");
}

string xjbazuq::ndrxpddiognkewmbjypkm(string gdudyl, double eqgrqlitoa) {
	string nbjoeznuvu = "cexpyvvzxvximdqhnhpipgpbvbvvvoocre";
	bool eoocwwdgszbnaa = false;
	string kmkftck = "qikvqxoktvrrybcmagfnmoychfcasbohbpkkjxlut";
	string cuxcxavlnvp = "tuynruhuohwgiaercxdvdydrowvnfxbrezoildofqbcpzvvnuozynktgwtmhigdpkrejsqbuxtdiuegwzirotaaitkhu";
	double bcqkg = 52094;
	double avzkb = 22610;
	if (string("cexpyvvzxvximdqhnhpipgpbvbvvvoocre") != string("cexpyvvzxvximdqhnhpipgpbvbvvvoocre")) {
		int ck;
		for (ck = 75; ck > 0; ck--) {
			continue;
		}
	}
	return string("yaldhxmwcbpiecg");
}

string xjbazuq::dfdcencnhfos(string tdsqk, double ziaicvxwc, string wctzmqrzhthr, double krmxhwfh, double lfpihdxtds, string tlkmvmsjvowbb, bool ulztefmnsuagfve, string zpqoh, string qfcxmrzsfg, double ydxon) {
	double vyzhvnsslxq = 21669;
	int lelcacjcyjr = 4294;
	int cpdusdote = 3136;
	int ldxumj = 3531;
	string huholeruh = "iustahjxvacdtcmayyuaeebcbifvjnprjsbcmjbhdmvwvfrcafixcdnvcedkqikfxjuhgdsfuwoorfvbkdrmritlhp";
	int ezlwxgfwsheslkj = 261;
	int fmjdrewa = 3400;
	bool dnhlsckatib = false;
	if (4294 != 4294) {
		int hnnvjdmr;
		for (hnnvjdmr = 9; hnnvjdmr > 0; hnnvjdmr--) {
			continue;
		}
	}
	if (261 == 261) {
		int kt;
		for (kt = 14; kt > 0; kt--) {
			continue;
		}
	}
	if (string("iustahjxvacdtcmayyuaeebcbifvjnprjsbcmjbhdmvwvfrcafixcdnvcedkqikfxjuhgdsfuwoorfvbkdrmritlhp") == string("iustahjxvacdtcmayyuaeebcbifvjnprjsbcmjbhdmvwvfrcafixcdnvcedkqikfxjuhgdsfuwoorfvbkdrmritlhp")) {
		int hastsds;
		for (hastsds = 30; hastsds > 0; hastsds--) {
			continue;
		}
	}
	if (21669 == 21669) {
		int eioah;
		for (eioah = 49; eioah > 0; eioah--) {
			continue;
		}
	}
	if (false != false) {
		int dwinhnkk;
		for (dwinhnkk = 39; dwinhnkk > 0; dwinhnkk--) {
			continue;
		}
	}
	return string("spafu");
}

int xjbazuq::llxmxbuesxt(int hyoeveqaxcc, double mqkgdvti) {
	int geviptrpb = 662;
	bool ofzmqcnngdon = true;
	string tlcfdsci = "ocjnetjpmrurxxieuabbahwgkxlqjdgfuitwmqolcwituflrwgoaueastpowhxcqiwegyqmieftjwhrzlvn";
	int dufcwwwqpc = 1682;
	bool bdjonnicsl = false;
	bool wnrdemiik = true;
	if (true != true) {
		int jh;
		for (jh = 73; jh > 0; jh--) {
			continue;
		}
	}
	if (1682 != 1682) {
		int yzw;
		for (yzw = 11; yzw > 0; yzw--) {
			continue;
		}
	}
	if (false == false) {
		int js;
		for (js = 54; js > 0; js--) {
			continue;
		}
	}
	if (false == false) {
		int nwqwqucw;
		for (nwqwqucw = 92; nwqwqucw > 0; nwqwqucw--) {
			continue;
		}
	}
	if (1682 == 1682) {
		int lefmdiibab;
		for (lefmdiibab = 83; lefmdiibab > 0; lefmdiibab--) {
			continue;
		}
	}
	return 69717;
}

int xjbazuq::dzbohhtjqscwngjcu(int cbbbzmhzbvhb) {
	double nvmixomqsltb = 12040;
	if (12040 == 12040) {
		int ohueqfal;
		for (ohueqfal = 100; ohueqfal > 0; ohueqfal--) {
			continue;
		}
	}
	if (12040 == 12040) {
		int fu;
		for (fu = 82; fu > 0; fu--) {
			continue;
		}
	}
	return 80224;
}

xjbazuq::xjbazuq() {
	this->agswygjnquvvzbznsbemtevda(string("ywtboderwawzhnqzcpzmaravchomudrevsnyhrjxscafa"), string("pgflqxjvxiejozacpytqfkhqfoskvkibooxxksdye"), 5438, true, false, string("auyefgzmswqccxjj"), 4384);
	this->otgtddzrebrtzuhzbxgm();
	this->dcudjeerlblhbfkzvsogkihv(string("fthyuezpeendiisyagevzkim"));
	this->smvvwqpudhknqlmj(1832, 49589, string("nljrglfdcnxpqalvmhrmyllermbgtqoznechozpyzwoskwbfhndojpjndlwnfphzwngceokilwv"), false, 5269, 50136, string("zidfrdrrjeoiceuczbccbgltgimjmtjfyylyuffrjybuelmtxiqizochjpexhakkxerjyxagye"));
	this->yjvkpuoyaalawxghy(true);
	this->ovqxekwcwjwlhnekdcbeoy(false, 1497, 51604, true, 1592, 2749, string("jrhmteawqvvdzsmbmoeypelikgwjrmdlugcavenkenfax"), true, string("nalimu"));
	this->ndrxpddiognkewmbjypkm(string("rdglpoumfswqcekjf"), 13274);
	this->dfdcencnhfos(string("rcbjvxbncxptzxqnbzroxgewtzbyvymncupogrsncqldzdeeanhybqdchjcutuadxdghdvurwayrfpthxygxkqlmcsa"), 14090, string("mrqxldjqexnkyhydphzsshybpbwpjoitnfycjsemdbkzlr"), 39758, 18121, string("wgmkeonwimlrgvjnondaklqcifdmgslrtiguahlkefeovkvnnruzimxdkquhwmscsvbulpqnyosklawvl"), false, string("dopzzwjjbbbfzojjkyqpxezrfiucksrdyzamrkyeddkwojyebtosanhbeynhegtjajxcyvefuymemwnsvodd"), string("pxmnxb"), 93695);
	this->llxmxbuesxt(3056, 32691);
	this->dzbohhtjqscwngjcu(2609);
	this->xkhdsdbawzzozxqwoqrzu(string("husyzoongiyuccicdkevqyprvdawwrkdtmwurxan"));
	this->fwqfeqjgtlbovanoecngym(18016, 11020, 326, string("dljcgev"), string("arwuqcysuskiibyxghuomqgyjaskgwqmdcciifvhfnewqifqpypzbaghnweyblxnzrmxlszsefbvkdqfquxcokvdfybjmezrwpw"), 2903, false, 961, 2107);
	this->embrumyfhficesweeqpylotxv(true, 947, 65503, true, 16, string("ywyfdxlvgbgwwtvkwlgzdioeoijalewiqlimjwwhjzusgjmvgculkh"), string("iqlyxdjxuehvbaoqakocasucmzyexfwtlziyiitvlzgbknhahssodzeinbvldbduxqblrevsgucakesuq"), 1657, 9030);
	this->qavymgwpea(801, 11155, 17712, string("lbtvlffkpadcitnjetsjszyebxshagtkvsdpknzzgrhjseiebtqqmskxuzpiffmlufemxnlqcurypegmauftj"), 4433);
	this->cchgeroekibkaryfupse(1191, 6471, string("bwolwtxpjdpzisjzsq"), false, 19093, string("obkxiazxxkamouwwhztujsdddxpsprjkmrigycffjeojqcibhgdwlbogqstzlfwfdhcwwpgivznrcvkxlorajvm"));
	this->jkorgvatjznvvivxbst(1601, 2483, 27706);
	this->rqkqtqnqazzl(3995, 984, 1439);
	this->fuidbjdifmp(false, false, false, 1932, false, 121, string("expdwpncwcyojdaxqmzsxzodzthepxmahvo"), string("nltzyhtipzuqbvquembgyhissccaxnzliquqgzwidoogkslexubmpelcujnayawqepfwclsqmkxqxgtbqhovuskqqrdtxzlto"), 25239, 1690);
	this->ndndtduddsvszc(3045);
	this->fcugworxynwosvlqveprarhf(36646, 9753, true, true, string("obbnnmzibpvcozalknptunmttocjcobljqbeutopikcqtfftabsgigoccqscbuvttptjdbdfaaedhqwrtfmsapjr"), 23205, false, string("hbhmoqaddgbinzdckshhhielvzuqvkrpeztosowyjsoyfupoltxpabweynutqdqgqdwn"), 69192);
}


class pxtkcpg {
public:
	bool bchtdslnkir;
	bool delcrsupjma;
	double lkhrsfhwxun;
	pxtkcpg();
	string hxailbohtneuzye(int kbknzcp, double ubmshmv, bool xcezx, double vihtfswjvolrm, string lvlzzsusnvmbq, string hewjzqnggmdrieg);
	double vrykjjjszvdydbfdgkthhua(double modccuhjyq, int vfqmcwwgafv, double lulfflb, string yijcnoxjhvp, double srmzddmnkmv, int kempcvu);
	void jqscjbgrilfl(double yukbbelgmhbg, bool vttudkskqre, string engasuiyy, int jilosvss, double qmmurcypvxglcg);
	bool msogbiaucsguwp(bool ksvnzpyw);
	bool uafoocnizesigld(double pkkgdtasnuennvy, int siiyhunecjfch, string wiolhjghhhprz, double xeezjttymg, bool ilefkao, string oykpezrzzuxie, string rlvxeuinysklch, string krdhifihdzg, bool venldlzqkvoulmh, bool gqqagjjkzhvcxf);

protected:
	bool jkikouonarkh;
	int qvmwoujwdq;
	string vfwxl;
	bool tkefwirvcx;

	void tnwhtvzdfvicxdeedsocut(int dzbrauvoj, bool mjedlu, string usozwubnw, bool ljivzbiwnowvouz, bool rofwswijsgx);
	int sxxkthltczpzez(bool ltnlrpgfcwrkmrq, bool hbuqne, double hcpherselaqphh, int wdczsagbnvcenv, double hrnutm, bool fzqvbtzllctlzv, string loukelzdksqqq);
	void kovsdlqrwnqpwecyooczdpj(string pexyc);
	void vwncqwtqeukb(double gbmuk, bool wmcvyinkuiddr, int wmshlwa, bool sjuelkpk, int yivzzyxjkdnlu, bool zubpvlopxdmylj, string xuxftrkjwubgznu, bool qanqsyuzuxc, bool oizdsxgfqkiyzxl);

private:
	double ctgeyyhciam;
	double jhsbxwsci;
	string ydqcxqkvqcihdn;
	int kdfvfhqzmo;

	string pfizibgextqmabfdousfrzko(double uvmwkkgvcjzi, int setmt, bool qcidkiigxf, double xxbzrrzhnu, double zziapm, bool dwuzpgbyyms);
	bool pfocqspesqfdzqmbpghpvyoiq(double lfxlsugy, bool sulbgdvespvw, string wbvrawoxprc, string mhxgbgk, double hjwvrmtoumysq);
	string masxgnxfypjquunyj();
	void swfgtkcnwzftsjizjc(int jwxsnmkyvuiz, int jbvfspaqer, bool waukhb, double doojuypvkppfu, string yexjnoochufrgvd, int kdlcxoeyt, int hckuduwlbkpzm, bool buypugzqhkkdt);
	double buontzxbmsikz(string zvxqvomrlglk, bool kiepaazdjfhc, bool pmhhwaws, int eixipthb, int nicqwyfccfwd, double fvjymrmum, int kpiurof, string vkbclnlsfzrdnak, bool vseyx, bool iatwrowcfglrz);
	int blpmqyluijqqoxqoyxsotbf(string gkgdrvrxbp, bool vqglomhfrwzn, int juqpergkywdg);
	void shnakyhnmaauoluqzxbyu(double kgbpyzuuosf, string myziypdiv, bool azgouas, int rnjvo, bool wkbwusvzdb, double gpdzzwiegk, double cxzrwuobxng);
	void zcltjwbqgcubkpuelbor(bool tnetfmrnzku, string kaedsahuts);
	string sojipmprklptsokim(bool pyawh, bool fplqplajxrmkwyx, int lmnzqfksu, int quoytulzgew, string alwetbpftalwxhs, double wktnad);
	int nmfwszhgzowpano(double nklselyrir, double bqhnntkrfbnlyte, double mcubdxptacldypw, int brschlvrogjouh, bool wnijjcqmy, string yunfagf, string larlkdxk);

};


string pxtkcpg::pfizibgextqmabfdousfrzko(double uvmwkkgvcjzi, int setmt, bool qcidkiigxf, double xxbzrrzhnu, double zziapm, bool dwuzpgbyyms) {
	bool ejprrpo = false;
	bool paqidoagynexirv = true;
	if (true != true) {
		int plhv;
		for (plhv = 61; plhv > 0; plhv--) {
			continue;
		}
	}
	if (true == true) {
		int ybxtbhq;
		for (ybxtbhq = 41; ybxtbhq > 0; ybxtbhq--) {
			continue;
		}
	}
	if (true == true) {
		int smtewbgghq;
		for (smtewbgghq = 37; smtewbgghq > 0; smtewbgghq--) {
			continue;
		}
	}
	if (true == true) {
		int drsyd;
		for (drsyd = 38; drsyd > 0; drsyd--) {
			continue;
		}
	}
	if (true != true) {
		int dk;
		for (dk = 2; dk > 0; dk--) {
			continue;
		}
	}
	return string("gnulcbgspqooathf");
}

bool pxtkcpg::pfocqspesqfdzqmbpghpvyoiq(double lfxlsugy, bool sulbgdvespvw, string wbvrawoxprc, string mhxgbgk, double hjwvrmtoumysq) {
	double fmsqgmkvbuug = 34490;
	int rwwgedbbwywim = 2428;
	bool bkxhxrknzslzrhe = true;
	string xjcxa = "sxsiixbbjndfhmsqnxdwsldqxcasbptrcprmjvnnribdhydjhtlpbklazxmphynimzdjcbddpaoeesna";
	string qbcxkm = "ypbpeihggpunexdcjzpouytuvmckwshv";
	int umwzjyjfsopb = 1263;
	string fmczs = "qshecasduzibjqboaifoyzzwyfgefnulfpgtkgwwjfneawopuaydcnsvjwcepcqzdholthnamyhuvpemdiqkxxlvq";
	if (true == true) {
		int abp;
		for (abp = 82; abp > 0; abp--) {
			continue;
		}
	}
	if (string("qshecasduzibjqboaifoyzzwyfgefnulfpgtkgwwjfneawopuaydcnsvjwcepcqzdholthnamyhuvpemdiqkxxlvq") == string("qshecasduzibjqboaifoyzzwyfgefnulfpgtkgwwjfneawopuaydcnsvjwcepcqzdholthnamyhuvpemdiqkxxlvq")) {
		int ejjbg;
		for (ejjbg = 93; ejjbg > 0; ejjbg--) {
			continue;
		}
	}
	if (34490 != 34490) {
		int dcc;
		for (dcc = 94; dcc > 0; dcc--) {
			continue;
		}
	}
	if (2428 != 2428) {
		int pskdh;
		for (pskdh = 22; pskdh > 0; pskdh--) {
			continue;
		}
	}
	if (true != true) {
		int dsajqabnia;
		for (dsajqabnia = 100; dsajqabnia > 0; dsajqabnia--) {
			continue;
		}
	}
	return false;
}

string pxtkcpg::masxgnxfypjquunyj() {
	bool xbrxu = false;
	double axesn = 13351;
	if (13351 == 13351) {
		int hmvavlqvxt;
		for (hmvavlqvxt = 81; hmvavlqvxt > 0; hmvavlqvxt--) {
			continue;
		}
	}
	if (13351 == 13351) {
		int nxt;
		for (nxt = 56; nxt > 0; nxt--) {
			continue;
		}
	}
	if (false == false) {
		int knoefb;
		for (knoefb = 66; knoefb > 0; knoefb--) {
			continue;
		}
	}
	return string("");
}

void pxtkcpg::swfgtkcnwzftsjizjc(int jwxsnmkyvuiz, int jbvfspaqer, bool waukhb, double doojuypvkppfu, string yexjnoochufrgvd, int kdlcxoeyt, int hckuduwlbkpzm, bool buypugzqhkkdt) {
	bool sfgopnculrjmiax = false;
	double bqooya = 1145;
	double qhhyqbyl = 25295;
	bool joqogfx = false;
	if (25295 != 25295) {
		int xedxp;
		for (xedxp = 36; xedxp > 0; xedxp--) {
			continue;
		}
	}
	if (false == false) {
		int magszkjzm;
		for (magszkjzm = 1; magszkjzm > 0; magszkjzm--) {
			continue;
		}
	}
	if (1145 == 1145) {
		int mz;
		for (mz = 86; mz > 0; mz--) {
			continue;
		}
	}
	if (false != false) {
		int ura;
		for (ura = 79; ura > 0; ura--) {
			continue;
		}
	}
	if (false == false) {
		int ad;
		for (ad = 64; ad > 0; ad--) {
			continue;
		}
	}

}

double pxtkcpg::buontzxbmsikz(string zvxqvomrlglk, bool kiepaazdjfhc, bool pmhhwaws, int eixipthb, int nicqwyfccfwd, double fvjymrmum, int kpiurof, string vkbclnlsfzrdnak, bool vseyx, bool iatwrowcfglrz) {
	string noyvrzsyq = "vfippuveoyhrzvyqvzegfptjfrhriqqjhikhbdhggyjefvnxruanacfrajdouymbwarqaagyflneqsrzrimcfaagf";
	bool eyxikrsfwuax = false;
	string iogerxzckbrx = "vywmcdcckmnoxoehhwgidvleladqzeylsgacuekdubzqhbefb";
	double wfjdqwl = 9474;
	double kimgailygfp = 24002;
	int uflemvdgf = 1094;
	bool gapedicceqjn = false;
	int vgfnjwjmolrcmq = 5327;
	int trmnswxhpb = 1780;
	if (1094 != 1094) {
		int kvxzvjhl;
		for (kvxzvjhl = 7; kvxzvjhl > 0; kvxzvjhl--) {
			continue;
		}
	}
	if (string("vywmcdcckmnoxoehhwgidvleladqzeylsgacuekdubzqhbefb") != string("vywmcdcckmnoxoehhwgidvleladqzeylsgacuekdubzqhbefb")) {
		int vzzbroj;
		for (vzzbroj = 21; vzzbroj > 0; vzzbroj--) {
			continue;
		}
	}
	if (24002 == 24002) {
		int hlis;
		for (hlis = 98; hlis > 0; hlis--) {
			continue;
		}
	}
	if (string("vfippuveoyhrzvyqvzegfptjfrhriqqjhikhbdhggyjefvnxruanacfrajdouymbwarqaagyflneqsrzrimcfaagf") == string("vfippuveoyhrzvyqvzegfptjfrhriqqjhikhbdhggyjefvnxruanacfrajdouymbwarqaagyflneqsrzrimcfaagf")) {
		int cygxovnr;
		for (cygxovnr = 14; cygxovnr > 0; cygxovnr--) {
			continue;
		}
	}
	return 25428;
}

int pxtkcpg::blpmqyluijqqoxqoyxsotbf(string gkgdrvrxbp, bool vqglomhfrwzn, int juqpergkywdg) {
	double kgkamnogveh = 27286;
	double ihojo = 17682;
	string qoramyyhqlxn = "duwgkm";
	bool pcxjxwfwfadlk = false;
	bool nwmywvr = true;
	bool ushbvovgjscwo = true;
	bool mwnfojapoebkd = false;
	double oznvuv = 13643;
	int dznbevd = 1317;
	string zhlyevimfsuca = "fnhqqmgofpahnmbfdymqosnmsjlkjdkpbpraflqinti";
	if (false == false) {
		int vge;
		for (vge = 44; vge > 0; vge--) {
			continue;
		}
	}
	if (false == false) {
		int nijszgezs;
		for (nijszgezs = 5; nijszgezs > 0; nijszgezs--) {
			continue;
		}
	}
	if (false != false) {
		int mfyccbltlq;
		for (mfyccbltlq = 6; mfyccbltlq > 0; mfyccbltlq--) {
			continue;
		}
	}
	return 70229;
}

void pxtkcpg::shnakyhnmaauoluqzxbyu(double kgbpyzuuosf, string myziypdiv, bool azgouas, int rnjvo, bool wkbwusvzdb, double gpdzzwiegk, double cxzrwuobxng) {
	bool kznqyoap = true;
	string gennvr = "hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn";
	bool dgenpmvdahxx = false;
	if (string("hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn") != string("hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn")) {
		int maoq;
		for (maoq = 65; maoq > 0; maoq--) {
			continue;
		}
	}
	if (false == false) {
		int phjgs;
		for (phjgs = 79; phjgs > 0; phjgs--) {
			continue;
		}
	}
	if (string("hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn") != string("hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn")) {
		int tnvxa;
		for (tnvxa = 47; tnvxa > 0; tnvxa--) {
			continue;
		}
	}
	if (string("hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn") == string("hlvjurbixzxvnpoaqvafehweyqmysmcirpdwdtrxibibvphgahegrsvufjmtojlreixibqriklsfgigjokvbxrjsn")) {
		int avnjpyylr;
		for (avnjpyylr = 3; avnjpyylr > 0; avnjpyylr--) {
			continue;
		}
	}
	if (false == false) {
		int epfp;
		for (epfp = 93; epfp > 0; epfp--) {
			continue;
		}
	}

}

void pxtkcpg::zcltjwbqgcubkpuelbor(bool tnetfmrnzku, string kaedsahuts) {
	bool ekhgmozfgmshnpx = true;
	double tnfalskw = 2262;
	bool hizxchwdfeuordd = false;
	bool swummtfxwrwkjba = false;
	bool kylhtrmnmwq = false;
	string lebqmmwufriqii = "zktwuhcfukoampfqnkoytapfxukzhhyagyrlspzzwg";
	double chymcdmjklfnce = 21500;
	double wcseernhbxwa = 14785;

}

string pxtkcpg::sojipmprklptsokim(bool pyawh, bool fplqplajxrmkwyx, int lmnzqfksu, int quoytulzgew, string alwetbpftalwxhs, double wktnad) {
	bool axrvmyuluivowfw = false;
	double efrgkxxxke = 14379;
	int djvpid = 975;
	bool slqteecuw = false;
	bool thblebneasceksp = false;
	bool nizayhwjzjzqvtk = false;
	if (false != false) {
		int ribfhmp;
		for (ribfhmp = 32; ribfhmp > 0; ribfhmp--) {
			continue;
		}
	}
	if (14379 != 14379) {
		int rbdtipyfv;
		for (rbdtipyfv = 63; rbdtipyfv > 0; rbdtipyfv--) {
			continue;
		}
	}
	if (false != false) {
		int aicgmectrg;
		for (aicgmectrg = 27; aicgmectrg > 0; aicgmectrg--) {
			continue;
		}
	}
	if (false != false) {
		int ufipan;
		for (ufipan = 38; ufipan > 0; ufipan--) {
			continue;
		}
	}
	return string("eaksbauffsgqcgb");
}

int pxtkcpg::nmfwszhgzowpano(double nklselyrir, double bqhnntkrfbnlyte, double mcubdxptacldypw, int brschlvrogjouh, bool wnijjcqmy, string yunfagf, string larlkdxk) {
	int lfdoiothyvvpdb = 1280;
	double brtzyhfrmek = 12299;
	string xevsuarbzuqn = "hxbhyxkdijigxizcnjspkxrgzsjzzdahznfrlnuewfzgagdyndzlcpwkloxedk";
	string xvemtutszpdatgm = "xooocjzquniuhndulacyasbjreoabshzeliaffmcdnz";
	string kbirzgnr = "afogvyslogwpxottdujhfmexrlzsezssydprhyctwbqdqqayqafvuegvctgersie";
	string fpsocl = "";
	double zhqrlbluqguna = 16482;
	string uxxebai = "whrezuifrevktcmdcxjvxyxturwbmikiiimofdpopcyeztbcwleiyepicswwl";
	bool ucstdqmwkvw = true;
	double gnduppnnzzwm = 30274;
	if (string("xooocjzquniuhndulacyasbjreoabshzeliaffmcdnz") != string("xooocjzquniuhndulacyasbjreoabshzeliaffmcdnz")) {
		int ubs;
		for (ubs = 7; ubs > 0; ubs--) {
			continue;
		}
	}
	return 34140;
}

void pxtkcpg::tnwhtvzdfvicxdeedsocut(int dzbrauvoj, bool mjedlu, string usozwubnw, bool ljivzbiwnowvouz, bool rofwswijsgx) {
	int cuyzrxplou = 1594;
	bool eqrsyzyfe = true;
	double bfinynsbddtx = 1048;
	double putoqitieeleu = 621;
	double cwsaclfdnqtstn = 32341;
	bool ynleedtpzt = true;
	int twojcexswuudkuv = 6492;
	if (621 != 621) {
		int cyatamryp;
		for (cyatamryp = 40; cyatamryp > 0; cyatamryp--) {
			continue;
		}
	}

}

int pxtkcpg::sxxkthltczpzez(bool ltnlrpgfcwrkmrq, bool hbuqne, double hcpherselaqphh, int wdczsagbnvcenv, double hrnutm, bool fzqvbtzllctlzv, string loukelzdksqqq) {
	double egckzqj = 57192;
	bool ozychd = true;
	double mbynboox = 16610;
	int shrbhng = 2954;
	double nljeb = 56282;
	if (16610 != 16610) {
		int usgecen;
		for (usgecen = 59; usgecen > 0; usgecen--) {
			continue;
		}
	}
	if (2954 != 2954) {
		int yxflcrj;
		for (yxflcrj = 53; yxflcrj > 0; yxflcrj--) {
			continue;
		}
	}
	if (16610 == 16610) {
		int cmt;
		for (cmt = 74; cmt > 0; cmt--) {
			continue;
		}
	}
	if (2954 != 2954) {
		int uqhfrzhmc;
		for (uqhfrzhmc = 7; uqhfrzhmc > 0; uqhfrzhmc--) {
			continue;
		}
	}
	return 7957;
}

void pxtkcpg::kovsdlqrwnqpwecyooczdpj(string pexyc) {
	bool rrkzbkgflk = true;
	string gpuqrgjk = "xzfevvgbp";
	bool urujqqmwe = true;
	string phvfbvq = "otcqrcqhhtfxifmivvleycfbijnikqejplhimqmylftwkqmqdjytmrclbmrrldtwehoantmdlhdnxo";
	double srpiylrbv = 48646;
	int ulxvltlgwicjksj = 4463;
	int ecgga = 4841;

}

void pxtkcpg::vwncqwtqeukb(double gbmuk, bool wmcvyinkuiddr, int wmshlwa, bool sjuelkpk, int yivzzyxjkdnlu, bool zubpvlopxdmylj, string xuxftrkjwubgznu, bool qanqsyuzuxc, bool oizdsxgfqkiyzxl) {
	int stfju = 2410;
	int pbascxsjrawjay = 2062;
	string fwstjfq = "edpatncokuqaidxrnjkpenyeiylnpfxx";
	if (string("edpatncokuqaidxrnjkpenyeiylnpfxx") == string("edpatncokuqaidxrnjkpenyeiylnpfxx")) {
		int au;
		for (au = 5; au > 0; au--) {
			continue;
		}
	}
	if (string("edpatncokuqaidxrnjkpenyeiylnpfxx") == string("edpatncokuqaidxrnjkpenyeiylnpfxx")) {
		int dr;
		for (dr = 59; dr > 0; dr--) {
			continue;
		}
	}
	if (2410 == 2410) {
		int hegrqmhl;
		for (hegrqmhl = 56; hegrqmhl > 0; hegrqmhl--) {
			continue;
		}
	}
	if (string("edpatncokuqaidxrnjkpenyeiylnpfxx") != string("edpatncokuqaidxrnjkpenyeiylnpfxx")) {
		int rfz;
		for (rfz = 0; rfz > 0; rfz--) {
			continue;
		}
	}
	if (2062 != 2062) {
		int aiirbx;
		for (aiirbx = 18; aiirbx > 0; aiirbx--) {
			continue;
		}
	}

}

string pxtkcpg::hxailbohtneuzye(int kbknzcp, double ubmshmv, bool xcezx, double vihtfswjvolrm, string lvlzzsusnvmbq, string hewjzqnggmdrieg) {
	int hdnfwlkvdtbdm = 1804;
	bool slnpwzgqfs = true;
	string kwphjxg = "ntgpnntsgohbtygymcszuubefdzadagtxatywm";
	bool bkuwunzwhctkisa = false;
	if (1804 == 1804) {
		int etgzxd;
		for (etgzxd = 10; etgzxd > 0; etgzxd--) {
			continue;
		}
	}
	if (string("ntgpnntsgohbtygymcszuubefdzadagtxatywm") == string("ntgpnntsgohbtygymcszuubefdzadagtxatywm")) {
		int oty;
		for (oty = 64; oty > 0; oty--) {
			continue;
		}
	}
	if (string("ntgpnntsgohbtygymcszuubefdzadagtxatywm") != string("ntgpnntsgohbtygymcszuubefdzadagtxatywm")) {
		int wctv;
		for (wctv = 66; wctv > 0; wctv--) {
			continue;
		}
	}
	if (1804 == 1804) {
		int stdjq;
		for (stdjq = 88; stdjq > 0; stdjq--) {
			continue;
		}
	}
	if (1804 == 1804) {
		int qksuw;
		for (qksuw = 67; qksuw > 0; qksuw--) {
			continue;
		}
	}
	return string("jxkbryfvwgrldyzmk");
}

double pxtkcpg::vrykjjjszvdydbfdgkthhua(double modccuhjyq, int vfqmcwwgafv, double lulfflb, string yijcnoxjhvp, double srmzddmnkmv, int kempcvu) {
	bool fcknbqpfoft = true;
	string obbryyn = "ssbkbzvmuyncsigeroggkqrqssiocucfqtslonxgwpsplgeclnfubndyxbahv";
	int nttwjxubat = 4897;
	bool nhxjxmti = true;
	string hrdnkboslncm = "capuycpgeeqzzkjshhsktudqdavxojuklpxvr";
	double npxbks = 2060;
	double iuwtwvnlesy = 8530;
	int mybiumhxuse = 7999;
	string djkthgthxdu = "tokjfktgcrz";
	if (string("tokjfktgcrz") != string("tokjfktgcrz")) {
		int zktz;
		for (zktz = 52; zktz > 0; zktz--) {
			continue;
		}
	}
	if (8530 == 8530) {
		int ha;
		for (ha = 79; ha > 0; ha--) {
			continue;
		}
	}
	if (4897 != 4897) {
		int cvlaywzbx;
		for (cvlaywzbx = 9; cvlaywzbx > 0; cvlaywzbx--) {
			continue;
		}
	}
	return 4869;
}

void pxtkcpg::jqscjbgrilfl(double yukbbelgmhbg, bool vttudkskqre, string engasuiyy, int jilosvss, double qmmurcypvxglcg) {
	string cqxqqgrmsuinpd = "wnjnrprgpdviqbdkliyjuieekhzxxbpebgsffmudkaxrlccbpfhjhpqfzfikiybytwtgbjrkxsf";
	string jqpxksgmwky = "zddfajsoezlzflwzhuoayvmybfbssmiyzlbyiemdwthkofjzlecxaifmm";
	int khvqrsdqoowvzet = 358;
	int dvucmeuiclqb = 732;

}

bool pxtkcpg::msogbiaucsguwp(bool ksvnzpyw) {
	string qmodnkpwccrzly = "pcfnmetsajhbprrmcamvnypwnqpezptpbvarxq";
	string rdmmmzyc = "zbuflooyl";
	int fxkef = 8815;
	string joetwcevzvpkaq = "rplflzxxrsjebqxvpvbwlvrbjehbedpzgtiflvvplxmzcvmn";
	bool kucsxsgkuapqwol = true;
	double xrrcdjysobeg = 3478;
	string cahslxop = "xpwkuiyocxogzlykwycbahrpxgfoqwyjyxdrxrxvfttjawkusynnmxzj";
	string jkubpqpwpsbrd = "goufmdkhqacwphvaluxpvlthyoaiwsoocqrfiqdfrlnpguuimlxqpwbexp";
	if (string("goufmdkhqacwphvaluxpvlthyoaiwsoocqrfiqdfrlnpguuimlxqpwbexp") == string("goufmdkhqacwphvaluxpvlthyoaiwsoocqrfiqdfrlnpguuimlxqpwbexp")) {
		int niaqwfzqhc;
		for (niaqwfzqhc = 29; niaqwfzqhc > 0; niaqwfzqhc--) {
			continue;
		}
	}
	if (string("rplflzxxrsjebqxvpvbwlvrbjehbedpzgtiflvvplxmzcvmn") == string("rplflzxxrsjebqxvpvbwlvrbjehbedpzgtiflvvplxmzcvmn")) {
		int xqwfquyq;
		for (xqwfquyq = 26; xqwfquyq > 0; xqwfquyq--) {
			continue;
		}
	}
	return false;
}

bool pxtkcpg::uafoocnizesigld(double pkkgdtasnuennvy, int siiyhunecjfch, string wiolhjghhhprz, double xeezjttymg, bool ilefkao, string oykpezrzzuxie, string rlvxeuinysklch, string krdhifihdzg, bool venldlzqkvoulmh, bool gqqagjjkzhvcxf) {
	int ystsifv = 269;
	bool bodcjdrahdxaf = false;
	double gnbbdkftapl = 74507;
	double jchijpnaanyzm = 18047;
	string ahrqpbatzi = "uydwhtzcjtambhfbtzycfdfmiychjjjjxmcwkpqmhxxiprjuimybtjhycaitinsneuhpsrgeqlup";
	string lgbuaoesz = "ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq";
	string cdoechvyhgd = "gulkdpvcepwxifpkgcqfqwodhjiiooegwbqfrxztlmouvtsvnnzpsfviihqswxeaddsod";
	int lxcpc = 1882;
	if (false != false) {
		int gdbcbow;
		for (gdbcbow = 24; gdbcbow > 0; gdbcbow--) {
			continue;
		}
	}
	if (string("ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq") != string("ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq")) {
		int tfqioc;
		for (tfqioc = 64; tfqioc > 0; tfqioc--) {
			continue;
		}
	}
	if (string("ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq") != string("ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq")) {
		int nmygkfts;
		for (nmygkfts = 86; nmygkfts > 0; nmygkfts--) {
			continue;
		}
	}
	if (string("ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq") != string("ydhjamktktswycozsktbortfaaodtdpgxxwuxyceckycopxeiblopmpfqiavsfutfshsutgmpq")) {
		int jtaptlup;
		for (jtaptlup = 39; jtaptlup > 0; jtaptlup--) {
			continue;
		}
	}
	return true;
}

pxtkcpg::pxtkcpg() {
	this->hxailbohtneuzye(2102, 42411, false, 28242, string("faafpqkwofwnwjwrpdjlnddwfztgtetvyiwqjcsqqdfhfndconvqyfcktqnvbzsigchfswwprboczdmmxurs"), string("letykbhrdvtkwglfhhlwdiksokbzxoqxawucstuhiiezdiocsxrsdzizrhdncspbexbcmkdqrvtjwshiqnbufornoeomdqv"));
	this->vrykjjjszvdydbfdgkthhua(12411, 2811, 1204, string("arneufxcjaewkyphyzxspendyviurkyubllfaiirhkqatbsveisttmsdhvrtycvgsvgroemjgjgnf"), 714, 1190);
	this->jqscjbgrilfl(26851, false, string("rbylyyzqrsjodtkjnpxrwgikqejnamqfrpgtjbskxziuowlxaiiuleswbqgopqlmrsjdihsldwbdubmegrfzgfbrsrkioj"), 3163, 15334);
	this->msogbiaucsguwp(true);
	this->uafoocnizesigld(58897, 3572, string("hheuffmqwezkcfdbrwobgfdtbdmtkfkskesj"), 6180, false, string("cjazynshvfmdkncmnxzvivjgjgbhegtzdpzdfrkdkpybsdwdyelxkwoshbemdfffpl"), string("hciectnxtltubfbwwijjkssrnakxqnycpilasqmevmdzjybmglprqdlsxwpaxyxsahqbkzoqevyfpxac"), string("cjitonn"), false, false);
	this->tnwhtvzdfvicxdeedsocut(5171, false, string("mpmkjkwajgmxkogbq"), true, false);
	this->sxxkthltczpzez(false, false, 8095, 2483, 23289, false, string("rygomoiosrdtmjiqmlysz"));
	this->kovsdlqrwnqpwecyooczdpj(string("hdxenzwrlctikxsizovkvhtjwjpuikrrojrcqxfeoidbfloleouhlozbhofhdiwddvktfrjryfxfgzfcnmpk"));
	this->vwncqwtqeukb(58039, false, 5420, false, 837, false, string("rkuge"), false, true);
	this->pfizibgextqmabfdousfrzko(34609, 3876, true, 36073, 32144, true);
	this->pfocqspesqfdzqmbpghpvyoiq(26486, false, string("itqrxatjepqd"), string("lccddsmkneddryakvfqrim"), 18696);
	this->masxgnxfypjquunyj();
	this->swfgtkcnwzftsjizjc(2648, 3824, true, 6267, string("vpizpfgfqanhtfdhzglprktemqcuhogwpgxooazltknil"), 5251, 239, true);
	this->buontzxbmsikz(string("clkxnovlhysg"), false, false, 3544, 141, 16266, 4448, string("ebfiscldrebcpqijsmkebgijfqahjfilwiyvpfpoyyalggurzheyigxdbjsjfueagjs"), false, false);
	this->blpmqyluijqqoxqoyxsotbf(string("dbvwgiskkzrchwp"), true, 3011);
	this->shnakyhnmaauoluqzxbyu(7173, string("srdqtlyvopjoxlatqbtouypdntrdtmicgvostdyirzaafjojvhvudghyphrkkgccjpsnypbfsvmnzlprft"), true, 2689, true, 18313, 34837);
	this->zcltjwbqgcubkpuelbor(true, string("wesdbmhkjdupktnmyavzqvzgienoibqdnsehewgwbgm"));
	this->sojipmprklptsokim(false, false, 7154, 2168, string("ekumlypvwfkwjrdpiijvlqdwfogubqcehjpymtu"), 8009);
	this->nmfwszhgzowpano(6355, 1857, 16680, 5593, true, string("ypqidcfzofgsmagsonlsfzktmfxicbzdisbrfprtmjmcmpclmweoadxkkehgsvirigutwcweerqekcbmngorkvfkwzuuqpisipmu"), string("yjxdrejajjdaznmmxcfsnekjyqefmtqtmjusyovcgjrntbuxe"));
}
