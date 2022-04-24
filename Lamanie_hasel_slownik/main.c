#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>

#define NUMTHREADS 8

//Zmienne globalne
char** zlamane_tab;
bool *czy_zlamane;
int zlamane_ilosc = 0;
int hasla_ilosc;
pthread_mutex_t aktualizacja_zlamanych;
char znaki_specjalne_tab[13] = {'_','-',' ','2','4','.','#','$','%','&','*','!','?'};

struct watek_dane
{
    char** hasla;
    char** slownik;
    int wielkosc_hasla;
    int wielkosc_slownik;
};

struct watek_dane watek_dane_tab[NUMTHREADS];

int ile_cyfr(int liczba) //funkcja do wyliczania ilosci cyfr danej liczby
{
    if(liczba == 0)
    {
        return 1;
    }
    else
    {
        return (int)floor(log10(abs(liczba)))+1; // taka kombinacja funkcji matematycznych zwraca ilosc cyfr dla kazdej liczby oprocz 0, dla zera zwraca -inf dlatego
    }                                            // specjalny if dla 0
}

void dodanie_z_przodu(char* slowo, int liczba, char** do_zmiany) //funkcja dodawania liczb z przodu slowa
{
    char zmienione[strlen(slowo)+ ile_cyfr(liczba)+1];
    char buffor[ile_cyfr(liczba)+1];
    sprintf(buffor,"%d",liczba); //pozwala wpisac liczbe (int) jako slowo (tablice charow)
    for (int i = 0; i < strlen(slowo)+ ile_cyfr(liczba)+1; i++)
    {
        if(i< strlen(buffor)) //na poczatku wpisujemy liczbe
        {
            zmienione[i] = buffor[i];
        }
        else
        {
            zmienione[i] = slowo[i- strlen(buffor)]; //nastepnie przepisujemy slowo
        }                                            // w ten sposob otrzymujemy np.: slowo, 123 -> 123slowo
    }
    zmienione[strlen(slowo)+ ile_cyfr(liczba)] = '\0'; //dla pewnosci, zeby nie bylo "smieci" na koncu slowa
    strcpy(*do_zmiany,zmienione); //w ten sposob przekazuje zmienione slowo
}

void dodanie_z_tylu(char* slowo, int liczba, char** do_zmiany) //funkcja dodawania liczby na koniec slowa
{                                                              //dziala analogidznie jak funkcja wyzej tylko ze dodaje liczbe na koniec
    char zmienione[strlen(slowo)+ ile_cyfr(liczba)+1];
    char buffor[ile_cyfr(liczba)+1];
    sprintf(buffor,"%d",liczba);
    for (int i = 0; i < strlen(slowo)+ ile_cyfr(liczba)+1; i++)
    {
        if(i< strlen(slowo))
        {
            zmienione[i] = slowo[i];
        }
        else
        {
            zmienione[i] = buffor[i- strlen(slowo)];
        }
    }
    zmienione[strlen(slowo)+ ile_cyfr(liczba)] = '\0';
    strcpy(*do_zmiany,zmienione);
}

void konwersja_wszystkie_male(char* slowo, char** do_zmiany) //funkcja konwertowania wszystkich liter w slowie na male
{
    char zmienione[strlen(slowo)+1];
    for (int j = 0; j < strlen(slowo); j++) //konwersja na male litery
    {
        if (slowo[j] >= 65 && slowo[j] <= 90) //przedzial 65-90 to w tablicy ascii duze litery
        {
            zmienione[j] = slowo[j] + 32; // liczba 32 to liczba ktora dzieli duza od malej litery tzn: A + 32 = a analogicznie A = a-32
        }
        else
        {
            zmienione[j] = slowo[j];
        }
    }
    zmienione[strlen(slowo)] = '\0'; //podobnie jak wczesniej upewniam sie ze tu jest koniec slowa (nie ma "smieci" na koncu)
    strcpy(*do_zmiany,zmienione);    // przekazuje zmienione slowo
}

