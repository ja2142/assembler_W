#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

const int BITOW_NA_ROZKAZ = 5;
const int BAD = 1;
//===================

class Bad_Syntax:public exception{
private:
	string wut;
public:
	Bad_Syntax() {
		this->wut="something gone wrong...";
	}

	Bad_Syntax(string &wth){
		this->wut = wth;
	}

	string what() {
		return wut;
	}

};

struct Etykieta{
	int linia;
	string nazwa;

	friend std::ostream& operator<<(std::ostream& os, const Etykieta& obj)
	{
		os << obj.linia << "\t" << obj.nazwa;
		return os;
	}
};

struct Rozkaz {
	string mnemo;
	int liczbaParametrow;
	int op;

	friend std::ostream& operator<<(std::ostream& os, const Rozkaz& obj)
	{
		os << obj.mnemo << "\t" << obj.liczbaParametrow << "\t" << obj.op;
		return os;
	}
};

template<class T>
void wypisz(vector<T> &slowa) {
	for (size_t i=0; i < slowa.size();i++) 
		cout<<i<<". " << slowa[i] /*<<"_"*/<< endl;
}

template<class T>
void obroc(vector<T> &slowa) {
	vector<T> b;
	while (slowa.size() > 0) {
		b.push_back(slowa.back());
		slowa.pop_back();
	}
	slowa.swap(b);
}

unsigned short etykietaNaAdres(vector<Etykieta> &etykiety,string &etykieta) {
	bool znaleziono = 0;
	for (size_t i = 0; i < etykiety.size(); i++){
		if (etykieta == etykiety[i].nazwa) {
			return etykiety[i].linia;
		}
	}

	throw Bad_Syntax(string()+"Nie znaleziono etykiety '"+etykieta+"'");
}

bool dajSlowaZLini(char l[], vector<string> &slowa){
	slowa.clear();
	string linia(l);

	while (linia.size() > 0) {
		//cout << linia.size()<<endl;
		while (linia[linia.size()-1] == ' ')linia=linia.substr(0,linia.size()-2);
		if (linia.find_last_of(' ') != string::npos) {
			slowa.push_back(linia.substr(linia.find_last_of(' ')+1));
			linia = linia.substr(0, linia.find_last_of(' '));
		}
		else {
			slowa.push_back(linia);
			return 1;
		}
	}
	return 1;
}

unsigned short procesuj(vector<string> &slowa, bool etykietaDlaTej, vector<Etykieta> &etykiety, vector<Rozkaz> &rozkazy, int linia, bool &koniec) {
	bool etykieta=0;
	obroc(slowa);
	bool znaleziono=0;
	unsigned short zwrot = 0;
	int plusET=0;
	if (etykietaDlaTej) plusET = 1;

	//pseudoinstukcje
	if (slowa[0 + plusET] == "KON") {
		znaleziono = 1;
		zwrot = -1;
		koniec = 1;
	}
	if (slowa[0 + plusET] == "RST") {
		if (1 > slowa.size() - 1 - plusET) {
			cout << "za malo argumentow w lini " << linia << ", instrukcja " << slowa[0 + plusET]
				<< " przyjmuje " << 1 << " parametrow (podano "
				<< slowa.size() - 1 - plusET << " argumentow)" << endl;
			throw Bad_Syntax(string(""));
		}
		if (1 < slowa.size() - 1 - plusET) {
			cout << "za duzo argumentow w lini " << linia << ", instrukcja " << slowa[0 + plusET]
				<< " przyjmuje " << 1 << " parametrow (podano "
				<< slowa.size() - 1 - plusET << " argumentow)\nDodatkowe parametry zostana zignorowane" << endl;
		}

		zwrot |= stoi(slowa[1 + plusET]);
		znaleziono = 1;

	}
	if (slowa[0 + plusET] == "RPA") {
		if (1 < slowa.size() - 1 - plusET) {
			cout << "za duzo argumentow w lini " << linia << ", instrukcja " << slowa[0 + plusET]
				<< " przyjmuje " << 0 << " parametrow (podano "
				<< slowa.size() - 1 - plusET << " argumentow)\nDodatkowe parametry zostana zignorowane" << endl;
		}
		znaleziono = 1;
	}

	//te prawdziwe
	for (size_t i = 0; i < rozkazy.size(); i++){
		if (slowa[0 + plusET] == rozkazy[i].mnemo) {
			znaleziono = 1;
			zwrot = rozkazy[i].op << 16-BITOW_NA_ROZKAZ;
			if (rozkazy[i].liczbaParametrow > slowa.size() - 1 - plusET) {
				cout << "za malo argumentow w lini "<<linia<<", instrukcja "<<slowa[0+plusET]
					<<" przyjmuje "<<rozkazy[i].liczbaParametrow<<" parametrow (podano "
					<< slowa.size() - 1 - plusET <<" argumentow)"<<endl;
				throw Bad_Syntax(string(""));
			}
			if (rozkazy[i].liczbaParametrow < slowa.size() - 1 - plusET) {
				cout << "za duzo argumentow w lini " << linia << ", instrukcja " << slowa[0 + plusET]
					<< " przyjmuje " << rozkazy[i].liczbaParametrow << " parametrow (podano "
					<< slowa.size() - 1 - plusET << " argumentow)\nDodatkowe parametry zostana zignorowane"<<endl;
			}
			
			if (rozkazy[i].liczbaParametrow) {//wiecej parametrow?
				//cout << zwrot << "\t";
				try {
					stoi(slowa[1 + plusET]);
				}
				catch (invalid_argument i) {
					etykieta = 1;
				}
				if (etykieta) zwrot |= etykietaNaAdres(etykiety,slowa[1 + plusET]);
				else zwrot |= stoi(slowa[1 + plusET]);
				//cout << zwrot << "\t";
				//cout << etykietaNaAdres(etykiety, slowa[1 + plusET])<<endl;
			}

			break;
		}
	}

	if(znaleziono) return zwrot;
	else throw Bad_Syntax(string("Nie znaleziono instrukcji ")+slowa[0+plusET]+string(" w lini ")+to_string(linia));
}

