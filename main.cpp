//Thiago Zilberknop
//thiago.zilberknop@edu.pucrs.br
//Ultima atualizacao: 5/19/2023

//OBJETIVO: Esse programa cria e manipula uma estratura de dados baseada em um arquivo .csv que contem informacoes sobre as ruas e sinalizacoes de Porto Alegre.
//          O programa permite com que o usuario faca uma serie de consultas: sobre a proporcao de ruas, travessas, e avenidas quanto ao todo, sobre o numero de sina-
//          -zacoes sem coordenadas geograficas, sobre os dez dias que mais tiveram instalacoes, e, finalmente, um modo de navegacao que permite com que o usuario
//          "navegue" a estrutura de dados de inicio a fim, apresentando o nome da rua e uma lista com as datas de instalacao de toda sinalizacao daquela rua.

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace std;

#define TOTAL_RUAS 91228

#define AV 0
#define TRAV 1
#define R 2
#define LG 3
#define INDEF -1

#define DATA_EXTRACAO 0
#define DESCRICAO 1
#define	ESTADO 2
#define	COMPLEMENTO 3	
#define IMPLANTACAO 4
#define LOGRADOURO_NOME 5
#define	NUM_INICIAL 6
#define	NUM_FINAL 7
#define	DEFRONTE 8
#define	CRUZAMENTO_NOME 9
#define	LADO 10
#define FLUXO 11
#define	LOCAL_DE_INSTAL 12
#define	LATITUDE 13
#define	LONGITUDE 14

int helper_countDates() {
    int count = 0;
    int d = 21, m = 5, y = 1974; // oldest registered date
    while (d != 21 || m != 3 || y != 2023) {
        count++;
        // correctly increment date
        int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        if (leap) daysInMonth[2] = 29;
        else daysInMonth[2] = 28;
        d++; // increment day
        if (d > daysInMonth[m]) {
            d = 1;
            m++;
            if (m > 12) {
                m = 1;
                y++;
            }
        }
    }
    return count;
}

//quickmerge functions, adapted to sort an array of ints and an array of strings based on the sorting of the first array
int partition(int a[], string b[], int start, int end) {
  int i, tmp, left = start, pivot = a[end];
  string aux, strPivot = b[end];
  //elements greater than pivot to the right
  for (i = start; i < end; i++) {
    if (a[i] > pivot) {
      tmp = a[left];
      aux = b[left];
      a[left] = a[i];
      b[left] = b[i];
      a[i] = tmp;
      b[i] = aux;
      left++;
    }
  }
  //move pivot inbetween left and right sides
  a[end] = a[left];
  b[end] = b[left];
  a[left] = pivot;
  b[left] = strPivot;
  return left;
}

void quickSort(int a[], string b[], int start, int end) {
  int pivPos;
  if (end - start + 1 <= 1) return;
  //partition array and get pivot pos
  pivPos = partition(a, b, start, end);
  //sort partitions
  quickSort(a, b, start, pivPos - 1);
  quickSort(a, b, pivPos+1, end);
}
//function to trim whitespaces from strings (used to get rid of the space that comes after the street type (e.g.: R TIRADENTES, R is the type, the remaining string is _TIRADENTES, where _ is the space that was between R and TIRADENTES. The function removes that space.)
string trim(const std::string &s)
{
    auto start = s.begin();
    while (start != s.end() && isspace(*start)) {
        start++;
    }
 
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && isspace(*end));
 
    return std::string(start, end + 1);
}