void konwersja_pierwsza_duza(char* slowo, char** do_zmiany) // funkcja konwertowania pierwszej litery na duza a pozostalych na male w slowie
{                                                           //dziala analogicznie co powyzsza funckcja tylko "wyifowany" jest
    char zmienione[strlen(slowo)+1];                        //pierwszy znak, ktory musi zostac ustawiony na duza litere
    for (int j = 0; j < strlen(slowo); j++)
    {
        if(j == 0)
        {
            if (slowo[j] >= 97 && slowo[j] <= 122)
            {
                zmienione[j] = slowo[j] - 32;
            }
        }
        else
        {
            if (slowo[j] >= 65 && slowo[j] <= 90)
            {
                zmienione[j] = slowo[j] + 32;
            }
            else
            {
                zmienione[j] = slowo[j];
            }
        }
    }
    zmienione[strlen(slowo)] = '\0';
    strcpy(*do_zmiany,zmienione);
}

void konwersja_wszystkie_duze(char* slowo, char** do_zmiany) //funkcja do konwertowania wszystkich liter w slowie na duze, dziala analogicznie
{                                                            // do funkcji konwertujacej wszystkie litery na male
    char zmienione[strlen(slowo)+1];
    for (int j = 0; j < strlen(slowo); j++)
    {
        if (slowo[j] >= 97 && slowo[j] <= 122)
        {
            zmienione[j] = slowo[j] - 32;
        }
        else
        {
            zmienione[j] = slowo[j];
        }
    }
    zmienione[strlen(slowo)] = '\0';
    strcpy(*do_zmiany,zmienione);
}

void lamanie(char* slowo, char** hasla, int wielkosc_hasla) //funkcja ktora przyjmuje slowo, haszuje je i porownuje z haslami do zlamania
{
    MD5_CTX ctx;
    unsigned char md5_tab[16];
    unsigned int dlugosc = (unsigned int) strlen(slowo);
    MD5_Init(&ctx);
    MD5_Update(&ctx, slowo, dlugosc); //funkcje z rodziny MD5 do haszowania slow
    MD5_Final(md5_tab, &ctx);
    char buffor[3];
    char hasz[33]; //tu bedzie przechowywany nasz hasz
    for (int k = 0; k < 16; k++)
    {
        sprintf(buffor, "%02x", md5_tab[k]);
        hasz[2 * k] = buffor[0]; //wpisuje wyliczone znaki z hasza
        hasz[2 * k + 1] = buffor[1];
    }
    hasz[32] = '\0'; //bez tego "smieci" na koncu naszego hasza
    for (int l = 0; l < wielkosc_hasla; l++) //w tym forze porownuje uzyskany przez nas hasz z haszami z tablicy hasel, jesli jakies hasze beda
    {                                        // sie zgadzaly znaczy to, ze dane haslo zostalo zlamane
        if(czy_zlamane[l] == 0) //sprawdzam czy dane haslo zostalo juz zlamane
        {
            if (strcmp(hasz, hasla[l]) == 0) //funkcja strcmp porownuje dwa ciagi znakow
            {
                pthread_mutex_lock(&aktualizacja_zlamanych); //tutaj watki beda pracowaly na zmiennych globalnych stad muteks
                czy_zlamane[l] = 1; //zaznaczam, ze dane haslo zostalo zlamane w globalnej tablicy booli
                zlamane_tab[l] = malloc(sizeof(char) * (strlen(slowo) + 1)); //w tablicy przetrzymujacej zlamane haslo alokuje pamiec
                strcpy(zlamane_tab[l], slowo); //wpisuje zlamane haslo do tablicy
                zlamane_ilosc++; //zwiekszam zmienna informujaca ile hasel zostalo juz zlamane (zmienna potrzebna do wyswietlenia statystyk)
                pthread_mutex_unlock(&aktualizacja_zlamanych); //odblokowywuje muteks
            }
        }
    }
}

