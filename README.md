Lamacz-hasel-Password-cracker

Program łamiący hasła metodą słownikową/ Password cracker using dictionary attack

Zadanie należy skompilować: gcc main.c -pthread -lm -lssl -lcrypto

program włączyć poprzez: ./a.out hasla.txt slownik1.txt (najpierw podac plik z haslami, nastepnie slownik)

Program składa się z 8 watków - 7 łamaczy i 1 konsument

łamacze 0 1 2 to łamacze jednowyrazowi. Zmieniają wielkość liter, a nastepnie dodają liczby z przodu i z tylu

łamacz 3 to łamacz haseł, które składaja się tylko i wyłącznie z cyfr

łamacze 4 5 6 to łamacze dwuwyrazowi. Łączą wyrazy w nastepujacy sposób - słowo+znak+słowo. Podobnie jak łamacze 0 1 2 zmieniają wielkości liter.

Wątek konsumenta obsługuje sygnał SIGHUP. Wysłać go mozna na przykład w nastepujący sposób:

kill -s SIGHUP PID_PROCESU

Wtedy wyświetlają się statystyki.

Wątek konsumenta na bierząco wyświetla odnalezione hasła.

Więcej informacji w komentarzach w kodzie programu.