class signNode { //nodes of  P L A C A S meu caro
    private:
        int day, month, year;
        double lati, longi;
        signNode* next;
    public:
        signNode() {
            day = month = year = 0;
            lati = longi = 0.0;
            next = nullptr;
        }
        signNode(string date, double la = 0.0, double lo = 0.0, signNode* n = nullptr) {
            stringstream ss(date);
            char trash;
            ss >> day >> trash >> month >> trash >> year;
            lati = la;
            longi = lo;
            next = n;
        }
        bool compare(const signNode* other) {
            if (year < other->year) {
                return true;
            }
            else if (year > other->year) {
                return false;
            }
            else {
                if (month < other->month) {
                    return true;
                }
                else if (month > other->month) {
                    return false;
                }
                else {
                    if (day < other->day) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }
        }
        signNode* getNext()             {return next;}
        string getDate()                {
            stringstream ss;
            ss << setw(2) << setfill('0') << day;
            ss << '/' << setw(2) << setfill('0') << month;
            ss << '/' << year;
            return ss.str();
        }
        int getDay()                    {return day;}
        int getMonth()                  {return month;}
        int getYear()                   {return year;}
        bool hasLatiLongi()             {if (lati != 0.0 && longi != 0.0) return true; else return false;}
        double getLati()                {return lati;}
        double getLongi()               {return longi;}
        void setNext(signNode* node)    {next = node;}
};

class signList { //list of  P L A C A S meu nobre
    private:
        signNode *first, *last;
        int size;
    public:
        signList() {
            first = last = nullptr;
            size = 0;
        }
        signList(signNode* node) {
            first = last = node;
            size = 1;
        }
        signList(signNode* f, signNode* l) {
            first = f;
            last = l;
            size = 2;
        }
        ~signList() {
            while (first != nullptr) {
                signNode* aux = first;
                first = first->getNext();
                delete aux;
            }
        }

        signNode* getFirst()    {return first;}
        signNode* getLast()     {return last;}
        int getSize()           {return size;}
        void addNode(signNode* node) {
            if (size == 0) {
                first = last = node;
                size++;
            } else if (size == 1) {
                if (node->compare(first)) {
                    node->setNext(first);
                    first = node;
                } else {
                    first->setNext(node);
                    last = node;
                }
                size++;
                return;
            } else {
                if (node->compare(first)) {
                    node->setNext(first);
                    first = node;
                } else if (!(node->compare(last))) {
                    last->setNext(node);
                    last = node;
                } else {
                    for (signNode *i = first, *l = first->getNext(); i != nullptr; i = i->getNext(), l = l->getNext()) {
                        if (node->compare(l)) {
                            node->setNext(l);
                            i->setNext(node);
                            return;
                        }
                    }
                }
                size++;
            }
        }
};

class streetNode {//nodes of SUTOREETO meu bom
    private:
        string name;
        signList* placas;
        int type;
        streetNode* next;
        streetNode* prev;
    public:
        streetNode(string nom = "", string tipo = "INDEF", streetNode* n = nullptr, streetNode* p = nullptr, signList* plac = new signList()) {
            name = nom;
            if (tipo == "AV") type = AV;
            else if (tipo == "TRAV") type = TRAV;
            else if (tipo == "R") type = R;
            else if (tipo == "LG") type = LG;
            else type = INDEF;
            next = n;
            prev = p;
            placas = plac;
        }
        string getName()        {return name;}
        string getType()           {
            switch (type) {
                case AV: return "AV"; break;
                case TRAV: return "TRAV"; break;
                case R: return "R"; break;
                case LG: return "LG";
            }
            return "";
        }
        int getTypeInt()        {return type;}
        streetNode* getNext()   {return next;}
        streetNode* getPrev()   {return prev;}
        signList* getPlacas()   {return placas;}