void* Lamacz0(void* arg)
{
    struct watek_dane *L0_dane; //rozpakowywanie danych watku
    L0_dane = (struct watek_dane*) arg;
    char** hasla = L0_dane->hasla;
    char** slownik = L0_dane->slownik;
    int wielkosc_hasla = L0_dane->wielkosc_hasla;
    int wielkosc_slownik = L0_dane->wielkosc_slownik;
    int liczba = 0;
    char *sprawdzane;
    char *liczba_tyl;
    char *liczba_przod;
    while(liczba<2147483647) //program dziala teoretycznie w nieskonczonosc (liczba w srodku - najwieksza wartosc dla inta)
    {
        for (int i = 0; i < wielkosc_slownik; i++)
        {
            sprawdzane = realloc(sprawdzane,sizeof(char) * ((strlen(slownik[i]) + 1)));
            liczba_tyl = realloc(liczba_tyl,sizeof(char) * ((strlen(slownik[i]) + ile_cyfr(liczba) + 1))); //alokowanie pamieci na zmienne, realloc szybszy niz malloc + free
            liczba_przod = realloc(liczba_przod,sizeof(char) * ((strlen(slownik[i]) + ile_cyfr(liczba) + 1)));
            konwersja_wszystkie_male(slownik[i],&sprawdzane);
            if (liczba == 0) //pierwsze przejscie bez liczby z przodu/tylu
            {
                lamanie(sprawdzane, hasla, wielkosc_hasla);
            }
            dodanie_z_tylu(sprawdzane, liczba, &liczba_tyl); //dodaje liczbe z tylu
            lamanie(liczba_tyl, hasla, wielkosc_hasla); //proba lamania kombinacji slowo+liczba
            dodanie_z_przodu(sprawdzane, liczba, &liczba_przod); //dodaje liczbe z przodu
            lamanie(liczba_przod, hasla, wielkosc_hasla);//proba lamania kombinacji liczba+slowo
        }
        liczba++;
    }
    pthread_exit(NULL);
}

void* Lamacz1(void* arg)
{
    struct watek_dane *L1_dane; //rozpakowywanie danych watku
    L1_dane = (struct watek_dane*) arg;
    char** hasla = L1_dane->hasla;
    char** slownik = L1_dane->slownik;
    int wielkosc_hasla = L1_dane->wielkosc_hasla;
    int wielkosc_slownik = L1_dane->wielkosc_slownik;
    int liczba = 0;
    char *sprawdzane;
    char *liczba_tyl;
    char *liczba_przod;
    while(liczba<2147483647) //cialo watku jest praktycznie takie samo co watku wyzej, to dlatego ze ten watek uzywa innej konwersji liter w slowie, po za tym
    {                        // jego operacje takie jak dodanie liczby z przodu/tylu czy zahaszowanie i sprawdzenie sa takie same
        for (int i = 0; i < wielkosc_slownik; i++)
        {
            sprawdzane = realloc(sprawdzane,sizeof(char) * ((strlen(slownik[i]) + 1)));
            liczba_tyl = realloc(liczba_tyl,sizeof(char) * ((strlen(slownik[i]) + ile_cyfr(liczba) + 1)));
            liczba_przod = realloc(liczba_przod,sizeof(char) * ((strlen(slownik[i]) + ile_cyfr(liczba) + 1)));
            konwersja_pierwsza_duza(slownik[i],&sprawdzane);
            if (liczba == 0)
            {
                lamanie(sprawdzane, hasla, wielkosc_hasla);
            }
            dodanie_z_tylu(sprawdzane, liczba, &liczba_tyl);
            lamanie(liczba_tyl, hasla, wielkosc_hasla);
            dodanie_z_przodu(sprawdzane, liczba, &liczba_przod);
            lamanie(liczba_przod, hasla, wielkosc_hasla);
        }
        liczba++;
    }
    pthread_exit(NULL);
}