int main(int argc,char* argv[]) {
	int styl=0;
	try {
		if (argc > 1)
			styl = stoi(string(argv[1]));
	}

	catch (invalid_argument i) {
		cout << "nie mozna odczytac argumentu jako liczby, wyjscie ustawione na normalne";
		styl = 0;
	}
	int numer_lini=0;
	vector<Etykieta> etykiety;
	vector<Rozkaz> rozkazy;
	vector<bool> czyEtykiety;
	fstream out/*, plikRozkazow*/, in;
	string nazwa;
	cout << "podaj nazwe pliku wejsciowego z kodem" << endl;
	cin >> nazwa;
	//nazwa = "in.txt";
	in.open(nazwa, std::fstream::in);
	if (!in.good()) {
		cout << "Nie udalo sie otworzyc pliku wejsciowego, lub plik jest pusty" << endl;
		system("pause");
		return 0;
	}

	/*
	cout << "podaj nazwe pliku wejsciowego z lista rozkazow" << endl; //TODO mo¿e kiedy indziej
	//cin >> nazwa;
	nazwa = "rozkazy.txt";
	plikRozkazow.open(nazwa, std::fstream::in);
	if (!plikRozkazow.good()) {
		cout << "Nie udalo sie otworzyc pliku wejsciowego, lub plik jest pusty" << endl;
		system("pause");
		return 0;
	}
	*/

	rozkazy.push_back({ "DOD",1,1 });
	rozkazy.push_back({ "ODE",1,2 });
	rozkazy.push_back({ "LAD",1,3 });
	rozkazy.push_back({ "POB",1,4 });
	rozkazy.push_back({ "SOB",1,5 });
	rozkazy.push_back({ "SOM",1,6 });
	rozkazy.push_back({ "STP",0,7 });
	//wypisz<Rozkaz>(rozkazy);

	cout << "podaj nazwe pliku wyjsciowego" << endl;
	cin >> nazwa;
	//nazwa = "out.txt";
	out.open(nazwa, std::fstream::out /*| std::fstream::binary*/ | std::fstream::trunc);
	
	vector<string>slowa;
	char linia[100];
	/*
	while (plikRozkazow.eof()) {
		
		plikRozkazow.getline(linia, 100);
		dajSlowaZLini(linia,slowa);
		wypisz(slowa);
	}
	*/
	
	numer_lini = 0;
	//pierwsze czytanie DONE
	while (!in.eof()) {
		in.getline(linia, 100);
		dajSlowaZLini(linia, slowa);
		if (slowa[0] == "KON") {

		}
		//cout << slowa[slowa.size() - 1]<<"\t"<< slowa[slowa.size() - 1][slowa[slowa.size() - 1].size() - 1]<<endl;
		if (slowa[slowa.size()-1][slowa[slowa.size() - 1].size() - 1] == ':'){
			etykiety.push_back({ numer_lini,slowa[slowa.size() - 1].substr(0,slowa[slowa.size() - 1].size() - 1) });
			//cout << "dodawanie etykiety"<<endl;
			czyEtykiety.push_back(1);
		}
		else czyEtykiety.push_back(0);
		numer_lini++;
		//wypisz<string>(slowa);
		//cout << endl;
	}
	//wypisz<Etykieta>(etykiety);
	bool koniec = 0;
	unsigned short instrukcja;
	unsigned int doWpisania[2];
	unsigned char cDoW[2];
	numer_lini = 0;
	in.seekg(0);
	//drugie czytanie
	while (!in.eof()) {

		in.getline(linia, 100);
		dajSlowaZLini(linia, slowa);
		try {
			instrukcja = procesuj(slowa, czyEtykiety[numer_lini], etykiety, rozkazy, numer_lini, koniec);
		}
		catch (Bad_Syntax b) {
			cout << b.what();
			koniec = 1;
		}
		catch (invalid_argument ia) {
			cout <<ia.what();
			koniec = 1;
		}

		if (koniec) break;
		if (styl==1) {
			doWpisania[0] = instrukcja >> 11;
			instrukcja << 11;
			doWpisania[1] = instrukcja & 0x04FF;
			out << (int)doWpisania[0] << "\t" << (unsigned short)doWpisania[1] << endl;
		}
		else if (styl == 2) {
			for (size_t i = 0; i < 3; i++){
				out << instrukcja % 2;
				instrukcja >>= 1;
			}
			out << "\t";
			for (size_t i = 0; i < 11; i++) {
				out << instrukcja % 2;
				instrukcja >>= 1;
			}
			out << endl;
		}
		else if (styl == 3) {
			for (size_t j = 0; j < 4; j++) {
				for (size_t i = 0; i < 4; i++) {
					out << instrukcja % 2;
					instrukcja >>= 1;
				}
				out << "\t";
			}
			out << endl;
		}
		else {
			cDoW[0] = instrukcja >> 8;
			instrukcja << 8;
			cDoW[1] = instrukcja & 0x00FF;
			out << cDoW[1] << cDoW[2];
		}

		numer_lini++;
	}

	out.close();
	in.close();
	system("pause");
	return 0;
}