        void setName(string n)  {name = n;}
        void setType(string tipo) {
            if (tipo == "AV") type = AV;
            else if (tipo == "TRAV") type = TRAV;
            else if (tipo == "R") type = R;
            else if (tipo == "LG") type = LG;
            else type = INDEF;
        }
        void setType(int tipo) {type = tipo;}
        void setNext(streetNode* n) {next = n;}
        void setPrev(streetNode* p) {prev = p;}
        void setListPtr(signList* p){placas = p;}
};

class streetList {//list of ストレーと、コノヤロー
    private:
        streetNode *header, *trailer;
        int size;
    public:
        streetList() {
            header = new streetNode();
            trailer = new streetNode();
            size = 0;
        }
        ~streetList() {
            streetNode *auxh = header->getNext(), *auxt = trailer->getPrev();
            while (auxh != auxt && auxh != nullptr && auxt != nullptr && auxh->getPrev() != auxt->getNext()) {
                streetNode *auxH = auxh, *auxT = auxt;
                delete auxh->getPlacas();
                delete auxt->getPlacas();
                auxh = auxh->getNext();
                auxt = auxt->getPrev();
                delete auxH;
                delete auxT;
            }
        }
        streetNode* getHeader()     {return header;}
        streetNode* getTrailer()    {return trailer;}
        int getSize()               {return size;}
        void setHeaderNext(streetNode* n)   {header->setNext(n);}
        void setTrailerPrev(streetNode* n)  {trailer->setPrev(n);}
        void addNode(streetNode* node) {
            streetNode* first = header->getNext();
            streetNode* last = trailer->getPrev();
            //cout << "Add Node initialzied" << endl;
            if (size == 0) {
                //cout << "--size = 0" << endl;
                header->setNext(node);
                trailer->setPrev(node);
                size++;
                return;
            } else if (size == 1) {
                //cout <<"--size = 1" << endl;
                if (node->getName() == first->getName()) {
                    //cout <<"---name is same" << endl;
                    first->getPlacas()->addNode(node->getPlacas()->getFirst());
                    delete node;
                    return;
                } else if (node->getName().compare(first->getName()) < 0) {
                    //cout <<"---name is not same and goes before" << endl;
                    first->setPrev(node);
                    node->setNext(first);
                    header->setNext(node);
                    node->setPrev(header);
                } else {
                    //cout <<"---name is not same and goes after" << endl;
                    last->setNext(node);
                    node->setPrev(last);
                    node->setNext(trailer);
                    trailer->setPrev(node);
                }
                size++;
                return;
            } else {
                //cout << "--size > 1" << endl;
                streetNode* curr = first;
                //cout << "--entering loop: ";
                while (curr != nullptr && ((node->getName().compare(curr->getName()) != 0) && node->getName().compare(curr->getName()) > 0)) {
                    //cout << "loop... ";
                    curr = curr->getNext();
                }
                //cout << "loop ended." << endl;
                if (curr == nullptr) {
                    //cout << "---name is not same and goes before all" << endl;
                    first->setPrev(node);
                    node->setNext(first);
                    header->setNext(node);
                    node->setPrev(header);
                } else if (curr->getName() == node->getName()) {
                    //cout << "---name is same" << endl;
                    curr->getPlacas()->addNode(node->getPlacas()->getFirst());
                    delete node;
                    return;
                } else if (curr == first) {
                    //cout << "---name is not same and goes before all" << endl;
                    first->setPrev(node);
                    node->setNext(first);
                    header->setNext(node);
                    node->setPrev(header);
                } else {
                    //cout <<"---name is not same and goes somewhere in the middle" << endl;
                    node->setPrev(curr->getPrev());
                    node->setNext(curr);
                    curr->getPrev()->setNext(node);
                    curr->setPrev(node);
                }
            //cout << "Done! Size incremented." << endl;
            size++;
            }
        }
        void read() {
            int nodes = 1;
            ifstream infile("data.csv");
            string line, word, streetName, streetType, streetDate = "";
            double lati, longi = 0.0;
            //int runs = 0;
            if (!(infile.bad())) {
                getline(infile, line); //delete header
                while (!(infile.eof()) /*&& runs < 10*/) {
                    getline(infile, line); //get a new line
                    stringstream fileLine(line);
                    int index = 0;
                    while (getline(fileLine, word, ';')) {
                        stringstream ss(word);
                        switch(index) {
                            case (IMPLANTACAO): streetDate = word; break;
                            case (LOGRADOURO_NOME): ss >> streetType;
                            getline(ss, streetName);
                            streetName = trim(streetName);
                            break;
                            case (LATITUDE): 
                                try {
                                    lati = stod(word);
                                } catch (const invalid_argument& e) {
                                    lati = 0.0;
                                }
                                break;
                            case (LONGITUDE):
                                try {
                                    longi = stod(word);
                                } catch (const invalid_argument& e) {
                                    longi = 0.0;
                                }
                                break;
                            default: break;
                        }
                        index++;
                    }
                    //runs++;
                    //cout << "streetName: " << streetName << endl;
                    //cout << "streetType: " << streetType << endl;
                    //cout << "streetDate: " << streetDate << endl;
                    //cout << "Lati & Longi: " << lati << "x" << longi << endl;
                    addNode(new streetNode(streetName, streetType, nullptr, nullptr, new signList(new signNode(streetDate, lati, longi, nullptr))));
                    if (nodes % 10000 == 0) cout << setprecision(4) << static_cast<double>(nodes) / static_cast<double>(TOTAL_RUAS) << "    /   1.0000" << endl; 
                    nodes++;
                    
                }
                infile.close();
                return;
            } else {
                exception();
                return;
            }
        }
        void printList() {
            cout << "Tem certeza? Digite [1] se sim, ou qualquer outro numero para retornar." << endl;
            int key;
            cin >> key;
            if (key != 1) return;
            for (streetNode *curr = header->getNext(); curr != nullptr; curr = curr->getNext()) {
                cout << curr->getType() << " " << curr->getName() << endl;
                cout << "-Placas" << endl;
                for (signNode *aux = curr->getPlacas()->getFirst(); aux != nullptr; aux = aux->getNext()) {
                    cout << "--" << aux->getDate() << endl;
                }
            }
        }
        void rAvTravPorCento() {
            int ruas, avs, travs = 0;
            for (streetNode *curr = header->getNext(); curr !=nullptr; curr = curr->getNext()) {
                int sizeToAdd = curr->getPlacas()->getSize();
                if (curr->getTypeInt() == R) ruas += sizeToAdd;
                if (curr->getTypeInt() == AV) avs += sizeToAdd;
                if (curr->getTypeInt() == TRAV) travs += sizeToAdd;
            }
            int indef = TOTAL_RUAS - ruas - travs - avs;
            double percentRuas = (static_cast<double>(ruas) / static_cast<double>(TOTAL_RUAS)) * 100;
            double percentAvs = (static_cast<double>(avs) / static_cast<double>(TOTAL_RUAS)) * 100;
            double percentTravs = (static_cast<double>(travs) / static_cast<double>(TOTAL_RUAS)) * 100;
            double percentIndef = (static_cast<double>(indef) / static_cast<double>(TOTAL_RUAS)) * 100;
            cout << "# TOTAL DE SINALIZACOES REGISTRADAS EM RUAS, AVENIDAS, E TRAVESSAS" << endl;
            cout << "Ruas: " << ruas << "/" << TOTAL_RUAS << " = " << setprecision(2) << fixed << percentRuas << "% do total." << endl;
            cout << "Avenidas: " << avs << "/" << TOTAL_RUAS << " = " << setprecision(2) << fixed << percentAvs << "% do total." << endl;
            cout << "Travessas: " << travs << "/" << TOTAL_RUAS << " = " << setprecision(2) << fixed << percentTravs << "% do total." << endl;
            cout << "Outras: " << indef << "/" << TOTAL_RUAS << " = " << setprecision(2) << fixed << percentIndef << "% do total." << endl << endl;
            return;
        }
        void nilLatiLongi() {
            int nil = 0;
            streetNode *worst;
            int most = 0;
            for (streetNode *curr = header->getNext(); curr != nullptr; curr = curr->getNext()) {
                int count = 0;
                for (signNode *aux = curr->getPlacas()->getFirst(); aux != nullptr; aux = aux->getNext()) {
                    if (!aux->hasLatiLongi()) {
                        nil++;
                        count++;
                    }
                }
                if (count > most) {
                    most = count;
                    worst = curr;
                }
            }
            cout << nil << " placas com latitude e longitudes indefinidas." << endl;
            cout << worst->getName() << " e a pior rua, com " << most << " placas sem coordenadas." << endl << endl;
        }
        //metodo homem das cavernas unga bunga pra averiguar top 10
        /*void topDez() {
            int top[10] = {0};
            string dates[10];
            int d = 21, m = 5, y = 1974; // oldest registered date
            while (d != 21 || m != 3 || y != 2023) { // most recent date + 1 day
                int count = 0;
                stringstream ss;
                ss << setw(2) << setfill('0') << d << '/';
                ss << setw(2) << setfill('0') << m << '/';
                ss << y;
                string date = ss.str();
                for (streetNode* curr = getHeader()->getNext(); curr != nullptr; curr = curr->getNext()) {
                    for (signNode* aux = curr->getPlacas()->getFirst(); aux != nullptr; aux = aux->getNext()) {
                        if (date == aux->getDate()) count++;
                    }
                }
                if (count != 0) { //if the count did go up at least once, find out if the new date and count should go on the arrays
                    for (int i = 9; i >= 0; i--) {
                        if (count > top[i] && i < 9) {
                            top[i + 1] = top[i];
                            dates[i + 1] = dates[i];
                        }
                        top[i] = count;
                        dates[i] = date;
                        break;
                    }
                }   
                // find the next date to search the lists
                int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 ==0);
                if (leap) daysInMonth[2] = 29;
                else daysInMonth[2] = 28;
                d++; // increment day
                if (d > daysInMonth[m]) {
                    d = 1;
                    m++;
                    if (m > 12) {
                        m = 1;
                        y++;
                    }
                }
            }
            cout << "TOP 10 DIAS COM MAIS INSTALACOES" << endl;
            for (int i = 0; i < 10; i++) {
                cout << "[" << i+1 << "] " << top[i] << " Placas no dia " << dates[i] << endl; 
            }
        }*/