void* Lamacz2(void* arg)
{
    struct watek_dane *L2_dane;
    L2_dane = (struct watek_dane*) arg;
    char** hasla = L2_dane->hasla;
    char** slownik = L2_dane->slownik;
    int wielkosc_hasla = L2_dane->wielkosc_hasla;
    int wielkosc_slownik = L2_dane->wielkosc_slownik;
    int liczba = 0;
    char *sprawdzane;
    char *liczba_tyl;
    char *liczba_przod;
    while(liczba<2147483647) //program dziala teoretycznie w nieskonczonosc
    {
        for (int i = 0; i < wielkosc_slownik; i++)
        {
            sprawdzane = realloc(sprawdzane,sizeof(char) * ((strlen(slownik[i]) + 1)));
            liczba_tyl = realloc(liczba_tyl,sizeof(char) * ((strlen(slownik[i]) + ile_cyfr(liczba) + 1)));
            liczba_przod = realloc(liczba_przod,sizeof(char) * ((strlen(slownik[i]) + ile_cyfr(liczba) + 1)));
            konwersja_wszystkie_duze(slownik[i],&sprawdzane);
            if (liczba == 0) //pierwsze przejscie bez liczby z przodu/tylu
            {
                lamanie(sprawdzane, hasla, wielkosc_hasla);
            }
            dodanie_z_tylu(sprawdzane, liczba, &liczba_tyl);
            lamanie(liczba_tyl, hasla, wielkosc_hasla);
            dodanie_z_przodu(sprawdzane, liczba, &liczba_przod);
            lamanie(liczba_przod, hasla, wielkosc_hasla);
        }
        liczba++;
    }
    pthread_exit(NULL);
}

void* Lamacz3(void* arg)
{
    struct watek_dane *L3_dane; //rozpakowanie danych
    L3_dane = (struct watek_dane*) arg;
    char** hasla = L3_dane->hasla;
    int wielkosc_hasla = L3_dane->wielkosc_hasla;
    int liczba = 0;
    char* sprawdzane;
    while(liczba<2147483647) //program dziala teoretycznie w nieskonczonosc
    {
        sprawdzane = realloc(sprawdzane,sizeof(char)* (ile_cyfr(liczba) + 1)); //alokuje pamiec
        sprintf(sprawdzane,"%d",liczba); //wczytuje liczbe do tablicy charow
        lamanie(sprawdzane, hasla, wielkosc_hasla); //proba zlamania hasla
        liczba++;
    }
    pthread_exit(NULL);
}

void lacznik_slow_szs(char* slowo1, char*slowo2, char** do_zmiany, int znak) // funkcja ktora laczy slowa w nastepujacy sposob slowo + znak specjalny + slowo
{                                                                            //szs = slowo znak slowo
    char zmienione[strlen(slowo1)+ strlen(slowo2)+1+1];
    for(int i=0;i<strlen(slowo1)+ strlen(slowo2)+1;i++)
    {
        if(i<strlen(slowo1)) //na poczatek wpisuje pierwsze slowo
        {
            zmienione[i] = slowo1[i];
        }
        if(i==strlen(slowo1)) // po pierwszym slowie dodaje znak specjalny
        {
            zmienione[i] = znaki_specjalne_tab[znak];
        }
        if(i>strlen(slowo1)) //nastepnie wpisuje drugie slowo
        {
            zmienione[i] = slowo2[i-strlen(slowo1)-1];
        }
    }
    zmienione[strlen(slowo1)+ strlen(slowo2)+1] = '\0'; //upewniam sie ze to koniec slowa (nie ma zandych "smieci" na koncu)
    strcpy(*do_zmiany,zmienione); // przekazuje uzyskany ciag znakow (slowo+znak+slowo)
}

