Nicuta Loredana Ionela 325CD - Tema 2 PC

	Pentru a rezolva aceasta tema, am utilizat o noua structura ce contine toate campurile
specifice unui singur client, date preluate din fisierul dat ca parametru.

--server.c--

	In acest fisier, se proceseaza cererile primite de la clienti si se ofera informatiile
cerute.Serverul asculta atat pe canalul de stdin cat si pe canalul pe care primeste informatii de
la client. Se trateaza in ordine urmatoarele situatii:

1. login
	In cazul in care serverul primeste de la client comanda login, verifica initial daca 
exista vreun client logat deja. In acest caz, se anunta prin mesajul specifi(sesiune deschisa).
Se trateaza si cazurile in care nu exista numarul de card oferit ca parametru, respectiv are o 
dimensiune diferita de 6, respectiv in cazul in care pinul este gresit. 
	Daca cardul exista totusi, iar pinul este introdus gresit, atunci se va incrementa numarul
de incercari de logare. Atunci cand se ajunge la 3, cardul este considerat blocat si nicio alta 
operatiune nu mai poate fi efectuata.
	Daca cardul exista iar pinul este corect, atunci se marcheaza starea clientului respectiv 
fiind drept 1, iar numarul clientului va fi egala cu numarul socketului. Acest luru ajuta la 
identificarea datelor, respectiv la verificarea cazului in care mai exista sesiuni deschise de pe
acel cont.

2. logout
	In cazul in care serverul primeste mesaju de logout, verifica daca contul respectiv este
intr-o sesiune deschisa. Daca da, atunci va marca starea clientului egala cu 0. In cazul in care
nu este deschisa nicio sesiune, atunci afisam mesajul specific(clientul nu este autentificat).

3. listsold
	In cazul in care serverul primeste mesajul de listsold, prima data se cauta utilizatorul
ce doreste acest lucru, iar in cazul in care exista un client autentificat de pe acel socket atunci
se acceseaza campul sold din structura specifica.

4. transfer
	In cazul in care serverul primeste mesajul de transfer, primul lucru verifica daca exista 
numarul de card dorit, respectiv daca exista o sesiune dechisa pe sochetul respeciv. Daca atat contul
actual, cat si cel in care se doreste a se realiza transferul exista, atunci se verifica daca suma dorita
poate fi sau nu transferata.
	Daca transferul se realizeaza cu succes, atunci ambele conturi vor avea sumele de bani actualizate,
conform cerintelor.
	
5. quit 
	In cazul in care servrul primeste mesajul de quit, inchide legatura cu acel socket si il 
sterge din multimea de socketuri. 
	Daca este introdus mesajul de quit de la tastatura in cadrul serverului, atunci acesta anunta
toti clientii de inchiderea conexiunii iar mai apoi inchide serverul complet. Nicio alta conexiune
nu se mai poate realiza dupa apelarea acestei functii.


--client.c--

	In acest fisier, se introduc omenzile dorite si se asteapta raspuns de la server. Clientul
asculta atat canalul de stdin, unde primeste comenzile de la utilizator, cat si pe canalul de server,
unde primeste raspuns la cereri.

	Se verifica la citirea de la tastatura, daca se primeste mesajul "quit" atunci clientul
va trimite mesajul acesta serverului si va inchide conexiunea cu serverul.

	Daca se primeste de la server mesajul de quit, atunci se inchide conexiunea cu toti clientii,
fara a se realiza in continuare actiunile introduse de la tastatura.
	