        //metodo homem moderno que nao sabe amarrar tenis
        void topDez() {
            cout << "Thinking";
            int size = helper_countDates();
            int top[size] = {0};
            string dates[size];
            for (int i = 0; i < size; i++) dates[i] = "";
            streetNode* curr = header->getNext();
            while (curr != nullptr) {
                signNode* aux = curr->getPlacas()->getFirst();
                    while (aux != nullptr) {
                        string date = aux->getDate();
                        bool dateFound = false;
                        for (int i = 0; i < size; i++) {
                            if (dates[i] == date) { //date do exist
                                top[i]++;
                                dateFound = true;
                                break;
                            }
                        }
                        if (!dateFound) { //if date dont exist
                            for (int i = 0; i < size; i++) {
                                if (dates[i] == "") { //find empty slot for new date
                                    dates[i] = date;
                                    top[i] = 1;
                                    break;
                                } else if (top[i] < 1 || (i < size && top[i + 1] < 1)) { //if no empty slot, check if any dates have zero counts or if the next date over has zero counts
                                    for (int j = size - 1; j > i; j--) {
                                        dates[j] = dates[j - 1]; //then, if true, make room
                                        top[j] = top[j - 1];
                                    }
                                dates[i] = date;
                                top[i] = 1;
                                if (i % 200 == 0) cout << '.';
                                break;
                                }
                            }
                        }
                    aux = aux->getNext();
                    }
                curr = curr->getNext();
                }
            quickSort(top, dates, 0, size-1);
            cout << endl << endl << "TOP 10 DIAS COM MAIS INSTALACOES" << endl;
            for (int i = 0; i < 10; i++) {
                cout << "[" << i + 1 << "] " << top[i] << " Placas no dia " << dates[i] << endl;
            }
            cout << endl;
        }
        void nav() {
            bool run = true;
            streetNode *curr = header->getNext();
            int ruas, avs, travs = 0;
            for (streetNode *curr = header->getNext(); curr !=nullptr; curr = curr->getNext()) {
                int sizeToAdd = curr->getPlacas()->getSize();
                if (curr->getTypeInt() == R) ruas += sizeToAdd;
                if (curr->getTypeInt() == AV) avs += sizeToAdd;
                if (curr->getTypeInt() == TRAV) travs += sizeToAdd;
            }
            int indef = TOTAL_RUAS - ruas - travs - avs;
            while (run) {
                double size = static_cast<double>(curr->getPlacas()->getSize());
                cout << curr->getType() << " " << curr->getName() << " [";
                switch (curr->getTypeInt()) {
                    case R: cout << setprecision(4) << fixed <<(size / static_cast<double>(ruas)) * 100 << "% das sinalizacoes em Ruas]["; break;
                    case TRAV: cout << setprecision(4) << fixed << (size / static_cast<double>(travs)) * 100 << "% das sinalizacoes em Travessas]["; break;
                    case AV: cout << setprecision(4) << fixed << (size / static_cast<double>(avs)) * 100 << "% das sinalizacoes em Avenidas]["; break;
                    case INDEF: cout << setprecision(4) << fixed << (size / static_cast<double>(indef)) * 100 << "% das sinalizacoes em Outras]["; break;
                }
                cout << setprecision(4) << fixed << (size / static_cast<double>(TOTAL_RUAS)) * 100 << "% de todas sinalizacoes]" << endl;
                cout << "-Placas (" << curr->getPlacas()->getSize() << ")" << endl;
                for (signNode *aux = curr->getPlacas()->getFirst(); aux != nullptr; aux = aux->getNext()) {
                    cout << "--" << aux->getDate() << endl;
                }
                cout << endl;
                cout << "Para operar o Modo de Navegação, digite:" << endl;
                cout << "[1] : Anterior." << endl;
                cout << "[2] : Anterior * 10." << endl;
                cout << "[3] : Anterior * 100." << endl;
                cout << "[4] : Proximo." << endl;
                cout << "[5] : Proximo * 10." << endl;
                cout << "[6] : Proximo * 100." << endl;
                cout << "[0] : Sair do Modo de Navegacao." << endl;
                int key;
                cin >> key;
                streetNode* aux = curr;
                switch (key) {
                    case 1: 
                        if (curr->getPrev() == nullptr) {
                            cout << "Fim da lista neste sentido. Tente navegar para direita." << endl;
                            continue;
                        } else {
                            curr = curr->getPrev();
                            continue;
                        } break;
                    case 2: 
                        for (int i = 0; aux != nullptr && i < 10; aux = aux->getPrev(), i++)
                        if (aux == nullptr) {
                            cout << "Fim da lista neste sentido com 10 ou menos passos. Tente navegar para direita." << endl;
                            continue;
                        } else {
                            curr = aux;
                            continue;
                        } break;
                    case 3: 
                        for (int i = 0; aux != nullptr && i < 100; aux = aux->getPrev(), i++)
                        if (aux == nullptr) {
                            cout << "Fim da lista neste sentido com 100 ou menos passos. Tente navegar para direita." << endl;
                            continue;
                        } else {
                            curr = aux;
                            continue;
                        } break;
                    case 4: 
                        if (curr->getNext() == nullptr) {
                            cout << "Fim da lista neste sentido. Tente navegar para a esquerda." << endl;
                            continue;
                        } else {
                            curr = curr->getNext();
                            continue;
                        } break;
                    case 5: 
                        for (int i = 0; aux != nullptr && i < 10; aux = aux->getNext(), i++);
                        if (aux == nullptr) {
                            cout << "Fim da lista neste sentido com 10 ou menos passos. Tente navegar para esquerda." << endl;
                            continue;
                        } else {
                            curr = aux;
                            continue;
                        } break;
                    case 6: 
                        for (int i = 0; aux != nullptr && i < 100; aux = aux->getNext(), i++);
                        if (aux == nullptr) {
                            cout << "Fim da lista neste sentido com 100 ou menos passos. Tente navegar para esquerda." << endl;
                            continue;
                        } else {
                            curr = aux;
                            continue;
                        } break;
                    case 0: run = false; break;
                    default: cout << "Por favor digite apenas comandos validos." << endl; continue;
                }
            }
        }
        /*void search() {
            cout << "Digite o nome da rua a ser pesquisada com apenas letras maisculuas e sem acentuacao: ";
            string search;
            cin >> search;
            for (streetNode* aux = getHeader()->getNext(); aux != nullptr; aux = aux->getNext()) {
                if (search.compare(aux->getName()) == 0) {
                    cout << aux->getType() << " " << aux->getName() << endl;
                    cout << "-Placas" << endl;
                    int i = 1;
                    for (signNode* temp = aux->getPlacas()->getFirst(); temp != nullptr; temp = temp->getNext()) {
                        cout << "----" << i << "----" << endl;
                        cout << "Implantacao: " << temp->getDate();
                        if (temp->hasLatiLongi()) {
                            cout << "Latitude: " << temp->getLati() << endl;
                            cout << "Longitude: " << temp->getLongi() << endl;
                        } else {
                            cout << "Latitude: INDEF" << endl;
                            cout << "Longitude: INDEF" << endl;
                        }
                        i++;
                    }
                }
            }
            return;
        }*/
        void printStreetNamesLinked() {
            cout << "Tem certeza? Digite [1] se sim, ou qualquer outro numero para retornar." << endl;
            int key;
            cin >> key;
            if (key != 1) return;
            cout << "Header <->" << endl;
            for (streetNode* aux = header->getNext(); aux != nullptr; aux = aux->getNext()) {
                cout << "<->" << aux->getName() << "<->" << endl;
            }
            cout << "<-> Trailer" << endl;
        }
        void dateSignCount() {
            cout << "Por favor insira uma data (formato: DD/MM/AAAA): ";
            string data;
            cin >> data;
            int count = 0;
            for (streetNode* curr = header->getNext(); curr != nullptr; curr = curr->getNext()) {
                for (signNode* aux = curr->getPlacas()->getFirst(); aux != nullptr; aux = aux->getNext()) {
                    if (data == aux->getDate()) count ++;
                }
            }
            cout << "Houveram " << count << " instalacoes nesse dia." << endl << endl;
            return;
        }
};

int main() {//main application
    streetList ruas;
    cout << "Aguarde uns instantes enquanto o banco de dados é criado..." << endl;
    ruas.read();
    while (1) {
        cout << "Para consultar informacoes, digite: " << endl;
        cout << "[1] : relatorio do total de sinalizacoes registradas em ruas, avenidas, e travessas em relacao ao todo." << endl;
        cout << "[2] : relatorio da quantidade de placas sem coordenadas." << endl;
        cout << "[3] : relatorio dos 10 dias com mais instalacoes." << endl;
        cout << "[4] : relatorio de instalacoes em um dia especifico." << endl;
        cout << "[5] : modo de navegacao" << endl;
        cout << "[6] : (perigoso) imprimir todas as ruas e todas as placas" << endl;
        cout << "[7] : (levemente perigoso) imprimir todas as ruas." << endl;
        cout << "[8] : fechar programa." << endl;
        int key = 0;
        cin >> key;
        switch (key) {
            case 1: ruas.rAvTravPorCento(); continue;
            case 2: ruas.nilLatiLongi(); continue;
            case 3: ruas.topDez(); continue;
            case 4: ruas.dateSignCount(); continue;
            case 5: ruas.nav(); continue;
            case 6: ruas.printList(); continue;
            case 7: ruas.printStreetNamesLinked(); continue;
            case 8: exit(1);
            default: cout << "Digite um comando valido." << endl << endl; continue;
        }
    }
}