void* Lamacz4(void* arg) // Lamacz4 - Lamacz6 - Lamacze dwu-wyrazowi podobnie jak lamacze 0-2 lamia te same slowa, ale dla roznych wielkosci liter
{                       // Lamacz4 lamie slowa dwuwyrazowe skladajace sie ze slow ktore skladaja sie z malych liter
    struct watek_dane *L4_dane; //rozpakowanie danych
    L4_dane = (struct watek_dane*) arg;
    char** hasla = L4_dane->hasla;
    char** slownik = L4_dane->slownik;
    int wielkosc_hasla = L4_dane->wielkosc_hasla;
    int wielkosc_slownik = L4_dane->wielkosc_slownik;
    char* slowo1;
    char* slowo2;
    char* sprawdzane;
    for(int i=0;i<wielkosc_slownik;i++)
    {
        slowo1 = realloc(slowo1,sizeof(char)* (strlen(slownik[i])+1)); //alokuje pamiec dla pierwszego slowa
        konwersja_wszystkie_male(slownik[i],&slowo1); //zamieniam liery na male
        for(int j=0;j<wielkosc_slownik;j++)
        {
            slowo2 = realloc(slowo2,sizeof(char)* (strlen(slownik[j])+1)); //alokuja pamiec dla drugiegoslowa
            konwersja_wszystkie_male(slownik[j],&slowo2); //zamieniam litery na male
            for(int k=0;k<13;k++) // 13 bo taka wielkosc tablicy znakow (tak wymyslilem/zalozylem)
            {
                sprawdzane = realloc(sprawdzane, sizeof(char)*(strlen(slownik[i]) + strlen(slownik[j]) +2)); //alokuje pamiec dla ciagu znakow = slowo+znak+slowo
                lacznik_slow_szs(slowo1,slowo2,&sprawdzane,k); //"sklejam" slowa w jeden ciag znakow
                lamanie(sprawdzane, hasla, wielkosc_hasla); //probuje zlamac haslo
            }
        }
    }
    pthread_exit(NULL);
}

void* Lamacz5(void* arg) //Lamacz 5 dziala analogicznie do Lamacza 4 tylko robi konwesje liter taka, ze pierwsza litera duza reszta mala
{
    struct watek_dane *L5_dane;
    L5_dane = (struct watek_dane*) arg;
    char** hasla = L5_dane->hasla;
    char** slownik = L5_dane->slownik;
    int wielkosc_hasla = L5_dane->wielkosc_hasla;
    int wielkosc_slownik = L5_dane->wielkosc_slownik;
    char* slowo1;
    char* slowo2;
    char* sprawdzane;
    for(int i=0;i<wielkosc_slownik;i++)
    {
        slowo1 = realloc(slowo1,sizeof(char)* (strlen(slownik[i])+1));
        konwersja_pierwsza_duza(slownik[i],&slowo1);
        for(int j=0;j<wielkosc_slownik;j++)
        {
            slowo2 = realloc(slowo2,sizeof(char)* (strlen(slownik[j])+1));
            konwersja_pierwsza_duza(slownik[j],&slowo2);
            for(int k=0;k<13;k++) // 13 bo taka wielkosc tablicy znakow (tak wymyslilem/zalozylem)
            {
                sprawdzane = realloc(sprawdzane, sizeof(char)*(strlen(slownik[i]) + strlen(slownik[j]) +2));
                lacznik_slow_szs(slowo1,slowo2,&sprawdzane,k);
                lamanie(sprawdzane, hasla, wielkosc_hasla);
            }
        }
    }
    pthread_exit(NULL);
}

void* Lamacz6(void* arg) // Lamacz6 dziala analogicznie do lamacza4 i 5 tylko zmienia wszystkie litery na duze w slowach
{
    struct watek_dane *L6_dane;
    L6_dane = (struct watek_dane*) arg;
    char** hasla = L6_dane->hasla;
    char** slownik = L6_dane->slownik;
    int wielkosc_hasla = L6_dane->wielkosc_hasla;
    int wielkosc_slownik = L6_dane->wielkosc_slownik;
    char* slowo1;
    char* slowo2;
    char* sprawdzane;
    for(int i=0;i<wielkosc_slownik;i++)
    {
        slowo1 = realloc(slowo1,sizeof(char)* (strlen(slownik[i])+1));
        konwersja_wszystkie_duze(slownik[i],&slowo1);
        for(int j=0;j<wielkosc_slownik;j++)
        {
            slowo2 = realloc(slowo2,sizeof(char)* (strlen(slownik[j])+1));
            konwersja_wszystkie_duze(slownik[j],&slowo2);
            for(int k=0;k<13;k++) // 13 bo taka wielkosc tablicy znakow (tak wymyslilem/zalozylem)
            {
                sprawdzane = realloc(sprawdzane, sizeof(char)*(strlen(slownik[i]) + strlen(slownik[j]) +2));
                lacznik_slow_szs(slowo1,slowo2,&sprawdzane,k);
                lamanie(sprawdzane, hasla, wielkosc_hasla);
            }
        }
    }
    pthread_exit(NULL);
}

void handler_sighup(int sig) //handler sygnalu SIGHUP
{
    printf("Obecne statystyki: %d/%d\n",zlamane_ilosc,hasla_ilosc); //Wypisuje statysyki, ile hasel zostalo zlamanych
}

void* Konsument(void* arg)
{
    struct sigaction sa; //Konstrukcja handlera sygnalow
    sa.sa_handler = &handler_sighup;
    //sa.sa_flags;
    sigaction(SIGHUP,&sa,NULL);
    struct watek_dane *L0_dane;
    L0_dane = (struct watek_dane*) arg;
    int wielkosc_hasla = L0_dane->wielkosc_hasla;
    bool czy_wyswietlone[wielkosc_hasla+1]; // tablica w ktorej przechowuje informacje o tym ktore haslo zostalo juz wyswietlone, by nie wyswietlac go ponownie
    while(1) // nieskonczona petla, konsument caly czas sprawdza czy jakies haslo zostalo zlamane, by je wyswietlic
    {
        for(int i=0;i<wielkosc_hasla;i++)
        {
            if(czy_zlamane[i]==1 && czy_wyswietlone[i]==0) //sprawdzam czy dane haslo zostalo juz zlamane oraz czy bylo juz wyswietlone
            {
                pthread_mutex_lock (&aktualizacja_zlamanych); //blokuje mutkes, poniewaz operuje na danych globalnych
                czy_wyswietlone[i]=1; // zaznaczam, ze dane haslo zostalo juz wyswietlone
                printf("Zlamane haslo to %s\n",zlamane_tab[i]); //wyswietlam haslo
                pthread_mutex_unlock (&aktualizacja_zlamanych); // odblokowywuje muteks
            }
        }
    }
    pthread_exit(NULL);
}
int wielkosc_pliku(char* nazwa_pliku) //funkcja do sprawdzenia wielkosci pliku (z ilu liniek sie sklada)
{
    FILE* plik = fopen(nazwa_pliku,"r"); //otwieram plik w trybie read
    if(plik == NULL)
    {
        printf("Error, nie odczytano pliku\n");
        exit(1);
    }
    int ilosc_linii =1;
    char znak;
    while(fscanf(plik,"%c",&znak) != EOF) // wczytuje znaki az dojde do konca pliku
    {                                            //EOF - end of file
        if(znak == 10) // z tablicy ASCII 10 = newline
        {
            ilosc_linii++;
        }
    }
    fclose(plik); //zamykam plik
    return ilosc_linii; //zwracam ilosc linii
}


int main(int argc, char *argv[])
{
    char* hasla = argv[1];
    char* slownik = argv[2];
    int wielkosc_hasla = wielkosc_pliku(hasla); //sprawdzam wielkosc plikow
    int wielkosc_slownik = wielkosc_pliku(slownik);
    char** hasla_tab = malloc(sizeof(char*)*wielkosc_hasla); //alokuje pamiec dla tablicy slow slownika
    char** slownik_tab = malloc(sizeof(char*)*wielkosc_slownik); //alokuje pamiec dla tablicy haszy hasel
    zlamane_tab = malloc(sizeof(char*)*wielkosc_hasla); //alokuje pamiec dla innych dynamicznych zmiennych globalnych
    czy_zlamane = calloc(wielkosc_hasla,sizeof(bool));
    hasla_ilosc = wielkosc_hasla;
    FILE* plik_hasla;
    FILE* plik_slownik;
    plik_hasla = fopen(hasla, "r"); //otwieram plik hasel
    int i=0;
    char* linia_tekstu = malloc(sizeof(char)*33); //tzw buffor
    size_t we=33;
    while(!feof(plik_hasla)) //dopoki nie dojdziemy do konca pliku
    {
        getline(&linia_tekstu, &we, plik_hasla); //getline zwraca newline jako ostatni znak
        if(linia_tekstu[strlen(linia_tekstu)-1] == '\n') // 10 lub '\n'
        {
            linia_tekstu[strlen(linia_tekstu) - 1] = '\0'; //dlatego chce go usunac. w jego miejsce wstawiam znak konca slowa
        }
        hasla_tab[i] = malloc(strlen(linia_tekstu)*sizeof(char)+1); //alokuje pamiec
        strcpy(hasla_tab[i],linia_tekstu); //przekopiowuje slowo do tablicy
        i++;
    }
    free(linia_tekstu); //zwalniam pamiec zmiennej pomocniczej
    fclose (plik_hasla); //zamykam plik
    plik_slownik = fopen(slownik,"r"); //analogicznie jak z haslami, tak samo wczytuje tablice slow slonika
    i=0;
    linia_tekstu = malloc(sizeof(char*));
    while(!feof(plik_slownik))
    {
        getline(&linia_tekstu, &we, plik_slownik);
        if(linia_tekstu[strlen(linia_tekstu)-1] == '\n') // 10 lub '\n'
        {
            linia_tekstu[strlen(linia_tekstu) - 1] = '\0';
        }
        slownik_tab[i] = malloc(strlen(linia_tekstu)*sizeof(char)+1);
        strcpy(slownik_tab[i],linia_tekstu);
        i++;
    }
    free(linia_tekstu); //zwalniam pamiec zmiennej pomocnicznej
    fclose (plik_slownik); //zamykam plik
    pthread_t watek_tab[NUMTHREADS]; //tablica watkow
    for(i=0;i<NUMTHREADS;i++)
    {
        //przypisanie wartosci argumentow ktore zostana przekazane watkowi
        watek_dane_tab[i].hasla = hasla_tab;
        watek_dane_tab[i].slownik = slownik_tab;
        watek_dane_tab[i].wielkosc_hasla = wielkosc_hasla;
        watek_dane_tab[i].wielkosc_slownik = wielkosc_slownik;
    }
    pthread_create(&watek_tab[0], NULL, Lamacz0, (void *)&watek_dane_tab[0]); //tworze watki
    pthread_create(&watek_tab[1], NULL, Lamacz1, (void *)&watek_dane_tab[1]);
    pthread_create(&watek_tab[2], NULL, Lamacz2, (void *)&watek_dane_tab[2]);
    pthread_create(&watek_tab[2], NULL, Lamacz3, (void *)&watek_dane_tab[3]);
    pthread_create(&watek_tab[2], NULL, Lamacz4, (void *)&watek_dane_tab[4]);
    pthread_create(&watek_tab[2], NULL, Lamacz5, (void *)&watek_dane_tab[5]);
    pthread_create(&watek_tab[2], NULL, Lamacz6, (void *)&watek_dane_tab[6]);
    pthread_create(&watek_tab[3], NULL, Konsument,(void *)&watek_dane_tab[7]);
    pthread_exit(NULL);
}

